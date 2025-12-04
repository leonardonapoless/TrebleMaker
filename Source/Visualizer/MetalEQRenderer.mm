#include "MetalEQRenderer.h"
#import <AppKit/AppKit.h>

#if JUCE_MAC

// embedded msl for simplicity
const char* mslSource = R"(
#include <metal_stdlib>

using namespace metal;

struct VertexOut {
    float4 position [[position]];
    float2 uv;
};

struct EQParams {
    float frequency;
    float gain;
    float q;
    float active; 
};

vertex VertexOut vertexMain(uint vertexID [[vertex_id]]) {
    float4 positions[4] = {
        float4(-1.0, -1.0, 0.0, 1.0),
        float4( 1.0, -1.0, 0.0, 1.0),
        float4(-1.0,  1.0, 0.0, 1.0),
        float4( 1.0,  1.0, 0.0, 1.0)
    };
    
    VertexOut out;
    out.position = positions[vertexID];
    out.uv = positions[vertexID].xy * 0.5 + 0.5;
    return out;
}

float getHighShelfMagnitude(float freqNorm, float cutoffNorm, float gainDb, float q) {
    // 20hz-20khz log mapping
    float f = 20.0 * pow(1000.0, freqNorm);
    float cutoff = 20.0 * pow(1000.0, cutoffNorm);
    
    // sigmoid approx for viz
    float slope = (f - cutoff) / (f + cutoff);
    float mag = gainDb * 0.5 * (1.0 + slope); 
    
    return 0.5 + (mag / 48.0);
}

fragment float4 fragmentMain(VertexOut in [[stage_in]],
                             constant EQParams& params [[buffer(0)]]) {
    
    float2 uv = in.uv;
    float3 color = float3(0.1, 0.1, 0.12);
    
    // grid lines
    float grid = step(0.98, fract(uv.x * 10.0)) * 0.1;
    color += grid;
    
    if (params.active > 0.5) {
        float curveY = getHighShelfMagnitude(uv.x, params.frequency, params.gain, params.q);
        
        float dist = abs(uv.y - curveY);
        float thickness = 0.005;
        float alpha = 1.0 - smoothstep(thickness - 0.002, thickness + 0.002, dist);
        
        color = mix(color, float3(1.0, 0.2, 0.2), alpha);
    }
    
    return float4(color, 1.0);
}
)";

MetalEQRenderer::MetalEQRenderer() {
    device = MTLCreateSystemDefaultDevice();
    commandQueue = [device newCommandQueue];
    frameSemaphore = dispatch_semaphore_create(kMaxFramesInFlight);
    buildShaders();
    createBuffers();
}

MetalEQRenderer::~MetalEQRenderer() {
}

void MetalEQRenderer::initialize(void* nativeWindowHandle, int width, int height, double pixelScale) {
    NSView* view = (__bridge NSView*)nativeWindowHandle;
    
    metalLayer = [CAMetalLayer layer];
    metalLayer.device = device;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalLayer.framebufferOnly = YES;
    metalLayer.frame = view.bounds;
    metalLayer.contentsScale = pixelScale;
    
    [view setWantsLayer:YES];
    [view setLayer:metalLayer];
}

void MetalEQRenderer::resize(int width, int height, double pixelScale) {
    if (metalLayer) {
        metalLayer.drawableSize = CGSizeMake(width * pixelScale, height * pixelScale);
    }
}

void MetalEQRenderer::buildShaders() {
    NSError* error = nil;
    NSString* source = [NSString stringWithUTF8String:mslSource];
    id<MTLLibrary> library = [device newLibraryWithSource:source options:nil error:&error];
    
    if (!library) {
        NSLog(@"Failed to compile shaders: %@", error);
        return;
    }
    
    id<MTLFunction> vertexFunc = [library newFunctionWithName:@"vertexMain"];
    id<MTLFunction> fragmentFunc = [library newFunctionWithName:@"fragmentMain"];
    
    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.vertexFunction = vertexFunc;
    pipelineDesc.fragmentFunction = fragmentFunc;
    pipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    if (!pipelineState) {
        NSLog(@"Failed to create pipeline state: %@", error);
    }
}

void MetalEQRenderer::createBuffers() {
    for (int i = 0; i < kMaxFramesInFlight; i++) {
        paramBuffer[i] = [device newBufferWithLength:sizeof(EQParameters) options:MTLResourceStorageModeShared];
    }
}

void MetalEQRenderer::updateParameters(const EQParameters& params) {
    // TODO: ring buffer
    memcpy(paramBuffer[currentBufferIndex].contents, &params, sizeof(EQParameters));
}

void MetalEQRenderer::updateFFTData(const std::vector<float>& fftData) {
}

void MetalEQRenderer::render() {
    if (!metalLayer || !pipelineState) return;
    
    dispatch_semaphore_wait(frameSemaphore, DISPATCH_TIME_FOREVER);
    
    id<CAMetalDrawable> drawable = [metalLayer nextDrawable];
    if (!drawable) {
        dispatch_semaphore_signal(frameSemaphore);
        return;
    }
    
    currentBufferIndex = (currentBufferIndex + 1) % kMaxFramesInFlight;
    
    id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
    
    MTLRenderPassDescriptor* passDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    passDesc.colorAttachments[0].texture = drawable.texture;
    passDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
    passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
    
    id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDesc];
    [encoder setRenderPipelineState:pipelineState];
    [encoder setVertexBuffer:paramBuffer[currentBufferIndex] offset:0 atIndex:0]; 
    [encoder setFragmentBuffer:paramBuffer[currentBufferIndex] offset:0 atIndex:0];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [encoder endEncoding];
    
    [commandBuffer presentDrawable:drawable];
    
    __block dispatch_semaphore_t blockSemaphore = frameSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(blockSemaphore);
    }];
    
    [commandBuffer commit];
}

void MetalEQRenderer::shutdown() {
}

#endif
