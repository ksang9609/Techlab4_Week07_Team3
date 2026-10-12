#include "EnginePCH.h"
#include "PointLightComponent.h"

#include "Render/LineBatcher.h"
#include "Render/Renderer.h"

namespace
{
    void AddCircles(
        FLineBatcher* LineBatcher,
        const FVector& Center,
        const FVector& Forward,
        const FVector& Right,
        const FVector& Up,
        float Radius,
        float Range,
        const FVector4& Color)
    {
        constexpr int32 SegmentCount = 32;
        constexpr float TwoPI = 2.0f * PI;

        // Forward, Right, Up 방향에 각각 원 생성
        const FVector CircleCenters[3] =
        {
            Center + Forward * Range,
            Center + Right * Range,
            Center + Up * Range
        };

        // 각 원이 놓이는 평면의 두 축
        const FVector Axis1[3] = { Right, Forward, Forward };
        const FVector Axis2[3] = { Up, Up, Right };

        for (int32 CircleIndex = 0; CircleIndex < 3; ++CircleIndex)
        {
            const FVector& CircleCenter = CircleCenters[CircleIndex];

            FVector PreviousPoint;

            for (int32 i = 0; i <= SegmentCount; ++i)
            {
                const float Angle = TwoPI * i / SegmentCount;

                const FVector Point =
                    CircleCenter +
                    Axis1[CircleIndex] * (std::cos(Angle) * Radius) +
                    Axis2[CircleIndex] * (std::sin(Angle) * Radius);

                if (i > 0)
                {
                    LineBatcher->AddLine(PreviousPoint, Point, Color);
                }

                PreviousPoint = Point;
            }
        }
    }
}

void UPointLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
    if (LineBatcher == nullptr)
    {
        return;
    }

    FTransform WorldTransform;
    WorldTransform.Location = GetWorldLocation();
    WorldTransform.Rotation = GetWorldRotation();

    const FVector RefUp = FVector(0, 0, 1.0f);

    FVector Apex = WorldTransform.Location;
    FVector Forward = WorldTransform.GetForward();
    FVector Right = WorldTransform.GetRight();
    FVector Up = WorldTransform.GetUp();
    FVector Center = GetWorldLocation();


    // 에디터에서 값을 직접 드래그하므로 범위를 신뢰할 수 없다.
    // tan(90도)는 무한대라 원뿔 정점이 NaN이 되므로 여기서 막는다.
    const float SafeRadius = (Radius > 0.0f) ? Radius : 0.0f;
    const float SafeRadiusFalloff = (RadiusFalloff > 0.0f) ? RadiusFalloff : 0.0f;

    // 바깥 원뿔은 라이트 색 그대로, 안쪽은 구분되도록 흐리게
    AddCircles(LineBatcher, Center, Forward, Right, Up, SafeRadius, 0, FVector4(LightColor, 1.0f));

    FVector4 InnerColor(LightColor.X * 0.5f, LightColor.Y * 0.5f, LightColor.Z * 0.5f, 1.0f);

    AddCircles(LineBatcher, Apex, Forward, Right, Up, SafeRadiusFalloff, 0, InnerColor);
}

void UPointLightComponent::UpdateConstants(FLightConstants& Constants)
{
	int32 Num = Constants.NumPointLights;
    if (Num >= 16) return;
	Constants.PointLights[Num].ColorIntensity = FVector4(LightColor, Intensity);
	Constants.PointLights[Num].FalloffEnabled = FVector4(RadiusFalloff, 1.0f, 0.0f, 0.0f);
	Constants.PointLights[Num].PositionRadius = FVector4(GetWorldLocation(), Radius);
	++Constants.NumPointLights;
}