// Custom Sessions plugin by juaxix - 2022-2023 - MIT License


#include "MenuSystemGameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void AMenuSystemGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (GameState)
	{
		const int32 NumPlayers = GameState->PlayerArray.Num();
		const APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 60.0f, FColor::Purple, 
				FString::Printf(TEXT("Players in game: %d"), NumPlayers));

			if (IsValid(PlayerState))
			{
				GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Cyan, 
					FString::Printf(TEXT("Player %s joined game"), 
						*PlayerState->GetPlayerName()));
			}
		}
	}
}

void AMenuSystemGameModeBase::Logout(AController* ExitingPlayer)
{
	if (GameState)
	{
		const int32 NumPlayers = GameState->PlayerArray.Num();
		const APlayerState* PlayerState = ExitingPlayer->GetPlayerState<APlayerState>();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 60.0f, FColor::Purple, 
				FString::Printf(TEXT("Players in game: %d"), NumPlayers));

			if (IsValid(PlayerState))
			{
				GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Cyan, 
					FString::Printf(TEXT("Player %s exited the game"), 
						*PlayerState->GetPlayerName()));
			}
		}
	}

	Super::Logout(ExitingPlayer);

}
