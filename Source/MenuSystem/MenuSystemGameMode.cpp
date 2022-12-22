// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

#include "MenuSystemGameMode.h"
#include "MenuSystemCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMenuSystemGameMode::AMenuSystemGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
