#pragma once
#include "LightComponent.h"


class UDirectionalLightComponent : public ULightComponent
{
	DECLARE_CLASS(UDirectionalLightComponent, ULightComponent)
	REFLECT_START(UDirectionalLightComponent)
		PROPERTY(Intensity)
		PROPERTY(LightDirection)
		PROPERTY_TYPE(LightColor, Color)
	REFLECT_END()

public:
	UDirectionalLightComponent() = default;
	virtual ~UDirectionalLightComponent() override = default;

	virtual void DrawDebug(FLineBatcher* LineBatcher) const override;

	const FVector GetDirection() const { return LightDirection; }
	void SetDirection(FVector InDir) { LightDirection = InDir; }

	virtual void UpdateConstants(FLightConstants& Constants) override;

public:
	FVector LightDirection = FVector(0,0,0);
};

