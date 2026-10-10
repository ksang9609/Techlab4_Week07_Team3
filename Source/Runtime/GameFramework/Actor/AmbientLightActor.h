#pragma once

#include "GameFramework/Actor.h"

class UBillboardComponent;
class UAmbientLightComponent;

class AAmbientLightActor : public AActor
{
	DECLARE_CLASS(AAmbientLightActor, AActor)
	REFLECT_START(ClassName)
		REFLECT_END()

public:
	AAmbientLightActor();
	virtual ~AAmbientLightActor() override = default;

	UBillboardComponent* GetBillboardComponent() const { return BillboardComponent; }
	UAmbientLightComponent* GetAmbientLightComponent() const { return AmbientLightComponent; }

	virtual void Serialize(FStructuredArchive::FRecord Record) override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

private:
	UBillboardComponent* BillboardComponent = nullptr;
	UAmbientLightComponent* AmbientLightComponent = nullptr;

	void RestoreLightIconLink();
};