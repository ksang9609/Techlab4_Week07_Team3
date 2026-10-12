#include "EnginePCH.h"
#include "LightActor.h"

#include "Component/AmbientLightComponent.h"
#include "Component/DirectionalLightComponent.h"

#include "Asset/AssetManager.h"

ALightActor::ALightActor()
{
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>("UBillboardComponent");
	BillboardComponent->SetupAttachment(GetRootComponent());

	PointLightComponent = CreateDefaultSubobject<UPointLightComponent>("UPointLightComponent");
	PointLightComponent->SetupAttachment(GetRootComponent());

	if (const char* IconPath = PointLightComponent->GetIconPath())
		BillboardComponent->SetSpriteTexture(UAssetManager::Get().LoadTexture(IconPath));
}
