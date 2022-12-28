// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

#include "CustomSessionSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

namespace
{
	const FName MatchTypeKey("MatchType");
}

void UCustomSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		OnlineSession = OnlineSubsystem->GetSessionInterface();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, 
				FString::Printf(TEXT("Found subsystem: %s"), *OnlineSubsystem->GetSubsystemName().ToString()));
		}
		OnCreateSessionCompleteDelegate.BindUObject(this, &ThisClass::CreateSessionCompleted);
		OnFindSessionsCompleteDelegate.BindUObject(this, &ThisClass::FindSessionCompleted);
		OnJoinSessionCompleteDelegate.BindUObject(this, &ThisClass::JoinSessionCompleted);
		OnStartSessionCompleteDelegate.BindUObject(this, &ThisClass::StartSessionCompleted);
		OnDestroySessionCompleteDelegate.BindUObject(this, &ThisClass::DestroySessionCompleted);
	}
}

void UCustomSessionSubsystem::CreateSession(FName SessionName, int32 NumPublicConnections, const FString& MatchType)
{
	if (!OnlineSession.IsValid())
	{
		UE_LOG(LogOnlineSession, Error, TEXT("Can't create a session without a valid OSS"));
		return;
	}
	

	if (CreateSessionCompleteDelegate_Handle.IsValid())
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController();
	if (!IsValid(LocalPlayer))
	{
		return;
	}

	if (OnlineSession->GetNamedSession(SessionName))
	{
		OnlineSession->DestroySession(SessionName);
	}

	const TSharedRef<FOnlineSessionSettings> NewSessionSettings = MakeShared<FOnlineSessionSettings>();
	NewSessionSettings->bIsLANMatch = false;
	NewSessionSettings->NumPublicConnections = 4;
	NewSessionSettings->bAllowJoinInProgress = true;
	NewSessionSettings->bShouldAdvertise = true;
	NewSessionSettings->bUseLobbiesIfAvailable = true;
	NewSessionSettings->Set(MatchTypeKey, MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	NewSessionSettings->bAllowJoinViaPresence = NewSessionSettings->bUsesPresence = true; // use world regions!
	CreateSessionCompleteDelegate_Handle = OnlineSession->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
	OnlineSession->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), SessionName, *NewSessionSettings);
}

void UCustomSessionSubsystem::FindSession(int32 MaxSearchResults)
{
	
}

void UCustomSessionSubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
}

void UCustomSessionSubsystem::StartSession()
{
}

void UCustomSessionSubsystem::DestroySession()
{
}

void UCustomSessionSubsystem::CreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
}

void UCustomSessionSubsystem::FindSessionCompleted(bool bWasSuccessful)
{
}

void UCustomSessionSubsystem::JoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
}

void UCustomSessionSubsystem::StartSessionCompleted(FName SessionName, bool bWasSuccessful)
{
}

void UCustomSessionSubsystem::DestroySessionCompleted(FName SessionName, bool bWasSuccessful)
{
}
