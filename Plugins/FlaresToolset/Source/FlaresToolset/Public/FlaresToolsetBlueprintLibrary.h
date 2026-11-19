#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FlaresToolsetTypes.h"
#include "FlaresToolsetBlueprintLibrary.generated.h"

class UCurveLinearColor;
class UMaterialParameterCollection;
class UFlaresPreset;

UCLASS()
class FLARESTOOLSET_API UFlaresToolsetBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Configure lens flare parameters at runtime. Call on the game thread. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Flares", meta=(AdvancedDisplay="Threshold,Rotation"))
    static void SetFlares(bool Enabled = true, EFlaresToolsetFlareType Type = EFlaresToolsetFlareType::Anamorphic, float Intensity = 0.4f, float Threshold = 4.0f, float Length = 0.25f, float Rotation = 0.0f, FLinearColor Tint = FLinearColor(0.35f, 0.65f, 1.0f, 1.0f));

    /** Configure flare spectral dispersion (chromatic aberration) and soft knee. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Flares")
    static void SetFlareSpectralDispersion(float ChromaticDispersion = 0.25f, float SoftKnee = 0.2f);

    /** Configure sensor blooming and register smear (CCD column overflow or CMOS row overload). */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Sensor Smear", meta=(AdvancedDisplay="Falloff"))
    static void SetSensorSmear(bool Enabled = true, EFlaresToolsetSmearOrientation Orientation = EFlaresToolsetSmearOrientation::Vertical, float Intensity = 0.6f, float Threshold = 15.0f, float Length = 0.85f, float Width = 1.5f, FLinearColor Tint = FLinearColor(0.92f, 0.88f, 1.0f, 1.0f), float Falloff = 1.5f);

    /** Configure cylindrical anamorphic lens barrel or pincushion distortion. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Lens Distortion", meta=(AdvancedDisplay="Scale"))
    static void SetLensDistortion(bool Enabled = true, float Strength = 0.15f, float AnamorphicSqueeze = 2.0f, float CurvatureAxisRatio = 0.35f, float Scale = 1.02f);

    /** Configure advanced chromatic aberration with radial corner falloff and luma edge gating. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Chromatic Aberration", meta=(AdvancedDisplay="EdgeThreshold,EdgeSensitivity"))
    static void SetAdvancedChromaticAberration(bool Enabled = true, float NormalizedStrength = 0.5f, float RadialExponent = 2.5f, float EdgeThreshold = 0.15f, float EdgeSensitivity = 0.85f);

    /** Dynamically bind a Material Parameter Collection (MPC) to modulate chromatic aberration strength from Light Functions or Blueprints. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Chromatic Aberration")
    static void SetAberrationMPC(UMaterialParameterCollection* Collection, FName ParameterName = FName(TEXT("AberrationStrength")));

    /** Configure cel shading bands and lighting response at runtime. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Cel Shading", meta=(AdvancedDisplay="ShadowBrightness,WhitePoint,EmissiveThreshold"))
    static void SetCelShading(bool Enabled = true, int32 Bands = 3, float Strength = 1.0f, float ShadowBrightness = 0.12f, float WhitePoint = 1.0f, float EmissiveThreshold = 4.0f);

    /** Configure comic halftone dots or manga crosshatching in shadow regions. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Cel Shading", meta=(AdvancedDisplay="Threshold,Softness,Tint"))
    static void SetHalftoneShading(bool Enabled = true, EFlaresToolsetHalftoneStyle Style = EFlaresToolsetHalftoneStyle::Dots, float Density = 12.0f, float Strength = 0.85f, float Threshold = 0.35f, float Softness = 0.15f, FLinearColor Tint = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f));

    /** Configure basic ink outline style at runtime. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Outlines", meta=(AdvancedDisplay="Color"))
    static void SetInkOutlines(bool Enabled = true, float Width = 1.0f, float Strength = 1.0f, FLinearColor Color = FLinearColor(0.015f, 0.012f, 0.01f, 1.0f));

    /** Configure adaptive (surface-albedo tinted) anime-style outlines. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Outlines")
    static void SetAdaptiveInking(bool Enabled = true, float Strength = 0.7f, float Darkness = 0.25f);

    /** Configure distance-dependent outline fading. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Outlines")
    static void SetOutlineDistanceFade(bool Enabled = true, float StartDistance = 1000.0f, float EndDistance = 5000.0f);

    /** Enables ink and separate styles for silhouettes and creases. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Outlines")
    static void SetOutlineStyles(bool Silhouettes, bool Creases, float SilhouetteWidth, float CreaseWidth, float SilhouetteStrength, float CreaseStrength, FLinearColor SilhouetteColor, FLinearColor CreaseColor);

    /** Configure colour ramp tinting for cel shading. Enable Cel Shading separately. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Cel Shading", meta=(AdvancedDisplay="ContinuousInput,LightColorInfluence"))
    static void SetColorRamp(bool Enabled, FLinearColor ShadowColor, FLinearColor HighlightColor, float Strength = 1.0f, UCurveLinearColor* Curve = nullptr, bool ContinuousInput = false, float LightColorInfluence = 1.0f);

    /** Apply an artistic preset data asset. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Presets")
    static void ApplyPreset(const UFlaresPreset* Preset);

    /** Apply one of the built-in presets by name. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Presets")
    static void ApplyBuiltinPreset(FName PresetName);

    /** Get the list of built-in preset names. */
    UFUNCTION(BlueprintPure, Category="Flares Toolset|Presets")
    static TArray<FName> GetBuiltinPresets();

    /** Set diagnostic debug view mode. */
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Diagnostics")
    static void SetDebugViewMode(EFlaresToolsetDebugViewMode Mode);
};
