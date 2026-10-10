#include "EnginePCH.h"
#include "PointLightComponent.h"

UPointLightComponent::UPointLightComponent()
	: Super()
	, AttenuationRadius(10.0f)
	, LightFalloffExponent(2.0f)
{
}