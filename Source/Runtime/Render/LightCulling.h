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

	inline const TArray<FFrustumPlanes>& GetTileFrustums() const { return TileFrustums; }
	inline const TArray<FTileLightRange>& GetPointTileLightRanges() const { return TilePointLightRanges; }
	inline const TArray<FTileLightRange>& GetSpotTileLightRanges() const { return TileSpotLightRanges; }
	inline const TArray<uint32>& GetPointLightIndices() const { return PointLightIndices; }
	inline const TArray<uint32>& GetSpotLightIndices() const { return SpotLightIndices; }

	uint32 GetTileSize() const { return TileSize; }
	uint32 GetTileCountX() const { return TileCountX; }
	uint32 GetTileCountY() const { return TileCountY; }

private:
	/* Tiles */
	uint32 TileSize = 16; // Tile size in pixels
	uint32 ViewportWidth = 0;
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