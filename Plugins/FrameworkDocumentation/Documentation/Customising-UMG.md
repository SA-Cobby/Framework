# Customising the UMG panel

The project working copy is `Content/StudioFramework/UI/EUW_FrameworkDocumentation`. Open it in Unreal's Widget Blueprint Designer after restarting the editor with version 1.5 or later installed.

The hierarchy is:

```text
EUW_FrameworkDocumentation (Editor Utility Widget)
└── PageSwitcher (UMG Widget Switcher, root)
    ├── [0] CustomPage (empty Canvas Panel)
    └── [1] DocumentationPage (Framework Documentation widget)
```

The active index starts at **0**, so the first page is intentionally blank. It is ready for your own introduction, branding or custom User Widget. Select index **1** in the Designer to preview the documentation.

## Add your own introduction

1. Add your custom User Widget, images, text and buttons inside `CustomPage`.
2. Set anchors and offsets to fit the Canvas Panel as needed.
3. Add a **Begin** button. In its **On Clicked** event, call **Show Documentation** on **Self**. You can also get `PageSwitcher` and set its active widget index to **1**.
4. Compile and save. Open **Window > FrameworkDocumentation** to run your version.

**Show Custom Page** switches back to index **0**. The built-in introduction is bypassed on the documentation page, so clicking your own Begin button opens the main topic browser directly.

## Display custom UI alongside the documentation

A Widget Switcher shows one page at a time. To display your widget and the documentation simultaneously, arrange a Horizontal Box or Overlay on its second page, and place your custom widget beside the **Framework Documentation** control. Keep `CustomPage` first and the documentation layout second. The control is reusable and resizable in UMG; its internal article rendering is implemented in native Slate.

## Preserve your customisations

The Window menu loads `/Game/StudioFramework/UI/EUW_FrameworkDocumentation` when it exists. Plugin updates must preserve this project asset. The separate template `/FrameworkDocumentation/UI/EUW_FrameworkDocumentation` stays in the plugin as a fallback for projects without a working copy.

For another project, enable **Show Plugin Content**, duplicate the template into `Content/StudioFramework/UI` using the same asset name, and customise the copy. Keep the parent class **QuickGuideWidget**, the root Widget Switcher and the page order so the helper functions continue to work.

This is an editor-only utility; it is not intended to be added to a packaged game's viewport.
