#pragma once
#include "LightActor.h"
#include "Component/AmbientLightComponent.h"

class AAmbientLightActor : public ALightActor
{
	DECLARE_CLASS(AAmbientLightActor, ALightActor)
	REFLECT_START(ClassName)
		REFLECT_END()
public:
	AAmbientLightActor();
	virtual ~AAmbientLightActor() override = default;
	UAmbientLightComponent* GetAmbientLightComponent() const { return AmbientLightComponent; }

private:
	UAmbientLightComponent* AmbientLightComponent = nullptr;
};