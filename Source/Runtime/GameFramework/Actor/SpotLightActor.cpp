#include "EnginePCH.h"
#include "SpotLightActor.h"

#include "Asset/AssetManager.h"

ASpotLightActor::ASpotLightActor()
{
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>("UBillboardComponent");
	BillboardComponent->SetupAttachment(GetRootComponent());
	UMaterial* IconMaterial = UMaterial::CreateInstance(UAssetManager::GetAssetByPath<UMaterial>("BillboardMaterial"));
	UTexture2D* IconTexture	= UAssetManager::GetAssetByPath<UTexture2D>("Icons/S_LightSpot.png");
	IconMaterial->Textures[0] = IconTexture;
	BillboardComponent->SetMaterial(0, IconMaterial);

	// 아이콘용 메시/머티리얼. Plane은 FVertex 포맷이라 DefaultShader로 그대로 그려진다.
	// 라이트 아이콘 텍스처가 준비되면 Property 창에서 머티리얼 슬롯에 끼우면 된다.

	SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>("USpotLightComponent");
	SpotLightComponent->SetupAttachment(GetRootComponent());
	
	RestoreLightIconLink();
}

void ASpotLightActor::RestoreLightIconLink()
{
	if (SpotLightComponent && BillboardComponent)
	{
		SpotLightComponent->SetSpriteComponent(BillboardComponent);
	}
}

void ASpotLightActor::Serialize(FStructuredArchive::FRecord Record)
{
	Super::Serialize(Record);

	FArchive& Ar = Record.GetUnderlyingArchive();
	if (Ar.IsLoading() &&
		!Ar.HasAnyPortFlags(EPropertyPortFlags::PPF_DuplicateForPIE))
	{
		RestoreLightIconLink();
	}
}

void ASpotLightActor::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	RestoreLightIconLink();
}