// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MenuSystemGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MENUSYSTEM_API AMenuSystemGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;
};
