#include "EnginePCH.h"
#include "DirectionalLightComponent.h"

#include "Render/LineBatcher.h"
#include "Render/Renderer.h"

namespace
{
	void AddDirection(
		FLineBatcher* LineBatcher,
		const FVector& Center,
		const FVector& Direction,
		const FVector4& Color
	)
	{
		/*float DirFactor = 10.0f;
		FVector NormDir = Direction.Normalized();
		FVector Edge = Center + NormDir * DirFactor;

		LineBatcher->AddLine(Center, Edge, Color);*/

        constexpr float DirFactor = 10.0f;
        constexpr float Radius = 1.0f;
        constexpr int32 SegmentCount = 24;

        const FVector NormDir = Direction.Normalized();
        const FVector Edge = Center + NormDir * DirFactor;

        // 진행 방향에 수직인 두 축 생성
        const FVector Reference =
            std::abs(NormDir.Z) < 0.99f
            ? FVector(0.0f, 0.0f, 1.0f)
            : FVector(0.0f, 1.0f, 0.0f);

        const FVector Right = Reference.Cross(NormDir).Normalized();
        const FVector Up = NormDir.Cross(Right).Normalized();

        FVector PrevStart;
        FVector PrevEnd;

        for (int32 i = 0; i < SegmentCount; ++i)
        {
            const float Angle = 2.0f * PI * i / SegmentCount;

            const FVector Offset =
                Right * (std::cos(Angle) * Radius) +
                Up * (std::sin(Angle) * Radius);

            const FVector StartPoint = Center + Offset;
            const FVector EndPoint = Edge + Offset;

            if (i > 0)
            {
                // 시작 원
                LineBatcher->AddLine(PrevStart, StartPoint, Color);

                // 끝 원
                LineBatcher->AddLine(PrevEnd, EndPoint, Color);
            }

            // 원기둥 측면을 연결하는 선
            // 너무 촘촘하지 않도록 6개만 생성
            if (i % 4 == 0)
            {
                LineBatcher->AddLine(StartPoint, EndPoint, Color);
            }

            PrevStart = StartPoint;
            PrevEnd = EndPoint;
        }

        // 두 원 닫기
        const FVector FirstOffset = Right * Radius;

        LineBatcher->AddLine(PrevStart, Center + FirstOffset, Color);
        LineBatcher->AddLine(PrevEnd, Edge + FirstOffset, Color);
	}
}

void UDirectionalLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
	FVector Center = GetWorldLocation();

	AddDirection(LineBatcher, Center, LightDirection ,FVector4(LightColor, 1.0f));
}

void UDirectionalLightComponent::UpdateConstants(FLightConstants& Constants)
{
	Constants.Directional.ColorIntensity = FVector4(LightColor, Intensity);
	Constants.Directional.PositionRadius = FVector4(GetWorldLocation(), 0.0f);
	Constants.Directional.LightDirection = FVector4(LightDirection, 0.0f);
	++Constants.NumDirectionalLight;
}