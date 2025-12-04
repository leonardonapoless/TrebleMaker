#include "OpenGLEQRenderer.h"

#if JUCE_WINDOWS

OpenGLEQRenderer::OpenGLEQRenderer() {
    openGLContext.setRenderer(this);
    openGLContext.setContinuousRepainting(false); 
}

OpenGLEQRenderer::~OpenGLEQRenderer() {
    openGLContext.detach();
}

void OpenGLEQRenderer::initialize(void* nativeWindowHandle, int width, int height, double pixelScale) {
    // assuming handle is component ptr for juce context
    openGLContext.attachTo(*static_cast<juce::Component*>(nativeWindowHandle)); 
}

void OpenGLEQRenderer::resize(int width, int height, double pixelScale) {
}

void OpenGLEQRenderer::render() {
    openGLContext.triggerRepaint();
}

void OpenGLEQRenderer::shutdown() {
    openGLContext.detach();
}

void OpenGLEQRenderer::updateParameters(const EQParameters& params) {
    currentParams = params;
}

void OpenGLEQRenderer::updateFFTData(const std::vector<float>& fftData) {
}

void OpenGLEQRenderer::newOpenGLContextCreated() {
    buildShaders();
}

void OpenGLEQRenderer::renderOpenGL() {
    juce::OpenGLHelpers::clear(juce::Colour(0xff1a1a1f));

    if (shaderProgram == nullptr) return;

    shaderProgram->use();

    if (uResolution != nullptr)
        uResolution->set((GLfloat)openGLContext.getRenderingScale(), (GLfloat)openGLContext.getRenderingScale()); 
        
    if (uParams != nullptr)
        uParams->set(currentParams.frequency, currentParams.gain, currentParams.q, currentParams.active ? 1.0f : 0.0f);

    // gl4.6 vertex shader handles coords via gl_vertexid
}

void OpenGLEQRenderer::openGLContextClosing() {
    shaderProgram = nullptr;
}

void OpenGLEQRenderer::buildShaders() {
    const char* vertexSource = R"(
        #version 460 core
        out vec2 uv;
        void main() {
            vec2 vertices[4] = vec2[](
                vec2(-1.0, -1.0), vec2(1.0, -1.0),
                vec2(-1.0,  1.0), vec2(1.0,  1.0)
            );
            gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
            uv = vertices[gl_VertexID] * 0.5 + 0.5;
        }
    )";

    const char* fragmentSource = R"(
        #version 460 core
        in vec2 uv;
        out vec4 fragColor;
        
        uniform vec4 uParams; // freq gain q active

        float getHighShelfMagnitude(float freqNorm, float cutoffNorm, float gainDb, float q) {
            float f = 20.0 * pow(1000.0, freqNorm);
            float cutoff = 20.0 * pow(1000.0, cutoffNorm);
            float slope = (f - cutoff) / (f + cutoff);
            float mag = gainDb * 0.5 * (1.0 + slope);
            return 0.5 + (mag / 48.0);
        }

        void main() {
            vec3 color = vec3(0.1, 0.1, 0.12);
            float grid = step(0.98, fract(uv.x * 10.0)) * 0.1;
            color += grid;
            
            if (uParams.w > 0.5) {
                float curveY = getHighShelfMagnitude(uv.x, uParams.x, uParams.y, uParams.z);
                float dist = abs(uv.y - curveY);
                float alpha = 1.0 - smoothstep(0.003, 0.007, dist);
                color = mix(color, vec3(1.0, 0.2, 0.2), alpha);
            }
            
            fragColor = vec4(color, 1.0);
        }
    )";

    std::unique_ptr<juce::OpenGLShaderProgram> newShader (new juce::OpenGLShaderProgram (openGLContext));
    if (newShader->addVertexShader (vertexSource)
        && newShader->addFragmentShader (fragmentSource)
        && newShader->link())
    {
        shaderProgram = std::move (newShader);
        shaderProgram->use();
        
        uResolution.reset(new juce::OpenGLShaderProgram::Uniform(*shaderProgram, "uResolution"));
        uParams.reset(new juce::OpenGLShaderProgram::Uniform(*shaderProgram, "uParams"));
    }
}

#endif
