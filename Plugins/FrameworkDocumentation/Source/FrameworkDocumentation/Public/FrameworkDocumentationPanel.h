#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "FrameworkDocumentationPanel.generated.h"

/** Reusable UMG documentation control; place it in any UMG layout. */
UCLASS(meta=(DisplayName="Framework Documentation"))
class FRAMEWORKDOCUMENTATION_API UFrameworkDocumentationPanel : public UWidget
{
    GENERATED_BODY()
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
};
