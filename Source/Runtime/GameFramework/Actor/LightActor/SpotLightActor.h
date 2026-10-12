#pragma once

#include "LightActor.h"
#include "Component/SpotLightComponent.h"

// 에디터에서 아이콘(빌보드)으로 보이고, 선택하면 스포트라이트 원뿔이 라인으로 그려진다.
class ASpotLightActor : public ALightActor
{
	DECLARE_CLASS(ASpotLightActor, ALightActor)

	REFLECT_START(ClassName)
	REFLECT_END()

public:
	ASpotLightActor();
	virtual ~ASpotLightActor() override = default;

	USpotLightComponent* GetSpotLightComponent() const { return SpotLightComponent; }

private:
	// 클릭해서 고를 수 있어야 하므로 프리미티브인 빌보드를 루트로 둔다
	USpotLightComponent* SpotLightComponent = nullptr;
};
