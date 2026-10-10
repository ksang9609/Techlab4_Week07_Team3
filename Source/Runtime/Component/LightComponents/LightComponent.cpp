#include "EnginePCH.h"

#include "LightComponent.h"

#include "Engine/World.h"

ULightComponent::ULightComponent()
	: Super()
	, Intensity(1.0f)
	, LightColor(FVector(1.0f, 1.0f, 1.0f))
	, bVisible(true)
{
}

ULightComponent::~ULightComponent()
{
	if (AActor* Owner = GetOwner())
	{
		if (UWorld* WorldToScreen = Owner->GetWorld())
		{
			WorldToScreen->GetScene().UnregisterLightComponent(*this);
		}
	}
}
void ULightComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (AActor* Owner = GetOwner())
	{
		if (UWorld* WorldToScreen = Owner->GetWorld())
		{
			WorldToScreen->GetScene().RegisterLightComponent(*this);
		}
	}
}

void ULightComponent::PostEditChangeProperty(const FProperty& Property)
{
	Super::PostEditChangeProperty(Property);
	if (Property.Name == "LightColor" || Property.Name == "Intensity")
	{
		UpdateEditorIconColor();
	}
}

void ULightComponent::UpdateEditorIconColor()
{
	if (SpriteComponent)
	{
		FVector Color = LightColor * Intensity;
		SpriteComponent->GetMaterial(0)->BaseColor = Color;
	}
}