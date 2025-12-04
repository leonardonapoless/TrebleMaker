#pragma once
#include <JuceHeader.h>

struct EQParameters {
    float frequency;
    float gain;
    float q;
    bool active;
};

class EQRenderer {
public:
    virtual ~EQRenderer() = default;

    // lifecycle
    virtual void initialize(void* nativeWindowHandle, int width, int height, double pixelScale) = 0;
    virtual void resize(int width, int height, double pixelScale) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;

    // data update
    virtual void updateParameters(const EQParameters& params) = 0;
    virtual void updateFFTData(const std::vector<float>& fftData) = 0;
};
