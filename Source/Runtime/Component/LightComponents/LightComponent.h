#pragma once

#include "Component/SceneComponent.h"

class ULightComponent : public USceneComponent
{
	DECLARE_CLASS(ULightComponent, USceneComponent)
	REFLECT_START(ULightComponent)
		PROPERTY(Intensity)
		PROPERTY_TYPE(LightColor, Color)
		PROPERTY(bVisible)
		REFLECT_END()
public:
	ULightComponent();
	~ULightComponent() override;

	virtual void InitializeComponent();

	inline void SetIntensity(float InIntensity) { Intensity = InIntensity; }
	inline FVector GetLightColor() const { return LightColor; }
	inline void SetLightColor(const FVector& InColor) { LightColor = InColor; }
	inline float GetIntensity() const { return Intensity; }

private:
	float Intensity;
	FVector LightColor;
	bool bVisible;
};