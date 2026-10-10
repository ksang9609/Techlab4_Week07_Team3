#pragma once

#include "GameFramework/Actor.h"

class UBillboardComponent;
class UPointLightComponent;

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

	virtual void Serialize(FStructuredArchive::FRecord Record) override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

private:
	UBillboardComponent* BillboardComponent = nullptr;
	UPointLightComponent* PointLightComponent = nullptr;

	void RestoreLightIconLink();
};