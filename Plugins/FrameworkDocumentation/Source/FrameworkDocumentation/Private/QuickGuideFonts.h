#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"

struct FSearchBoxStyle;

namespace QuickGuideFonts
{
    FSlateFontInfo Get(const FName& Typeface, float Size);
    const FSearchBoxStyle& SearchBoxStyle();
}
