#include "EnginePCH.h"

#include "LightCulling.h"

#include "Render/RenderCommand.h"
#include "Render/RenderResourceManager.h"
#include "Render/Shader.h"

void FTileBasedLightCulling::Resize(uint32 Width, uint32 Height)
{
	ViewportWidth = Width;
	ViewportHeight = Height;
	TileCountX = (Width + TileSize - 1) / TileSize;
	TileCountY = (Height + TileSize - 1) / TileSize;

	if (bUseCS)
	{
		bFrustumsReady = false;
		bGPUResultsReady = false;

		const uint32 TileCount = TileCountX * TileCountY;

		if (TileCount == 0)
		{
			return;
		}

		if (!FrustumBuffer ||
			FrustumBuffer->GetCapacity() < TileCount)
		{
			// float4 * 6 = 96 bytes per tile frustum
			FrustumBuffer = 
				RenderCommand::CreateRWStructuredBuffer(96,	TileCount);
		}
	}
	else
	{
		TileFrustums.SetNum(TileCountX * TileCountY);
		TilePointLightRanges.SetNum(TileCountX * TileCountY);
		TileSpotLightRanges.SetNum(TileCountX * TileCountY);
	}
}

void FTileBasedLightCulling::BuildTileFrustums(const FMatrix& Projection)
{
	if (bUseCS)
	{
		bFrustumsReady = false;
		bGPUResultsReady = false;

		if (TileCountX == 0 || TileCountY == 0)
		{
			return;
		}

		if (!BuildFrustumCS)
		{
			BuildFrustumCS =
				FRenderResourceManager::GetComputeShader(FString(BuildTileFrustumsCSKey));
		}

		if (!FrustumBuildCB)
		{
			FrustumBuildCB = 
				RenderCommand::CreateConstantBuffer(sizeof(FTileFrustumBuildConstants));
		}

		FTileFrustumBuildConstants BuildConstants{
			.Projection = Projection,
			.ViewportWidth = ViewportWidth,
			.ViewportHeight = ViewportHeight,
			.TileSize = TileSize,
			.TileCountX = TileCountX,
			.TileCountY = TileCountY
		};

		RenderCommand::UpdateBufferData(
			FrustumBuildCB.get(),
			&BuildConstants,
			sizeof(FTileFrustumBuildConstants)
		);

		auto* Context = RenderCommand::GetContext();

		ID3D11ShaderResourceView* NullSRV = nullptr;
		Context->CSSetShaderResources(0, 1, &NullSRV);

		RenderCommand::BindComputeShader(BuildFrustumCS);

		RenderCommand::BindConstantBuffer(
			0,
			FrustumBuildCB.get(),
			EShaderBindFlagBits::Compute
		);

		ID3D11UnorderedAccessView* UAV = FrustumBuffer->GetUAV();
		Context->CSSetUnorderedAccessViews(0, 1, &UAV, nullptr);

		RenderCommand::Dispatch(
			(TileCountX + 7) / 8,
			(TileCountY + 7) / 8,
			1);

		ID3D11UnorderedAccessView* NullUAV = nullptr;
		Context->CSSetUnorderedAccessViews(0, 1, &NullUAV, nullptr);

		RenderCommand::BindComputeShader(nullptr);

		bFrustumsReady = true;
	}
	else
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
}

void FTileBasedLightCulling::CullLights(const FMatrix& View, const FForwardLightConstants& Lights)
{
	ResetResults();

	if (bUseCS)
	{
		// GPU-based light culling
		bGPUResultsReady = false;

		if (!bFrustumsReady ||
			TileCountX == 0 ||
			TileCountY == 0 ||
			!FrustumBuffer ||
			!FrustumBuffer->GetSRV())
		{
			return;
		}

		ID3D11DeviceContext* Context = RenderCommand::GetContext();

		// 이전 Draw에서 결과 버퍼를 읽던 슬롯을 해제한다.
		ID3D11ShaderResourceView* NullSRVs[4] = {};
		Context->PSSetShaderResources(8, 4, NullSRVs);
		Context->VSSetShaderResources(8, 4, NullSRVs);

		if (!EnsureGPUCullingResources())
		{
			HTR_LOG(Warning, "GPU Light Culling Resources Failed!");
			return;
		}

		FTileLightCullConstants Constants{};
		Constants.View = View;
		Constants.TileCountX = TileCountX;
		Constants.TileCountY = TileCountY;

		RenderCommand::UpdateBufferData(
			CullCB.get(), &Constants, sizeof(Constants));

		RenderCommand::UpdateBufferData(
			CullLightCB.get(), &Lights, sizeof(Lights));

		RenderCommand::BindComputeShader(CullingCS);

		RenderCommand::BindConstantBuffer(
			0, CullCB.get(), EShaderBindFlagBits::Compute);

		RenderCommand::BindConstantBuffer(
			4, CullLightCB.get(), EShaderBindFlagBits::Compute);

		RenderCommand::BindShaderResource(
			0, FrustumBuffer.get(), EShaderBindFlagBits::Compute);

		ID3D11UnorderedAccessView* UAVs[4] =
		{
			GPUPointRanges->GetUAV(),
			GPUPointIndices->GetUAV(),
			GPUSpotRanges->GetUAV(),
			GPUSpotIndices->GetUAV()
		};

		Context->CSSetUnorderedAccessViews(0, 4, UAVs, nullptr);

		RenderCommand::Dispatch(
			(TileCountX + 7) / 8,
			(TileCountY + 7) / 8,
			1);

		// 결과를 이후 PS에서 읽을 수 있도록 UAV 해제.
		ID3D11UnorderedAccessView* NullUAVs[4] = {};
		Context->CSSetUnorderedAccessViews(0, 4, NullUAVs, nullptr);

		ID3D11ShaderResourceView* NullFrustumSRV = nullptr;
		Context->CSSetShaderResources(0, 1, &NullFrustumSRV);

		RenderCommand::BindComputeShader(nullptr);

		bGPUResultsReady = true;
	}
	else
	{
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

bool FTileBasedLightCulling::EnsureGPUCullingResources()
{
	if (!CullingCS)
	{
		CullingCS = FRenderResourceManager::GetComputeShader(
			FString(LightCullingCSKey));
	}

	if (!CullCB)
	{
		CullCB = RenderCommand::CreateConstantBuffer(
			sizeof(FTileLightCullConstants));
	}

	if (!CullLightCB)
	{
		CullLightCB = RenderCommand::CreateConstantBuffer(
			sizeof(FForwardLightConstants));
	}

	if (!CullingCS ||
		!CullingCS->IsValid() ||
		!CullCB || !CullCB->GetBuffer() ||
		!CullLightCB || !CullLightCB->GetBuffer())
	{
		return false;
	}

	const uint32 TileCount = TileCountX * TileCountY;

	auto EnsureBuffer = [](
		TUniquePtr<FStructuredBuffer>& Buffer,
		uint32 Stride,
		uint32 Count) -> bool
		{
			if (Buffer &&
				Buffer->GetBuffer() &&
				Buffer->GetSRV() &&
				Buffer->GetUAV() &&
				Buffer->GetStride() == Stride &&
				Buffer->GetCapacity() >= Count)
			{
				return true;
			}

			auto NewBuffer = RenderCommand::CreateRWStructuredBuffer(
				Stride, std::max(Count, 1u));

			if (!NewBuffer ||
				!NewBuffer->GetBuffer() ||
				!NewBuffer->GetSRV() ||
				!NewBuffer->GetUAV())
			{
				return false;
			}

			Buffer = std::move(NewBuffer);
			return true;
		};

	return
		EnsureBuffer(GPUPointRanges, sizeof(FTileLightRange), TileCount) &&
		EnsureBuffer(GPUPointIndices, sizeof(uint32),
			TileCount * MaxPointLights) &&
		EnsureBuffer(GPUSpotRanges, sizeof(FTileLightRange), TileCount) &&
		EnsureBuffer(GPUSpotIndices, sizeof(uint32),
			TileCount * MaxSpotLights);
}
