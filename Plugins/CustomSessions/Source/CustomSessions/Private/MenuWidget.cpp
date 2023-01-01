// Custom Sessions plugin by juaxix - 2022-2023 - MIT License


#include "MenuWidget.h"
#include "CustomSessionSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/SpinBox.h"

void UMenuWidget::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	const UWorld* World = GetWorld();
	if (IsValid(World))
	{
		CustomSessionSubsystem = World->GetGameInstance()->GetSubsystem<UCustomSessionSubsystem>();

		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (IsValid(PlayerController))
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}

void UMenuWidget::MenuTearDown()
{
	RemoveFromParent();
	const UWorld* World = GetWorld();
	APlayerController* PlayerController = IsValid(World) ? World->GetFirstPlayerController() : nullptr;
	if (IsValid(PlayerController))
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
}

void UMenuWidget::ButtonHostClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString(TEXT("Hosting Game...")));
	}

	if (IsValid(CustomSessionSubsystem) && SpinBox_NumConnections && EditableTextBox_MatchType)
	{
		CustomSessionSubsystem->OnCustomSessionCreateSessionCompleted.AddUniqueDynamic(this, &ThisClass::OnHostCreated);
		CustomSessionSubsystem->CreateSession(NAME_GameSession,
			static_cast<uint8>(SpinBox_NumConnections->Value), 
			EditableTextBox_MatchType->GetText().ToString());
	}
}

void UMenuWidget::OnHostCreated_Implementation(bool bWasSuccessful)
{
	if (CustomSessionSubsystem)
	{
		CustomSessionSubsystem->OnCustomSessionCreateSessionCompleted.RemoveAll(this);
	}
}

void UMenuWidget::ButtonJoinClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString(TEXT("Finding and Joining game...")));
	}

	if (IsValid(CustomSessionSubsystem) && EditableTextBox_MatchType)
	{
		CustomSessionSubsystem->FindSession(MaxSearchResults, NAME_GameSession, EditableTextBox_MatchType->GetText().ToString());
	}
}

bool UMenuWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (Button_Host)
	{
		Button_Host->OnClicked.AddUniqueDynamic(this, &ThisClass::ButtonHostClicked);
	}

	if (Button_Join)
	{
		Button_Join->OnClicked.AddUniqueDynamic(this, &ThisClass::ButtonJoinClicked);
	}

	return true;
}
