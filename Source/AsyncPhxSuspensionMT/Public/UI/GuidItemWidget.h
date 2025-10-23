// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "GuidItemWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS(Abstract, Blueprintable)
class ASYNCPHXSUSPENSIONMT_API UGuidItemWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRowAction, const FGuid&, Guid);
	UPROPERTY(BlueprintAssignable, Category = "AsyncPhxSuspensionMT")
	FOnRowAction OnActionRequested;
	
protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GuidText = nullptr;
	UPROPERTY(meta = (BindWidget))
	UButton* DestroyButton = nullptr;
	
protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;

private:
	UPROPERTY()
	FGuid Guid = FGuid();

private:
	UFUNCTION()
	void HandleDestroyButtonClicked();
};
