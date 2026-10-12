#pragma once
#include "Render/Renderer.h"

class ULightComponent;
class FScene;

enum class ELightType : uint8 { Ambient, Directional, Point, Spot };

class FLightSceneProxy
{
public:
	explicit FLightSceneProxy(ULightComponent* InComponent);
	virtual ~FLightSceneProxy() = default;

	// 각 라이트 프록시가 자기 타입에 맞게 Constants 구조체에 데이터를 채워 넣는다
	virtual void GatherLightConstants(FForwardLightConstants& InOutConstants) const;

	ULightComponent* GetComponent() const { return Component; }
	ELightType GetLightType() const { return LightType; }
private:
	friend class FScene;
	FScene* Scene = nullptr;
	ULightComponent* Component = nullptr;
	int32 PackedIndex = INDEX_NONE;
	ELightType LightType = ELightType::Ambient;
};