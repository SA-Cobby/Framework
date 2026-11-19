#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "QuickGuideTopics.h"
#include "QuickGuideWidget.h"
#include "QuickGuideFonts.h"
#include "Fonts/CompositeFont.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Tests/AutomationCommon.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/CanvasPanel.h"
#include "Components/WidgetSwitcher.h"
#include "FrameworkDocumentationPanel.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "Slate/SObjectWidget.h"

namespace
{
    bool SaveGuideAsset(UEditorUtilityWidgetBlueprint* Blueprint)
    {
        const FString Filename = FPackageName::LongPackageNameToFilename(Blueprint->GetOutermost()->GetName(), FPackageName::GetAssetPackageExtension());
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
        FSavePackageArgs Args;
        Args.TopLevelFlags = RF_Public | RF_Standalone;
        Args.SaveFlags = SAVE_NoError;
        return UPackage::SavePackage(Blueprint->GetOutermost(), Blueprint, *Filename, Args);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFrameworkDocumentationUMGTest,
    "FrameworkDocumentation.Editor.UMGLayout",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFrameworkDocumentationUMGTest::RunTest(const FString& Parameters)
{
    const TCHAR* AssetPath = TEXT("/FrameworkDocumentation/UI/EUW_FrameworkDocumentation.EUW_FrameworkDocumentation");
    UEditorUtilityWidgetBlueprint* Blueprint = nullptr;
    const bool bGenerate = FParse::Param(FCommandLine::Get(), TEXT("FrameworkDocumentationGenerateAsset"));
    if (bGenerate && !FPackageName::DoesPackageExist(TEXT("/FrameworkDocumentation/UI/EUW_FrameworkDocumentation")))
    {
        UPackage* Package = CreatePackage(TEXT("/FrameworkDocumentation/UI/EUW_FrameworkDocumentation"));
        Blueprint = CastChecked<UEditorUtilityWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(
            UQuickGuideWidget::StaticClass(), Package, TEXT("EUW_FrameworkDocumentation"), BPTYPE_Normal,
            UEditorUtilityWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass()));
        Blueprint->bCanCallInitializedWithoutPlayerContext = true;
        UQuickGuideWidget::BuildDefaultLayout(Blueprint->WidgetTree);
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        FKismetEditorUtilities::CompileBlueprint(Blueprint);
        FAssetRegistryModule::AssetCreated(Blueprint);
        if (!TestTrue(TEXT("Designer template saved"), SaveGuideAsset(Blueprint))) return false;
    }
    else Blueprint = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, AssetPath);
    if (!TestNotNull(TEXT("Editable Editor Utility Widget Blueprint exists"), Blueprint)) return false;
    TestTrue(TEXT("Blueprint compiles"), Blueprint->Status != BS_Error && Blueprint->GeneratedClass != nullptr);
    UWidgetSwitcher* Switcher = Cast<UWidgetSwitcher>(Blueprint->WidgetTree->RootWidget);
    if (!TestNotNull(TEXT("The real UMG root is a Widget Switcher"), Switcher)) return false;
    TestEqual(TEXT("Exactly two pages"), Switcher->GetChildrenCount(), 2);
    if (Switcher->GetChildrenCount() != 2) return false;
    UCanvasPanel* Canvas = Cast<UCanvasPanel>(Switcher->GetChildAt(0));
    if (TestNotNull(TEXT("First page is a Canvas Panel"), Canvas))
        TestEqual(TEXT("The custom Canvas is empty"), Canvas->GetChildrenCount(), 0);
    TestTrue(TEXT("Second page is the reusable documentation UMG widget"), Switcher->GetChildAt(1)->IsA<UFrameworkDocumentationPanel>());
    TestEqual(TEXT("The custom page opens first"), Switcher->GetActiveWidgetIndex(), 0);
    if (bGenerate && !FPackageName::DoesPackageExist(TEXT("/Game/StudioFramework/UI/EUW_FrameworkDocumentation")))
    {
        IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
        UEditorUtilityWidgetBlueprint* Copy = Cast<UEditorUtilityWidgetBlueprint>(AssetTools.DuplicateAsset(
            TEXT("EUW_FrameworkDocumentation"), TEXT("/Game/StudioFramework/UI"), Blueprint));
        if (TestNotNull(TEXT("Editable project copy created"), Copy)) TestTrue(TEXT("Project copy saved"), SaveGuideAsset(Copy));
    }
    UQuickGuideWidget* Instance = NewObject<UQuickGuideWidget>(GetTransientPackage(), Blueprint->GeneratedClass);
    Instance->Initialize();
    Instance->TakeWidget();
    TestNotNull(TEXT("UMG runtime root exists"), Instance->GetPageSwitcher());
    Instance->ShowDocumentation();
    TestEqual(TEXT("Show Documentation selects page 1"), Instance->GetPageSwitcher()->GetActiveWidgetIndex(), 1);
    Instance->ShowCustomPage();
    TestEqual(TEXT("Show Custom Page selects page 0"), Instance->GetPageSwitcher()->GetActiveWidgetIndex(), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFrameworkDocumentationSmokeTest,
    "FrameworkDocumentation.Editor.Smoke",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFrameworkDocumentationSmokeTest::RunTest(const FString& Parameters)
{
    const auto& Topics = GetQuickGuideTopics();
    TestEqual(TEXT("Framework, design, genre and camera topics are present"), Topics.Num(), 50);
    const FCompositeFont* Font = QuickGuideFonts::Get(TEXT("Regular"), 14).GetCompositeFont();
    if (TestNotNull(TEXT("Bundled Montserrat font family is available"), Font))
    {
        TestEqual(TEXT("Regular and bold styles are bundled"), Font->DefaultTypeface.Fonts.Num(), 2);
        for (const FTypefaceEntry& Face : Font->DefaultTypeface.Fonts)
        {
            const FString& Filename = Face.Font.GetFontFilename();
            TestTrue(TEXT("Font is bundled inside the plugin: ") + Filename, Filename.Contains(TEXT("Resources/Fonts/Montserrat-")));
            TestTrue(TEXT("Bundled font file exists: ") + Filename, IFileManager::Get().FileSize(*Filename) > 10000);
        }
    }
    TSet<FString> Titles;
    for (const auto& Topic : Topics)
    {
        TestFalse(TEXT("Unique topic: ") + Topic.Title, Titles.Contains(Topic.Title));
        Titles.Add(Topic.Title);
        TestFalse(Topic.Title + TEXT(" has a description"), Topic.Summary.IsEmpty());
        TestFalse(Topic.Title + TEXT(" has an example"), Topic.Example.IsEmpty());
        TestFalse(Topic.Title + TEXT(" has a caution"), Topic.Pitfall.IsEmpty());
        TestEqual(Topic.Title + TEXT(" has two use cases"), Topic.UseCases.Num(), 2);
        for (const FString& UseCase : Topic.UseCases)
        {
            TestFalse(Topic.Title + TEXT(" has no blank use cases"), UseCase.TrimStartAndEnd().IsEmpty());
        }
        TestNotNull(Topic.Title + TEXT(" has a UE 5.4 icon"), FAppStyle::Get().GetOptionalBrush(FName(*Topic.Icon), nullptr, nullptr));
    }
    TestTrue(TEXT("Uses a real Editor Utility Widget"), UQuickGuideWidget::StaticClass()->IsChildOf(UEditorUtilityWidget::StaticClass()));
    for (const FString& RequiredTitle : {FString(TEXT("Level Design & Gameplay Flow")), FString(TEXT("Designing for a Genre")),
        FString(TEXT("Platformers")), FString(TEXT("Shooters")), FString(TEXT("First-person vs Third-person"))})
    {
        TestTrue(TEXT("Required design topic: ") + RequiredTitle, Titles.Contains(RequiredTitle));
    }
    const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->TryInvokeTab(FName(TEXT("FrameworkDocumentation")));
    TestTrue(TEXT("Registered editor tab constructs successfully"), Tab.IsValid());
    if (Tab.IsValid())
    {
        Tab->RequestCloseTab();
        const TSharedPtr<SDockTab> Reopened = FGlobalTabmanager::Get()->TryInvokeTab(FName(TEXT("FrameworkDocumentation")));
        TestTrue(TEXT("Tab can be reopened"), Reopened.IsValid());
        if (Reopened.IsValid()) Reopened->RequestCloseTab();
    }
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSaveFrameworkDocumentationSnapshot,
    TSharedPtr<SDockTab>, Tab, FAutomationTestBase*, Test);

bool FSaveFrameworkDocumentationSnapshot::Update()
{
    TArray<FColor> Pixels;
    FIntVector Size = FIntVector::ZeroValue;
    const bool bCaptured = Tab.IsValid() && FSlateApplication::Get().TakeScreenshot(Tab->GetContent(), Pixels, Size);
    Test->TestTrue(TEXT("The guide renders to an image"), bCaptured);
    if (bCaptured)
    {
        Test->TestTrue(TEXT("Snapshot includes the article body, not just the tab label"), Size.X > 400 && Size.Y > 250);
        const FString Output = FPaths::ProjectSavedDir() / TEXT("Automation/FrameworkDocumentation.png");
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Output), true);
        TArray64<uint8> PNG;
        FImageUtils::PNGCompressImageArray(Size.X, Size.Y, Pixels, PNG);
        Test->TestTrue(TEXT("Visual snapshot saved"), FFileHelper::SaveArrayToFile(PNG, *Output));
        Test->AddInfo(TEXT("Visual snapshot: ") + Output);
    }
    if (Tab.IsValid()) Tab->RequestCloseTab();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFrameworkDocumentationVisualTest,
    "FrameworkDocumentation.Editor.VisualSnapshot",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFrameworkDocumentationVisualTest::RunTest(const FString& Parameters)
{
    const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->TryInvokeTab(FName(TEXT("FrameworkDocumentation")));
    if (!TestTrue(TEXT("Guide tab opens for visual check"), Tab.IsValid())) return false;
    UQuickGuideWidget* Shell = Cast<UQuickGuideWidget>(StaticCastSharedRef<SObjectWidget>(Tab->GetContent())->GetWidgetObject());
    if (!TestNotNull(TEXT("Tab hosts the UMG shell"), Shell)) return false;
    Shell->ShowDocumentation();
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FSaveFrameworkDocumentationSnapshot(Tab, this));
    return true;
}
#endif
