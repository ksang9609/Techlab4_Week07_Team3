#include "EnginePCH.h"
#include "DirectionalLightComponent.h"
#include "Render/LineBatcher.h"

void UDirectionalLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
	if (!LineBatcher) return;
	const FVector Start = GetWorldLocation();
	const FVector End = Start + GetLightDirection() * 100.0f;
	const FVector4 Color = GetLightColor();
	LineBatcher->AddLine(Start, End, Color);
}