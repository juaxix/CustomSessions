// Custom Sessions plugin by juaxix - 2022-2023 - MIT License


#include "MenuWidget.h"
#include "CustomSessionSubsystem.h"
#include "OnlineSessionSettings.h"
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

void UMenuWidget::EnableDisableInputs(bool bEnable)
{
	if (IsValid(Button_Host))
	{
		Button_Host->SetIsEnabled(bEnable);
	}

	if (IsValid(Button_Join))
	{
		Button_Join->SetIsEnabled(bEnable);
	}

	if (IsValid(EditableTextBox_MatchType))
	{
		EditableTextBox_MatchType->SetIsEnabled(bEnable);
	}

	if (IsValid(SpinBox_NumConnections))
	{
		SpinBox_NumConnections->SetIsEnabled(bEnable);
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

		EnableDisableInputs(false);
	}
}

void UMenuWidget::OnHostCreated_Implementation(bool bWasSuccessful)
{
	EnableDisableInputs(!bWasSuccessful);

	if (IsValid(CustomSessionSubsystem))
	{
		CustomSessionSubsystem->OnCustomSessionCreateSessionCompleted.RemoveAll(this);
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	World->ServerTravel(LobbyMap + "?listen");
}

void UMenuWidget::ButtonJoinClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString(TEXT("Finding and Joining game...")));
	}

	if (IsValid(CustomSessionSubsystem) && EditableTextBox_MatchType)
	{
		CustomSessionSubsystem->OnCustomSessionFindSessionsCompleted.AddUObject(this, &ThisClass::OnFindSessionCompleted);
		CustomSessionSubsystem->FindSession(MaxSearchResults, NAME_GameSession, EditableTextBox_MatchType->GetText().ToString());
		EnableDisableInputs(false);
	}
}

void UMenuWidget::OnHostJoined_Implementation(bool bWasSuccessful, const FString& Address)
{
	EnableDisableInputs(!bWasSuccessful);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Emerald,
			FString::Printf(TEXT("Joined, travel address: %s..."), 
			*Address));
	}

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!IsValid(PlayerController))
	{
		return;
	}

	PlayerController->ClientTravel(Address, TRAVEL_Absolute);
}

void UMenuWidget::OnFindSessionCompleted(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (!IsValid(CustomSessionSubsystem))
	{
		return;
	}

	CustomSessionSubsystem->OnCustomSessionFindSessionsCompleted.RemoveAll(this);

	const FOnlineSessionSearchResult* SessionToJoin = nullptr;
	for (const FOnlineSessionSearchResult& Result : SessionResults)
	{
		if (!Result.IsValid() || !Result.IsSessionInfoValid())
		{
			continue;
		}

		const FString IdStr = Result.GetSessionIdStr();
		const FString& User = Result.Session.OwningUserName;
		FString MatchType;
		Result.Session.SessionSettings.Get(CustomSessionsApi::MatchTypeKey, MatchType);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green,
				FString::Printf(TEXT("Session Id: %s, owner: %s, Type: %s"), 
				*IdStr, *User, *MatchType));
		}

		if (!SessionToJoin && MatchType.Equals(CustomSessionSubsystem->CurrentMatchType))
		{
			SessionToJoin = &Result;
		}
	}

	if (SessionToJoin != nullptr)
	{
		CustomSessionSubsystem->OnCustomsessionJoinSessionCompleted.AddUObject(this, &UMenuWidget::OnJoinSessionCompleted);
		CustomSessionSubsystem->JoinSession(*SessionToJoin);
	}
	else
	{
		UE_LOG(LogOnlineSession, Warning, TEXT("Could not find any session of match type: %s"), *CustomSessionSubsystem->CurrentMatchType);
	}
}

void UMenuWidget::OnJoinSessionCompleted(EOnJoinSessionCompleteResult::Type JoinResult)
{
	if (!IsValid(CustomSessionSubsystem) || !CustomSessionSubsystem->OnlineSession.IsValid())
	{
		UE_LOG(LogOnlineSession, Error, TEXT("Could not get the Online session"));
		OnHostJoined(false, "");
		return;
	}

	CustomSessionSubsystem->OnCustomsessionJoinSessionCompleted.RemoveAll(this);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Emerald,
			FString::Printf(TEXT("Join Session [%s] completed with Result: %d"),
			*CustomSessionSubsystem->CurrentGameSession.ToString(), static_cast<uint8>(JoinResult)));
	}
	
	switch (JoinResult)
	{
		case EOnJoinSessionCompleteResult::Success:
		{
			FString Address;
			if (!CustomSessionSubsystem->OnlineSession->GetResolvedConnectString(CustomSessionSubsystem->CurrentGameSession, Address))
			{
				UE_LOG(LogOnlineSession, Error, TEXT("Could not get the address to travel to"));
				OnHostJoined(false, "");

				return;
			}

			

			OnHostJoined(true, Address);
			
		}
		break;

		default: OnHostJoined(false, "");
		break;
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
