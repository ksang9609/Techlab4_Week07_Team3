#pragma once
#include "LightComponent.h"

struct FLightConstants;

class UPointLightComponent : ULightComponent
{
public:
	const float GetRadius() const { return Radius; }
	void SetRadius(float InRadius) { Radius = InRadius; }

	const float GetRadiusFalloff() const { return RadiusFalloff; }
	void SetRadiusFalloff(float Falloff) { RadiusFalloff = Falloff; }

	virtual void UpdateConstants(FLightConstants& Constants) override;

public:
	float Radius = 10.0f; 
	float RadiusFalloff = 1.0f;
};


