#include "EQRendererFactory.h"

#if JUCE_MAC
    #include "MetalEQRenderer.h"
#elif JUCE_WINDOWS
    #include "OpenGLEQRenderer.h"
#endif

std::unique_ptr<EQRenderer> EQRendererFactory::createRenderer() {
    #if JUCE_MAC
        return std::make_unique<MetalEQRenderer>();
    #elif JUCE_WINDOWS
        return std::make_unique<OpenGLEQRenderer>();
    #else
        return nullptr;
    #endif
}
