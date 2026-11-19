#include "FlaresPreset.h"
#include "FlaresToolsetSettings.h"

void UFlaresPreset::ApplyToSettings(UFlaresToolsetSettings* S) const
{
    if (!S) return;

    // Flares
    S->bFlaresEnabled = bFlaresEnabled;
    S->FlareType = FlareType;
    S->FlareIntensity = FlareIntensity;
    S->FlareThreshold = FlareThreshold;
    S->FlareSoftKnee = FlareSoftKnee;
    S->FlareLength = FlareLength;
    S->FlareRotation = FlareRotation;
    S->FlareTint = FlareTint;
    S->FlareChromaticDispersion = FlareChromaticDispersion;
    S->bHighQualityFlares = bHighQualityFlares;

    // Smear
    S->bSensorSmearEnabled = bSensorSmearEnabled;
    S->SmearOrientation = SmearOrientation;
    S->SmearThreshold = SmearThreshold;
    S->SmearIntensity = SmearIntensity;
    S->SmearLength = SmearLength;
    S->SmearWidth = SmearWidth;
    S->SmearTint = SmearTint;
    S->SmearFalloff = SmearFalloff;

    // Distortion
    S->bLensDistortionEnabled = bLensDistortionEnabled;
    S->DistortionStrength = DistortionStrength;
    S->AnamorphicSqueeze = AnamorphicSqueeze;
    S->CurvatureAxisRatio = CurvatureAxisRatio;
    S->DistortionScale = DistortionScale;

    // Chromatic Aberration
    S->bAdvancedChromaticAberration = bAdvancedChromaticAberration;
    S->AberrationStrength = AberrationStrength;
    S->AberrationRadialExponent = AberrationRadialExponent;
    S->AberrationEdgeThreshold = AberrationEdgeThreshold;
    S->AberrationEdgeSensitivity = AberrationEdgeSensitivity;

    // Cel Shading
    S->bCelShading = bCelShading;
    S->CelBands = CelBands;
    S->CelStrength = CelStrength;
    S->CelShadowBrightness = CelShadowBrightness;
    S->CelWhitePoint = CelWhitePoint;
    S->CelEmissiveThreshold = CelEmissiveThreshold;

    // Halftone
    S->bHalftoneEnabled = bHalftoneEnabled;
    S->HalftoneStyle = HalftoneStyle;
    S->HalftoneDensity = HalftoneDensity;
    S->HalftoneStrength = HalftoneStrength;
    S->HalftoneThreshold = HalftoneThreshold;
    S->HalftoneSoftness = HalftoneSoftness;
    S->HalftoneTint = HalftoneTint;

    // Colour Ramp
    S->bCelColorRamp = bCelColorRamp;
    S->CelShadowColor = CelShadowColor;
    S->CelHighlightColor = CelHighlightColor;
    S->CelColorRampStrength = CelColorRampStrength;
    S->bCelContinuousRamp = bCelContinuousRamp;
    S->CelLightColorInfluence = CelLightColorInfluence;

    // Outlines
    S->bInkOutlines = bInkOutlines;
    S->bSeparateOutlineStyles = bSeparateOutlineStyles;
    S->OutlineWidth = OutlineWidth;
    S->OutlineStrength = OutlineStrength;
    S->OutlineColor = OutlineColor;
    S->bAdaptiveInkColor = bAdaptiveInkColor;
    S->AdaptiveInkStrength = AdaptiveInkStrength;
    S->AdaptiveInkDarkness = AdaptiveInkDarkness;
    S->bOutlineDistanceFade = bOutlineDistanceFade;
    S->OutlineFadeStartDistance = OutlineFadeStartDistance;
    S->OutlineFadeEndDistance = OutlineFadeEndDistance;

    // Silhouettes & Creases
    S->bSilhouetteOutlines = bSilhouetteOutlines;
    S->SilhouetteWidth = SilhouetteWidth;
    S->SilhouetteStrength = SilhouetteStrength;
    S->SilhouetteColor = SilhouetteColor;
    S->OutlineDepthThreshold = OutlineDepthThreshold;
    S->bCreaseOutlines = bCreaseOutlines;
    S->CreaseWidth = CreaseWidth;
    S->CreaseStrength = CreaseStrength;
    S->CreaseColor = CreaseColor;
    S->OutlineNormalThreshold = OutlineNormalThreshold;
}

void UFlaresPreset::CopyFromSettings(const UFlaresToolsetSettings* S)
{
    if (!S) return;

    bFlaresEnabled = S->bFlaresEnabled;
    FlareType = S->FlareType;
    FlareIntensity = S->FlareIntensity;
    FlareThreshold = S->FlareThreshold;
    FlareSoftKnee = S->FlareSoftKnee;
    FlareLength = S->FlareLength;
    FlareRotation = S->FlareRotation;
    FlareTint = S->FlareTint;
    FlareChromaticDispersion = S->FlareChromaticDispersion;
    bHighQualityFlares = S->bHighQualityFlares;

    bSensorSmearEnabled = S->bSensorSmearEnabled;
    SmearOrientation = S->SmearOrientation;
    SmearThreshold = S->SmearThreshold;
    SmearIntensity = S->SmearIntensity;
    SmearLength = S->SmearLength;
    SmearWidth = S->SmearWidth;
    SmearTint = S->SmearTint;
    SmearFalloff = S->SmearFalloff;

    bLensDistortionEnabled = S->bLensDistortionEnabled;
    DistortionStrength = S->DistortionStrength;
    AnamorphicSqueeze = S->AnamorphicSqueeze;
    CurvatureAxisRatio = S->CurvatureAxisRatio;
    DistortionScale = S->DistortionScale;

    bAdvancedChromaticAberration = S->bAdvancedChromaticAberration;
    AberrationStrength = S->AberrationStrength;
    AberrationRadialExponent = S->AberrationRadialExponent;
    AberrationEdgeThreshold = S->AberrationEdgeThreshold;
    AberrationEdgeSensitivity = S->AberrationEdgeSensitivity;

    bCelShading = S->bCelShading;
    CelBands = S->CelBands;
    CelStrength = S->CelStrength;
    CelShadowBrightness = S->CelShadowBrightness;
    CelWhitePoint = S->CelWhitePoint;
    CelEmissiveThreshold = S->CelEmissiveThreshold;

    bHalftoneEnabled = S->bHalftoneEnabled;
    HalftoneStyle = S->HalftoneStyle;
    HalftoneDensity = S->HalftoneDensity;
    HalftoneStrength = S->HalftoneStrength;
    HalftoneThreshold = S->HalftoneThreshold;
    HalftoneSoftness = S->HalftoneSoftness;
    HalftoneTint = S->HalftoneTint;

    bCelColorRamp = S->bCelColorRamp;
    CelShadowColor = S->CelShadowColor;
    CelHighlightColor = S->CelHighlightColor;
    CelColorRampStrength = S->CelColorRampStrength;
    bCelContinuousRamp = S->bCelContinuousRamp;
    CelLightColorInfluence = S->CelLightColorInfluence;

    bInkOutlines = S->bInkOutlines;
    bSeparateOutlineStyles = S->bSeparateOutlineStyles;
    OutlineWidth = S->OutlineWidth;
    OutlineStrength = S->OutlineStrength;
    OutlineColor = S->OutlineColor;
    bAdaptiveInkColor = S->bAdaptiveInkColor;
    AdaptiveInkStrength = S->AdaptiveInkStrength;
    AdaptiveInkDarkness = S->AdaptiveInkDarkness;
    bOutlineDistanceFade = S->bOutlineDistanceFade;
    OutlineFadeStartDistance = S->OutlineFadeStartDistance;
    OutlineFadeEndDistance = S->OutlineFadeEndDistance;

    bSilhouetteOutlines = S->bSilhouetteOutlines;
    SilhouetteWidth = S->SilhouetteWidth;
    SilhouetteStrength = S->SilhouetteStrength;
    SilhouetteColor = S->SilhouetteColor;
    OutlineDepthThreshold = S->OutlineDepthThreshold;
    bCreaseOutlines = S->bCreaseOutlines;
    CreaseWidth = S->CreaseWidth;
    CreaseStrength = S->CreaseStrength;
    CreaseColor = S->CreaseColor;
    OutlineNormalThreshold = S->OutlineNormalThreshold;
}

TArray<FName> UFlaresPreset::GetBuiltinPresetList()
{
    return {
        FName(TEXT("Panavision 70s Anamorphic")),
        FName(TEXT("Retro CCTV Overload")),
        FName(TEXT("Studio Anime Cel")),
        FName(TEXT("Graphic Novel Noir")),
        FName(TEXT("Cyberpunk Neon")),
        FName(TEXT("Pop-Art Comic Halftone"))
    };
}

void UFlaresPreset::ApplyBuiltinPresetByName(FName PresetName, UFlaresToolsetSettings* S)
{
    if (!S) return;

    if (PresetName == FName(TEXT("Panavision 70s Anamorphic")))
    {
        S->bFlaresEnabled = true;
        S->FlareType = EFlaresToolsetFlareType::Anamorphic;
        S->FlareIntensity = 0.55f;
        S->FlareThreshold = 3.5f;
        S->FlareSoftKnee = 0.25f;
        S->FlareLength = 0.35f;
        S->FlareRotation = 0.0f;
        S->FlareTint = FLinearColor(0.95f, 0.72f, 0.45f, 1.0f);
        S->FlareChromaticDispersion = 0.35f;
        S->bHighQualityFlares = false;

        S->bLensDistortionEnabled = true;
        S->DistortionStrength = 0.16f;
        S->AnamorphicSqueeze = 2.0f;
        S->CurvatureAxisRatio = 0.3f;
        S->DistortionScale = 1.03f;

        S->bAdvancedChromaticAberration = true;
        S->AberrationStrength = 0.65f;
        S->AberrationRadialExponent = 2.2f;
        S->AberrationEdgeThreshold = 0.12f;
        S->AberrationEdgeSensitivity = 0.85f;

        S->bSensorSmearEnabled = false;
        S->bCelShading = false;
        S->bInkOutlines = false;
    }
    else if (PresetName == FName(TEXT("Retro CCTV Overload")))
    {
        S->bSensorSmearEnabled = true;
        S->SmearOrientation = EFlaresToolsetSmearOrientation::Vertical;
        S->SmearIntensity = 0.85f;
        S->SmearThreshold = 6.0f;
        S->SmearLength = 0.95f;
        S->SmearWidth = 1.8f;
        S->SmearTint = FLinearColor(0.85f, 0.75f, 1.0f, 1.0f);
        S->SmearFalloff = 1.2f;

        S->bLensDistortionEnabled = true;
        S->DistortionStrength = -0.06f; // slight CRT pincushion
        S->AnamorphicSqueeze = 1.0f;
        S->CurvatureAxisRatio = 1.0f;
        S->DistortionScale = 1.0f;

        S->bAdvancedChromaticAberration = true;
        S->AberrationStrength = 0.5f;
        S->AberrationRadialExponent = 1.8f;

        S->bFlaresEnabled = false;
        S->bCelShading = false;
        S->bInkOutlines = false;
    }
    else if (PresetName == FName(TEXT("Studio Anime Cel")))
    {
        S->bCelShading = true;
        S->CelBands = 3;
        S->CelStrength = 1.0f;
        S->CelShadowBrightness = 0.14f;
        S->CelWhitePoint = 1.2f;
        S->CelEmissiveThreshold = 3.5f;

        S->bInkOutlines = true;
        S->bSeparateOutlineStyles = false;
        S->OutlineWidth = 1.2f;
        S->OutlineStrength = 1.0f;
        S->OutlineColor = FLinearColor(0.015f, 0.012f, 0.01f, 1.0f);
        S->bAdaptiveInkColor = true;
        S->AdaptiveInkStrength = 0.75f;
        S->AdaptiveInkDarkness = 0.2f;
        S->bOutlineDistanceFade = true;
        S->OutlineFadeStartDistance = 1200.0f;
        S->OutlineFadeEndDistance = 6000.0f;

        S->bHalftoneEnabled = false;
        S->bFlaresEnabled = true;
        S->FlareType = EFlaresToolsetFlareType::Anamorphic;
        S->FlareIntensity = 0.25f;
        S->FlareThreshold = 5.0f;
        S->FlareSoftKnee = 0.4f;
        S->FlareLength = 0.2f;
        S->FlareTint = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);
        S->FlareChromaticDispersion = 0.15f;

        S->bSensorSmearEnabled = false;
        S->bLensDistortionEnabled = false;
        S->bAdvancedChromaticAberration = false;
    }
    else if (PresetName == FName(TEXT("Graphic Novel Noir")))
    {
        S->bCelShading = true;
        S->CelBands = 2;
        S->CelStrength = 1.0f;
        S->CelShadowBrightness = 0.04f;
        S->CelWhitePoint = 0.9f;

        S->bHalftoneEnabled = true;
        S->HalftoneStyle = EFlaresToolsetHalftoneStyle::Crosshatch;
        S->HalftoneDensity = 9.0f;
        S->HalftoneStrength = 1.0f;
        S->HalftoneThreshold = 0.45f;
        S->HalftoneSoftness = 0.15f;
        S->HalftoneTint = FLinearColor(0.01f, 0.01f, 0.01f, 1.0f);

        S->bInkOutlines = true;
        S->bSeparateOutlineStyles = true;
        S->bSilhouetteOutlines = true;
        S->SilhouetteWidth = 2.2f;
        S->SilhouetteStrength = 1.0f;
        S->SilhouetteColor = FLinearColor::Black;
        S->bCreaseOutlines = true;
        S->CreaseWidth = 1.2f;
        S->CreaseStrength = 0.8f;
        S->CreaseColor = FLinearColor::Black;

        S->bFlaresEnabled = false;
        S->bSensorSmearEnabled = false;
        S->bLensDistortionEnabled = false;
        S->bAdvancedChromaticAberration = false;
    }
    else if (PresetName == FName(TEXT("Cyberpunk Neon")))
    {
        S->bFlaresEnabled = true;
        S->FlareType = EFlaresToolsetFlareType::Cross;
        S->FlareIntensity = 0.75f;
        S->FlareThreshold = 2.5f;
        S->FlareSoftKnee = 0.3f;
        S->FlareLength = 0.35f;
        S->FlareRotation = 0.0f;
        S->FlareTint = FLinearColor(0.15f, 0.8f, 1.0f, 1.0f);
        S->FlareChromaticDispersion = 0.45f;

        S->bCelShading = true;
        S->CelBands = 4;
        S->CelStrength = 0.85f;
        S->bCelColorRamp = true;
        S->CelShadowColor = FLinearColor(0.08f, 0.0f, 0.2f, 1.0f);
        S->CelHighlightColor = FLinearColor(0.9f, 0.35f, 1.0f, 1.0f);
        S->CelColorRampStrength = 0.9f;

        S->bLensDistortionEnabled = true;
        S->DistortionStrength = 0.12f;
        S->AnamorphicSqueeze = 1.8f;
        S->CurvatureAxisRatio = 0.4f;

        S->bAdvancedChromaticAberration = true;
        S->AberrationStrength = 0.75f;
        S->AberrationRadialExponent = 2.5f;
        S->AberrationEdgeThreshold = 0.1f;
        S->AberrationEdgeSensitivity = 0.9f;

        S->bSensorSmearEnabled = false;
        S->bHalftoneEnabled = false;
    }
    else if (PresetName == FName(TEXT("Pop-Art Comic Halftone")))
    {
        S->bCelShading = true;
        S->CelBands = 3;
        S->CelStrength = 1.0f;
        S->CelShadowBrightness = 0.15f;

        S->bHalftoneEnabled = true;
        S->HalftoneStyle = EFlaresToolsetHalftoneStyle::Dots;
        S->HalftoneDensity = 14.0f;
        S->HalftoneStrength = 0.9f;
        S->HalftoneThreshold = 0.4f;
        S->HalftoneSoftness = 0.12f;
        S->HalftoneTint = FLinearColor(0.04f, 0.02f, 0.02f, 1.0f);

        S->bInkOutlines = true;
        S->OutlineWidth = 1.4f;
        S->OutlineStrength = 1.0f;
        S->OutlineColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);

        S->bFlaresEnabled = false;
        S->bSensorSmearEnabled = false;
        S->bLensDistortionEnabled = false;
        S->bAdvancedChromaticAberration = false;
    }
}
