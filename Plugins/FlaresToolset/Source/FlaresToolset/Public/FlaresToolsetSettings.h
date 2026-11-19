#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FlaresToolsetTypes.h"
#include "FlaresToolsetSettings.generated.h"

class UCurveLinearColor;
class UMaterialParameterCollection;

/** Runtime settings for Flares Toolset: lens flares, cel shading, ink outlines, halftone, sensor smear, lens distortion, and chromatic aberration. */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Flares Toolset"))
class FLARESTOOLSET_API UFlaresToolsetSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    // ==========================================
    // Lens Flares
    // ==========================================
    UPROPERTY(Config, EditAnywhere, Category="Lens Flares", meta=(DisplayName="Enable Lens Flares"))
    bool bFlaresEnabled = false;

    UPROPERTY(Config, EditAnywhere, Category="Lens Flares", meta=(EditCondition="bFlaresEnabled"))
    EFlaresToolsetFlareType FlareType = EFlaresToolsetFlareType::Anamorphic;

    UPROPERTY(Config, EditAnywhere, Category="Lens Flares", meta=(ClampMin="0", ClampMax="10", EditCondition="bFlaresEnabled"))
    float FlareIntensity = 0.4f;

    UPROPERTY(Config, EditAnywhere, Category="Lens Flares", meta=(ClampMin="0", ClampMax="100000", EditCondition="bFlaresEnabled", ToolTip="Scene-linear brightness threshold, independent of pre-exposure."))
    float FlareThreshold = 4.0f;

    UPROPERTY(Config, EditAnywhere, Category="Lens Flares", meta=(ClampMin="0", ClampMax="1", EditCondition="bFlaresEnabled", ToolTip="Soft-knee transition around threshold to prevent temporal highlight popping."))
    float FlareSoftKnee = 0.2f;

    UPROPERTY(Config, EditAnywhere, Category="Lens Flares", meta=(ClampMin="0.02", ClampMax="0.75", EditCondition="bFlaresEnabled", ToolTip="Approximate streak half-length as a fraction of viewport width."))
    float FlareLength = 0.25f;

    UPROPERTY(Config, EditAnywhere, Category="Lens Flares", meta=(ClampMin="-90", ClampMax="90", EditCondition="bFlaresEnabled", Units="deg", ToolTip="Angular offset for streak orientation."))
    float FlareRotation = 0.0f;

    UPROPERTY(Config, EditAnywhere, Category="Lens Flares", meta=(EditCondition="bFlaresEnabled"))
    FLinearColor FlareTint = FLinearColor(0.35f, 0.65f, 1.0f, 1.0f);

    UPROPERTY(Config, EditAnywhere, Category="Lens Flares", meta=(ClampMin="0", ClampMax="1", EditCondition="bFlaresEnabled", ToolTip="Spectral color dispersion / chromatic aberration along the streak direction."))
    float FlareChromaticDispersion = 0.25f;

    UPROPERTY(Config, EditAnywhere, Category="Lens Flares", meta=(EditCondition="bFlaresEnabled", ToolTip="Half-resolution extraction instead of quarter resolution. Helps smaller highlights; costs more GPU work."))
    bool bHighQualityFlares = false;

    // ==========================================
    // Sensor Smear (CCD/CMOS Overload)
    // ==========================================
    UPROPERTY(Config, EditAnywhere, Category="Sensor Smear", meta=(DisplayName="Enable Sensor Smear", ToolTip="Replicates vertical CCD column register overflow or horizontal CMOS line overload from extreme bright lights."))
    bool bSensorSmearEnabled = false;

    UPROPERTY(Config, EditAnywhere, Category="Sensor Smear", meta=(EditCondition="bSensorSmearEnabled"))
    EFlaresToolsetSmearOrientation SmearOrientation = EFlaresToolsetSmearOrientation::Vertical;

    UPROPERTY(Config, EditAnywhere, Category="Sensor Smear", meta=(ClampMin="1", ClampMax="100000", EditCondition="bSensorSmearEnabled", ToolTip="High scene-linear brightness threshold required to trigger sensor charge overflow."))
    float SmearThreshold = 15.0f;

    UPROPERTY(Config, EditAnywhere, Category="Sensor Smear", meta=(ClampMin="0", ClampMax="10", EditCondition="bSensorSmearEnabled"))
    float SmearIntensity = 0.6f;

    UPROPERTY(Config, EditAnywhere, Category="Sensor Smear", meta=(ClampMin="0.05", ClampMax="1", EditCondition="bSensorSmearEnabled", ToolTip="Length of the smear bleeding along the sensor column or row."))
    float SmearLength = 0.85f;

    UPROPERTY(Config, EditAnywhere, Category="Sensor Smear", meta=(ClampMin="0.5", ClampMax="16", EditCondition="bSensorSmearEnabled", ToolTip="Width of the smeared column/row in pixels."))
    float SmearWidth = 1.5f;

    UPROPERTY(Config, EditAnywhere, Category="Sensor Smear", meta=(EditCondition="bSensorSmearEnabled", ToolTip="Sensor bleed tint color (e.g. pale violet or warm white)."))
    FLinearColor SmearTint = FLinearColor(0.92f, 0.88f, 1.0f, 1.0f);

    UPROPERTY(Config, EditAnywhere, Category="Sensor Smear", meta=(ClampMin="0.1", ClampMax="5", EditCondition="bSensorSmearEnabled", ToolTip="Exponential falloff rate away from the overload center."))
    float SmearFalloff = 1.5f;

    // ==========================================
    // Anamorphic Lens Distortion
    // ==========================================
    UPROPERTY(Config, EditAnywhere, Category="Lens Distortion", meta=(DisplayName="Enable Anamorphic Distortion", ToolTip="Non-destructive optical screen warp simulating cylindrical anamorphic lens barrel/pincushion curvature."))
    bool bLensDistortionEnabled = false;

    UPROPERTY(Config, EditAnywhere, Category="Lens Distortion", meta=(ClampMin="-1", ClampMax="1", EditCondition="bLensDistortionEnabled", ToolTip="Distortion strength: positive values produce barrel distortion, negative values produce pincushion distortion."))
    float DistortionStrength = 0.15f;

    UPROPERTY(Config, EditAnywhere, Category="Lens Distortion", meta=(ClampMin="1", ClampMax="3", EditCondition="bLensDistortionEnabled", ToolTip="Anamorphic squeeze factor (e.g. 2.0x for standard anamorphic, 1.33x for 16:9). Warps horizontal axis preferentially."))
    float AnamorphicSqueeze = 2.0f;

    UPROPERTY(Config, EditAnywhere, Category="Lens Distortion", meta=(ClampMin="0", ClampMax="2", EditCondition="bLensDistortionEnabled", ToolTip="Vertical vs horizontal curvature balance. Real anamorphic lenses warp heavily on horizontal with flatter vertical curvature."))
    float CurvatureAxisRatio = 0.35f;

    UPROPERTY(Config, EditAnywhere, Category="Lens Distortion", meta=(ClampMin="0.8", ClampMax="1.5", EditCondition="bLensDistortionEnabled", ToolTip="Zoom scale compensation to crop black warped borders."))
    float DistortionScale = 1.02f;

    // ==========================================
    // Radial Chromatic Aberration
    // ==========================================
    UPROPERTY(Config, EditAnywhere, Category="Radial Chromatic Aberration", meta=(DisplayName="Enable Radial Aberration", ToolTip="Advanced chromatic aberration scaling aggressively toward corners and gated by high-contrast luma edges."))
    bool bAdvancedChromaticAberration = false;

    UPROPERTY(Config, EditAnywhere, Category="Radial Chromatic Aberration", meta=(ClampMin="0", ClampMax="1", EditCondition="bAdvancedChromaticAberration", ToolTip="Normalized 0-1 baseline strength."))
    float AberrationStrength = 0.5f;

    UPROPERTY(Config, EditAnywhere, Category="Radial Chromatic Aberration", meta=(ClampMin="1", ClampMax="6", EditCondition="bAdvancedChromaticAberration", ToolTip="Radial falloff exponent. Higher values keep the center pristine and ramp sharply at borders."))
    float AberrationRadialExponent = 2.5f;

    UPROPERTY(Config, EditAnywhere, Category="Radial Chromatic Aberration", meta=(ClampMin="0.01", ClampMax="1", EditCondition="bAdvancedChromaticAberration", ToolTip="Luma gradient threshold. Aberration selectively triggers on high-contrast edges and specular highlights."))
    float AberrationEdgeThreshold = 0.15f;

    UPROPERTY(Config, EditAnywhere, Category="Radial Chromatic Aberration", meta=(ClampMin="0", ClampMax="1", EditCondition="bAdvancedChromaticAberration", ToolTip="1.0 = strictly gated to high-contrast edges; 0.0 = uniform radial aberration across entire corners."))
    float AberrationEdgeSensitivity = 0.85f;

    UPROPERTY(Config, EditAnywhere, Category="Radial Chromatic Aberration", meta=(EditCondition="bAdvancedChromaticAberration", DisplayName="Material Parameter Collection (MPC)", ToolTip="Optional MPC used by Light Function materials or Blueprints to drive aberration strength in real time on a 0-1 normalized scale."))
    TSoftObjectPtr<UMaterialParameterCollection> AberrationMPC;

    UPROPERTY(Config, EditAnywhere, Category="Radial Chromatic Aberration", meta=(EditCondition="bAdvancedChromaticAberration", ToolTip="Scalar parameter name inside the Material Parameter Collection (normalized 0-1)."))
    FName AberrationParameterName = FName(TEXT("AberrationStrength"));

    // ==========================================
    // Cel Shading
    // ==========================================
    UPROPERTY(Config, EditAnywhere, Category="Cel Shading", meta=(DisplayName="Enable Cel Shading"))
    bool bCelShading = false;

    UPROPERTY(Config, EditAnywhere, Category="Cel Shading", meta=(ClampMin="2", ClampMax="8", EditCondition="bCelShading", ToolTip="Number of diffuse lighting quantization bands."))
    int32 CelBands = 3;

    UPROPERTY(Config, EditAnywhere, Category="Cel Shading", meta=(ClampMin="0", ClampMax="1", EditCondition="bCelShading", ToolTip="Blend between original lighting (0) and quantized cel bands (1)."))
    float CelStrength = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category="Cel Shading", meta=(ClampMin="0", ClampMax="1", EditCondition="bCelShading", ToolTip="Minimum diffuse lighting in the darkest band."))
    float CelShadowBrightness = 0.12f;

    UPROPERTY(Config, EditAnywhere, Category="Cel Shading", meta=(ClampMin="0.05", ClampMax="20", EditCondition="bCelShading", ToolTip="Lighting level mapped to the brightest band. Lower values brighten the bands."))
    float CelWhitePoint = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category="Cel Shading", meta=(ClampMin="0.1", ClampMax="100", ToolTip="Pixels brighter than this scene-linear luminance keep their original colour, including outlines."))
    float CelEmissiveThreshold = 4.0f;

    // ==========================================
    // Halftone & Manga Hatching
    // ==========================================
    UPROPERTY(Config, EditAnywhere, Category="Cel Shading|Halftone", meta=(DisplayName="Enable Halftone / Hatching", ToolTip="Generates comic Ben-Day dots or manga crosshatching patterns in the darkest cel shadow regions."))
    bool bHalftoneEnabled = false;

    UPROPERTY(Config, EditAnywhere, Category="Cel Shading|Halftone", meta=(EditCondition="bHalftoneEnabled"))
    EFlaresToolsetHalftoneStyle HalftoneStyle = EFlaresToolsetHalftoneStyle::Dots;

    UPROPERTY(Config, EditAnywhere, Category="Cel Shading|Halftone", meta=(ClampMin="4", ClampMax="64", EditCondition="bHalftoneEnabled", ToolTip="Pattern grid scale in pixels (smaller = denser pattern)."))
    float HalftoneDensity = 12.0f;

    UPROPERTY(Config, EditAnywhere, Category="Cel Shading|Halftone", meta=(ClampMin="0", ClampMax="1", EditCondition="bHalftoneEnabled", ToolTip="Blend strength of the halftone pattern into the shadow areas."))
    float HalftoneStrength = 0.85f;

    UPROPERTY(Config, EditAnywhere, Category="Cel Shading|Halftone", meta=(ClampMin="0.05", ClampMax="1", EditCondition="bHalftoneEnabled", ToolTip="Normalized lighting threshold below which halftone dots/hatching appear."))
    float HalftoneThreshold = 0.35f;

    UPROPERTY(Config, EditAnywhere, Category="Cel Shading|Halftone", meta=(ClampMin="0.01", ClampMax="0.5", EditCondition="bHalftoneEnabled", ToolTip="Edge softness of the halftone dots or hatching lines."))
    float HalftoneSoftness = 0.15f;

    UPROPERTY(Config, EditAnywhere, Category="Cel Shading|Halftone", meta=(EditCondition="bHalftoneEnabled", ToolTip="Color of the halftone dots or hatching lines."))
    FLinearColor HalftoneTint = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);

    // ==========================================
    // Colour Ramp
    // ==========================================
    UPROPERTY(Config, EditAnywhere, Category="Colour Ramp", meta=(EditCondition="bCelShading", DisplayName="Enable Colour Ramp"))
    bool bCelColorRamp = false;

    UPROPERTY(Config, EditAnywhere, Category="Colour Ramp", meta=(EditCondition="bCelShading && bCelColorRamp"))
    FLinearColor CelShadowColor = FLinearColor::White;

    UPROPERTY(Config, EditAnywhere, Category="Colour Ramp", meta=(EditCondition="bCelShading && bCelColorRamp"))
    FLinearColor CelHighlightColor = FLinearColor::White;

    UPROPERTY(Config, EditAnywhere, Category="Colour Ramp", meta=(EditCondition="bCelShading && bCelColorRamp", DisplayName="Custom Colour Ramp", ToolTip="Optional tint curve sampled from time 0 (shadow) to 1 (highlight), multiplied by the shadow/highlight tint. Include this asset in cooked builds. RGB only; alpha is ignored."))
    TSoftObjectPtr<UCurveLinearColor> CelColorRamp;

    UPROPERTY(Config, EditAnywhere, Category="Colour Ramp", meta=(ClampMin="0", ClampMax="1", EditCondition="bCelShading && bCelColorRamp"))
    float CelColorRampStrength = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category="Colour Ramp", meta=(EditCondition="bCelShading && bCelColorRamp", ToolTip="Sample ramp colour from continuous lighting instead of the quantized band. Useful for custom transitions or a constant-interpolation curve."))
    bool bCelContinuousRamp = false;

    UPROPERTY(Config, EditAnywhere, Category="Colour Ramp", meta=(ClampMin="0", ClampMax="1", EditCondition="bCelShading && bCelColorRamp", ToolTip="0 lets ramp colours replace the light's colour; 1 retains its original colour beneath the tint."))
    float CelLightColorInfluence = 1.0f;

    // ==========================================
    // Ink Outlines
    // ==========================================
    UPROPERTY(Config, EditAnywhere, Category="Ink Outlines", meta=(DisplayName="Enable Ink Outlines"))
    bool bInkOutlines = false;

    UPROPERTY(Config, EditAnywhere, Category="Ink Outlines", meta=(EditCondition="bInkOutlines", ToolTip="Use independent silhouette and crease colours, widths and strengths. Off preserves the shared outline style."))
    bool bSeparateOutlineStyles = false;

    UPROPERTY(Config, EditAnywhere, Category="Ink Outlines", meta=(ClampMin="0.5", ClampMax="4", EditCondition="bInkOutlines && !bSeparateOutlineStyles", ToolTip="Shared width in internal render pixels, used when Separate Outline Styles is off."))
    float OutlineWidth = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category="Ink Outlines", meta=(ClampMin="0", ClampMax="1", EditCondition="bInkOutlines"))
    float OutlineStrength = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category="Ink Outlines", meta=(EditCondition="bInkOutlines && !bSeparateOutlineStyles"))
    FLinearColor OutlineColor = FLinearColor(0.015f, 0.012f, 0.01f, 1.0f);

    // Adaptive Inking (Anime-style colored outlines)
    UPROPERTY(Config, EditAnywhere, Category="Ink Outlines", meta=(EditCondition="bInkOutlines", ToolTip="Tint outline ink with darkened surface albedo for an anime-style soft outline instead of flat black."))
    bool bAdaptiveInkColor = false;

    UPROPERTY(Config, EditAnywhere, Category="Ink Outlines", meta=(ClampMin="0", ClampMax="1", EditCondition="bInkOutlines && bAdaptiveInkColor", ToolTip="Blend between flat ink color and surface-tinted adaptive ink."))
    float AdaptiveInkStrength = 0.7f;

    UPROPERTY(Config, EditAnywhere, Category="Ink Outlines", meta=(ClampMin="0", ClampMax="1", EditCondition="bInkOutlines && bAdaptiveInkColor", ToolTip="Darkness factor applied to surface albedo for adaptive ink."))
    float AdaptiveInkDarkness = 0.25f;

    // Distance Attenuation
    UPROPERTY(Config, EditAnywhere, Category="Ink Outlines", meta=(EditCondition="bInkOutlines", ToolTip="Fade outline width and opacity over distance to prevent far objects turning into black blobs."))
    bool bOutlineDistanceFade = false;

    UPROPERTY(Config, EditAnywhere, Category="Ink Outlines", meta=(ClampMin="10", ClampMax="50000", EditCondition="bInkOutlines && bOutlineDistanceFade", Units="cm"))
    float OutlineFadeStartDistance = 1000.0f;

    UPROPERTY(Config, EditAnywhere, Category="Ink Outlines", meta=(ClampMin="50", ClampMax="100000", EditCondition="bInkOutlines && bOutlineDistanceFade", Units="cm"))
    float OutlineFadeEndDistance = 5000.0f;

    // ==========================================
    // Silhouettes
    // ==========================================
    UPROPERTY(Config, EditAnywhere, Category="Silhouettes", meta=(EditCondition="bInkOutlines"))
    bool bSilhouetteOutlines = true;

    UPROPERTY(Config, EditAnywhere, Category="Silhouettes", meta=(ClampMin="0.5", ClampMax="8", EditCondition="bInkOutlines && bSeparateOutlineStyles && bSilhouetteOutlines"))
    float SilhouetteWidth = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category="Silhouettes", meta=(ClampMin="0", ClampMax="1", EditCondition="bInkOutlines && bSeparateOutlineStyles && bSilhouetteOutlines"))
    float SilhouetteStrength = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category="Silhouettes", meta=(EditCondition="bInkOutlines && bSeparateOutlineStyles && bSilhouetteOutlines"))
    FLinearColor SilhouetteColor = FLinearColor(0.015f, 0.012f, 0.01f, 1.0f);

    UPROPERTY(Config, EditAnywhere, Category="Silhouettes", meta=(ClampMin="0.001", ClampMax="0.2", EditCondition="bInkOutlines", DisplayName="Silhouette Depth Threshold", ToolTip="Relative depth discontinuity that produces a silhouette or occlusion-boundary line. Also rejects crease samples across different surfaces in separate-style mode."))
    float OutlineDepthThreshold = 0.015f;

    // ==========================================
    // Creases
    // ==========================================
    UPROPERTY(Config, EditAnywhere, Category="Creases", meta=(EditCondition="bInkOutlines"))
    bool bCreaseOutlines = true;

    UPROPERTY(Config, EditAnywhere, Category="Creases", meta=(ClampMin="0.5", ClampMax="8", EditCondition="bInkOutlines && bSeparateOutlineStyles && bCreaseOutlines"))
    float CreaseWidth = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category="Creases", meta=(ClampMin="0", ClampMax="1", EditCondition="bInkOutlines && bSeparateOutlineStyles && bCreaseOutlines"))
    float CreaseStrength = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category="Creases", meta=(EditCondition="bInkOutlines && bSeparateOutlineStyles && bCreaseOutlines"))
    FLinearColor CreaseColor = FLinearColor(0.015f, 0.012f, 0.01f, 1.0f);

    UPROPERTY(Config, EditAnywhere, Category="Creases", meta=(ClampMin="0.01", ClampMax="1", EditCondition="bInkOutlines && bCreaseOutlines", DisplayName="Crease Normal Threshold", ToolTip="Shading-normal discontinuity threshold, including normal maps; larger values draw fewer surface creases."))
    float OutlineNormalThreshold = 0.25f;

    // ==========================================
    // Diagnostics / Debug
    // ==========================================
    UPROPERTY(Config, EditAnywhere, Category="Diagnostics", meta=(ToolTip="Diagnostic view modes to isolate flares, outlines, lighting bands, sensor smear, or calibration grid."))
    EFlaresToolsetDebugViewMode DebugViewMode = EFlaresToolsetDebugViewMode::None;

    virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
};
