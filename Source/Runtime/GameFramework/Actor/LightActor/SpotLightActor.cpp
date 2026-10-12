#include "EnginePCH.h"
#include "SpotLightActor.h"
#include "Asset/AssetManager.h"
#include "Render/Texture2D.h"

ASpotLightActor::ASpotLightActor()
{
	if (UTexture2D* SpotIcon = UAssetManager::Get().LoadTexture("Assets/Icons/S_LightSpot.png"))
	{
		BillboardComponent->SetSprite(SpotIcon);
	}

	SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>("USpotLightComponent");
	SpotLightComponent->SetupAttachment(GetRootComponent());
}
