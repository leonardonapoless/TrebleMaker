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
    float f = 20.0 * pow(1000.0, freqNorm);
    float cutoff = 20.0 * pow(1000.0, cutoffNorm);
    float slope = (f - cutoff) / (f + cutoff);
    float mag = gainDb * 0.5 * (1.0 + slope);
    return 0.5 + (mag / 48.0);
}

fragment float4 fragmentMain(VertexOut in [[stage_in]],
                             constant EQParams& params [[buffer(0)]]) {
    float2 uv = in.uv;
    float3 color = float3(0.1, 0.1, 0.12);
    float grid = step(0.98, fract(uv.x * 10.0)) * 0.1;
    color += grid;
    
    if (params.active > 0.5) {
        float curveY = getHighShelfMagnitude(uv.x, params.frequency, params.gain, params.q);
        float dist = abs(uv.y - curveY);
        float alpha = 1.0 - smoothstep(0.003, 0.007, dist);
        color = mix(color, float3(1.0, 0.2, 0.2), alpha);
    }
    
    return float4(color, 1.0);
}
