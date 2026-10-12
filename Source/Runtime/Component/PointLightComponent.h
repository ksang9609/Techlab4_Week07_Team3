#pragma once
#include "LightComponent.h"

class UPointLightComponent : public ULightComponent
{
	DECLARE_CLASS(UPointLightComponent, ULightComponent)
	REFLECT_START(ClassName)
		PROPERTY(AttenuationRadius)
		PROPERTY(bUseInverseSquaredFalloff)
		PROPERTY(LightFalloffExponent)
		REFLECT_END()
public:
	UPointLightComponent() = default;
	virtual ~UPointLightComponent() override = default;

	virtual void DrawDebug(FLineBatcher* LineBatcher) const override;

	float GetAttenuationRadius() const { return AttenuationRadius; }
	void SetAttenuationRadius(float InRadius) { AttenuationRadius = InRadius; }

	bool IsUseInverseSquaredFalloff() const { return bUseInverseSquaredFalloff; }
	void SetUseInverseSquaredFalloff(bool bInUse) { bUseInverseSquaredFalloff = bInUse; }

	float GetLightFalloffExponent() const { return LightFalloffExponent; }
	void SetLightFalloffExponent(float InExponent) { LightFalloffExponent = InExponent; }

protected:
	float AttenuationRadius = 5.0f;
	bool bUseInverseSquaredFalloff = true;
	float LightFalloffExponent = 2.0f;
};