#pragma once

#include "Math/Vector4.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"

constexpr uint32 MaxPointLights = 4;
constexpr uint32 MaxSpotLights = 4;

struct alignas(16) FAmbientLightInfo
{
    FVector Color; // RGB
    float Intensity;
};

struct alignas(16) FDirectionalLightInfo
{
    FVector Color; // RGB
    float Intensity;

    FVector Direction;
	float Padding; // Padding to align to 16 bytes
};

struct alignas(16) FPointLightInfo
{
    FVector Color; // RGB
    float Intensity;

    FVector Position;
    float Radius;

	float LightFalloffExponent;
	float Padding[3]; // Padding to align to 16 bytes
};

struct alignas(16) FSpotLightInfo
{
	FVector Color; // RGB
	float Intensity;

	FVector Position;
	float Radius;

    FVector Direction;
	float InnerConeCosine; // Cosine of the inner cone angle

	float OuterConeCosine; // Cosine of the outer cone angle
	float LightFalloffExponent;
    float Padding[2];
};

struct alignas(16) FLightingConstants
{
	FAmbientLightInfo AmbientLight;
	FDirectionalLightInfo DirectionalLight;
	FPointLightInfo PointLight[MaxPointLights];
	FSpotLightInfo SpotLight[MaxSpotLights];
};