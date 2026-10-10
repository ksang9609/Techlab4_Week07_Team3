#include "EnginePCH.h"
#include "PointLightComponent.h"

UPointLightComponent::UPointLightComponent()
	: Super()
	, AttenuationRadius(10.0f)
	, LightFalloffExponent(2.0f)
{
}

void UPointLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
	if (!LineBatcher)
		return;

	auto DrawCircle = [&](const FVector& Center, const FVector& Up, const FVector& Right, float Radius, const FVector4& Color)
		{
			const int32 NumSegments = 32;
			const float AngleStep = 2.0f * PI / NumSegments;
			for (int32 i = 0; i < NumSegments; ++i)
			{
				float Angle1 = i * AngleStep;
				float Angle2 = (i + 1) * AngleStep;
				FVector Point1 = Center + (Right * std::cos(Angle1) + Up * std::sin(Angle1)) * Radius;
				FVector Point2 = Center + (Right * std::cos(Angle2) + Up * std::sin(Angle2)) * Radius;
				LineBatcher->AddLine(Point1, Point2, Color);
			}
		};

	const FVector Center = GetWorldLocation();
	const FVector Forward = GetWorldRotation().GetForwardVector();
	const FVector Up = GetWorldRotation().GetUpVector();
	const FVector Right = GetWorldRotation().GetRightVector();
	const FVector4 Color = { GetLightColor(), 1.0f };

	DrawCircle(Center, Up, Right, AttenuationRadius, Color);
	DrawCircle(Center, Right, Forward, AttenuationRadius, Color);
	DrawCircle(Center, Forward, Up, AttenuationRadius, Color);

	// Draw a line indicating the forward direction of the light
	const FVector LineEnd = Center + Forward * AttenuationRadius;
	LineBatcher->AddLine(Center, LineEnd, Color);
}