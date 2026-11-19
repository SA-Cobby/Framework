#include "QuickGuideWidget.h"
#include "FrameworkDocumentationPanel.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WidgetSwitcherSlot.h"

void UQuickGuideWidget::BuildDefaultLayout(UWidgetTree* Tree)
{
    if (!Tree || Tree->RootWidget) return;
    UWidgetSwitcher* Switcher = Tree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("PageSwitcher"));
    Tree->RootWidget = Switcher;
    Switcher->bIsVariable = true;
    UCanvasPanel* Canvas = Tree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CustomPage"));
    UFrameworkDocumentationPanel* Documentation = Tree->ConstructWidget<UFrameworkDocumentationPanel>(
        UFrameworkDocumentationPanel::StaticClass(), TEXT("DocumentationPage"));
    Canvas->bIsVariable = true;
    Documentation->bIsVariable = true;
    Switcher->AddChild(Canvas);
    Switcher->AddChild(Documentation);
    for (int32 Index = 0; Index < Switcher->GetChildrenCount(); ++Index)
    {
        UWidgetSwitcherSlot* Slot = CastChecked<UWidgetSwitcherSlot>(Switcher->GetChildAt(Index)->Slot);
        Slot->SetHorizontalAlignment(HAlign_Fill);
        Slot->SetVerticalAlignment(VAlign_Fill);
        Slot->SetPadding(FMargin(0));
    }
    Switcher->SetActiveWidgetIndex(0);
}

void UQuickGuideWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildDefaultLayout(WidgetTree);
}

TSharedRef<SWidget> UQuickGuideWidget::RebuildWidget()
{
    BuildDefaultLayout(WidgetTree);
    return Super::RebuildWidget();
}

UWidgetSwitcher* UQuickGuideWidget::GetPageSwitcher() const
{
    return WidgetTree ? Cast<UWidgetSwitcher>(WidgetTree->RootWidget) : nullptr;
}

void UQuickGuideWidget::ShowDocumentation()
{
    if (UWidgetSwitcher* Switcher = GetPageSwitcher()) Switcher->SetActiveWidgetIndex(1);
}

void UQuickGuideWidget::ShowCustomPage()
{
    if (UWidgetSwitcher* Switcher = GetPageSwitcher()) Switcher->SetActiveWidgetIndex(0);
}
