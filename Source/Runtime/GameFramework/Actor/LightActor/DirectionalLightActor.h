#pragma once
#include "LightActor.h"
#include "Component/DirectionalLightComponent.h"

class ADirectionalLightActor : public ALightActor
{
	DECLARE_CLASS(ADirectionalLightActor, ALightActor)
	REFLECT_START(ClassName)
		REFLECT_END()
public:
	ADirectionalLightActor();
	virtual ~ADirectionalLightActor() override = default;
	UDirectionalLightComponent* GetDirectionalLightComponent() const { return DirectionalLightComponent; }

private:
	UDirectionalLightComponent* DirectionalLightComponent = nullptr;
};