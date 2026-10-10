#include "EnginePCH.h"
#include "DirectionalLightComponent.h"
#include "Render/LineBatcher.h"

void UDirectionalLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
	if (!LineBatcher) return;
	const FVector Start = GetWorldLocation();
	const FVector End = Start + GetLightDirection() * 100.0f;
	LineBatcher->AddLine(Start, End, FVector4(0.0f, 0.0f, 0.0f, 1.0f));
}