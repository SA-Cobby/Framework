# FrameworkDocumentation

An editor-only plugin for **Unreal Engine 5.4**, written for students aged **10-18 learning with a tutor**. The dockable, Designer-editable UMG Editor Utility Widget includes 50 topics, labelled icons, colour-coded groups and 100 practical game use cases. All text is bundled and works offline.

The documentation uses **Montserrat Regular and Bold**, bundled in `Resources/Fonts` with their SIL Open Font License. No system font installation is needed. Topic buttons, descriptions, cards, search box and footer use the same font family. Keep the `Resources/Fonts` directory when installing or redistributing the plugin.

The panel opens automatically when the project starts. Its **root Widget Switcher** has an **empty Canvas Panel at index 0** for your custom UI and the **documentation at index 1**. The empty custom page opens first. Add your own welcome widget and Begin button in the UMG Designer; call **Show Documentation** from the button event to select page 1. The previous built-in introduction is replaced by your custom page.

The documentation control retains its optional **0.55-second fade and slide** and footer switches for **Open when this project starts** and **Welcome animation**. Create animations for your custom page with the UMG Designer. Preferences are saved for the current user and project. Automatic opening waits until the main editor window is ready and is skipped during unattended automation, cooking and commandlets. It does not reopen on each Play session or map change.

Each article has **What it is**, **Use case 1**, **Use case 2**, **Try it together**, and **Remember**. Blueprint articles also show side-by-side reminders: **a variable remembers information; a function carries out a task**. Mini activities are intended to be worked through with a tutor; they are not automatic asset-generation buttons.

## Install

1. Close the Unreal Editor.
2. Copy the plugin folder (containing `FrameworkDocumentation.uplugin`, `Source`, and, for a compiled distribution, `Binaries`) into `<YourProject>/Plugins/FrameworkDocumentation`.
3. Open the project in Unreal Engine 5.4. Enable **FrameworkDocumentation** under **Edit > Plugins** if necessary, then restart when prompted.
4. Use **Window > FrameworkDocumentation** to open the panel on the empty custom page. In the Framework project, open **Content/StudioFramework/UI/EUW_FrameworkDocumentation** in the UMG Designer to add your custom UI. In another project, duplicate the plugin template into that Content path first; see [Customising the UMG widget](Documentation/CustomisingUMG.md).

When updating, close the editor and replace the old plugin folder with the new `FrameworkDocumentation` folder. Keep only one installed copy of the plugin in the project/engine plugin paths.

The Win64 compiled distribution requires a matching UE 5.4 build. Source installation requires Visual Studio 2022 with the Unreal C++ toolchain; rebuild the project editor target if prompted or use the BuildPlugin command below. Other engine versions and platforms have not been validated.

## Topics

### Blueprints

- Blueprint Classes
- Variables
- Functions
- Events
- Macros
- Event Dispatchers
- Blueprint Interfaces
- Blueprint Function Libraries

### Gameplay, data and AI

- Actors
- Components
- Pawns / Characters
- Data Assets
- Structs
- Enumerators (Enums)
- AI Controllers
- Behaviour Trees / Blackboards

### Level design

- Level Design & Gameplay Flow
- Blockouts & Scale
- Routes & Player Guidance
- Pacing & Checkpoints
- Space, Cover & Sightlines
- Designing for a Genre

These articles explain how the space supports the actions a game asks players to repeat. Examples connect readable landing spots to platforming, sightlines and connected spaces to shooters, focused rooms to puzzles, and track shapes to racing. These are common patterns to test, not rules that every game must follow.

### Game types and cameras

- Game Genres & Core Loops
- Platformers
- Shooters
- Puzzle Games
- Adventure Games & RPGs
- Racing Games
- Survival Games
- Sandbox Games
- First-person vs Third-person
- Side-view & Top-down

The guide separates genre (what players do) from viewpoint (how they see the game). The camera comparison explains visibility, judging jumps, inspecting objects and leaving room for a third-person camera. Each new topic has two game use cases and a short tutor-led activity.

### Interface and animation
- Widgets
- Project Packaging
- Animation Blueprints
- Animation Blend Spaces
- Animation Chooser Tables

### Common assets

- Static Meshes and Skeletal Meshes
- Materials and Material Instances
- Textures
- Data Tables
- Animation Sequences and Animation Montages
- Levels / Maps and Level Sequences
- Niagara Systems
- Sounds
- Input Actions / Mapping Contexts

For example, the Chooser Table article shows how stance and movement direction can select crouch-forward, crouch-backward, crouch-left or crouch-right animations, with a crouch-idle fallback. The result must still be played by the animation setup.

The complete text is also available in [Documentation/Topics.md](Documentation/Topics.md) for tutor preparation. See the [level-design preview](Documentation/LevelDesign.png) and [camera comparison preview](Documentation/Cameras.png) for the documentation control. The custom first page is intentionally empty.

Search filters topic names, categories, explanations, use cases, activities and tips. The current article stays open when filtering; clear the search to restore every topic. Click any result to read it.

## Implementation

`UQuickGuideWidget` derives from `UEditorUtilityWidget` and hosts a genuine UMG Widget Tree. The shipped template `Content/UI/EUW_FrameworkDocumentation.uasset` is an Editor Utility Widget Blueprint with a root `UWidgetSwitcher`, empty `UCanvasPanel` and reusable `UFrameworkDocumentationPanel`. The documentation control renders the existing article UI in native Slate inside UMG. It can be positioned and composed in the Designer; individual article controls are not separate editable UMG Designer widgets.

The Window menu prefers `/Game/StudioFramework/UI/EUW_FrameworkDocumentation`, allowing you to customise a project copy without plugin updates overwriting it. It falls back to `/FrameworkDocumentation/UI/EUW_FrameworkDocumentation` when no project copy exists. Both use the `QuickGuideWidget` parent. Edit the topic authoring script and regenerate/rebuild to change the documentation text.

The plugin has no runtime module, does not alter gameplay assets and is excluded from game builds. Reading the Chooser topic does not enable or require Chooser. In UE 5.4, Chooser is **Beta**; enable it separately if following the example.

## Build and check

Run in PowerShell, adapting the engine and output paths:

```powershell
& 'D:\Engines\UE_5.4\Engine\Build\BatchFiles\RunUAT.bat' BuildPlugin -Plugin='G:\Assistant\FrameworkDocumentation\FrameworkDocumentation.uplugin' -Package='G:\Assistant\FrameworkDocumentationBuild' -TargetPlatforms=Win64 -Rocket
```

In a UE 5.4 test project with the plugin enabled, run the **FrameworkDocumentation.Editor** tests from **Tools > Test Automation** (enable the built-in **Session Frontend** plugin if needed). **UMGLayout** verifies the saved Blueprint, root Switcher, empty Canvas, documentation page and page-switching helper functions. **Smoke** validates documentation completeness and tab close/reopen. **VisualSnapshot** switches the actual UMG shell to documentation and captures its rendering. **WelcomeNavigation** also exercises the retained internal Slate navigation and captures sample articles.

Manual acceptance checks: confirm automatic opening on project startup; open from Window and click Begin; select all 50 topics; search for `platformer`, `sightline`, `first-person` and a nonexistent term; clear search; resize and dock the panel; close and reopen it; change levels with the panel open; turn off automatic opening, close the panel and restart to confirm it stays closed; turn off welcome animation and reopen to check reduced motion; confirm a packaged game does not include this editor module.

## References

- [UE 5.4 release notes: Choosers and Proxy Tables (Beta)](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5.4-release-notes?application_version=5.4)
- [Editor Utility Widgets](https://dev.epicgames.com/documentation/en-us/unreal-engine/editor-utility-widgets-in-unreal-engine?application_version=5.4)
- [Packaging Unreal Engine projects](https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-your-project?application_version=5.4)
- [Blueprint variables](https://dev.epicgames.com/documentation/en-us/unreal-engine/blueprint-variables-in-unreal-engine?application_version=5.4)
- [Blueprint functions](https://dev.epicgames.com/documentation/en-us/unreal-engine/functions-in-unreal-engine?application_version=5.4)
- [Blueprint macros](https://dev.epicgames.com/documentation/en-us/unreal-engine/macros-in-unreal-engine?application_version=5.4)
- [Third Person template and camera](https://dev.epicgames.com/documentation/en-us/unreal-engine/third-person-template-in-unreal-engine?application_version=5.4)
