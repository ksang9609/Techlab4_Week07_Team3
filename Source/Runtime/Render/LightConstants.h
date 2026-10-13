#pragma once

#include "Math/Vector4.h"

inline constexpr uint32 MaxFireBalls = 16;
inline constexpr uint32 MaxPointLights = 128;
inline constexpr uint32 MaxSpotLights = 128;

struct alignas(16) FFireBallLight
{
	FVector4 PositionRadius; // xyz: 위치, w: 반경
	FVector4 ColorIntensity; // rgb: 색상, w: 강도
	FVector4 FalloffEnabled; // x: 반경 감쇠, y: 사용 여부, zw: 패딩
};

struct alignas(16) FFireBallLightConstants
{
	FFireBallLight Light[MaxFireBalls];
	uint32 LightCount;
	uint32 Padding[3] = {};
};

struct alignas(16) FDirectionalLightConstants
{
	FVector4 Direction; // xyz: 방향, w: 패딩
	FVector4 ColorIntensity; // rgb: 색상, w: 강도
};

struct alignas(16) FPointLightConstants
{
	FVector4 PositionRadius; // xyz: 위치, w: 반경
	FVector4 ColorIntensity; // rgb: 색상, w: 강도
	FVector4 FalloffParams;  // x: LightFalloffExponent, y: bUseInverseSquaredFalloff (1.0 or 0.0), zw: 패딩
};

struct alignas(16) FSpotLightConstants
{
	FVector4 PositionRadius; // xyz: 위치, w: 반경
	FVector4 DirectionAngle; // xyz: 방향, w: 빈 공간 패딩
	FVector4 ColorIntensity; // rgb: 색상, w: 강도
	FVector4 ConeScaleOffset; // x: 각도 보정 크기, y: 각도 보정 offset, z: LightFalloffExponent, w: bUseInverseSquaredFalloff
};

struct alignas(16) FForwardLightConstants
{
	FVector4 CameraPosition; // xyz: 카메라 위치, w: 패딩
	FVector4 AmbientColor; // rgb: 환경광 색상, w: 패딩
	FDirectionalLightConstants DirectionalLight;
	FPointLightConstants PointLights[MaxPointLights];
	FSpotLightConstants SpotLights[MaxSpotLights];
	uint32 DirectionalLightCount;
	uint32 PointLightCount;
	uint32 SpotLightCount;
	uint32 Padding;
};

struct alignas(16) FTileLightConstants
{
	uint32 TileSize = 0;
	uint32 TileCountX = 0;
	uint32 TileCountY = 0;
	uint32 Enabled = 0;

	uint32 ViewportStartX = 0;
	uint32 ViewportStartY = 0;
	uint32 Padding[2] = {};
};