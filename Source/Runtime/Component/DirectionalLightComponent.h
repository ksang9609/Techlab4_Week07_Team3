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

	void DrawDebug(FLineBatcher* LineBatcher) const;

	FVector GetLightDirection() const {return GetWorldRotation().GetForward(); }
private:
};