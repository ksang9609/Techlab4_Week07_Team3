#include "EnginePCH.h"
#include "DirectionalLightActor.h"
#include "Asset/AssetManager.h"
#include "Render/Texture2D.h"

ADirectionalLightActor::ADirectionalLightActor()
{
	if (UTexture2D* DirectionalIcon = UAssetManager::Get().LoadTexture("Editor/Icon/PointLight_64x.png"))
	{
		BillboardComponent->SetSprite(DirectionalIcon);
	}

	DirectionalLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>("UDirectionalLightComponent");
	DirectionalLightComponent->SetupAttachment(GetRootComponent());
}