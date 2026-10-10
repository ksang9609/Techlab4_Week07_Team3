#pragma once

#include "GameFramework/Actor.h"

class UBillboardComponent;
class UDirectionalLightComponent;

class ADirectionalLightActor : public AActor
{
	DECLARE_CLASS(ADirectionalLightActor, AActor)
	REFLECT_START(ClassName)
		REFLECT_END()

public:
	ADirectionalLightActor();
	virtual ~ADirectionalLightActor() override = default;

	UBillboardComponent* GetBillboardComponent() const { return BillboardComponent; }
	UDirectionalLightComponent* GetDirectionalLightComponent() const { return DirectionalLightComponent; }

	virtual void Serialize(FStructuredArchive::FRecord Record) override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

private:
	UBillboardComponent* BillboardComponent = nullptr;
	UDirectionalLightComponent* DirectionalLightComponent = nullptr;

	void RestoreLightIconLink();
};