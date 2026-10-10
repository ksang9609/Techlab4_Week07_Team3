#pragma once

#include "Component/SceneComponent.h"
#include "Component/BillboardComponent.h"
#include "Render/LineBatcher.h"

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

	virtual void DrawDebug(FLineBatcher* LineBatcher) const {}

	virtual void PostEditChangeProperty(const FProperty& Property) override;

	/* Getter Setter */
	inline void SetIntensity(float InIntensity) { Intensity = InIntensity; }
	inline FVector GetLightColor() const { return LightColor; }
	inline void SetColor(const FVector& InColor) { LightColor = InColor; UpdateEditorIconColor(); }
	inline float GetIntensity() const { return Intensity; }

	inline void SetSpriteComponent(UBillboardComponent* InSprite) { SpriteComponent = InSprite; UpdateEditorIconColor(); }

private:
	float Intensity;
	FVector LightColor;
	bool bVisible;

	// Sprite icon for editor visualization
	UBillboardComponent* SpriteComponent = nullptr;

	void UpdateEditorIconColor();
};