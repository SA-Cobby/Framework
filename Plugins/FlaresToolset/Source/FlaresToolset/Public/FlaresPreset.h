#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FlaresToolsetTypes.h"
#include "FlaresPreset.generated.h"

class UFlaresToolsetSettings;

/** Data asset packaging a complete artistic optical profile for Flares Toolset. */
UCLASS(BlueprintType)
class FLARESTOOLSET_API UFlaresPreset : public UDataAsset
{
    GENERATED_BODY()

public:
    // ==========================================
    // Lens Flares
    // ==========================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Flares")
    bool bFlaresEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Flares")
    EFlaresToolsetFlareType FlareType = EFlaresToolsetFlareType::Anamorphic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Flares")
    float FlareIntensity = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Flares")
    float FlareThreshold = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Flares")
    float FlareSoftKnee = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Flares")
    float FlareLength = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Flares")
    float FlareRotation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Flares")
    FLinearColor FlareTint = FLinearColor(0.35f, 0.65f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Flares")
    float FlareChromaticDispersion = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Flares")
    bool bHighQualityFlares = false;

    // ==========================================
    // Sensor Smear
    // ==========================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sensor Smear")
    bool bSensorSmearEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sensor Smear")
    EFlaresToolsetSmearOrientation SmearOrientation = EFlaresToolsetSmearOrientation::Vertical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sensor Smear")
    float SmearThreshold = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sensor Smear")
    float SmearIntensity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sensor Smear")
    float SmearLength = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sensor Smear")
    float SmearWidth = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sensor Smear")
    FLinearColor SmearTint = FLinearColor(0.92f, 0.88f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sensor Smear")
    float SmearFalloff = 1.5f;

    // ==========================================
    // Lens Distortion
    // ==========================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Distortion")
    bool bLensDistortionEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Distortion")
    float DistortionStrength = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Distortion")
    float AnamorphicSqueeze = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Distortion")
    float CurvatureAxisRatio = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lens Distortion")
    float DistortionScale = 1.02f;

    // ==========================================
    // Radial Chromatic Aberration
    // ==========================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chromatic Aberration")
    bool bAdvancedChromaticAberration = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chromatic Aberration")
    float AberrationStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chromatic Aberration")
    float AberrationRadialExponent = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chromatic Aberration")
    float AberrationEdgeThreshold = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chromatic Aberration")
    float AberrationEdgeSensitivity = 0.85f;

    // ==========================================
    // Cel Shading
    // ==========================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading")
    bool bCelShading = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading")
    int32 CelBands = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading")
    float CelStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading")
    float CelShadowBrightness = 0.12f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading")
    float CelWhitePoint = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading")
    float CelEmissiveThreshold = 4.0f;

    // ==========================================
    // Halftone & Hatching
    // ==========================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading|Halftone")
    bool bHalftoneEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading|Halftone")
    EFlaresToolsetHalftoneStyle HalftoneStyle = EFlaresToolsetHalftoneStyle::Dots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading|Halftone")
    float HalftoneDensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading|Halftone")
    float HalftoneStrength = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading|Halftone")
    float HalftoneThreshold = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading|Halftone")
    float HalftoneSoftness = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cel Shading|Halftone")
    FLinearColor HalftoneTint = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);

    // ==========================================
    // Colour Ramp
    // ==========================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colour Ramp")
    bool bCelColorRamp = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colour Ramp")
    FLinearColor CelShadowColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colour Ramp")
    FLinearColor CelHighlightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colour Ramp")
    float CelColorRampStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colour Ramp")
    bool bCelContinuousRamp = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colour Ramp")
    float CelLightColorInfluence = 1.0f;

    // ==========================================
    // Ink Outlines
    // ==========================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ink Outlines")
    bool bInkOutlines = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ink Outlines")
    bool bSeparateOutlineStyles = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ink Outlines")
    float OutlineWidth = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ink Outlines")
    float OutlineStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ink Outlines")
    FLinearColor OutlineColor = FLinearColor(0.015f, 0.012f, 0.01f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ink Outlines")
    bool bAdaptiveInkColor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ink Outlines")
    float AdaptiveInkStrength = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ink Outlines")
    float AdaptiveInkDarkness = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ink Outlines")
    bool bOutlineDistanceFade = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ink Outlines")
    float OutlineFadeStartDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ink Outlines")
    float OutlineFadeEndDistance = 5000.0f;

    // Silhouettes & Creases
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Silhouettes")
    bool bSilhouetteOutlines = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Silhouettes")
    float SilhouetteWidth = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Silhouettes")
    float SilhouetteStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Silhouettes")
    FLinearColor SilhouetteColor = FLinearColor(0.015f, 0.012f, 0.01f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Silhouettes")
    float OutlineDepthThreshold = 0.015f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Creases")
    bool bCreaseOutlines = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Creases")
    float CreaseWidth = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Creases")
    float CreaseStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Creases")
    FLinearColor CreaseColor = FLinearColor(0.015f, 0.012f, 0.01f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Creases")
    float OutlineNormalThreshold = 0.25f;

    // ==========================================
    // Methods
    // ==========================================
    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Presets")
    void ApplyToSettings(UFlaresToolsetSettings* TargetSettings) const;

    UFUNCTION(BlueprintCallable, Category="Flares Toolset|Presets")
    void CopyFromSettings(const UFlaresToolsetSettings* SourceSettings);

    static void ApplyBuiltinPresetByName(FName PresetName, UFlaresToolsetSettings* TargetSettings);
    static TArray<FName> GetBuiltinPresetList();
};
