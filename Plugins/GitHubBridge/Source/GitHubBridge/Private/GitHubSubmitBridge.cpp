#include "GitHubSubmitBridge.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "SourceControlOperations.h"
#include "Virtualization/VirtualizationSystem.h"
#include "Logging/MessageLog.h"

FGitHubSubmitBridge::~FGitHubSubmitBridge()
{
    if (auto* Windows = ISourceControlWindowsModule::TryGet())
        if (Windows->SubmitOverrideDelegate.IsBoundToObject(this)) Windows->SubmitOverrideDelegate.Unbind();
}

void FGitHubSubmitBridge::SetEnabled(bool bInEnabled)
{
    bEnabled = bInEnabled;
    auto& Override = ISourceControlWindowsModule::Get().SubmitOverrideDelegate;
    if (!bEnabled)
    {
        if (Override.IsBoundToObject(this)) Override.Unbind();
    }
    else if (!Override.IsBound()) Override.BindRaw(this, &FGitHubSubmitBridge::Submit);
}

FString FGitHubSubmitBridge::GetStatus() const
{
    if (!bEnabled) return TEXT("Automatic push after Unreal submit is off.");
    const auto* Windows = ISourceControlWindowsModule::TryGet();
    if (!Windows || !Windows->SubmitOverrideDelegate.IsBoundToObject(this))
        return TEXT("Automatic submit push is unavailable: another extension owns Unreal's submit override. Its handler has been left unchanged.");
    return TEXT("Successful submissions from Unreal's built-in Git submit dialog will queue a push. Other providers are unaffected.");
}

FSubmitOverrideReply FGitHubSubmitBridge::Submit(SSubmitOverrideParameters Parameters)
{
    auto& Provider = ISourceControlModule::Get().GetProvider();
    if (!bEnabled || Provider.GetName() != FName(TEXT("Git")) || !Parameters.ToSubmit.HasSubtype<TArray<FString>>())
        return FSubmitOverrideReply::ProviderNotSupported;
    const auto& Files = Parameters.ToSubmit.GetSubtype<TArray<FString>>();
    if (Files.IsEmpty()) return FSubmitOverrideReply::ProviderNotSupported;
    if (!CanSubmit())
    {
        FMessageLog(TEXT("SourceControl")).Error(FText::FromString(TEXT("GitHub Bridge is busy. Wait for its current operation to finish, then submit again.")));
        return FSubmitOverrideReply::Error;
    }

    // Match the engine's normal post-selection/pre-CheckIn virtualization phase.
    // The regular submit dialog has already performed selection, validation and MarkForAdd.
    FText Description = FText::FromString(Parameters.Description);
    TArray<FText> Errors;
    TArray<FText> Tags;
    PRAGMA_DISABLE_DEPRECATION_WARNINGS
    ISourceControlModule::Get().GetOnPreSubmitFinalize().Broadcast(Files, Tags, Errors);
    PRAGMA_ENABLE_DEPRECATION_WARNINGS
    if (!Errors.IsEmpty())
    {
        for (const FText& Error : Errors) FMessageLog(TEXT("SourceControl")).Error(Error);
        return FSubmitOverrideReply::Error;
    }
    auto& Virtualization = UE::Virtualization::IVirtualizationSystem::Get();
    if (Virtualization.IsEnabled())
    {
        const auto Result = Virtualization.TryVirtualizePackages(Files, UE::Virtualization::EVirtualizationOptions::None);
        if (!Result.WasSuccessful() && !Virtualization.AllowSubmitIfVirtualizationFailed())
        {
            for (const FText& Error : Result.Errors) FMessageLog(TEXT("SourceControl")).Error(Error);
            return FSubmitOverrideReply::Error;
        }
        if (Result.WasSuccessful())
            for (const FText& Tag : Result.DescriptionTags)
                Description = FText::FromString(Description.ToString() + TEXT("\n") + Tag.ToString());
        else
            for (const FText& Error : Result.Errors) FMessageLog(TEXT("SourceControl")).Warning(Error);
    }
    auto CheckIn = ISourceControlOperation::Create<FCheckIn>();
    CheckIn->SetDescription(Description);
    const auto Result = Provider.Execute(CheckIn, Files, EConcurrency::Synchronous);
    if (Result != ECommandResult::Succeeded) return FSubmitOverrideReply::Error;
    FMessageLog(TEXT("SourceControl")).Info(CheckIn->GetSuccessMessage());
    OnCommitted.ExecuteIfBound();
    return FSubmitOverrideReply::Handled;
}
