#include "EnginePCH.h"
#include "LightComponent.h"
#include "Engine/World.h"

void ULightComponent::InitializeComponent()
{
    Super::InitializeComponent();

    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            World->GetScene().RegisterLight(this);
        }
    }
}

ULightComponent::~ULightComponent()
{
    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            World->GetScene().UnregisterLight(this);
        }
    }
}