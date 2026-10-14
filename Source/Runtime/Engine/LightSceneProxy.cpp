#include "EnginePCH.h"
#include "LightSceneProxy.h"
#include "Component/AmbientLightComponent.h"
#include "Component/DirectionalLightComponent.h"
#include "Component/PointLightComponent.h"
#include "Component/SpotLightComponent.h"

FLightSceneProxy::FLightSceneProxy(ULightComponent* InComponent)
	: Component(InComponent)
{
	if (InComponent->IsA<UDirectionalLightComponent>())
		LightType = ELightType::Directional;
	else if (InComponent->IsA<USpotLightComponent>()) // Spot이 Point를 상속하므로 먼저 검사
		LightType = ELightType::Spot;
	else if (InComponent->IsA<UPointLightComponent>())
		LightType = ELightType::Point;
	else if (InComponent->IsA<UAmbientLightComponent>())
		LightType = ELightType::Ambient;
}

void FLightSceneProxy::GatherLightConstants(FForwardLightConstants& InOutConstants) const
{
	if(!Component || !Component->IsVisible())
	{
		return;
	}

	const FLinearColor Color = FLinearColor(Component->GetLightColor());
	const float Intensity = Component->GetIntensity();

	switch(LightType)
	{
	case ELightType::Ambient:
	{
		InOutConstants.AmbientColor = FVector4(Color.R, Color.G, Color.B, Intensity);
		break;
	}
	case ELightType::Directional:
	{
		if (InOutConstants.DirectionalLightCount >= 1) break;
		const UDirectionalLightComponent* DirectionalLight = static_cast<UDirectionalLightComponent*>(Component);
		InOutConstants.DirectionalLight.Direction = FVector4(DirectionalLight->GetLightDirection(), 0.0f);
		InOutConstants.DirectionalLight.ColorIntensity = FVector4(Color.R, Color.G, Color.B, Intensity);
		InOutConstants.DirectionalLightCount = 1;
		break;
	}
	case ELightType::Point:
	{
		if (InOutConstants.PointLightCount >= MaxPointLights) break;
		const UPointLightComponent* PointLight = static_cast<UPointLightComponent*>(Component);
		const FVector Position = PointLight->GetWorldLocation();
		const float Radius = std::max(PointLight->GetAttenuationRadius(), 0.001f);
		const float FalloffExponent = std::max(PointLight->GetLightFalloffExponent(), 0.001f);
		const float UseInverseSquared = PointLight->IsUseInverseSquaredFalloff() ? 1.0f : 0.0f;

		const uint32 PointIndex = InOutConstants.PointLightCount++;
		InOutConstants.PointLights[PointIndex].PositionRadius = FVector4(Position, Radius);
		InOutConstants.PointLights[PointIndex].ColorIntensity = FVector4(Color.R, Color.G, Color.B, Intensity);
		InOutConstants.PointLights[PointIndex].FalloffParams = FVector4(FalloffExponent, UseInverseSquared, 0.0f, 0.0f);
		break;
	}
	case ELightType::Spot:
	{
		if (InOutConstants.SpotLightCount >= MaxSpotLights) break;
		const USpotLightComponent* SpotLight = static_cast<USpotLightComponent*>(Component);
		const FVector SpotPosition = SpotLight->GetWorldLocation();
		const FVector SpotDirection = SpotLight->GetLightDirection();
		const float SpotRadius = std::max(SpotLight->GetAttenuationRadius(), 0.001f);
		const float FalloffExponent = std::max(SpotLight->GetLightFalloffExponent(), 0.001f);
		const float UseInverseSquared = SpotLight->IsUseInverseSquaredFalloff() ? 1.0f : 0.0f;

		const float InnerRadian = FMath::DegreesToRadians(SpotLight->GetInnerConeAngle());
		const float OuterRadian = FMath::DegreesToRadians(SpotLight->GetOuterConeAngle());
		const float cosinner = std::cos(InnerRadian);
		const float cosouter = std::cos(OuterRadian);
		const float Scale = 1.0f / std::max(cosinner - cosouter, 0.001f);
		const float Offset = -cosouter * Scale;

		const uint32 SpotIndex = InOutConstants.SpotLightCount++;
		InOutConstants.SpotLights[SpotIndex].PositionRadius = FVector4(SpotPosition, SpotRadius);
		InOutConstants.SpotLights[SpotIndex].DirectionAngle = FVector4(SpotDirection, 0.0f);
		InOutConstants.SpotLights[SpotIndex].ConeScaleOffset = FVector4(Scale, Offset, FalloffExponent, UseInverseSquared);
		InOutConstants.SpotLights[SpotIndex].ColorIntensity = FVector4(Color.R, Color.G, Color.B, Intensity);
		break;
	}
	}
}