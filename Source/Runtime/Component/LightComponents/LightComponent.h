#pragma once

#include "Component/SceneComponent.h"

class ULightComponent : public USceneComponent
{
	DECLARE_CLASS(ULightComponent, USceneComponent)
	REFLECT_START(ULightComponent)
		PROPERTY(Intensity)
		PROPERTY_TYPE(Color, Color)
		PROPERTY(bVisible)
		REFLECT_END()
public:
	ULightComponent();
	~ULightComponent() override;

	virtual void InitializeComponent();

	inline FVector GetLightColor() const { return Color; }
	inline float GetIntensity() const { return Intensity; }

private:
	float Intensity;
	FVector Color;
	bool bVisible;
};