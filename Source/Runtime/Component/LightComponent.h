#pragma once
#include "LightComponentBase.h"
#include "Render/LineBatcher.h"

class ULightComponent : public ULightComponentBase
{
	DECLARE_CLASS(ULightComponent, ULightComponentBase)
	REFLECT_START(ClassName)
		REFLECT_END()
public:
	ULightComponent() = default;
	virtual ~ULightComponent() override = default;

	virtual void DrawDebug(FLineBatcher* LineBatcher) const {};
private:
};