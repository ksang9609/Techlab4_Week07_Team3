#pragma once
#include "LightComponent.h"

class UPointLightComponent : public ULightComponent
{
	DECLARE_CLASS(UPointLightComponent, ULightComponent)
	REFLECT_START(ClassName)
		PROPERTY(AttenuationRadius)
		PROPERTY(LightFalloffExponent)
		REFLECT_END()
public:
	UPointLightComponent() = default;
	virtual ~UPointLightComponent() override = default;

	virtual void DrawDebug(FLineBatcher* LineBatcher) const override;

	float GetAttenuationRadius() const { return AttenuationRadius; }
	void SetAttenuationRadius(float InRadius) { AttenuationRadius = InRadius; }

	float GetLightFalloffExponent() const { return LightFalloffExponent; }
	void SetLightFalloffExponent(float InExponent) { LightFalloffExponent = InExponent; }

private:
	float AttenuationRadius = 5.0f;
	float LightFalloffExponent = 2.0f;
};