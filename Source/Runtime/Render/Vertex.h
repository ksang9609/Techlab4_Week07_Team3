#pragma once

#include <d3d11.h>

struct FVertex
{
	FVector Position = FVector();
	FVector2 UV = FVector2();
	FVector4 Color = FVector4();
};

struct FLineVertex
{
	FVector Position;
	FVector4 Color;
};

struct FTextVertex
{
	FVector Position;
	FVector2 UV;
};

// Todo: subuv
struct FParticleVertex
{
	FVector Position;
	FVector2 UV;
};

// StaticMesh용 PNCTT Vertex: Position / Normal / Color / Texcoord / Tangent
struct FVertexPNCTT
{
	FVector Position = FVector();
	FVector Normal = FVector();
	FVector4 Color = FVector4();
	FVector2 UV = FVector2();
	FVector4 Tangent = FVector4(0.0f, 0.0f, 0.0f, 1.0f);
};
