#include "EnginePCH.h"

#include "DirectionalLightComponent.h"

void UDirectionalLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
	if (!LineBatcher)
		return;

	const FVector Forward = GetWorldRotation().GetForwardVector();
	const FVector Right = GetWorldRotation().GetRightVector();
	const FVector Up = GetWorldRotation().GetUpVector();

	// Draw a simple representation of the directional light as a line
	const FVector Start = GetWorldLocation();
	const FVector End = Start + Forward * 100.0f; // Extend the line in the forward direction

	const FVector4 Color = { GetLightColor(), 1.0f }; // Use the light's color for the line
	LineBatcher->AddLine(Start, End, Color);

	// Draw circle lines to make it more visible
	for (int i = 0; i < 360; i += 30)
	{
		float AngleRad = FMath::DegreesToRadians(static_cast<float>(i));
		FVector CircleStartPoint = Start + (Right * std::cos(AngleRad) + Up * std::sin(AngleRad)) * 0.5f; // Circle radius of 0.5 units
		FVector CircleEndPoint = CircleStartPoint + Forward * 100.0f; // Extend the line in the forward direction
		LineBatcher->AddLine(CircleStartPoint, CircleEndPoint, Color);
	}
}