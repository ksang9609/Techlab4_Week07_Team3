#pragma once

#include "LightComponent.h"

class UDirectionalLightComponent : public ULightComponent
{
    DECLARE_CLASS(UDirectionalLightComponent, ULightComponent)

    REFLECT_START(ClassName)
    REFLECT_END()

public:
    UDirectionalLightComponent() = default;
    virtual ~UDirectionalLightComponent() override = default;

    const char* GetIconPath() const override { return "Resources/Icons/S_LightDirectional.png"; }
};