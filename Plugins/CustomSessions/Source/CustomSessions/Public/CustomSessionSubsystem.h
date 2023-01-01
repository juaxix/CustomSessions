// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CustomSessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomSessionCreateSessionCompleted, bool, bWasSuccessful);

UCLASS()
class CUSTOMSESSIONS_API UCustomSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override
	{
		return true;
	}

	void CreateSession(FName SessionName, int32 NumPublicConnections, const FString& MatchType = TEXT("FreeForAll"));

	bool FindSession(int32 MaxSearchResults, FName SessionName = NAME_GameSession, const FString& MatchType = TEXT("FreeForAll"));

	void JoinSession(const FOnlineSessionSearchResult& SearchResult);

	void StartSession();
	
	void DestroySession();

	TSharedRef<FOnlineSessionSettings> GetSessionSettings() const { return SessionSettings.ToSharedRef(); }
	
	FCustomSessionCreateSessionCompleted OnCustomSessionCreateSessionCompleted;

private:
	void CreateSessionCompleted(FName SessionName, bool bWasSuccessful);
	void FindSessionCompleted(bool bWasSuccessful);
	void JoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);
	void StartSessionCompleted(FName SessionName, bool bWasSuccessful);
	void DestroySessionCompleted(FName SessionName, bool bWasSuccessful);
	
	FDelegateHandle CreateSessionCompleteDelegate_Handle,
					FindSessionsCompleteDelegate_Handle,
					JoinSessionCompleteDelegate_Handle,
					StartSessionCompleteDelegate_Handle,
					DestroySessionCompleteDelegate_Handle;

	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	IOnlineSessionPtr OnlineSession = nullptr;
	
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FName CurrentGameSession = NAME_GameSession;
	FString CurrentMatchType = "";
};
