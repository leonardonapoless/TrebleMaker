#pragma once
#include "EQRenderer.h"

#if JUCE_WINDOWS

#include <juce_opengl/juce_opengl.h>

class OpenGLEQRenderer : public EQRenderer, public juce::OpenGLRenderer {
public:
    OpenGLEQRenderer();
    ~OpenGLEQRenderer() override;

    void initialize(void* nativeWindowHandle, int width, int height, double pixelScale) override;
    void resize(int width, int height, double pixelScale) override;
    void render() override;
    void shutdown() override;

    void updateParameters(const EQParameters& params) override;
    void updateFFTData(const std::vector<float>& fftData) override;

    // OpenGLRenderer overrides
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

private:
    juce::OpenGLContext openGLContext;
    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgram;
    
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uResolution;
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> uParams; 

    EQParameters currentParams;
    
    void buildShaders();
};

#endif
