#include "EnginePCH.h"
#include "AmbientLightActor.h"

#include "Asset/AssetManager.h"
#include "Component/BillboardComponent.h"
#include "Component/LightComponents/AmbientLightComponent.h"

AAmbientLightActor::AAmbientLightActor()
{
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>("UBillboardComponent");
	BillboardComponent->SetupAttachment(GetRootComponent());
	UMaterial* IconMaterial = UMaterial::CreateInstance(UAssetManager::GetAssetByPath<UMaterial>("BillboardMaterial"));
	UTexture2D* IconTexture = UAssetManager::GetAssetByPath<UTexture2D>("Icons/SkyLight.png");
	IconMaterial->Textures[0] = IconTexture;
	BillboardComponent->SetMaterial(0, IconMaterial);

	// 아이콘용 메시/머티리얼. Plane은 FVertex 포맷이라 DefaultShader로 그대로 그려진다.
	// 라이트 아이콘 텍스처가 준비되면 Property 창에서 머티리얼 슬롯에 끼우면 된다.
	
	AmbientLightComponent = CreateDefaultSubobject<UAmbientLightComponent>("UAmbientLightComponent");
	AmbientLightComponent->SetupAttachment(GetRootComponent());

	RestoreLightIconLink();
}

void AAmbientLightActor::RestoreLightIconLink()
{
	if (AmbientLightComponent && BillboardComponent)
	{
		AmbientLightComponent->SetSpriteComponent(BillboardComponent);
	}
}

void AAmbientLightActor::Serialize(FStructuredArchive::FRecord Record)
{
	Super::Serialize(Record);

	FArchive& Ar = Record.GetUnderlyingArchive();
	if (Ar.IsLoading() &&
		!Ar.HasAnyPortFlags(EPropertyPortFlags::PPF_DuplicateForPIE))
	{
		RestoreLightIconLink();
	}
}

void AAmbientLightActor::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	RestoreLightIconLink();
}