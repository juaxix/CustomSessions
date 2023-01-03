// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

#include "CustomSessionSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

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
		OnCustomSessionCreateSessionCompleted.Broadcast(false);

		return;
	}

	if (CreateSessionCompleteDelegate_Handle.IsValid())
	{
		OnCustomSessionCreateSessionCompleted.Broadcast(false);

		return;
	}

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		OnCustomSessionCreateSessionCompleted.Broadcast(false);

		return;
	}

	const ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController();
	if (!IsValid(LocalPlayer))
	{
		OnCustomSessionCreateSessionCompleted.Broadcast(false);

		return;
	}

	if (OnlineSession->GetNamedSession(SessionName))
	{
		OnlineSession->DestroySession(SessionName);
	}

	SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName().IsEqual(NULL_SUBSYSTEM);
	SessionSettings->NumPublicConnections = NumPublicConnections;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bUsesPresence = true; // use world regions!
	SessionSettings->Set(CustomSessionsApi::MatchTypeKey, MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	CreateSessionCompleteDelegate_Handle = OnlineSession->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
	if (!OnlineSession->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), SessionName, SessionSettings.ToSharedRef().Get()))
	{
		OnlineSession->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate_Handle);
		OnCustomSessionCreateSessionCompleted.Broadcast(false);
	}
}

bool UCustomSessionSubsystem::FindSession(int32 MaxSearchResults, FName SessionName, const FString& MatchType)
{
	if (!OnlineSession.IsValid())
	{
		UE_LOG(LogOnlineSession, Error, TEXT("Can't create a session without a valid OSS"));
		FindSessionCompleted(false);

		return false;
	}

	if (FindSessionsCompleteDelegate_Handle.IsValid())
	{
		FindSessionCompleted(false);

		return false;
	}

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		FindSessionCompleted(false);

		return false;
	}

	const ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController();
	if (!IsValid(LocalPlayer))
	{
		FindSessionCompleted(false);

		return false;
	}

	CurrentGameSession = SessionName;
	CurrentMatchType = MatchType;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = MaxSearchResults;
	SessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName().IsEqual(NULL_SUBSYSTEM);
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	FindSessionsCompleteDelegate_Handle = OnlineSession->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
	if (!OnlineSession->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		FindSessionCompleted(false);

		return false;
	}

	return true;
}

void UCustomSessionSubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	if (!OnlineSession.IsValid())
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

	const FString IdStr = SearchResult.GetSessionIdStr();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Emerald,
			FString::Printf(TEXT("Joining Session Id: %s..."), 
			*IdStr));
	}

	JoinSessionCompleteDelegate_Handle = OnlineSession->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
	OnlineSession->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), CurrentGameSession, SearchResult);
}

void UCustomSessionSubsystem::StartSession()
{
}

void UCustomSessionSubsystem::DestroySession()
{
}

void UCustomSessionSubsystem::CreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSession)
	{
		OnlineSession->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate_Handle);
	}

	CreateSessionCompleteDelegate_Handle.Reset();

	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("Created session with name: %s"), *SessionName.ToString()));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString(TEXT("Failed to create session")));
		}
	}

	CurrentGameSession = SessionName;
	OnCustomSessionCreateSessionCompleted.Broadcast(bWasSuccessful);
}

void UCustomSessionSubsystem::FindSessionCompleted(bool bWasSuccessful)
{
	if (OnlineSession)
	{
		OnlineSession->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate_Handle);
	}

	FindSessionsCompleteDelegate_Handle.Reset();

	if (!SessionSearch.IsValid())
	{
		OnCustomSessionFindSessionsCompleted.Broadcast(TArray<FOnlineSessionSearchResult>(), false);

		return;
	}

	if (JoinSessionCompleteDelegate_Handle.IsValid())
	{
		OnCustomSessionFindSessionsCompleted.Broadcast(TArray<FOnlineSessionSearchResult>(), false);

		return;
	}

	if (!bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red,
				FString("Error finding sessions"));
		}

		OnCustomSessionFindSessionsCompleted.Broadcast(TArray<FOnlineSessionSearchResult>(), false);

		return;
	}

	if (SessionSearch->SearchResults.IsEmpty())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange,
				FString("No sessions found"));
		}

		OnCustomSessionFindSessionsCompleted.Broadcast(TArray<FOnlineSessionSearchResult>(), false);

		return;
	}

	OnCustomSessionFindSessionsCompleted.Broadcast(SessionSearch->SearchResults, true);
}

void UCustomSessionSubsystem::JoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
	if (!OnlineSession)
	{
		UE_LOG(LogOnlineSession, Error, TEXT("AMenuSystemCharacter::OnJoinSessionComplete : OnlineSession is not valid"));
		OnCustomsessionJoinSessionCompleted.Broadcast(EOnJoinSessionCompleteResult::UnknownError);

		return;
	}

	OnlineSession->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate_Handle);
	JoinSessionCompleteDelegate_Handle.Reset();
	OnCustomsessionJoinSessionCompleted.Broadcast(JoinResult);
}

void UCustomSessionSubsystem::StartSessionCompleted(FName SessionName, bool bWasSuccessful)
{
}

void UCustomSessionSubsystem::DestroySessionCompleted(FName SessionName, bool bWasSuccessful)
{
}
