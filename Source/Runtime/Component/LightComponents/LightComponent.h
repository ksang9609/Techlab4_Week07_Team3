#pragma once

#include "Component/SceneComponent.h"

class ULightComponent : public USceneComponent
{
	DECLARE_CLASS(ULightComponent, USceneComponent)
	REFLECT_START(ULightComponent)
	REFLECT_END()
public:
	ULightComponent() = default;

	virtual void InitializeComponent();

private:
	float Intensity;
	FVector Color;
	bool bVisible;
};