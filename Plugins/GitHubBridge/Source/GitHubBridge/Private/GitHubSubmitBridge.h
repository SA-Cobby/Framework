#pragma once
#include "CoreMinimal.h"
#include "ISourceControlWindowsModule.h"

// The public UE 5.4 submit override covers the file-based submit dialog used by
// the built-in Git provider. Other providers and unsupported requests fall through.
class FGitHubSubmitBridge
{
public:
    explicit FGitHubSubmitBridge(FSimpleDelegate InOnCommitted, TFunction<bool()> InCanSubmit = [] { return true; })
        : OnCommitted(MoveTemp(InOnCommitted)), CanSubmit(MoveTemp(InCanSubmit)) {}
    ~FGitHubSubmitBridge();
    void SetEnabled(bool bInEnabled);
    FString GetStatus() const;
    FSubmitOverrideReply Submit(SSubmitOverrideParameters Parameters);
private:
    bool bEnabled = false;
    FSimpleDelegate OnCommitted;
    TFunction<bool()> CanSubmit;
};
