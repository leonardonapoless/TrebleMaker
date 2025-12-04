#pragma once
#include "EQRenderer.h"

#if JUCE_MAC

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

class MetalEQRenderer : public EQRenderer {
public:
    MetalEQRenderer();
    ~MetalEQRenderer() override;

    void initialize(void* nativeWindowHandle, int width, int height, double pixelScale) override;
    void resize(int width, int height, double pixelScale) override;
    void render() override;
    void shutdown() override;

    void updateParameters(const EQParameters& params) override;
    void updateFFTData(const std::vector<float>& fftData) override;

private:
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    id<MTLRenderPipelineState> pipelineState;
    CAMetalLayer* metalLayer = nullptr;
    
    static const int kMaxFramesInFlight = 3;
    dispatch_semaphore_t frameSemaphore;
    int currentBufferIndex = 0;
    
    id<MTLBuffer> paramBuffer[kMaxFramesInFlight];
    id<MTLBuffer> fftBuffer[kMaxFramesInFlight];

    void buildShaders();
    void createBuffers();
};

#endif
