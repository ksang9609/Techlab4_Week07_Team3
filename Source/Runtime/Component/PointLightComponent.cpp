#include "EnginePCH.h"
#include "PointLightComponent.h"
#include "Render/Renderer.h"

void UPointLightComponent::UpdateConstants(FLightConstants& Constants)
{
	int32 Num = Constants.NumPointLights;
	Constants.PointLights[Num].ColorIntensity = FVector4(LightColor, Intensity);
	Constants.PointLights[Num].FalloffEnabled = FVector4(RadiusFalloff, 1.0f, 0.0f, 0.0f);
	Constants.PointLights[Num].PositionRadius = FVector4(GetWorldLocation(), Radius);
	++Constants.NumPointLights;
}