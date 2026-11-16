# FrameworkDocumentation

An editor-only plugin for **Unreal Engine 5.4**, written for students aged **10-18 learning with a tutor**. The dockable Editor Utility Widget includes 50 topics, labelled icons, colour-coded groups and 100 practical game use cases. All text is bundled and works offline.

The documentation uses **Montserrat Regular and Bold**, bundled in `Resources/Fonts` with their SIL Open Font License. No system font installation is needed. The welcome screen, topic buttons, descriptions, cards, search box and footer use the same font family. Keep the `Resources/Fonts` directory when installing or redistributing the plugin.

The panel opens automatically when the project starts and greets students with **Welcome to the Studio Framework Project**. A short introduction explains how to explore topics, read game examples and practise with a tutor. Students click **Begin** to enter the main documentation page. Each newly opened panel starts with this introduction.

The panel uses a gentle **0.55-second fade and slide** when opened. After Begin, the footer switches **Open when this project starts** and **Welcome animation** let tutors change either behaviour. Preferences are saved for the current user and project. Automatic opening waits until the main editor window is ready and is skipped during unattended automation, cooking and commandlets. It does not reopen on each Play session or map change.

Each article has **What it is**, **Use case 1**, **Use case 2**, **Try it together**, and **Remember**. Blueprint articles also show side-by-side reminders: **a variable remembers information; a function carries out a task**. Mini activities are intended to be worked through with a tutor; they are not automatic asset-generation buttons.

## Install

1. Close the Unreal Editor.
2. Copy the plugin folder (containing `FrameworkDocumentation.uplugin`, `Source`, and, for a compiled distribution, `Binaries`) into `<YourProject>/Plugins/FrameworkDocumentation`.
3. Open the project in Unreal Engine 5.4. Enable **FrameworkDocumentation** under **Edit > Plugins** if necessary, then restart when prompted.
4. The welcome panel opens as the project starts. Click **Begin** to enter the documentation. You can also select **Window > FrameworkDocumentation** in the main editor menu. Dock or resize the panel as desired.

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

The complete text is also available in [Documentation/Topics.md](Documentation/Topics.md) for tutor preparation. See the [welcome preview](Documentation/Welcome.png), [level-design preview](Documentation/LevelDesign.png) and [camera comparison preview](Documentation/Cameras.png) for the student layout.

Search filters topic names, categories, explanations, use cases, activities and tips. The current article stays open when filtering; clear the search to restore every topic. Click any result to read it.

## Implementation

`UQuickGuideWidget` derives from `UEditorUtilityWidget`. Its native Slate implementation builds the interface without generated binary Blueprint assets, Python or setup scripts. The Window menu creates the widget in a dockable editor tab. This is a native Editor Utility Widget, not a pre-authored UMG Designer asset. Edit the text in `Source/FrameworkDocumentation/Private/QuickGuideTopics.cpp` and rebuild to change documentation.

The plugin has no runtime module, does not alter gameplay assets and is excluded from game builds. Reading the Chooser topic does not enable or require Chooser. In UE 5.4, Chooser is **Beta**; enable it separately if following the example.

## Build and check

Run in PowerShell, adapting the engine and output paths:

```powershell
& 'D:\Engines\UE_5.4\Engine\Build\BatchFiles\RunUAT.bat' BuildPlugin -Plugin='G:\Assistant\FrameworkDocumentation\FrameworkDocumentation.uplugin' -Package='G:\Assistant\FrameworkDocumentationBuild' -TargetPlatforms=Win64 -Rocket
```

In a UE 5.4 test project with the plugin enabled, run the **FrameworkDocumentation.Editor** tests from **Tools > Test Automation** (enable the built-in **Session Frontend** plugin if needed). The smoke check validates documentation completeness and native widget inheritance, constructs the tab, and closes/reopens it. **WelcomeNavigation** clicks the real Begin button and checks that it switches from the introduction to the main documentation page. **VisualSnapshot** saves a rendered welcome-page screenshot for inspection.

Manual acceptance checks: confirm automatic opening on project startup; open from Window and click Begin; select all 50 topics; search for `platformer`, `sightline`, `first-person` and a nonexistent term; clear search; resize and dock the panel; close and reopen it; change levels with the panel open; turn off automatic opening, close the panel and restart to confirm it stays closed; turn off welcome animation and reopen to check reduced motion; confirm a packaged game does not include this editor module.

## References

- [UE 5.4 release notes: Choosers and Proxy Tables (Beta)](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5.4-release-notes?application_version=5.4)
- [Editor Utility Widgets](https://dev.epicgames.com/documentation/en-us/unreal-engine/editor-utility-widgets-in-unreal-engine?application_version=5.4)
- [Packaging Unreal Engine projects](https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-your-project?application_version=5.4)
- [Blueprint variables](https://dev.epicgames.com/documentation/en-us/unreal-engine/blueprint-variables-in-unreal-engine?application_version=5.4)
- [Blueprint functions](https://dev.epicgames.com/documentation/en-us/unreal-engine/functions-in-unreal-engine?application_version=5.4)
- [Blueprint macros](https://dev.epicgames.com/documentation/en-us/unreal-engine/macros-in-unreal-engine?application_version=5.4)
- [Third Person template and camera](https://dev.epicgames.com/documentation/en-us/unreal-engine/third-person-template-in-unreal-engine?application_version=5.4)
