#include "EnginePCH.h"
#include "PointLightActor.h"
#include "Asset/AssetManager.h"
#include "Render/Texture2D.h"

APointLightActor::APointLightActor()
{
	if (UTexture2D* PointIcon = UAssetManager::Get().LoadTexture("Assets/Icons/S_LightPoint.png"))
	{
		BillboardComponent->SetSprite(PointIcon);
	}

	PointLightComponent = CreateDefaultSubobject<UPointLightComponent>("UPointLightComponent");
	PointLightComponent->SetupAttachment(GetRootComponent());
}