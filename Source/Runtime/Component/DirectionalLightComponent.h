#pragma once
#include "LightComponent.h"

class FLineBatcher;

class UDirectionalLightComponent : public ULightComponent
{
	DECLARE_CLASS(UDirectionalLightComponent, ULightComponent)
	REFLECT_START(ClassName)
		REFLECT_END()
public:
	UDirectionalLightComponent() = default;
	virtual ~UDirectionalLightComponent() override = default;

	virtual void DrawDebug(FLineBatcher* LineBatcher) const override;

	FVector GetLightDirection() const {return GetWorldRotation().GetForward(); }
private:
};