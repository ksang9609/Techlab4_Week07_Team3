#pragma once

#include "LightComponent.h"

class FLineBatcher;

class USpotLightComponent : public ULightComponent
{
	DECLARE_CLASS(USpotLightComponent, ULightComponent)

	REFLECT_START(ClassName)
		PROPERTY(InnerConeAngle)
		PROPERTY(OuterConeAngle)
		PROPERTY(AttenuationRadius)
		PROPERTY(Falloff)
	REFLECT_END()

public:
	USpotLightComponent() = default;
	virtual ~USpotLightComponent() override = default;

	virtual void DrawDebug(FLineBatcher* LineBatcher) const override;

	float GetInnerConeAngle() const { return InnerConeAngle; }
	inline float GetInnerConeCosine() const { return std::cosf(FMath::DegreesToRadians(InnerConeAngle)); }
	void SetInnerConeAngle(float InAngle) { InnerConeAngle = InAngle; }

	float GetOuterConeAngle() const { return OuterConeAngle; }
	inline float GetOuterConeCosine() const { return std::cosf(FMath::DegreesToRadians(OuterConeAngle)); }
	void SetOuterConeAngle(float InAngle) { OuterConeAngle = InAngle; }

	float GetFalloff() const { return Falloff; }
	void SetFalloff(float InFalloff) { Falloff = InFalloff; }

	float GetAttenuationRadius() const { return AttenuationRadius; }
	void SetAttenuationRadius(float InRadius) { AttenuationRadius = InRadius; }

	inline float GetLightFalloffExponent() const { return Falloff; }

private:
	// 원뿔 중심축 기준 반각(도). 
	float InnerConeAngle = 15.0f;
	float OuterConeAngle = 30.0f;

	// 빛이 닿는 거리
	float AttenuationRadius = 5.0f;

	// Inner에서 Outer로 가며 밝기가 떨어지는 정도.
	float Falloff = 1.0f;
};
