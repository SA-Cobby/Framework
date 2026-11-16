#include "QuickGuideFonts.h"
#include "Fonts/CompositeFont.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateTypes.h"

FSlateFontInfo QuickGuideFonts::Get(const FName& Typeface, float Size)
{
    // Resolve from the installed plugin, never from a machine's system fonts.
    static const TSharedPtr<const FCompositeFont> Font = []() -> TSharedPtr<const FCompositeFont> {
        const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("FrameworkDocumentation"));
        checkf(Plugin.IsValid(), TEXT("FrameworkDocumentation must be installed to resolve its fonts."));
        const FString Directory = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Resources/Fonts"));
        TSharedRef<FCompositeFont> Composite = MakeShared<FCompositeFont>();
        Composite->DefaultTypeface.Fonts.Emplace(TEXT("Regular"), Directory / TEXT("Montserrat-Regular.ttf"),
            EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
        Composite->DefaultTypeface.Fonts.Emplace(TEXT("Bold"), Directory / TEXT("Montserrat-Bold.ttf"),
            EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
        return Composite;
    }();
    return FSlateFontInfo(Font, Size, Typeface);
}

const FSearchBoxStyle& QuickGuideFonts::SearchBoxStyle()
{
    static const FSearchBoxStyle Style = [] {
        FSearchBoxStyle Result = FAppStyle::Get().GetWidgetStyle<FSearchBoxStyle>(TEXT("SearchBox"));
        Result.TextBoxStyle.TextStyle.SetFont(Get(TEXT("Regular"), 12));
        Result.SetActiveFont(Get(TEXT("Regular"), 12));
        return Result;
    }();
    return Style;
}
