// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CustomSessionSubsystem.generated.h"

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

	void CreateSession(FName SessionName, int32 NumPublicConnections, const FString& MatchType);

	void FindSession(int32 MaxSearchResults);

	void JoinSession(const FOnlineSessionSearchResult& SearchResult);

	void StartSession();
	
	void DestroySession();

private:
	void CreateSessionCompleted(FName SessionName, bool bWasSuccessful);
	void FindSessionCompleted(bool bWasSuccessful);
	void JoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);
	void StartSessionCompleted(FName SessionName, bool bWasSuccessful);
	void DestroySessionCompleted(FName SessionName, bool bWasSuccessful);
	
	IOnlineSessionPtr OnlineSession = nullptr;
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	FDelegateHandle CreateSessionCompleteDelegate_Handle,
					FindSessionsCompleteDelegate_Handle,
					JoinSessionCompleteDelegate_Handle,
					StartSessionCompleteDelegate_Handle,
					DestroySessionCompleteDelegate_Handle;
};
