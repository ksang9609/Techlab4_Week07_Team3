#pragma once
#include "GameFramework/Actor.h"
#include "Component/BillboardComponent.h"
#include "Component/DirectionalLightComponent.h"

class ADirectionalLightActor : public AActor
{
	DECLARE_CLASS(ADirectionalLightActor, AActor)

	REFLECT_START(ClassName)
		REFLECT_END()

public:
	ADirectionalLightActor();
	virtual ~ADirectionalLightActor() override = default;

	UBillboardComponent* GetBillboardComponent() const { return BillboardComponent; }
	UDirectionalLightComponent* GetSpotLightComponent() const { return DirectionalLightComponent; }

private:
	// 클릭해서 고를 수 있어야 하므로 프리미티브인 빌보드를 루트로 둔다
	UBillboardComponent* BillboardComponent = nullptr;
	UDirectionalLightComponent* DirectionalLightComponent = nullptr;
};

