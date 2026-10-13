#include "EnginePCH.h"

#include "LightCulling.h"

void FTileBasedLightCulling::Resize(uint32 Width, uint32 Height)
{
	ViewportWidth = Width;
	ViewportHeight = Height;
	TileCountX = (Width + TileSize - 1) / TileSize;
	TileCountY = (Height + TileSize - 1) / TileSize;
	TileFrustums.SetNum(TileCountX * TileCountY);
	TilePointLightRanges.SetNum(TileCountX * TileCountY);
	TileSpotLightRanges.SetNum(TileCountX * TileCountY);
}

void FTileBasedLightCulling::BuildTileFrustums(const FMatrix& Projection)
{
	for (uint32 TileY = 0; TileY < TileCountY; ++TileY)
	{
		for (uint32 TileX = 0; TileX < TileCountX; ++TileX)
		{
			const float MinX = static_cast<float>(TileX * TileSize) / static_cast<float>(ViewportWidth) * 2.0f - 1.0f;
			const float MaxX = static_cast<float>(std::min((TileX + 1) * TileSize, ViewportWidth)) / static_cast<float>(ViewportWidth) * 2.0f - 1.0f;
			const float MinY =
				1.0f - 2.0f *
				static_cast<float>(std::min((TileY + 1) * TileSize, ViewportHeight)) /
				static_cast<float>(ViewportHeight);

			const float MaxY =
				1.0f - 2.0f *
				static_cast<float>(TileY * TileSize) /
				static_cast<float>(ViewportHeight);
			TileFrustums[TileY * TileCountX + TileX] = ExtractFrustumPlanes(Projection, MinX, MaxX, MinY, MaxY);
		}
	}
}

void FTileBasedLightCulling::CullLights(const FMatrix& View, const FForwardLightConstants& Lights)
{
	ResetResults();

	// CPU-based light culling
	
	uint32 PointOffset = 0;
	uint32 SpotOffSet = 0;
	for (uint32 TileY = 0; TileY < TileCountY; ++TileY)
	{
		for (uint32 TileX = 0; TileX < TileCountX; ++TileX)
		{
			const uint32 TileIndex = TileY * TileCountX + TileX;
			const FFrustumPlanes& Frustum = TileFrustums[TileIndex];

			/* Point Lights */
			FTileLightRange& PointLightRange = TilePointLightRanges[TileIndex];
			PointLightRange.Offset = PointOffset;
			uint32 PointLightCount = 0;
			for (uint32 LightIndex = 0; LightIndex < Lights.PointLightCount; ++LightIndex)
			{
				const FPointLightConstants& Light = Lights.PointLights[LightIndex];
				const FVector LightPositionWS = FVector(Light.PositionRadius.X, Light.PositionRadius.Y, Light.PositionRadius.Z);
				const FVector LightPositionVS = View.TransformPosition(LightPositionWS);
				if (IsSphereInFrustum(LightPositionVS, Light.PositionRadius.W, Frustum))
				{
					PointLightIndices.Push(LightIndex);
					++PointLightCount;
				}
			}
			PointLightRange.Count = PointLightCount;
			PointOffset += PointLightRange.Count;

			/* Spot Lights */
			// TODO: Replace sphere culling with cone culling for spot lights
			FTileLightRange& SpotLightRange = TileSpotLightRanges[TileIndex];
			SpotLightRange.Offset = SpotOffSet;
			uint32 SpotLightCount = 0;
			for (uint32 LightIndex = 0; LightIndex < Lights.SpotLightCount; ++LightIndex)
			{
				const FSpotLightConstants& Light = Lights.SpotLights[LightIndex];
				const FVector LightPositionWS = FVector(Light.PositionRadius.X, Light.PositionRadius.Y, Light.PositionRadius.Z);
				const FVector LightPositionVS = View.TransformPosition(LightPositionWS);
				if (IsSphereInFrustum(LightPositionVS, Light.PositionRadius.W, Frustum))
				{
					SpotLightIndices.Push(LightIndex);
					++SpotLightCount;
				}
			}
			SpotLightRange.Count = SpotLightCount;
			SpotOffSet += SpotLightRange.Count;
		}
	}

}

void FTileBasedLightCulling::ResetResults()
{
	PointLightIndices.Empty();
	SpotLightIndices.Empty();
	for (FTileLightRange& Range : TilePointLightRanges)
	{
		Range.Offset = 0;
		Range.Count = 0;
	}
	for (FTileLightRange& Range : TileSpotLightRanges)
	{
		Range.Offset = 0;
		Range.Count = 0;
	}
}

uint32 FTileBasedLightCulling::GetTileIndex(uint32 PixelX, uint32 PixelY) const
{
	const uint32 TileX = PixelX / TileSize;
	const uint32 TileY = PixelY / TileSize;
	return TileY * TileCountX + TileX;
}

