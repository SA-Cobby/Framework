# Verification - 1.4.0

Validated on Windows x64 using Unreal Engine **5.4.4**, build `35576357`.

- UE 5.4 Win64 editor build and AutomationTool BuildPlugin packaging: **passed**. The package includes both Montserrat font files and their OFL licence, verified against the source files using SHA-256.
- `FrameworkDocumentation.Editor.Smoke`, normal editor with DirectX 11: **passed** with zero errors and warnings. Confirms all 50 complete, uniquely named articles, two nonempty use cases per article (100 total), available UE 5.4 icons, required level-design/genre/camera topics, Editor Utility Widget inheritance, tab construction, close and reopen.
- Font checks: **passed**. Both files identify as static Montserrat Regular/Bold TrueType fonts. The smoke test checks that both typefaces resolve to files in the installed plugin. Slate logs confirm both bundled fonts loaded successfully from the test plugin; no system font installation was used.
- `FrameworkDocumentation.Editor.WelcomeNavigation`: **passed**. The test checks that a new panel shows the introduction, clicks the real Begin button, verifies the main documentation page is selected, and confirms another newly opened panel starts on the introduction. It also selects the level-design and camera articles and captures both after layout.
- `FrameworkDocumentation.Editor.VisualSnapshot`: **passed**. Captures the panel body after layout. The rendered welcome page, level-design page and first/third-person comparison were inspected. Headings, icons, descriptions, two example cards, tutor activity and tips are readable. See `Welcome.png`, `Preview.png`, `LevelDesign.png` and `Cameras.png`.
- Automatic opening: **verified** in a separate normal editor launch. The panel was discovered as a window and the plugin logged `Documentation panel opened automatically after the editor became ready.`
- All three final tests passed with zero errors. The visual snapshot test recorded unrelated Epic Online Services connection warnings from the engine. No font-loading errors were reported. Rendered welcome and article pages were inspected for Montserrat appearance and text wrapping.
- A NullRHI run could not validate native docking: Unreal's generic window implementation raised `GetRestoredDimensions is not expected to be called on this platform`. Use a normal editor for this UI test.
- Visual inspection used Unreal's Slate screenshot API. Manual clicks through every article, animation timing, resizing/docking and preference persistence across restarts were not exercised end to end.
- No game package, other platform, or other engine version was tested. The module is declared `Editor`, so it is excluded from runtime targets.

Build and automation logs are kept alongside the source in `G:\Assistant\FrameworkDocumentationLogs`. The final structured report is `Tests-v1.4/index.json`.
