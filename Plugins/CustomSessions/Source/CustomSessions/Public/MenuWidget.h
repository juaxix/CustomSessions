// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

protected:
	UFUNCTION()
	virtual void ButtonHostClicked();

	UFUNCTION(BlueprintNativeEvent, Category = "Custom Sessions", meta = (AllowPrivateAccess = true)) 
	void OnHostCreated(bool bWasSuccessful);

	UFUNCTION()
	virtual void ButtonJoinClicked();

	virtual bool Initialize() override;

	virtual void NativeDestruct() override
	{
		MenuTearDown();
		Super::NativeDestruct();
	}

public:
	UPROPERTY(EditAnywhere, Category = "Search sessions")
	int32 MaxSearchResults = 32;

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
