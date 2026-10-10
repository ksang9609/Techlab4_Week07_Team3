#pragma once
#include "SceneComponent.h"

class ULightComponentBase : public USceneComponent
{
	DECLARE_CLASS(ULightComponentBase, USceneComponent)
	REFLECT_START(ClassName)
		PROPERTY(Intensity)
		PROPERTY_TYPE(LightColor, Color)
		PROPERTY(bVisible)
		REFLECT_END()
public:
	ULightComponentBase() = default;
	virtual ~ULightComponentBase() override = default;
	void SetIntensity(float InIntensity) { Intensity = InIntensity; }
	float GetIntensity() const { return Intensity; }
	void SetLightColor(const FVector4& InColor) { LightColor = InColor; }
	const FVector4& GetLightColor() const { return LightColor; }
	void SetVisible(bool bInVisible) { bVisible = bInVisible; }
	bool IsVisible() const { return bVisible; }

protected:
	float Intensity = 1.0f;
	FVector4 LightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool bVisible = true;
};
