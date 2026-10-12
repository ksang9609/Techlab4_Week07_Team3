#pragma once
#include "GameFramework/Actor.h"
#include "Component/BillboardComponent.h"

class ALightActor : public AActor
{
	DECLARE_CLASS(ALightActor, AActor)

	REFLECT_START(ClassName)
	REFLECT_END()
public:
	ALightActor();
	~ALightActor() override = default;

	UBillboardComponent* GetBillboardComponent() const { return BillboardComponent; }
	void SetBillboardComponent(UBillboardComponent* InBillboardComponent) { BillboardComponent = InBillboardComponent; }
	void SetBillboardVisibility(bool bVisible) { bIsBillboardVisible = bVisible; }
	bool GetBillboardVisibility() const { return bIsBillboardVisible; }
	void SetLightVisibility(bool bVisible) { bIsLightVisible = bVisible; }
	bool GetLightVisibility() const { return bIsLightVisible; }

protected:
	UBillboardComponent* BillboardComponent = nullptr;
	// Todo: 헬퍼 함수를 이용해 헬퍼함수로 생성자에서 생성하도록 변경
	bool bIsBillboardVisible = true;
	bool bIsLightVisible = true;
};