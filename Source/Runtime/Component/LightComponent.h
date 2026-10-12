#pragma once
#include "LightComponentBase.h"
#include "Render/LineBatcher.h"

class FLightSceneProxy;

class ULightComponent : public ULightComponentBase
{
	DECLARE_CLASS(ULightComponent, ULightComponentBase)
	REFLECT_START(ClassName)
		REFLECT_END()
public:
	ULightComponent() = default;
	virtual ~ULightComponent() override = default;

	virtual void DrawDebug(FLineBatcher* LineBatcher) const {};

	FVector GetLightDirection() const { return GetWorldRotation().GetForward(); }

	FLightSceneProxy* GetSceneProxy() const { return SceneProxy; }
private:
	friend class FScene;
	FLightSceneProxy* SceneProxy = nullptr;
};