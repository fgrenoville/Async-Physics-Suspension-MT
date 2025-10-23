// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "CoreMinimal.h"
#include "GuidItem.generated.h"

UCLASS()
class ASYNCPHXSUSPENSIONMT_API UGuidItem : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGuid Guid = FGuid();
};
