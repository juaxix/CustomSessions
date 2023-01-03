// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CustomSessionSubsystem.generated.h"

namespace CustomSessionsApi
{
	const FName MatchTypeKey("MatchType");
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomSessionCreateSessionCompleted, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCustomSessionFindSessionsCompleted, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FCustomsessionJoinSessionCompleted, EOnJoinSessionCompleteResult::Type SessionResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomSessionStartSessionCompleted, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomSessionDestroySessionCompleted, bool, bWasSuccessful);

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
	
	UFUNCTION(BlueprintCallable, Category = "Custom Sessions")
	void CreateSession(FName SessionName, int32 NumPublicConnections, const FString& MatchType = TEXT("FreeForAll"));

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Custom Sessions", meta = (AdvancedDisplay = true))
	bool FindSession(int32 MaxSearchResults = 1000, 
					 FName SessionName = TEXT("GameSession"), 
					 const FString& MatchType = TEXT("FreeForAll"));


	void JoinSession(const FOnlineSessionSearchResult& SearchResult);

	void StartSession();
	
	void DestroySession();

	TSharedRef<FOnlineSessionSettings> GetSessionSettings() const { return SessionSettings.ToSharedRef(); }
	
	FCustomSessionCreateSessionCompleted OnCustomSessionCreateSessionCompleted;
	FCustomSessionFindSessionsCompleted OnCustomSessionFindSessionsCompleted;
	FCustomsessionJoinSessionCompleted OnCustomsessionJoinSessionCompleted;
	FCustomSessionStartSessionCompleted OnCustomSessionStartSessionCompleted;
	FCustomSessionDestroySessionCompleted OnCustomSessionDestroySessionCompleted;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Session")
	FName CurrentGameSession = NAME_None;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Session")
	FString CurrentMatchType = "";

	IOnlineSessionPtr OnlineSession = nullptr;

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
	
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
};
