#pragma once

#include "LightComponent.h"

class UDirectionalLightComponent : public ULightComponent
{
	DECLARE_CLASS(UDirectionalLightComponent, ULightComponent)
	REFLECT_START(ClassName)
		REFLECT_END()

public:
	virtual void DrawDebug(FLineBatcher* LineBatcher) const override;
private:
};