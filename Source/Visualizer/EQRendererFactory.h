#pragma once
#include "EQRenderer.h"
class EQRendererFactory {
public:
    static std::unique_ptr<EQRenderer> createRenderer();
};
