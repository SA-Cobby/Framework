#include "Modules/ModuleManager.h"
#include "QuickGuideWidget.h"
#include "ToolMenus.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "UObject/StrongObjectPtr.h"
#include "QuickGuideSettings.h"
#include "Containers/Ticker.h"
#include "Interfaces/IMainFrameModule.h"
#include "Misc/App.h"
#include "Editor.h"
#include "Styling/AppStyle.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Misc/PackageName.h"

static const FName GuideTabName(TEXT("FrameworkDocumentation"));
DEFINE_LOG_CATEGORY_STATIC(LogFrameworkDocumentation, Log, All);

class FFrameworkDocumentationModule : public IModuleInterface
{
    TStrongObjectPtr<UQuickGuideWidget> Widget;
    TWeakPtr<SDockTab> OpenTab;
    FTSTicker::FDelegateHandle StartupTicker;
public:
    virtual void StartupModule() override
    {
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GuideTabName,
            FOnSpawnTab::CreateRaw(this, &FFrameworkDocumentationModule::SpawnTab))
            .SetDisplayName(FText::FromString(TEXT("FrameworkDocumentation")))
            .SetMenuType(ETabSpawnerMenuType::Hidden);
        UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FFrameworkDocumentationModule::RegisterMenus));
        // Wait for the real editor window; never open UI during automation or cooking.
        if (!IsRunningCommandlet() && !FApp::IsUnattended() && QuickGuideSettings::OpenOnStartup())
        {
            StartupTicker = FTSTicker::GetCoreTicker().AddTicker(
                FTickerDelegate::CreateRaw(this, &FFrameworkDocumentationModule::OpenWhenReady), 0.25f);
        }
    }

    bool OpenWhenReady(float DeltaTime)
    {
        if (!QuickGuideSettings::OpenOnStartup()) return false;
        if (!GEditor || !FModuleManager::Get().IsModuleLoaded("MainFrame")) return true;
        const IMainFrameModule& MainFrame = FModuleManager::GetModuleChecked<IMainFrameModule>("MainFrame");
        if (!MainFrame.IsWindowInitialized()) return true;
        if (GEditor->PlayWorld) return false;
        if (FGlobalTabmanager::Get()->TryInvokeTab(GuideTabName).IsValid())
        {
            UE_LOG(LogFrameworkDocumentation, Display, TEXT("Documentation panel opened automatically after the editor became ready."));
        }
        return false;
    }

    void RegisterMenus()
    {
        FToolMenuOwnerScoped Owner(this);
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
        Menu->FindOrAddSection("FrameworkDocumentation").AddMenuEntry(
            "OpenFrameworkDocumentation", FText::FromString(TEXT("FrameworkDocumentation")),
            FText::FromString(TEXT("Brief explanations of Unreal Engine 5.4 essentials.")),
            FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.Function_24x"), FUIAction(FExecuteAction::CreateLambda([] {
                FGlobalTabmanager::Get()->TryInvokeTab(GuideTabName);
            })));
    }

    TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args)
    {
        // Prefer the project's editable copy. Plugin template is the fallback for other projects.
        const TCHAR* ProjectPackage = TEXT("/Game/StudioFramework/UI/EUW_FrameworkDocumentation");
        const TCHAR* BlueprintPath = FPackageName::DoesPackageExist(ProjectPackage)
            ? TEXT("/Game/StudioFramework/UI/EUW_FrameworkDocumentation.EUW_FrameworkDocumentation")
            : TEXT("/FrameworkDocumentation/UI/EUW_FrameworkDocumentation.EUW_FrameworkDocumentation");
        UEditorUtilityWidgetBlueprint* Blueprint = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, BlueprintPath);
        UClass* WidgetClass = Blueprint ? Blueprint->GeneratedClass : nullptr;
        if (!WidgetClass || !WidgetClass->IsChildOf(UQuickGuideWidget::StaticClass()))
        {
            UE_LOG(LogFrameworkDocumentation, Error, TEXT("Documentation Widget Blueprint is missing or has an incompatible parent: %s"), BlueprintPath);
            WidgetClass = UQuickGuideWidget::StaticClass();
        }
        Widget.Reset(NewObject<UQuickGuideWidget>(GetTransientPackage(), WidgetClass));
        Widget->Initialize();
        TSharedRef<SDockTab> Tab = SNew(SDockTab).TabRole(ETabRole::NomadTab)[Widget->TakeWidget()];
        OpenTab = Tab;
        Tab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda([this](TSharedRef<SDockTab>) {
            Widget.Reset();
            OpenTab.Reset();
        }));
        return Tab;
    }

    virtual void ShutdownModule() override
    {
        FTSTicker::GetCoreTicker().RemoveTicker(StartupTicker);
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
        if (TSharedPtr<SDockTab> Tab = OpenTab.Pin())
        {
            Tab->SetOnTabClosed(SDockTab::FOnTabClosedCallback());
            Tab->RequestCloseTab();
        }
        Widget.Reset();
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GuideTabName);
    }
};

IMPLEMENT_MODULE(FFrameworkDocumentationModule, FrameworkDocumentation)
