#pragma once
#include "LightActor.h"
#include "Component/PointLightComponent.h"

class APointLightActor : public ALightActor
{
	DECLARE_CLASS(APointLightActor, ALightActor)
	REFLECT_START(ClassName)
		REFLECT_END()
public:
	APointLightActor();
	virtual ~APointLightActor() override = default;
	UPointLightComponent* GetPointLightComponent() const { return PointLightComponent; }

private:
	UPointLightComponent* PointLightComponent = nullptr;
};