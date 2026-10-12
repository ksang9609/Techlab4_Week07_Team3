#pragma once

#include "LightComponent.h"

class FLineBatcher;

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

	void DrawDebug(FLineBatcher* LineBatcher) const;

	float GetFalloff() const { return LightFalloffExponent; }
	void SetFalloff(float InFalloff) { LightFalloffExponent = InFalloff; }

	float GetAttenuationRadius() const { return AttenuationRadius; }
	void SetAttenuationRadius(float InRadius) { AttenuationRadius = InRadius; }

	const char* GetIconPath() const override { return "Resources/Icons/S_LightPoint.png"; }

private: 

	// 빛이 닿는 거리
	float AttenuationRadius = 5.0f;

	// 광원과의 거리에 따른 빛의 감쇠 지수
	float LightFalloffExponent = 1.0f;
};