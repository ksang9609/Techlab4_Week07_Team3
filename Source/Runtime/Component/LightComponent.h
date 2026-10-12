#pragma once

#include "LightComponentBase.h"

struct FLightConstants;
class FLineBatcher;

class ULightComponent : public ULightComponentBase
{
	DECLARE_CLASS(ULightComponent, ULightComponentBase)
	REFLECT_START(ULightComponent)
	REFLECT_END()
public:
	ULightComponent() = default;
	virtual ~ULightComponent() override;
	virtual void InitializeComponent() override;
	virtual void UpdateConstants(FLightConstants& Constants) {};
	virtual void DrawDebug(FLineBatcher* LineBatcher) const {};

protected:
	void SetIntensity(float NewIntensity) { Intensity = NewIntensity; }

private:



};