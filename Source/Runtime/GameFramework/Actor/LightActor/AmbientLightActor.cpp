#include "EnginePCH.h"
#include "AmbientLightActor.h"
#include "Asset/AssetManager.h"
#include "Render/Texture2D.h"

AAmbientLightActor::AAmbientLightActor()
{
	if (UTexture2D* AmbientIcon = UAssetManager::Get().LoadTexture("Assets/Icons/SkyLight.png"))
	{
		BillboardComponent->SetSprite(AmbientIcon);
	}

	AmbientLightComponent = CreateDefaultSubobject<UAmbientLightComponent>("UAmbientLightComponent");
	AmbientLightComponent->SetupAttachment(GetRootComponent());
}