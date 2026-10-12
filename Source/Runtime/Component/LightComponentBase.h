#pragma once
#include "SceneComponent.h"

class ULightComponentBase : public USceneComponent
{
	DECLARE_CLASS(ULightComponentBase, USceneComponent)
	REFLECT_START(ClassName)
		PROPERTY(Intensity)
		PROPERTY(LightColor)
		PROPERTY(bVisible)
		REFLECT_END()
public:
	ULightComponentBase() = default;
	virtual ~ULightComponentBase() override = default;
	void SetIntensity(float InIntensity) { Intensity = InIntensity; }
	float GetIntensity() const { return Intensity; }
	void SetLightColor(const FColor& InColor) { LightColor = InColor; }
	const FColor& GetLightColor() const { return LightColor; }
	void SetVisible(bool bInVisible) { bVisible = bInVisible; }
	bool IsVisible() const { return bVisible; }

protected:
	float Intensity = 1.0f;
	FColor LightColor = FColor::White;
	bool bVisible = true;
};
