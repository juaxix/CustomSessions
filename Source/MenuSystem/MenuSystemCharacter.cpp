// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

#include "MenuSystemCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include <Interfaces/OnlineSessionInterface.h>
#include <OnlineSessionSettings.h>
#include <OnlineSubsystem.h>

//////////////////////////////////////////////////////////////////////////
// AMenuSystemCharacter

namespace
{
	const FName MatchGameSessionName(NAME_GameSession);
	const FName MatchTypeName("MatchType");
	const FString MatchTypeFree4a("FreeForAll");
}

AMenuSystemCharacter::AMenuSystemCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AMenuSystemCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Online subsystem addition
	const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		OnlineSession = OnlineSubsystem->GetSessionInterface();
		if (!OnlineSession.IsValid())
		{
			return;
		}

		if (!CreateSessionCompleteDelegate.IsBoundToObject(this))
		{
			CreateSessionCompleteDelegate.BindUObject(this, &ThisClass::OnCreateSessionComplete);
		}

		if (!FindSessionsCompleteDelegate.IsBoundToObject(this))
		{
			FindSessionsCompleteDelegate.BindUObject(this, &ThisClass::OnFindSessionsComplete);
		}

		if (!JoinSessionCompleteDelegate.IsBoundToObject(this))
		{
			JoinSessionCompleteDelegate.BindUObject(this, &ThisClass::OnJoinSessionComplete);
		}
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, 
				FString::Printf(TEXT("Found subsystem: %s"), *OnlineSubsystem->GetSubsystemName().ToString()));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMenuSystemCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AMenuSystemCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AMenuSystemCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AMenuSystemCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AMenuSystemCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMenuSystemCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMenuSystemCharacter::TouchStopped);
}

void AMenuSystemCharacter::CreateGameSession()
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

	if (OnlineSession->GetNamedSession(MatchGameSessionName))
	{
		OnlineSession->DestroySession(MatchGameSessionName);
	}

	const TSharedRef<FOnlineSessionSettings> NewSessionSettings = MakeShared<FOnlineSessionSettings>();
	NewSessionSettings->bIsLANMatch = false;
	NewSessionSettings->NumPublicConnections = 4;
	NewSessionSettings->bAllowJoinInProgress = true;
	NewSessionSettings->bShouldAdvertise = true;
	NewSessionSettings->bUseLobbiesIfAvailable = true;
	NewSessionSettings->Set(MatchTypeName, MatchTypeFree4a, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	NewSessionSettings->bAllowJoinViaPresence = NewSessionSettings->bUsesPresence = true; // use world regions!
	CreateSessionCompleteDelegate_Handle = OnlineSession->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	OnlineSession->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), MatchGameSessionName, *NewSessionSettings);
}

void AMenuSystemCharacter::JoinGameSession()
{
	if (!OnlineSession.IsValid())
	{
		UE_LOG(LogOnlineSession, Error, TEXT("Can't create a session without a valid OSS"));
		return;
	}

	if (FindSessionsCompleteDelegate_Handle.IsValid())
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

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	FindSessionsCompleteDelegate_Handle = OnlineSession->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	OnlineSession->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}

void AMenuSystemCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSession)
	{
		OnlineSession->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate_Handle);
	}

	CreateSessionCompleteDelegate_Handle.Reset();

	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (!IsValid(World))
		{
			return;
		}

		World->ServerTravel(FString(TEXT("/Game/ThirdPerson/Maps/ThirdPersonMap")));
		
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
}

void AMenuSystemCharacter::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (OnlineSession)
	{
		OnlineSession->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate_Handle);
	}

	FindSessionsCompleteDelegate_Handle.Reset();

	if (!SessionSearch.IsValid())
	{
		return;
	}

	if (JoinSessionCompleteDelegate_Handle.IsValid())
	{
		return;
	}

	if (!bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red,
				FString("Error finding sessions"));
		}

		return;
	}

	if (SessionSearch->SearchResults.IsEmpty())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange,
				FString("No sessions found"));
		}

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

	const FOnlineSessionSearchResult* SessionToJoin = nullptr;
	for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
	{
		if (!Result.IsValid() || !Result.IsSessionInfoValid())
		{
			continue;
		}

		const FString IdStr = Result.GetSessionIdStr();
		const FString& User = Result.Session.OwningUserName;
		FString MatchType;
		Result.Session.SessionSettings.Get(MatchTypeName, MatchType);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green,
				FString::Printf(TEXT("Session Id: %s, owner: %s, Type: %s"), 
				*IdStr, *User, *MatchType));
		}

		if (!SessionToJoin && MatchType.Equals(MatchTypeFree4a))
		{
			SessionToJoin = &Result;
		}
	}

	if (SessionToJoin != nullptr)
	{
		const FString IdStr = SessionToJoin->GetSessionIdStr();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Emerald,
				FString::Printf(TEXT("Joining Session Id: %s..."), 
				*IdStr));
		}

		JoinSessionCompleteDelegate_Handle = OnlineSession->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
		OnlineSession->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), MatchGameSessionName, *SessionToJoin);
	}
}

void AMenuSystemCharacter::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSession)
	{
		UE_LOG(LogOnlineSession, Error, TEXT("AMenuSystemCharacter::OnJoinSessionComplete : OnlineSession is not valid"));
		return;
	}
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Emerald,
			FString::Printf(TEXT("Join Session [%s] completed with Result: %d"),
			*SessionName.ToString(), static_cast<uint8>(Result)));
	}
	
	switch (Result)
	{
		case EOnJoinSessionCompleteResult::Success:
			{
				FString Address;
				if (!OnlineSession->GetResolvedConnectString(MatchGameSessionName, Address))
				{
					UE_LOG(LogOnlineSession, Error, TEXT("Could not get the address to travel to"));
					return;
				}
				
				APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
				if (!IsValid(PlayerController))
				{
					UE_LOG(LogOnlineSession, Error, TEXT("Could not find a valid PC to travel with"));
					return;
				}
				
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Emerald,
						FString::Printf(TEXT("Joined, traveling to: %s..."), 
						*Address));
				}
				
				PlayerController->ClientTravel(Address, TRAVEL_Absolute);
			}
			break;

		default: break;
	}

	OnlineSession->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate_Handle);
	JoinSessionCompleteDelegate_Handle.Reset();
}

void AMenuSystemCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AMenuSystemCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AMenuSystemCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMenuSystemCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMenuSystemCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMenuSystemCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
