#pragma once

#include "Core/Types.h"
#include "Math/Frustum.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Containers/Array.h"

#include "LightConstants.h"


struct FTileLightRange
{
	uint32 Offset = 0;
	uint32 Count = 0;
};

class FTileBasedLightCulling
{
public:
	FTileBasedLightCulling() = default;

	void Resize(uint32 Width, uint32 Height);
	void BuildTileFrustums(const FMatrix& Projection);

	void CullLights(const FMatrix& View, const FForwardLightConstants& Lights);
	
	void ResetResults();

	uint32 GetTileIndex(uint32 PixelX, uint32 PixelY) const;

	inline TArray<FFrustumPlanes>& GetTileFrustums() { return TileFrustums; }
	inline TArray<FTileLightRange>& GetPointTileLightRanges() { return TilePointLightRanges; }
	inline TArray<FTileLightRange>& GetSpotTileLightRanges() { return TileSpotLightRanges; }
	inline TArray<uint32>& GetPointLightIndices() { return PointLightIndices; }
	inline TArray<uint32>& GetSpotLightIndices() { return SpotLightIndices; }

private:
	/* Tiles */
	uint32 TileSize = 16; // Tile size in pixels
	uint32 ViewportWidht = 0;
	uint32 ViewportHeight = 0;
	uint32 TileCountX = 0;
	uint32 TileCountY = 0;

	/* Tile Data */
	TArray<FFrustumPlanes> TileFrustums;
	TArray<FTileLightRange> TilePointLightRanges;
	TArray<FTileLightRange> TileSpotLightRanges;
	TArray<uint32> PointLightIndices;
	TArray<uint32> SpotLightIndices;

	/* Debug Statistics */

};