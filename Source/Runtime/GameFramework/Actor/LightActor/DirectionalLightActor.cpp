#include "EnginePCH.h"
#include "DirectionalLightActor.h"
#include "Asset/AssetManager.h"
#include "Render/Texture2D.h"

ADirectionalLightActor::ADirectionalLightActor()
{
	if (UTexture2D* DirectionalIcon = UAssetManager::Get().LoadTexture("Assets/Icons/S_LightDirectional.png"))
	{
		BillboardComponent->SetSprite(DirectionalIcon);
	}

	DirectionalLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>("UDirectionalLightComponent");
	DirectionalLightComponent->SetupAttachment(GetRootComponent());
}