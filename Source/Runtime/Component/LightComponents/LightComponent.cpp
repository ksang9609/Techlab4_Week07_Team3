#include "EnginePCH.h"

#include "LightComponent.h"

#include "Engine/World.h"

ULightComponent::ULightComponent()
	: Intensity(0.2f)
	, Color(FVector(1.0f, 1.0f, 1.0f))
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