#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickGuideWidget.generated.h"

class UWidgetTree;
class UWidgetSwitcher;

/** Designer-editable UMG shell for the Studio Framework documentation. */
UCLASS(BlueprintType, Blueprintable)
class FRAMEWORKDOCUMENTATION_API UQuickGuideWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Framework Documentation")
    void ShowDocumentation();

    UFUNCTION(BlueprintCallable, Category="Framework Documentation")
    void ShowCustomPage();

    UFUNCTION(BlueprintPure, Category="Framework Documentation")
    UWidgetSwitcher* GetPageSwitcher() const;

    static void BuildDefaultLayout(UWidgetTree* Tree);
protected:
    virtual void NativeOnInitialized() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
};
