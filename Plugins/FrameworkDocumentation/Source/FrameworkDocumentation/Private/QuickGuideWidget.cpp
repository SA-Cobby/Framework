#include "QuickGuideWidget.h"
#include "QuickGuideTopics.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"
#include "QuickGuideSettings.h"
#include "QuickGuideFonts.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Animation/CurveSequence.h"
#include "Misc/App.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Framework/Application/SlateApplication.h"

namespace
{
    FLinearColor CategoryColour(const FString& Category)
    {
        if (Category == TEXT("Blueprints")) return FLinearColor(0.32f, 0.65f, 1.0f);
        if (Category == TEXT("Level design")) return FLinearColor(1.0f, 0.72f, 0.32f);
        if (Category == TEXT("Game types and cameras")) return FLinearColor(0.6f, 0.65f, 1.0f);
        if (Category == TEXT("Animation")) return FLinearColor(0.8f, 0.45f, 0.95f);
        if (Category == TEXT("Data and organisation")) return FLinearColor(0.3f, 0.85f, 0.65f);
        if (Category == TEXT("Artificial intelligence")) return FLinearColor(1.0f, 0.65f, 0.25f);
        if (Category == TEXT("Art and materials")) return FLinearColor(1.0f, 0.5f, 0.6f);
        return FLinearColor(0.4f, 0.8f, 0.9f);
    }

    const FSlateBrush* TopicIcon(const FQuickGuideTopic& Topic)
    {
        return FAppStyle::Get().GetOptionalBrush(FName(*Topic.Icon), nullptr,
            FAppStyle::GetBrush("ClassIcon.BlueprintCore"));
    }
}

class SQuickGuide : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SQuickGuide) {} SLATE_END_ARGS()

    void Construct(const FArguments& Args)
    {
        Welcome.AddCurve(0, 0.55f, ECurveEaseFunction::QuadOut);
        ChildSlot
        [
            SNew(SBorder).Padding(20)
            .BorderImage(FAppStyle::GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0.018f, 0.024f, 0.04f))
            .ColorAndOpacity_Lambda([this] { return FLinearColor(1, 1, 1, 0.65f + 0.35f * Welcome.GetLerp()); })
            .RenderTransform_Lambda([this]() -> TOptional<FSlateRenderTransform> {
                return FSlateRenderTransform(FVector2D(0, 8.0f * (1.0f - Welcome.GetLerp())));
            })
            [
                SAssignNew(Pages, SWidgetSwitcher).WidgetIndex(0)
                + SWidgetSwitcher::Slot()
                [BuildWelcomePage()]
                + SWidgetSwitcher::Slot()
                [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
                [SNew(STextBlock).Text(FText::FromString(TEXT("FRAMEWORK DOCUMENTATION")))
                    .Font(QuickGuideFonts::Get("Bold", 22)).AutoWrapText(true)
                    .ColorAndOpacity(FLinearColor(0.45f, 0.78f, 1.0f))]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 18)
                [SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("UE 5.4  |  %d topics  |  Pick a topic. Explore an example. Try it with your tutor."), GetQuickGuideTopics().Num())))
                    .Font(QuickGuideFonts::Get("Regular", 12)).AutoWrapText(true)]
                + SVerticalBox::Slot().FillHeight(1)
                [
                    SNew(SSplitter)
                    + SSplitter::Slot().Value(0.32f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 12, 12)
                        [SAssignNew(SearchBox, SSearchBox).Style(&QuickGuideFonts::SearchBoxStyle()).HintText(FText::FromString(TEXT("Find a topic...")))
                            .OnTextChanged_Lambda([this](const FText& Text) { Filter = Text.ToString(); RefreshTopics(); })]
                        + SVerticalBox::Slot().FillHeight(1).Padding(0, 0, 12, 0)
                        [SAssignNew(TopicList, SScrollBox)]
                    ]
                    + SSplitter::Slot().Value(0.68f)
                    [
                        SNew(SBorder).Padding(22)
                        .BorderImage(FAppStyle::GetBrush("WhiteBrush"))
                        .BorderBackgroundColor(FLinearColor(0.028f, 0.038f, 0.06f))
                        [SAssignNew(DetailsScroll, SScrollBox)
                            + SScrollBox::Slot()[SAssignNew(Details, SVerticalBox)]]
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 14, 0, 0)
                [
                    SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(22, 8))
                    + SWrapBox::Slot()
                    [SNew(SCheckBox)
                        .IsChecked_Lambda([] { return QuickGuideSettings::OpenOnStartup() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
                        .OnCheckStateChanged_Lambda([](ECheckBoxState State) { QuickGuideSettings::SetOpenOnStartup(State == ECheckBoxState::Checked); })
                        .ToolTipText(FText::FromString(TEXT("Remembered for you in this project. You can always reopen this guide from the Window menu.")))
                        [SNew(STextBlock).Text(FText::FromString(TEXT("Open when this project starts"))).Font(QuickGuideFonts::Get("Regular", 11))]]
                    + SWrapBox::Slot()
                    [SNew(SCheckBox)
                        .IsChecked_Lambda([] { return QuickGuideSettings::PlayWelcomeAnimation() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
                        .OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
                            QuickGuideSettings::SetPlayWelcomeAnimation(State == ECheckBoxState::Checked);
                            if (State != ECheckBoxState::Checked) Welcome.JumpToEnd();
                        })
                        .ToolTipText(FText::FromString(TEXT("A gentle 0.55-second fade and slide when the panel opens. Turn off for reduced motion.")))
                        [SNew(STextBlock).Text(FText::FromString(TEXT("Welcome animation"))).Font(QuickGuideFonts::Get("Regular", 11))]]
                ]
                ]
            ]
        ];
        RefreshTopics();
        SelectTopic(0);
        if (QuickGuideSettings::PlayWelcomeAnimation() && !FApp::IsUnattended()) Welcome.Play(AsShared());
        else Welcome.JumpToEnd();
    }

private:
    friend class FFrameworkDocumentationNavigationTest;
    TSharedPtr<SWidgetSwitcher> Pages;
    TSharedPtr<SButton> BeginButton;
    TSharedPtr<SSearchBox> SearchBox;
    TSharedPtr<SScrollBox> TopicList;
    TSharedPtr<SScrollBox> DetailsScroll;
    TSharedPtr<SVerticalBox> Details;
    FString Filter;
    int32 Selected = 0;
    FCurveSequence Welcome;

    TSharedRef<SWidget> BuildWelcomePage()
    {
        return SNew(SScrollBox)
            + SScrollBox::Slot().HAlign(HAlign_Center)
            [
                SNew(SBox).MaxDesiredWidth(900).Padding(24, 28)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(0, 0, 0, 14)
                    [SNew(SBox).WidthOverride(52).HeightOverride(52)
                        [SNew(SImage).Image(FAppStyle::GetBrush("ClassIcon.Blueprint"))
                            .ColorAndOpacity(FLinearColor(0.45f, 0.78f, 1.0f))]]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
                    [SNew(STextBlock).Text(FText::FromString(TEXT("WELCOME TO THE")))
                        .Font(QuickGuideFonts::Get("Bold", 13))
                        .ColorAndOpacity(FLinearColor(0.45f, 0.78f, 1.0f)).AutoWrapText(true)]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 14)
                    [SNew(STextBlock).Text(FText::FromString(TEXT("Studio Framework Project")))
                        .Font(QuickGuideFonts::Get("Bold", 32)).AutoWrapText(true)]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 24)
                    [SNew(STextBlock).Text(FText::FromString(TEXT("Your starting point for learning how games are built. Explore Unreal Engine with your tutor, try out ideas and use what you learn to create your own game.")))
                        .Font(QuickGuideFonts::Get("Regular", 16)).LineHeightPercentage(1.2f).AutoWrapText(true)]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
                    [MakeCard(TEXT("1. Explore the building blocks"), TEXT("Explore game types, level design, Blueprints and animation. Pick a topic or use the search box to find an idea."), FLinearColor(0.4f, 0.7f, 1.0f))]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
                    [MakeCard(TEXT("2. See how they are used"), TEXT("Read two game examples for each topic. Connect each idea to something you could make."), FLinearColor(0.4f, 0.85f, 0.65f))]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 24)
                    [MakeCard(TEXT("3. Try it together"), TEXT("Work through a short activity with your tutor. Ask questions, experiment and build your skills one step at a time."), FLinearColor(0.8f, 0.55f, 1.0f))]
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)
                    [SAssignNew(BeginButton, SButton).ContentPadding(FMargin(48, 14))
                        .ButtonColorAndOpacity(FLinearColor(0.12f, 0.38f, 0.6f))
                        .ToolTipText(FText::FromString(TEXT("Open the main documentation page.")))
                        .OnClicked_Lambda([this] {
                            Pages->SetActiveWidgetIndex(1);
                            FSlateApplication::Get().SetKeyboardFocus(SearchBox, EFocusCause::SetDirectly);
                            return FReply::Handled();
                        })
                        [SNew(STextBlock).Text(FText::FromString(TEXT("Begin")))
                            .Font(QuickGuideFonts::Get("Bold", 18))]]
                ]
            ];
    }

    void RefreshTopics()
    {
        TopicList->ClearChildren();
        FString LastCategory;
        int32 Matches = 0;
        const auto& Topics = GetQuickGuideTopics();
        for (int32 Index = 0; Index < Topics.Num(); ++Index)
        {
            const auto& Topic = Topics[Index];
            const bool bUseCaseMatch = Topic.UseCases.ContainsByPredicate([this](const FString& UseCase) { return UseCase.Contains(Filter); });
            if (!Filter.IsEmpty() && !Topic.Title.Contains(Filter) && !Topic.Category.Contains(Filter) && !Topic.Summary.Contains(Filter)
                && !Topic.Example.Contains(Filter) && !Topic.Pitfall.Contains(Filter) && !bUseCaseMatch)
                continue;
            ++Matches;
            if (LastCategory != Topic.Category)
            {
                LastCategory = Topic.Category;
                TopicList->AddSlot().Padding(0, 12, 0, 6)
                [SNew(STextBlock).Text(FText::FromString(Topic.Category)).AutoWrapText(true)
                    .Font(QuickGuideFonts::Get("Bold", 11))
                    .ColorAndOpacity(CategoryColour(Topic.Category))];
            }
            TopicList->AddSlot().Padding(0, 2)
            [
                SNew(SButton).ContentPadding(FMargin(10, 8)).HAlign(HAlign_Fill)
                .ButtonColorAndOpacity_Lambda([this, Index] {
                    return Index == Selected ? FLinearColor(0.12f, 0.38f, 0.6f) : FLinearColor::White;
                })
                .OnClicked_Lambda([this, Index] { SelectTopic(Index); return FReply::Handled(); })
                [SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
                    [SNew(SBox).WidthOverride(22).HeightOverride(22)
                        [SNew(SImage).Image(TopicIcon(Topic)).ColorAndOpacity(CategoryColour(Topic.Category))]]
                    + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
                    [SNew(STextBlock).Text(FText::FromString(Topic.Title))
                        .Font(QuickGuideFonts::Get("Regular", 12)).AutoWrapText(true)]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6, 0)
                    [SNew(STextBlock).Font(QuickGuideFonts::Get("Regular", 12)).Text_Lambda([this, Index] { return FText::FromString(Selected == Index ? TEXT(">") : TEXT("")); })]
                ]
            ];
        }
        if (Matches == 0)
        {
            TopicList->AddSlot()[SNew(STextBlock).Text(FText::FromString(TEXT("No matching topics. Clear the search to see all topics."))).Font(QuickGuideFonts::Get("Regular", 12)).AutoWrapText(true)];
        }
    }

    void AddSection(const TCHAR* Label, const FString& Text)
    {
        Details->AddSlot().AutoHeight().Padding(0, 18, 0, 6)
        [SNew(STextBlock).Text(FText::FromString(Label)).Font(QuickGuideFonts::Get("Bold", 13))];
        Details->AddSlot().AutoHeight()
        [SNew(STextBlock).Text(FText::FromString(Text)).AutoWrapText(true)
            .Font(QuickGuideFonts::Get("Regular", 14)).LineHeightPercentage(1.25f)];
    }

    TSharedRef<SWidget> MakeCard(const FString& Label, const FString& Text, const FLinearColor& Accent)
    {
        return SNew(SBorder).BorderImage(FAppStyle::GetBrush("WhiteBrush"))
            .BorderBackgroundColor(Accent * FLinearColor(0.1f, 0.1f, 0.1f, 1)).Padding(14)
            [SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 7)
                [SNew(STextBlock).Text(FText::FromString(Label)).Font(QuickGuideFonts::Get("Bold", 13))
                    .ColorAndOpacity(Accent).AutoWrapText(true)]
                + SVerticalBox::Slot().AutoHeight()
                [SNew(STextBlock).Text(FText::FromString(Text)).AutoWrapText(true)
                    .Font(QuickGuideFonts::Get("Regular", 14)).LineHeightPercentage(1.25f)]];
    }

    void SelectTopic(int32 Index)
    {
        Selected = Index;
        const auto& Topic = GetQuickGuideTopics()[Index];
        Details->ClearChildren();
        Details->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
        [SNew(STextBlock).Text(FText::FromString(Topic.Category + TEXT("  /  UE 5.4"))).Font(QuickGuideFonts::Get("Regular", 11)).AutoWrapText(true)];
        Details->AddSlot().AutoHeight()
        [SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 12, 0)
            [SNew(SBox).WidthOverride(34).HeightOverride(34)[SNew(SImage).Image(TopicIcon(Topic)).ColorAndOpacity(CategoryColour(Topic.Category))]]
            + SHorizontalBox::Slot().FillWidth(1)
            [SNew(STextBlock).Text(FText::FromString(Topic.Title))
                .Font(QuickGuideFonts::Get("Bold", 24)).AutoWrapText(true)]];
        if (Topic.Category == TEXT("Blueprints"))
        {
            Details->AddSlot().AutoHeight().Padding(0, 18, 0, 0)
            [SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1).Padding(0, 0, 5, 0)
                [MakeCard(TEXT("VARIABLE = REMEMBERS"), TEXT("Health = 100\nKeeps a piece of information."), FLinearColor(0.4f, 0.85f, 0.65f))]
                + SHorizontalBox::Slot().FillWidth(1).Padding(5, 0, 0, 0)
                [MakeCard(TEXT("FUNCTION = DOES"), TEXT("TakeHit(Damage)\nRuns steps using that information."), FLinearColor(0.4f, 0.7f, 1.0f))]];
        }
        AddSection(TEXT("What it is"), Topic.Summary);
        for (int32 UseCaseIndex = 0; UseCaseIndex < Topic.UseCases.Num(); ++UseCaseIndex)
        {
            const FString Label = FString::Printf(TEXT("Use case %d"), UseCaseIndex + 1);
            Details->AddSlot().AutoHeight().Padding(0, 14, 0, 0)
            [MakeCard(Label, Topic.UseCases[UseCaseIndex], CategoryColour(Topic.Category))];
        }
        AddSection(TEXT("Try it together"), Topic.Example);
        AddSection(TEXT("Remember"), Topic.Pitfall);
        DetailsScroll->ScrollToStart();
    }
};

TSharedRef<SWidget> UQuickGuideWidget::RebuildWidget()
{
    return SNew(SQuickGuide);
}

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Widgets/SWindow.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFrameworkDocumentationNavigationTest,
    "FrameworkDocumentation.Editor.WelcomeNavigation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFrameworkDocumentationNavigationTest::RunTest(const FString& Parameters)
{
    const TSharedRef<SQuickGuide> Guide = SNew(SQuickGuide);
    TestEqual(TEXT("Students are greeted by the introduction"), Guide->Pages->GetActiveWidgetIndex(), 0);
    Guide->BeginButton->SimulateClick();
    TestEqual(TEXT("Begin opens the main documentation page"), Guide->Pages->GetActiveWidgetIndex(), 1);
    TestTrue(TEXT("Topic list is available after Begin"), Guide->TopicList.IsValid());
    const TSharedRef<SQuickGuide> Reopened = SNew(SQuickGuide);
    TestEqual(TEXT("A newly opened panel starts with the introduction"), Reopened->Pages->GetActiveWidgetIndex(), 0);
    const int32 DesignIndex = GetQuickGuideTopics().IndexOfByPredicate([](const FQuickGuideTopic& Topic) {
        return Topic.Title == TEXT("Level Design & Gameplay Flow");
    });
    const int32 CameraIndex = GetQuickGuideTopics().IndexOfByPredicate([](const FQuickGuideTopic& Topic) {
        return Topic.Title == TEXT("First-person vs Third-person");
    });
    if (!TestTrue(TEXT("Design and camera articles can be selected"), DesignIndex != INDEX_NONE && CameraIndex != INDEX_NONE)) return false;
    Guide->SelectTopic(DesignIndex);
    const TSharedRef<SWindow> PreviewWindow = SNew(SWindow)
        .Title(FText::FromString(TEXT("FrameworkDocumentation Test Preview")))
        .ClientSize(FVector2D(1280, 900)).SupportsMaximize(false).SupportsMinimize(false)[Guide];
    FSlateApplication::Get().AddWindow(PreviewWindow);
    auto SavePreview = [this, Guide](const TCHAR* Filename) {
        TArray<FColor> Pixels;
        FIntVector Size = FIntVector::ZeroValue;
        const bool bCaptured = FSlateApplication::Get().TakeScreenshot(Guide, Pixels, Size);
        TestTrue(TEXT("New design content renders"), bCaptured && Size.X > 400 && Size.Y > 250);
        if (bCaptured)
        {
            const FString Output = FPaths::ProjectSavedDir() / TEXT("Automation") / Filename;
            IFileManager::Get().MakeDirectory(*FPaths::GetPath(Output), true);
            TArray64<uint8> PNG;
            FImageUtils::PNGCompressImageArray(Size.X, Size.Y, Pixels, PNG);
            TestTrue(TEXT("Design preview saved"), FFileHelper::SaveArrayToFile(PNG, *Output));
        }
    };
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([Guide, SavePreview, CameraIndex] {
        SavePreview(TEXT("FrameworkDocumentation-LevelDesign.png"));
        Guide->SelectTopic(CameraIndex);
    }, 0.5f));
    ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([SavePreview, PreviewWindow] {
        SavePreview(TEXT("FrameworkDocumentation-Cameras.png"));
        PreviewWindow->RequestDestroyWindow();
    }, 0.5f));
    return true;
}
#endif
