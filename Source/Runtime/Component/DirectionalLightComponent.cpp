#include "EnginePCH.h"
#include "DirectionalLightComponent.h"
#include "Render/LineBatcher.h"

void UDirectionalLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
	if (!LineBatcher) return;

	const FVector Forward = GetLightDirection();
	const FVector Right = GetWorldRotation().GetRightVector();
	const FVector Up = GetWorldRotation().GetUpVector();

	const FVector Start = GetWorldLocation();
	const FVector End = Start + Forward * 100.0f;
	const FVector4 Color = GetLightColor().ReinterpretAsLinear().ToFVector4();
	LineBatcher->AddLine(Start, End, Color);

	for (int i = 0; i < 360; i += 30)
	{
		float AngleRad = FMath::DegreesToRadians(static_cast<float>(i));
		FVector CircleStartPoint = Start + (Right * std::cos(AngleRad) + Up * std::sin(AngleRad)) * 0.5f;
		FVector CircleEndPoint = CircleStartPoint + Forward * 100.0f;
		LineBatcher->AddLine(CircleStartPoint, CircleEndPoint, Color);
	}
}