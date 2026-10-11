#include "EnginePCH.h"
#include "PointLightComponent.h"
#include "Render/LineBatcher.h"

void UPointLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
	if (!LineBatcher) return;
	const FVector4 Color = GetLightColor();
	const FVector Location = GetWorldLocation();
	// 빛이 닿는 범위를 구형으로 그린다.
	LineBatcher->AddWireSphere(Location, AttenuationRadius, 32, Color);
}