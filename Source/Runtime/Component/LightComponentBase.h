#pragma once
#include "Component/SceneComponent.h"
#include "Math/MathUtility.h"

class ULightComponentBase : public USceneComponent
{
	DECLARE_CLASS(ULightComponentBase, USceneComponent)
	REFLECT_START(ULightComponentBase)
	REFLECT_END()

public:

	FVector GetLightColor() { return LightColor; }

	float Intensity = 1.0f;
	FVector LightColor = FVector(1.0f, 1.0f, 1.0f);
	bool bVisible = true;

	// Guid


};