// Copyright (c) [2025] [Federico Grenoville]

#include "UI/SuspensionWidget.h"
#include "SWPSuspension.h"
#include "Components/TextBlock.h"

void USuspensionWidget::SetWidgetName(FText Label)
{
	if (WidgetName) WidgetName->SetText(Label);
}

void USuspensionWidget::ReconfigureWidget(USWPSuspension* NewSuspension)
{
	Suspension = NewSuspension;
	
	if (!IsValid(TravelSlider) || !IsValid(SpringStiffnessSlider) || !IsValid(ShockBumpSlider) || !IsValid(ShockReboundSlider)
	|| !IsValid(MaxForceSlider) || !IsValid(WheelRadiusSlider))
		return;
	
	if (Suspension.IsValid())
	{
		bUpdatingFromCode = true;
		
		InitSlider(TravelSlider, TravelCmMin, TravelCmMax, TravelCmStep, Suspension->TravelCm);
		InitSlider(SpringStiffnessSlider, SpringStiffnessMin, SpringStiffnessMax, SpringStiffnessStep, Suspension->SpringStiffness);
		InitSlider(ShockBumpSlider, ShockBumpMin, ShockBumpMax,	ShockBumpStep, Suspension->ShockBump);
		InitSlider(ShockReboundSlider, ShockReboundMin, ShockReboundMax, ShockReboundStep, Suspension->ShockRebound);
		InitSlider(MaxForceSlider, MaxForceMin, MaxForceMax, MaxForceStep, Suspension->MaxForce);
		InitSlider(WheelRadiusSlider, WheelRadiusCmMin, WheelRadiusCmMax, WheelRadiusCmStep, Suspension->WheelRadiusCm);

		UpdateText(TravelValue, Suspension->TravelCm);
		UpdateText(SpringStiffnessValue, Suspension->SpringStiffness);
		UpdateText(ShockBumpValue, Suspension->ShockBump);
		UpdateText(ShockReboundValue, Suspension->ShockRebound);
		UpdateText(MaxForceValue, Suspension->MaxForce);
		UpdateText(WheelRadiusValue, Suspension->WheelRadiusCm);

		bUpdatingFromCode = false;
	}
}

void USuspensionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TravelSlider->OnValueChanged.AddDynamic(this, &USuspensionWidget::OnTravelValueChanged);
	SpringStiffnessSlider->OnValueChanged.AddDynamic(this, &USuspensionWidget::OnSpringStiffnessValueChanged);
	ShockBumpSlider->OnValueChanged.AddDynamic(this, &USuspensionWidget::OnShockBumpValueChanged);
	ShockReboundSlider->OnValueChanged.AddDynamic(this, &USuspensionWidget::OnShockReboundValueChanged);
	MaxForceSlider->OnValueChanged.AddDynamic(this, &USuspensionWidget::OnMaxForceValueChanged);
	WheelRadiusSlider->OnValueChanged.AddDynamic(this, &USuspensionWidget::OnWheelRadiusValueChanged);
}

void USuspensionWidget::NativeDestruct()
{
	if (IsValid(TravelSlider)) 
		TravelSlider->OnValueChanged.RemoveAll(this);

	if (IsValid(SpringStiffnessSlider)) 
		SpringStiffnessSlider->OnValueChanged.RemoveAll(this);

	if (IsValid(ShockBumpSlider)) 
		ShockBumpSlider->OnValueChanged.RemoveAll(this);

	if (IsValid(ShockReboundSlider)) 
		ShockReboundSlider->OnValueChanged.RemoveAll(this);

	if (IsValid(MaxForceSlider)) 
		MaxForceSlider->OnValueChanged.RemoveAll(this);

	if (IsValid(WheelRadiusSlider)) 
		WheelRadiusSlider->OnValueChanged.RemoveAll(this);

	Super::NativeDestruct();
}

void USuspensionWidget::InitSlider(USlider* Slider, float Min, float Max, float Step, float StartValue)
{
	if (!Slider) return;
	
	Slider->SetMinValue(Min);
	Slider->SetMaxValue(Max);
	Slider->SetStepSize(Step);
	Slider->SetValue(FMath::Clamp(StartValue, Min, Max));
}

void USuspensionWidget::UpdateText(UTextBlock* Text, float Value, int32 Decimals) const
{
	if (!Text) return;
	
	FNumberFormattingOptions Opt;
	Opt.MinimumFractionalDigits = Decimals;
	Opt.MaximumFractionalDigits = Decimals;
	Text->SetText(FText::AsNumber(Value, &Opt));
}

void USuspensionWidget::OnTravelValueChanged(float NewValue)
{
	if (bUpdatingFromCode) return;
	if (!Suspension.IsValid()) return;
	
	ApplySliderChange(Suspension, &USWPSuspension::TravelCm,
					  TravelSlider, TravelValue, NewValue, TravelCmMin, TravelCmMax, TravelCmStep, 0);
}

void USuspensionWidget::OnSpringStiffnessValueChanged(float NewValue)
{
	if (bUpdatingFromCode) return;
	if (!Suspension.IsValid()) return;

	ApplySliderChange(Suspension, &USWPSuspension::SpringStiffness, SpringStiffnessSlider, SpringStiffnessValue,
					  NewValue, SpringStiffnessMin, SpringStiffnessMax, SpringStiffnessStep, 0);
}

void USuspensionWidget::OnShockBumpValueChanged(float NewValue)
{
	if (bUpdatingFromCode) return;
	if (!Suspension.IsValid()) return;

	ApplySliderChange(Suspension, &USWPSuspension::ShockBump, ShockBumpSlider, ShockBumpValue,
					  NewValue, ShockBumpMin, ShockBumpMax, ShockBumpStep, 0);
}

void USuspensionWidget::OnShockReboundValueChanged(float NewValue)
{
	if (bUpdatingFromCode) return;
	if (!Suspension.IsValid()) return;

	ApplySliderChange(Suspension, &USWPSuspension::ShockRebound, ShockReboundSlider, ShockReboundValue,
					  NewValue, ShockReboundMin, ShockReboundMax, ShockReboundStep, 0);
}

void USuspensionWidget::OnMaxForceValueChanged(float NewValue)
{
	if (bUpdatingFromCode) return;
	if (!Suspension.IsValid()) return;

	ApplySliderChange(Suspension, &USWPSuspension::MaxForce, MaxForceSlider, MaxForceValue,
					  NewValue, MaxForceMin, MaxForceMax, MaxForceStep, 0);
}

void USuspensionWidget::OnWheelRadiusValueChanged(float NewValue)
{
	if (bUpdatingFromCode) return;
	if (!Suspension.IsValid()) return;

	ApplySliderChange(Suspension, &USWPSuspension::WheelRadiusCm, WheelRadiusSlider, WheelRadiusValue,
					  NewValue, WheelRadiusCmMin, WheelRadiusCmMax, WheelRadiusCmStep, 0);
}
