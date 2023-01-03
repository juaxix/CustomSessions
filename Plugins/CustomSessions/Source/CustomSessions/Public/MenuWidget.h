// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuWidget.generated.h"

class UButton;
class UEditableTextBox;
class USpinBox;

UCLASS()
class CUSTOMSESSIONS_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Custom Sessions|UI|Menu")
	void MenuSetup();

	UFUNCTION(BlueprintCallable, Category = "Custom Sessions|UI|Menu")
	void MenuTearDown();

	UFUNCTION(BlueprintCallable, Category = "Custom Sessions|UI|Menu")
	void EnableDisableInputs(bool bEnable);

protected:
	UFUNCTION()
	virtual void ButtonHostClicked();

	UFUNCTION(BlueprintNativeEvent, Category = "Custom Sessions", meta = (AllowPrivateAccess = true, Tooltip = "When a host is created, the BP code should contain a server travel")) 
	void OnHostCreated(bool bWasSuccessful);

	UFUNCTION()
	virtual void ButtonJoinClicked();

	UFUNCTION(BlueprintNativeEvent, Category = "Custom Sessions", meta = (AllowPrivateAccess = true, Tooltip = "When a host is created, the BP code should contain a client travel to the address")) 
	void OnHostJoined(bool bWasSuccessful, const FString& Address);

	virtual void OnFindSessionCompleted(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);

	virtual void OnJoinSessionCompleted(EOnJoinSessionCompleteResult::Type SessionResult);

	virtual bool Initialize() override;

	virtual void NativeDestruct() override
	{
		MenuTearDown();
		Super::NativeDestruct();
	}

public:
	UPROPERTY(EditAnywhere, Category = "Search sessions")
	int32 MaxSearchResults = 32;

	UPROPERTY(EditAnywhere, Category = "Sessions")
	FString LobbyMap{TEXT("")};

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Host = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Join = nullptr;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* EditableTextBox_MatchType = nullptr;

	UPROPERTY(meta = (BindWidget))
	USpinBox* SpinBox_NumConnections = nullptr;

	UPROPERTY(Transient)
	class UCustomSessionSubsystem* CustomSessionSubsystem = nullptr;
};
