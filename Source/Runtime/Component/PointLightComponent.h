#pragma once
#include "LightComponent.h"

struct FLightConstants;

class UPointLightComponent : public ULightComponent
{
	DECLARE_CLASS(UPointLightComponent, ULightComponent)
	REFLECT_START(UPointLightComponent)
		PROPERTY(Intensity)
		PROPERTY(Radius)
		PROPERTY(RadiusFalloff)
		PROPERTY_TYPE(LightColor, Color)
	REFLECT_END()
public:
	UPointLightComponent() = default;
	virtual ~UPointLightComponent() override = default;

	virtual void DrawDebug(FLineBatcher* LineBatcher) const override;

	const float GetRadius() const { return Radius; }
	void SetRadius(float InRadius) { Radius = InRadius; }

	const float GetRadiusFalloff() const { return RadiusFalloff; }
	void SetRadiusFalloff(float Falloff) { RadiusFalloff = Falloff; }

	virtual void UpdateConstants(FLightConstants& Constants) override;

public:
	float Radius = 10.0f; 
	float RadiusFalloff = 1.0f;
};


