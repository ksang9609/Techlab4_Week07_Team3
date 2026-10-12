#pragma once

#include "LightComponent.h"

class UAmbientLightComponent : public ULightComponent
{
	DECLARE_CLASS(UAmbientLightComponent, ULightComponent)

	REFLECT_START(ClassName)
	REFLECT_END()

public:
	UAmbientLightComponent() = default;
	virtual ~UAmbientLightComponent() override = default;

	const char* GetIconPath() const override { return "Resources/Icons/S_LightSky.png"; }
};
