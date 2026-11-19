#include "FlaresToolsetSettings.h"
#include "FlaresPreset.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "FlaresToolsetEditor"

class FFlaresToolsetEditorModule : public IModuleInterface
{
    const FName TabName = TEXT("FlaresToolsetControls");

    TSharedRef<SDockTab> Spawn(const FSpawnTabArgs&)
    {
        FDetailsViewArgs Args;
        Args.bAllowSearch = true;
        Args.bHideSelectionTip = true;

        auto Details = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor").CreateDetailView(Args);
        Details->SetObject(GetMutableDefault<UFlaresToolsetSettings>());

        auto MakePresetMenu = [Details]()
        {
            FMenuBuilder MenuBuilder(true, nullptr);
            const TArray<FName> Presets = UFlaresPreset::GetBuiltinPresetList();
            for (const FName& PresetName : Presets)
            {
                MenuBuilder.AddMenuEntry(
                    FText::FromName(PresetName),
                    FText::Format(LOCTEXT("ApplyPresetTip", "Apply the {0} preset"), FText::FromName(PresetName)),
                    FSlateIcon(),
                    FUIAction(FExecuteAction::CreateLambda([PresetName, Details]()
                    {
                        UFlaresPreset::ApplyBuiltinPresetByName(PresetName, GetMutableDefault<UFlaresToolsetSettings>());
                        Details->SetObject(GetMutableDefault<UFlaresToolsetSettings>(), true);
                    }))
                );
            }
            return MenuBuilder.MakeWidget();
        };

        return SNew(SDockTab).TabRole(ETabRole::NomadTab)
        [
            SNew(SBox).Padding(12).MinDesiredWidth(480)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
                [
                    SNew(STextBlock).AutoWrapText(true).Text(LOCTEXT("Notice",
                        "Flares Toolset provides advanced optical rendering for Unreal Engine 5.4: "
                        "anamorphic and multi-axis lens flares, sensor blooming & smear (CCD column / CMOS row overload), "
                        "cylindrical lens distortion, edge-gated radial chromatic aberration, cel shading, comic halftone & manga crosshatching, "
                        "and one-click artist presets. Changes apply live in editor viewports and PIE."))
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 10)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("PresetLabel", "One-Click Presets:"))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
                    ]
                    + SHorizontalBox::Slot().AutoWidth()
                    [
                        SNew(SComboButton)
                        .OnGetMenuContent_Lambda(MakePresetMenu)
                        .ButtonContent()
                        [
                            SNew(STextBlock).Text(LOCTEXT("SelectPreset", "Choose Built-in Preset..."))
                        ]
                    ]
                ]
                + SVerticalBox::Slot().FillHeight(1)
                [
                    Details
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)
                [
                    SNew(SButton).Text(LOCTEXT("Save", "Save as project defaults"))
                    .OnClicked_Lambda([]()
                    {
                        const bool bSaved = GetMutableDefault<UFlaresToolsetSettings>()->TryUpdateDefaultConfigFile();
                        FMessageDialog::Open(EAppMsgType::Ok, bSaved
                            ? LOCTEXT("Saved", "Flares Toolset defaults saved to the project's DefaultGame.ini.")
                            : LOCTEXT("SaveFailed", "Could not save Flares Toolset defaults. Check that Config/DefaultGame.ini is writable."));
                        return FReply::Handled();
                    })
                ]
            ]
        ];
    }

    void RegisterMenu()
    {
        FToolMenuOwnerScoped Owner(this);
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
        Menu->FindOrAddSection("WindowLayout").AddMenuEntry(
            "FlaresToolsetControls",
            LOCTEXT("Title", "Flares Toolset"),
            LOCTEXT("Tip", "Open the Flares Toolset controls window"),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([this]()
            {
                FGlobalTabmanager::Get()->TryInvokeTab(TabName);
            }))
        );
    }

public:
    virtual void StartupModule() override
    {
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName, FOnSpawnTab::CreateRaw(this, &FFlaresToolsetEditorModule::Spawn))
            .SetDisplayName(LOCTEXT("Title", "Flares Toolset"))
            .SetMenuType(ETabSpawnerMenuType::Hidden);

        UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FFlaresToolsetEditorModule::RegisterMenu));
    }

    virtual void ShutdownModule() override
    {
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
    }
};

IMPLEMENT_MODULE(FFlaresToolsetEditorModule, FlaresToolsetEditor)

#undef LOCTEXT_NAMESPACE
