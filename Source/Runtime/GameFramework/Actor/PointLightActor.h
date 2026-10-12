#pragma once
#include "GameFramework/Actor.h"
#include "Component/BillboardComponent.h"
#include "Component/PointLightComponent.h"

class APointLightActor : public AActor
{
	DECLARE_CLASS(APointLightActor, AActor)

	REFLECT_START(ClassName)
		REFLECT_END()

public:
	APointLightActor();
	virtual ~APointLightActor() override = default;

	UBillboardComponent* GetBillboardComponent() const { return BillboardComponent; }
	UPointLightComponent* GetPointLightComponent() const { return PointLightComponent; }

private:
	// 클릭해서 고를 수 있어야 하므로 프리미티브인 빌보드를 루트로 둔다
	UBillboardComponent* BillboardComponent = nullptr;
	UPointLightComponent* PointLightComponent = nullptr;
};

