#include "EnginePCH.h"
#include "Component/PointLightComponent.h"
#include "Render/LineBatcher.h"

void UPointLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
	if (LineBatcher == nullptr)
	{
		return;
	}

	const FVector Center = GetWorldLocation();
	const float Radius = GetAttenuationRadius();
	const FVector End = Center + FVector(Radius, 0.0f, 0.0f);

	LineBatcher->AddLine(Center, End, GetLightColor());

	// XY 평면상의 디버그 원 생성
	for (int i = 0; i < 24; i++)
	{
		const float CurrentAngle = 2.0f * PI * i / 24;
		const FVector A = Center + FVector(std::cosf(CurrentAngle) * Radius, std::sinf(CurrentAngle) * Radius, 0.0f);

		const float NextAngle = 2.0f * PI * (i + 1) / 24;
		const FVector B = Center + FVector(std::cosf(NextAngle) * Radius, std::sinf(NextAngle) * Radius, 0.0f);

		LineBatcher->AddLine(A, B, GetLightColor());
	}

	// XZ 평면상의 디버그 원 생성
	for (int i = 0; i < 24; i++)
	{
		const float CurrentAngle = 2.0f * PI * i / 24;
		const FVector A = Center + FVector(std::cosf(CurrentAngle) * Radius, 0.0f, std::sinf(CurrentAngle) * Radius);

		const float NextAngle = 2.0f * PI * (i + 1) / 24;
		const FVector B = Center + FVector(std::cosf(NextAngle) * Radius, 0.0f, std::sinf(NextAngle) * Radius);

		LineBatcher->AddLine(A, B, GetLightColor());
	}

	// YZ 평면상의 디버그 원 생성
	for (int i = 0; i < 24; i++)
	{
		const float CurrentAngle = 2.0f * PI * i / 24;
		const FVector A = Center + FVector(0.0f, std::cosf(CurrentAngle) * Radius, std::sinf(CurrentAngle) * Radius);

		const float NextAngle = 2.0f * PI * (i + 1) / 24;
		const FVector B = Center + FVector(0.0f, std::cosf(NextAngle) * Radius, std::sinf(NextAngle) * Radius);

		LineBatcher->AddLine(A, B, GetLightColor());
	}

	//FTransform WorldTransform;
	//WorldTransform.Location = GetWorldLocation();
	//WorldTransform.Rotation = GetWorldRotation();

	//FVector Apex = WorldTransform.Location;
	//FVector Forward = WorldTransform.GetForward();
	//FVector Right = WorldTransform.GetRight();
	//FVector Up = WorldTransform.GetUp();

	//// 에디터에서 값을 직접 드래그하므로 범위를 신뢰할 수 없다.
	//// tan(90도)는 무한대라 원뿔 정점이 NaN이 되므로 여기서 막는다.
	//const float SafeOuterAngle = FMath::Clamp(OuterConeAngle, 0.0f, 89.0f);
	//const float SafeInnerAngle = FMath::Clamp(InnerConeAngle, 0.0f, SafeOuterAngle);
	//const float SafeRadius = FMath::Max(GetAttenuationRadius(), 0.0f);

	//// 바깥 원뿔은 라이트 색 그대로, 안쪽은 구분되도록 흐리게
	//const FVector4 LightColor = GetLightColor();
	//AddWireCone(LineBatcher, Apex, Forward, Right, Up, SafeOuterAngle, SafeRadius, LightColor);

	//FVector4 InnerColor(LightColor.X * 0.5f, LightColor.Y * 0.5f, LightColor.Z * 0.5f, LightColor.W);
	//AddWireCone(LineBatcher, Apex, Forward, Right, Up, SafeInnerAngle, SafeRadius, InnerColor);
}
