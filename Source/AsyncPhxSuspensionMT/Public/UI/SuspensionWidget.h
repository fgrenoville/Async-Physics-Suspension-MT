// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "SuspensionWidget.generated.h"

class USWPSuspension;
class USlider;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class ASYNCPHXSUSPENSIONMT_API USuspensionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float TravelCmMin = 5.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float TravelCmMax = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float TravelCmStep = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float SpringStiffnessMin = 5000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float SpringStiffnessMax = 80000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float SpringStiffnessStep = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float ShockBumpMin = 50.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float ShockBumpMax = 10000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float ShockBumpStep = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float ShockReboundMin = 50.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float ShockReboundMax = 10000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float ShockReboundStep = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float MaxForceMin = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float MaxForceMax = 15000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float MaxForceStep = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float WheelRadiusCmMin = 5.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float WheelRadiusCmMax = 80.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT")
	float WheelRadiusCmStep = 1.0f;
	
public:
	void SetWidgetName(FText Label);
	void ReconfigureWidget(USWPSuspension* NewSuspension);
	
protected:
	/** Suspension name label (e.g., "Front Left"). */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WidgetName;

	// --- UI Controls: each parameter has a slider and a text block showing the numeric value ---
	UPROPERTY(meta = (BindWidget))
	USlider* TravelSlider;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TravelValue;

	UPROPERTY(meta = (BindWidget))
	USlider* SpringStiffnessSlider;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SpringStiffnessValue;

	UPROPERTY(meta = (BindWidget))
	USlider* ShockBumpSlider;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShockBumpValue;

	UPROPERTY(meta = (BindWidget))
	USlider* ShockReboundSlider;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShockReboundValue;

	UPROPERTY(meta = (BindWidget))
	USlider* MaxForceSlider;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxForceValue;

	UPROPERTY(meta = (BindWidget))
	USlider* WheelRadiusSlider;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WheelRadiusValue;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	/** The suspension component bound to this widget (weak ref to avoid lifetime issues). */
	UPROPERTY()
	TWeakObjectPtr<USWPSuspension> Suspension;

	bool bUpdatingFromCode = false;
	
private:
	void InitSlider(USlider* Slider, float Min, float Max, float Step, float StartValue);
	
	void UpdateText(UTextBlock* Text, float Value, int32 Decimals = 0) const;
	
	template<typename MemberPtr>
	void ApplySliderChange(TWeakObjectPtr<USWPSuspension> S,
						   MemberPtr Field,
						   USlider* Slider, UTextBlock* Text,
						   float NewValue, float Min, float Max, float Step, int32 Decimals = 0)
	{
		if (!S.IsValid() || !Slider || !Text) return;

		const float Clamped = FMath::Clamp(NewValue, Min, Max);
		const float Snapped = (Step > 0.f) ? FMath::RoundToFloat(Clamped / Step) * Step : Clamped;

		S.Get()->*Field = Snapped;

		if (!FMath::IsNearlyEqual(Slider->GetValue(), Snapped))
			Slider->SetValue(Snapped);

		UpdateText(Text, Snapped, Decimals);
	}

	// --- Individual slider callbacks ---
	UFUNCTION()
	void OnTravelValueChanged(float NewValue);
	UFUNCTION()
	void OnSpringStiffnessValueChanged(float NewValue);
	UFUNCTION()
	void OnShockBumpValueChanged(float NewValue);
	UFUNCTION()
	void OnShockReboundValueChanged(float NewValue);
	UFUNCTION()
	void OnMaxForceValueChanged(float NewValue);
	UFUNCTION()
	void OnWheelRadiusValueChanged(float NewValue);
};
