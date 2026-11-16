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
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FSaveFrameworkDocumentationSnapshot(Tab, this));
    return true;
}
#endif
