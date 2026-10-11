#include "EnginePCH.h"
#include "DirectionalLightComponent.h"
#include "Render/Renderer.h"

void UDirectionalLightComponent::UpdateConstants(FLightConstants& Constants)
{
	Constants.Directional.ColorIntensity = FVector4(LightColor, Intensity);
	Constants.Directional.PositionRadius = FVector4(GetWorldLocation(), 0.0f);
	Constants.Directional.LightDirection = FVector4(LightDirection, 0.0f);
}