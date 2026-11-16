#pragma once

#include "CoreMinimal.h"

struct FQuickGuideTopic
{
    FString Title;
    FString Category;
    FString Summary;
    FString Example;
    FString Pitfall;
    TArray<FString> UseCases;
    FString Icon;
};

const TArray<FQuickGuideTopic>& GetQuickGuideTopics();
