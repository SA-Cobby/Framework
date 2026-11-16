#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickGuideWidget.generated.h"

/** Native Editor Utility Widget; all guide content is available offline. */
UCLASS(BlueprintType, Blueprintable)
class FRAMEWORKDOCUMENTATION_API UQuickGuideWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
};
