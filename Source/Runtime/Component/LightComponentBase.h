#pragma once

#include "SceneComponent.h"

class ULightComponentBase : public USceneComponent
{
	DECLARE_CLASS(ULightComponentBase, USceneComponent)
	REFLECT_START(ULightComponentBase)
		PROPERTY_TYPE(LightColor, Color)
		PROPERTY(Intensity)
		PROPERTY(bVisible)
	REFLECT_END()

public:
	FVector4 GetLightColor() const { return LightColor; }
	void SetLightColor(FVector4 InColor) { LightColor = InColor; }

	float GetLightIntensity() const { return Intensity; }
	void SetLightIntensity(float InIntensity) { Intensity = InIntensity; }

	bool IsLightVisible() const { return bVisible; }


	virtual const char* GetIconPath() const { return nullptr; }

private:
	float Intensity = 1.0f;
	FVector4 LightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool bVisible = true;
};