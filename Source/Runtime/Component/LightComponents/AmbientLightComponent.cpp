#include "EnginePCH.h"

#include "AmbientLightComponent.h"

UAmbientLightComponent::UAmbientLightComponent()
	: Super()
{
	SetIntensity(0.2f);
}