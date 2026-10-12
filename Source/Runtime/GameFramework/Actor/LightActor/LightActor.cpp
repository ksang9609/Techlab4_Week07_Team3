#include "EnginePCH.h"
#include "LightActor.h"


ALightActor::ALightActor()
{
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>("UBillboardComponent");
	BillboardComponent->SetupAttachment(GetRootComponent());
	BillboardComponent->SetRelativeScale3D({ 0.4f, 0.4f, 0.4f });
}
