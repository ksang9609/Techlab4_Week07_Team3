#pragma once
#include "LightComponent.h"


class UDirectionalLightComponent : public ULightComponent
{
public:
	const FVector GetDirection() const { return LightDirection; }
	void SetDirection(FVector InDir) { LightDirection = InDir; }

	virtual void UpdateConstants(FLightConstants& Constants) override;

public:
	FVector LightDirection = FVector(0,0,0);
};

