#include "EnginePCH.h"
#include "Scene.h"

void FScene::AddPrimitive(UPrimitiveComponent* Component)
{
	if (!Component || Component->SceneProxy) return;

	FPrimitiveSceneProxy* Proxy = new FPrimitiveSceneProxy(Component);
	Proxy->Scene = this;
	Proxy->PackedIndex = Proxies.Num();
	Component->SceneProxy = Proxy;
	MarkDirty(Proxy);
	MarkRenderStateDirty(Proxy);

	Proxies.Add(Proxy);
	PrimitiveBounds.Add(FAABB{});
	PrimitiveFlags.Add(0);

	bElementListChanged = true;
}

void FScene::RemovePrimitive(UPrimitiveComponent* Component)
{
	if (!Component || !Component->SceneProxy) return;

	FPrimitiveSceneProxy* Proxy = Component->SceneProxy;
	if (!Proxy) return;

	const uint32 Index = static_cast<uint32>(Proxy->PackedIndex);

	Proxies.RemoveAtSwap(Index);
	PrimitiveBounds.RemoveAtSwap(Index);
	PrimitiveFlags.RemoveAtSwap(Index);

	if (Index < static_cast<uint32>(Proxies.Num()))
		Proxies[Index]->PackedIndex = static_cast<int32>(Index);

	if (Proxy->bQueuedForUpdate)
	{
		const int32 Found = DirtyProxies.Find(Proxy);   // 프로젝트 TArray의 Find 이름에 맞게
		if (Found != INDEX_NONE) DirtyProxies.RemoveAtSwap(Found);
	}

	if (Proxy->bRenderStateQueued)
	{
		const int32 Found = RenderStateDirtyProxies.Find(Proxy);
		if (Found != INDEX_NONE) RenderStateDirtyProxies.RemoveAtSwap(Found);
	}

	Component->SceneProxy = nullptr;
	delete Proxy;

	bElementListChanged = true;
}

void FScene::RemoveAllPrimitives()
{
	// 하나씩 RemovePrimitive하면 대기열 검색(Find)이 물체 수만큼 반복되므로 통째로 비운다.
	for (FPrimitiveSceneProxy* Proxy : Proxies)
	{
		if (UPrimitiveComponent* Component = Proxy->GetComponent())
			Component->SceneProxy = nullptr;
		delete Proxy;
	}
	Proxies.Reset();
	PrimitiveBounds.Reset();
	PrimitiveFlags.Reset();
	DirtyProxies.Reset();
	RenderStateDirtyProxies.Reset();
	BVH.Clear();
	bElementListChanged = true;
}

void FScene::AddFogInfo(uint32 Id, const FFogInfo& FogInfo)
{
	FExponentialHeightFogSceneInfo sceneFogInfo;
	sceneFogInfo.Id = Id;
	sceneFogInfo.FogInfo = FogInfo;


	FogInfos.Add(sceneFogInfo);
}

void FScene::RemoveFogInfo(uint32 Id)
{
	// 렌더러는 FogInfos[0]만 쓰므로, 순서를 유지하려고 RemoveAtSwap 대신 RemoveAt을 쓴다.
	for (int32 i = 0; i < FogInfos.Num(); ++i)
	{
		if (FogInfos[i].Id == Id)
		{
			FogInfos.RemoveAt(i);
			return;
		}
	}
}

void FScene::RemoveAllFogInfo()
{
	FogInfos.Reset();

}

void FScene::UpdateFogInfo(uint32 Id, const FFogInfo& FogInfo)
{
	for (FExponentialHeightFogSceneInfo& SceneFogInfo : FogInfos)
	{
		if (SceneFogInfo.Id == Id)
		{
			SceneFogInfo.FogInfo = FogInfo;
			return;
		}


	}
}

//void FScene::RemoveFogInfo(uint32 Id)
//{
//	for (int32 i = 0; i < FogInfos.Num(); ++i)
//	{
//		if (FogInfos[i].Id == Id)
//		{
//			FogInfos.RemoveAtSwap(i);
//			return;
//		}
//	}
//}


void FScene::UpdateAllTransforms()
{
	for (FPrimitiveSceneProxy* Proxy : RenderStateDirtyProxies)
	{
		Proxy->UpdateRenderState();
		Proxy->bRenderStateQueued = false;
	}
	RenderStateDirtyProxies.Reset();

	for (FPrimitiveSceneProxy* Proxy : DirtyProxies)
	{
		Proxy->UpdateTransform();
		PrimitiveBounds[Proxy->PackedIndex] = Proxy->GetBounds();
		PrimitiveFlags[Proxy->PackedIndex] = Proxy->GetComponent()->IsVisible() ? 1 : 0;
		Proxy->bQueuedForUpdate = false;
	}

	//const int32 Count = Proxies.Num();
	//for (int32 i = 0; i < Count; ++i)
	//{
	//	FPrimitiveSceneProxy* Proxy = Proxies[i];
	//	Proxy->UpdateTransform();

	//	const FAABB& Bounds = Proxy->GetBounds();
	//	if (PrimitiveBounds[i].Center != Bounds.Center || PrimitiveBounds[i].Extent != Bounds.Extent)
	//	{
	//		bBoundsChanged = true;
	//	}

	//	PrimitiveBounds[i] = Proxy->GetBounds();
	//	PrimitiveFlags[i] = Proxy->GetComponent()->IsVisible() ? 1 : 0;
	//}

	const bool bAnyMoved = DirtyProxies.Num() > 0;
	DirtyProxies.Reset();


	if (bElementListChanged)
	{
		BuildBVH();
		bElementListChanged = false;
	}
	else if (bAnyMoved)
	{
		BVH.Refit();
	}
}

void FScene::BuildBVH()
{
	BVH.Clear();
	// BillboardComponents.Reset();

	TArray<FPrimitiveSceneProxy*> Elements;
	Elements.Reserve(Proxies.Num());
	for (FPrimitiveSceneProxy* Proxy : Proxies)
	{
		if (Proxy && Proxy->GetComponent())
			Elements.Add(Proxy);
	}

	BVH.Build(std::span(Elements.GetData(), Elements.Num()));
}

void FScene::MarkDirty(FPrimitiveSceneProxy* Proxy)
{
	if (Proxy->bQueuedForUpdate) return;            // 중복 추가 방지
	Proxy->bQueuedForUpdate = true;
	DirtyProxies.Add(Proxy);
}

void FScene::MarkRenderStateDirty(FPrimitiveSceneProxy* Proxy)
{
	if (!Proxy || Proxy->bRenderStateQueued) return;
	Proxy->bRenderStateQueued = true;
	RenderStateDirtyProxies.Add(Proxy);
}


void FScene::UpdateFireBallLight(FRenderer* Renderer)
{
	if (!Renderer)
		return;

	FFireBallLight Data{};
	FFireBallLightConstants Constants{};

	Constants.LightCount = 0;

	// 기본값: 활성 FireBall 없음
	Data.PositionRadius = FVector4(0, 0, 0, 0);
	Data.ColorIntensity = FVector4(0, 0, 0, 0);
	Data.FalloffEnabled = FVector4(1, 0, 0, 0);

	for (UFireBallComponent* FireBall : FireBallComponents)
	{
		if (Constants.LightCount >= MaxFireBalls)
		{
			break; // 최대 FireBall 수를 초과하면 루프 종료
		}

		if (!FireBall)
			continue;

		const FVector Position = FireBall->GetWorldLocation();
		const FVector4 Color = FireBall->GetLightColor();

		Data.PositionRadius = FVector4(
			Position.X,
			Position.Y,
			Position.Z,
			std::max<float>(FireBall->GetRadius(), 0.001f)
		);

		Data.ColorIntensity = FVector4(
			Color.X,
			Color.Y,
			Color.Z,
			std::max<float>(FireBall->GetIntensity(), 0.0f)
		);

		Data.FalloffEnabled = FVector4(
			std::max<float>(FireBall->GetRadiusFalloff(), 0.001f),
			1.0f,
			0.0f,
			0.0f
		);

		Constants.Light[Constants.LightCount] = Data;
		Constants.LightCount++;
	}

	Renderer->SetFireBallLight(Constants);
}

void FScene::RegisterFireBall(UFireBallComponent* FireBall)
{
	if (!FireBall)
		return;

	for (UFireBallComponent* Existing : FireBallComponents)
	{
		if (Existing == FireBall)
			return;
	}
	FireBallComponents.Add(FireBall);
}
void FScene::UnregisterFireBall(UFireBallComponent* FireBall)
{
	for (uint32 i = 0; i < FireBallComponents.Num(); ++i)
	{
		if (FireBallComponents[i] == FireBall)
		{
			FireBallComponents.RemoveAt(i, 1);
			return;
		}
	}
}

void FScene::UpdateLight(FRenderer* Renderer, const FVector& CameraPosition)
{
	FForwardLightConstants Constants{};
	Constants.CameraPosition = FVector4(CameraPosition, 1.0f);
	// 환경광이 있으면 그 색상과 세기를 사용하고, 없으면 기본값(0.1f, 0.1f, 0.1f, 0.1f)을 사용한다.
	if (!AmbientLightComponents.IsEmpty())
	{
		Constants.AmbientColor = AmbientLightComponents[0]->GetLightColor() * AmbientLightComponents[0]->GetIntensity();
	}
	else
	{
		Constants.AmbientColor = FVector4(0.1f, 0.1f, 0.1f, 0.1f);
	}
	if (!DirectionalLightComponents.IsEmpty() && DirectionalLightComponents[0]->IsVisible())
	{
		Constants.DirectionalLight.Direction = DirectionalLightComponents[0]->GetLightDirection();
		const FVector4 Color = DirectionalLightComponents[0]->GetLightColor();
		float Intensity = DirectionalLightComponents[0]->GetIntensity();
		Constants.DirectionalLight.ColorIntensity = FVector4(Color.X, Color.Y, Color.Z, Intensity);
		Constants.DirectionalLightCount = 1;
	}
	if (!PointLightComponents.IsEmpty())
	{
		Constants.PointLightCount = std::min(static_cast<uint32>(PointLightComponents.Num()), MaxPointLights);
		for (uint32 i = 0; i < Constants.PointLightCount; ++i)
		{
			FVector Position = PointLightComponents[i]->GetWorldLocation();
			FVector4 Color = PointLightComponents[i]->GetLightColor();
			float Intensity = PointLightComponents[i]->GetIntensity();
			float Radius = std::max(PointLightComponents[i]->GetAttenuationRadius(), 0.001f);
			Constants.PointLights[i].PositionRadius = FVector4(Position, Radius);
			Constants.PointLights[i].ColorIntensity = FVector4(Color.X, Color.Y, Color.Z, Intensity);
		}
	}
	else
	{
		Constants.PointLightCount = 0;
	}
	if (!SpotLightComponents.IsEmpty())
	{
		Constants.SpotLightCount = std::min(static_cast<uint32>(SpotLightComponents.Num()), MaxSpotLights);
		for (uint32 i = 0; i < Constants.SpotLightCount; ++i)
		{
			FVector Position = SpotLightComponents[i]->GetWorldLocation();
			FVector Direction = SpotLightComponents[i]->GetLightDirection();
			FVector4 Color = SpotLightComponents[i]->GetLightColor();
			float Intensity = SpotLightComponents[i]->GetIntensity();
			float Radius = std::max(SpotLightComponents[i]->GetAttenuationRadius(), 0.001f);
			float InnerConeAngle = SpotLightComponents[i]->GetInnerConeAngle();
			float OuterConeAngle = SpotLightComponents[i]->GetOuterConeAngle();
			Constants.SpotLights[i].PositionRadius = FVector4(Position, Radius);
			Constants.SpotLights[i].DirectionAngle = FVector4(Direction, 0.0f);

			Constants.SpotLights[i].ColorIntensity = FVector4(Color.X, Color.Y, Color.Z, Intensity);
		}

	}
	else
	{
		Constants.SpotLightCount = 0;
	}

	Renderer->SetForwardLight(Constants);
}

void FScene::RegisterLight(ULightComponent* Light)
{
	if (!Light) return;

	// DirectionalLightComponents, PointLightComponents, SpotLightComponents, AmbientLightComponents에 이미 등록된 Light인지 확인
	if (Light->IsA<UDirectionalLightComponent>())
	{
		// 이미 등록된 경우 중복 등록 방지
		for (ULightComponent* L : DirectionalLightComponents)
		{
			if (L == Light)
			{
				return;
			}
		}
		DirectionalLightComponents.Add(static_cast<UDirectionalLightComponent*>(Light));
	}
	else if (Light->IsA<UPointLightComponent>())
	{
		// 이미 등록된 경우 중복 등록 방지
		for (ULightComponent* L : PointLightComponents)
		{
			if (L == Light)
			{
				return;
			}
		}
		PointLightComponents.Add(static_cast<UPointLightComponent*>(Light));
	}
	else if (Light->IsA<USpotLightComponent>())
	{
		// 이미 등록된 경우 중복 등록 방지
		for (ULightComponent* L : SpotLightComponents)
		{
			if (L == Light)
			{
				return;
			}
		}
		SpotLightComponents.Add(static_cast<USpotLightComponent*>(Light));
	}
	else if (Light->IsA<UAmbientLightComponent>())
	{
		// 이미 등록된 경우 중복 등록 방지
		for (ULightComponent* L : AmbientLightComponents)
		{
			if (L == Light)
			{
				return;
			}
		}
		AmbientLightComponents.Add(static_cast<UAmbientLightComponent*>(Light));
	}
}

void FScene::UnregisterLight(ULightComponent* Light)
{
	for (uint32 i = 0; i < DirectionalLightComponents.Num(); ++i)
	{
		if (DirectionalLightComponents[i] == Light)
		{
			DirectionalLightComponents.RemoveAt(i, 1);
			return;
			}
	}
	for (uint32 i = 0; i < PointLightComponents.Num(); ++i)
	{
		if (PointLightComponents[i] == Light)
		{
			PointLightComponents.RemoveAt(i, 1);
			return;
		}
	}
	for (uint32 i = 0; i < SpotLightComponents.Num(); ++i)
	{
		if (SpotLightComponents[i] == Light)
		{
			SpotLightComponents.RemoveAt(i, 1);
			return;
		}
	}
	for (uint32 i = 0; i < AmbientLightComponents.Num(); ++i)
	{
		if (AmbientLightComponents[i] == Light)
		{
			AmbientLightComponents.RemoveAt(i, 1);
			return;
		}
	}
}