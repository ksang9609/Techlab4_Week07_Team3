#pragma once

#include "LightComponent.h"

class UPointLightComponent final : public ULightComponent
{
	DECLARE_CLASS(UPointLightComponent, ULightComponent)
	REFLECT_START(ClassName)
		PROPERTY(AttenuationRadius)
		PROPERTY(LightFalloffExponent)
		REFLECT_END()
public:
	UPointLightComponent();

	virtual void DrawDebug(FLineBatcher* LineBatcher) const override;

	inline float GetAttenuationRadius() const { return AttenuationRadius; }
	inline float GetLightFalloffExponent() const { return LightFalloffExponent; }

private:
	float AttenuationRadius;
	float LightFalloffExponent;
};