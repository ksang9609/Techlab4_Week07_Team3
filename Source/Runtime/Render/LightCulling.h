#pragma once

#include "Core/Types.h"
#include "Math/Frustum.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Containers/Array.h"
#include "Render/Buffer.h"

#include "LightConstants.h"

inline constexpr std::string_view BuildTileFrustumsCSKey
= "Resources/Shader/Compute/BuildTileFrustumsCS.hlsl";

inline constexpr std::string_view LightCullingCSKey
= "Resources/Shader/Compute/LightCullingCS.hlsl";

struct FTileLightRange
{
	uint32 Offset = 0;
	uint32 Count = 0;
};

class FComputeShader;

struct alignas(16) FTileFrustumBuildConstants
{
	FMatrix Projection;

	uint32 ViewportWidth;
	uint32 ViewportHeight;
	uint32 TileSize;
	uint32 TileCountX;

	uint32 TileCountY;
	uint32 Padding[3] = {};
};

struct alignas(16) FTileLightCullConstants
{
	FMatrix View;

	uint32 TileCountX;
	uint32 TileCountY;
	uint32 Paddoing[2] = {};
};

class FTileBasedLightCulling
{
public:
	FTileBasedLightCulling(bool bUseCS = false) : bUseCS(bUseCS) {}

	bool Resize(uint32 Width, uint32 Height);
	void BuildTileFrustums(const FMatrix& Projection);

	void CullLights(const FMatrix& View, const FForwardLightConstants& Lights);

	void ResetResults();

	uint32 GetTileIndex(uint32 PixelX, uint32 PixelY) const;

	inline const TArray<FFrustumPlanes>& GetTileFrustums() const { return TileFrustums; }
	inline const TArray<FTileLightRange>& GetPointTileLightRanges() const { return TilePointLightRanges; }
	inline const TArray<FTileLightRange>& GetSpotTileLightRanges() const { return TileSpotLightRanges; }
	inline const TArray<uint32>& GetPointLightIndices() const { return PointLightIndices; }
	inline const TArray<uint32>& GetSpotLightIndices() const { return SpotLightIndices; }
	inline const FStructuredBuffer* GetPointLightRangesBuffer() const { return GPUPointRanges.get(); }
	inline const FStructuredBuffer* GetPointLightIndicesBuffer() const { return GPUPointIndices.get(); }
	inline const FStructuredBuffer* GetSpotLightRangesBuffer() const { return GPUSpotRanges.get(); }
	inline const FStructuredBuffer* GetSpotLightIndicesBuffer() const { return GPUSpotIndices.get(); }

	uint32 GetTileSize() const { return TileSize; }
	uint32 GetTileCountX() const { return TileCountX; }
	uint32 GetTileCountY() const { return TileCountY; }

	FStructuredBuffer* GetFrustumBuffer() const { return bFrustumsReady ? FrustumBuffer.get() : nullptr; }

	bool AreGPUResultsReady() const { return bGPUResultsReady; }

private:
	bool bUseCS = false;

	/* Tiles */
	uint32 TileSize = 16; // Tile size in pixels
	uint32 ViewportWidth = 0;
	uint32 ViewportHeight = 0;
	uint32 TileCountX = 0;
	uint32 TileCountY = 0;

	/* Tile Data CPU Resources */
	TArray<FFrustumPlanes> TileFrustums;
	TArray<FTileLightRange> TilePointLightRanges;
	TArray<FTileLightRange> TileSpotLightRanges;
	TArray<uint32> PointLightIndices;
	TArray<uint32> SpotLightIndices;

	/* GPU Resources */
	FComputeShader* BuildFrustumCS = nullptr;

	TUniquePtr<FConstantBuffer> FrustumBuildCB;
	TUniquePtr<FStructuredBuffer> FrustumBuffer;

	bool bFrustumsReady = false;

	FComputeShader* CullingCS = nullptr;

	TUniquePtr<FConstantBuffer> CullCB;
	TUniquePtr<FConstantBuffer> CullLightCB;

	TUniquePtr<FStructuredBuffer> GPUPointRanges;
	TUniquePtr<FStructuredBuffer> GPUPointIndices;
	TUniquePtr<FStructuredBuffer> GPUSpotRanges;
	TUniquePtr<FStructuredBuffer> GPUSpotIndices;

	bool bGPUResultsReady = false;

	bool EnsureGPUCullingResources();


	/* Debug Statistics */

};