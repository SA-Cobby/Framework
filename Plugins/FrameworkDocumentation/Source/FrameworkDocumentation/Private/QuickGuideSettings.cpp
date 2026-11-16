#include "QuickGuideSettings.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
    const TCHAR* Section = TEXT("FrameworkDocumentation");
    bool Read(const TCHAR* Key)
    {
        bool bEnabled = true;
        GConfig->GetBool(Section, Key, bEnabled, GEditorPerProjectIni);
        return bEnabled;
    }
    void Write(const TCHAR* Key, bool bEnabled)
    {
        GConfig->SetBool(Section, Key, bEnabled, GEditorPerProjectIni);
        GConfig->Flush(false, GEditorPerProjectIni);
    }
}

bool QuickGuideSettings::OpenOnStartup() { return Read(TEXT("OpenOnStartup")); }
bool QuickGuideSettings::PlayWelcomeAnimation() { return Read(TEXT("PlayWelcomeAnimation")); }
void QuickGuideSettings::SetOpenOnStartup(bool bEnabled) { Write(TEXT("OpenOnStartup"), bEnabled); }
void QuickGuideSettings::SetPlayWelcomeAnimation(bool bEnabled) { Write(TEXT("PlayWelcomeAnimation"), bEnabled); }
