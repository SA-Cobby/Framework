# Flares Toolset — Unreal Engine 5.4 / Win64

**Flares Toolset** is an advanced rendering and cinematic post-processing plugin for Unreal Engine 5.4. It packages:
1. **Anamorphic & Multi-Axis Lens Flares** with spectral dispersion (chromatic aberration) and soft-knee thresholding.
2. **Sensor Blooming & Smear (CCD/CMOS Overload)** replicating vertical charge register overflow and horizontal line saturation.
3. **Anamorphic Lens Distortion** with cylindrical barrel/pincushion curvature, squeeze ratios, and aspect-ratio tuning.
4. **Radial Chromatic Aberration** scaling aggressively into screen corners, selectively gated on high-contrast luma edges, and dynamically modulated on a normalized 0–1 scale via **Light Function materials** or **Material Parameter Collections (MPC)**.
5. **Stepped Cel Shading & Manga / Comic Halftone Shading** with Ben-Day print dots, crosshatching, diffuse quantization, and custom color ramps.
6. **Ink Outlines** with planar slope compensation, adaptive anime-style surface inking, and distance-based depth attenuation.
7. **One-Click Artist Presets System (`UFlaresPreset` DataAsset)** with built-in instant looks and dropdown selection in the editor panel.
8. **Diagnostic View Modes** for real-time look development in editor viewports and PIE.

---

## Features

### 1. Optical Lens Flares
- **Anamorphic Streaks & Multi-Axis Filters**: Generate wide cinematic horizontal anamorphic streaks, vertical streaks, cross flares (2-axis), or 3-axis star flares.
- **Spectral Dispersion (Chromatic Aberration)**: Splits light into realistic chromatic fringing along the streak axis (cool and warm spectral fringes).
- **Soft-Knee Thresholding**: Smooth transition around `FlareThreshold` to prevent temporal popping as bright lights move across pixels.
- **Dilation-Based Kernel**: Uses 6 dilated 5-tap filter passes for smooth, continuous light streaks without discrete sample gaps.
- **Quality Options**: Choose between quarter-resolution (optimized performance) or half-resolution extraction (captures finer specular highlights).

### 2. Advanced Sensor Blooming & Smear (CCD/CMOS Overload)
- **Sensor Overload Simulation**: Replicates how retro sci-fi video feeds, security cameras, or high-end digital sensors vertically or horizontally bleed when hit with overwhelming light sources (welding arcs, sun-blinds, explosions, headlights).
- **Vertical CCD Column Smear**: Models charge leakage along vertical shift registers across the entire sensor column.
- **Horizontal CMOS Smear**: Models line-readout saturation across sensor scan lines.
- **High Overload Threshold**: Configured with a dedicated `SmearThreshold` (default `15.0`) so normal highlights only flare, while extreme sources trigger register bleeding.
- **Smear Width & Falloff**: Calibrate column pixel thickness and exponential decay.

### 3. Anamorphic Barrel & Pincushion Distortion
- **Cylindrical Anamorphic Screen Warp**: Simulates the physical optical distortion of anamorphic lenses (such as Panavision C-Series or Kowa Prominars).
- **Anamorphic Squeeze Ratio**: Control the horizontal compression factor ($1.0\times$ spherical up to $3.0\times$ ultra-wide anamorphic).
- **Curvature Axis Ratio**: Fine-tune the balance between horizontal barrel curvature and vertical flatness to match specific real-world lens kits.
- **Scale Compensation**: Zoom crop factor to eliminate black border stretching.

### 4. Advanced Chromatic Aberration (Radial Falloff & Light Function / MPC Control)
- **Non-Uniform Radial Dispersion**: Rather than shifting RGB channels uniformly across the whole screen, chromatic displacement ramps aggressively from zero in the optical center to maximum in the screen corners ($r^{\text{RadialExponent}}$).
- **High-Contrast Luma Edge Gating**: Only surfaces with strong luma gradients or specular highlights undergo chromatic separation. Flat surfaces and text remain clean without muddying the image.
- **Light Function & MPC Modulation (0–1 Normalized Scale)**:
  - Assign a **Material Parameter Collection** (`AberrationMPC`) and parameter name (`AberrationParameterName`, default `"AberrationStrength"`).
  - Any **Light Function Material**, Blueprint, or Level Sequence modifying this scalar parameter (0.0 to 1.0) dynamically modulates the screen aberration strength in real time!
  - Also directly controllable through Blueprints via `SetAdvancedChromaticAberration` and `SetAberrationMPC`.

### 5. Stepped Cel Shading, Halftone & Manga Crosshatching
- **Diffuse Quantization**: Derives lighting luminance by dividing scene HDR radiance by GBuffer base-color albedo, quantizing luminance into 2 to 8 distinct lighting bands.
- **Comic Halftone & Manga Hatching**:
  - **Ben-Day Dots** (45° rotated comic print dot grid).
  - **Manga Crosshatch** (dual diagonal lines scaling with shadow depth).
  - **Diagonal Hatching** (single-direction sketch lines).
  - Dots and lines scale organically with shadow depth, bringing graphic novel art styles (*Spider-Verse*, *Arcane*, manga) directly to life without per-asset material graphs.
- **Emissive & Sky Protection**: Pixels exceeding `CelEmissiveThreshold`, unlit shading models, and infinite-depth background (sky) bypass cel quantization.
- **Colour Ramp Tinting**: Endpoint shadow/highlight gradients and 64-sample `UCurveLinearColor` curve assets (banded or continuous).
- **Adaptive Inking (Anime-style outlines)**: Tints outline ink with darkened surface albedo for soft, colorful lines (e.g. deep maroon on skin, navy on blue clothing) instead of uniform harsh black lines.
- **Distance-Dependent Attenuation**: Attenuates outline width and opacity over distance (`OutlineFadeStartDistance` to `OutlineFadeEndDistance`).

### 6. One-Click Artist Presets (`UFlaresPreset`)
Instant look development from the **Window → Flares Toolset** panel or via Blueprints:
- **`Panavision 70s Anamorphic`**: Warm horizontal streaks, 2.0x squeeze, barrel curvature, and corner chromatic dispersion.
- **`Retro CCTV Overload`**: Vertical CCD shift-register smear, CRT pincushion warp, and pale violet overload tint.
- **`Studio Anime Cel`**: Stepped diffuse lighting, adaptive anime-style surface inking, distance fade, and subtle soft streaks.
- **`Graphic Novel Noir`**: High-contrast black outlines, manga crosshatch shadows, and 2 stark cel bands.
- **`Cyberpunk Neon`**: Multi-axis cross flares with spectral dispersion, purple/cyan colour ramp, and edge-gated chromatic aberration.
- **`Pop-Art Comic Halftone`**: 45° Ben-Day dots in deep shadows with crisp cartoon outlines.

### 7. Diagnostic View Modes
- **Normal**: Full composite of all active effects.
- **Isolated Flares**: Visualizes isolated flare bloom over a black background.
- **Isolated Outlines**: Visualizes ink outlines as black lines over a pure white canvas.
- **Bright Extraction Mask**: Visualizes luminance pixels exceeding `FlareThreshold`.
- **Cel Lighting Bands Only**: Visualizes quantized diffuse lighting bands without base color texture interference.
- **Isolated Sensor Smear**: Isolates vertical/horizontal sensor register smear over black.
- **Lens Distortion Calibration Grid**: Overlays an alignment grid to accurately calibrate barrel/pincushion curvature.

---

## Controls Reference

### Halftone & Manga Hatching
| Parameter | Default | Range | Description |
| :--- | :--- | :--- | :--- |
| `bHalftoneEnabled` | `false` | Boolean | Enable halftone dots or hatching in shadows |
| `HalftoneStyle` | `Dots` | Enum | Ben-Day Dots (45°), Manga Crosshatch, or Diagonal Lines |
| `HalftoneDensity` | `12.0 px` | 4 – 64 px | Grid pattern spacing (smaller = finer dots) |
| `HalftoneStrength` | `0.85` | 0 – 1 | Blending intensity into shadow areas |
| `HalftoneThreshold` | `0.35` | 0.05 – 1.0 | Normalized lighting level where halftone begins |
| `HalftoneSoftness` | `0.15` | 0.01 – 0.5 | Edge softness of dots or hatching lines |
| `HalftoneTint` | `(0.02, 0.02, 0.02)` | Color | Ink color for the halftone pattern |

### Sensor Smear (CCD / CMOS Overload)
| Parameter | Default | Range | Description |
| :--- | :--- | :--- | :--- |
| `bSensorSmearEnabled` | `false` | Boolean | Enable sensor register bleeding |
| `SmearOrientation` | `Vertical` | Enum | Vertical (CCD Column Overload) or Horizontal (CMOS Line Overload) |
| `SmearThreshold` | `15.0` | 1 – 100,000 | Overload scene-linear brightness threshold |
| `SmearIntensity` | `0.6` | 0 – 10 | Intensity multiplier for sensor smear |
| `SmearLength` | `0.85` | 0.05 – 1.0 | Smear bleed length along sensor column/row |
| `SmearWidth` | `1.5 px` | 0.5 – 16 px | Smear column pixel width |
| `SmearTint` | Pale Violet `(0.92, 0.88, 1.0)` | Color | Characteristic sensor bleed color tint |
| `SmearFalloff` | `1.5` | 0.1 – 5.0 | Exponential falloff rate away from overload source |

### Anamorphic Lens Distortion
| Parameter | Default | Range | Description |
| :--- | :--- | :--- | :--- |
| `bLensDistortionEnabled` | `false` | Boolean | Enable cylindrical anamorphic screen warp |
| `DistortionStrength` | `0.15` | -1.0 – 1.0 | Positive = Barrel distortion, Negative = Pincushion |
| `AnamorphicSqueeze` | `2.0x` | 1.0 – 3.0 | Horizontal squeeze ratio |
| `CurvatureAxisRatio` | `0.35` | 0.0 – 2.0 | Balance between horizontal and vertical curvature |
| `DistortionScale` | `1.02` | 0.8 – 1.5 | Zoom compensation factor to hide edge borders |

### Radial Chromatic Aberration
| Parameter | Default | Range | Description |
| :--- | :--- | :--- | :--- |
| `bAdvancedChromaticAberration` | `false` | Boolean | Enable radial edge-gated chromatic aberration |
| `AberrationStrength` | `0.5` | 0 – 1 | Base normalized 0–1 strength |
| `AberrationRadialExponent` | `2.5` | 1.0 – 6.0 | Corner ramp exponent (higher = sharper center) |
| `AberrationEdgeThreshold` | `0.15` | 0.01 – 1.0 | Luma contrast threshold for edge gating |
| `AberrationEdgeSensitivity` | `0.85` | 0.0 – 1.0 | 1.0 = strictly edge-gated, 0.0 = uniform edge falloff |
| `AberrationMPC` | `None` | Soft Object | Optional Material Parameter Collection to modulate strength |
| `AberrationParameterName` | `"AberrationStrength"` | Name | Scalar parameter in the MPC (0–1 normalized) |

---

## Blueprint API

All settings and presets can be dynamically altered at runtime through the Blueprint function library under `Flares Toolset`:

- **`Apply Preset`**: Applies any `UFlaresPreset` data asset.
- **`Apply Builtin Preset`**: Applies a preset by name (`Panavision 70s Anamorphic`, `Retro CCTV Overload`, `Studio Anime Cel`, `Graphic Novel Noir`, `Cyberpunk Neon`, `Pop-Art Comic Halftone`).
- **`Get Builtin Presets`**: Returns array of all available built-in preset names.
- **`Set Halftone Shading`**: Configures halftone toggle, style (Dots/Crosshatch/Diagonal), density, strength, threshold, softness, and tint.
- **`Set Sensor Smear`**: Configures smear toggle, orientation, intensity, overload threshold, length, width, tint, and falloff.
- **`Set Lens Distortion`**: Configures barrel/pincushion strength, anamorphic squeeze factor, curvature axis ratio, and zoom scale.
- **`Set Advanced Chromatic Aberration`**: Configures normalized strength (0–1), radial exponent, edge threshold, and edge sensitivity.
- **`Set Aberration MPC`**: Dynamically binds a Material Parameter Collection and scalar parameter name.
- **`Set Flares`** & **`Set Flare Spectral Dispersion`**: Configures streak flares, spectral dispersion, and soft-knee transitions.
- **`Set Cel Shading`** & **`Set Color Ramp`**: Configures cel banding, white point, shadow floor, and curve palettes.
- **`Set Ink Outlines`** & **`Set Adaptive Inking`**: Configures outlines, surface-tinted anime inking, and distance fading.
- **`Set Debug View Mode`**: Switches diagnostic view mode dynamically.
