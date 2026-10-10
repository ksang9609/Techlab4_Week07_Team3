#pragma once

#include "LightComponent.h"

class UAmbientLightComponent : public ULightComponent
{
	DECLARE_CLASS(UAmbientLightComponent, ULightComponent)
	REFLECT_START(UAmbientLightComponent)
		REFLECT_END()

public:
	UAmbientLightComponent();
	~UAmbientLightComponent() override = default;

	// UActorComponent interface
	//virtual void InitializeComponent() override;
	// End of UActorComponent interface
private:
};