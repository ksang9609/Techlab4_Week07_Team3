#pragma once

#include "LightComponent.h"

class FLineBatcher;

class USpotLightComponent : public ULightComponent
{
	DECLARE_CLASS(USpotLightComponent, ULightComponent)

	REFLECT_START(ClassName)
		PROPERTY(Intensity)
		PROPERTY(Radius)
		PROPERTY(RadiusFalloff)
		PROPERTY(LightDirection)
		PROPERTY_TYPE(LightColor, Color)
		PROPERTY(InnerConeAngle)
		PROPERTY(OuterConeAngle)
	REFLECT_END()

public:
	USpotLightComponent() = default;
	virtual ~USpotLightComponent() override = default;

	virtual void DrawDebug(FLineBatcher* LineBatcher) const override;

	float GetInnerConeAngle() const { return InnerConeAngle; }
	void SetInnerConeAngle(float InAngle) { InnerConeAngle = InAngle; }

	float GetOuterConeAngle() const { return OuterConeAngle; }
	void SetOuterConeAngle(float InAngle) { OuterConeAngle = InAngle; }

	const float GetRadius() const { return Radius; }
	void SetRadius(float InRadius) { Radius = InRadius; }

	const float GetRadiusFalloff() const { return RadiusFalloff; }
	void SetRadiusFalloff(float Falloff) { RadiusFalloff = Falloff; }

	const FVector GetDirection() const { return LightDirection; }
	void SetDirection(FVector InDir) { LightDirection = InDir; }

	virtual void UpdateConstants(FLightConstants& Constants) override;

private:
	// 원뿔 중심축 기준 반각(도). 
	float InnerConeAngle = 15.0f;
	float OuterConeAngle = 30.0f;

	float Radius = 10.0f;
	float RadiusFalloff = 1.0f;

	FVector LightDirection = FVector(0, 0, 0);
};
