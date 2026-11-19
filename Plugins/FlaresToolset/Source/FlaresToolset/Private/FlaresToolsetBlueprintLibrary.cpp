#include "FlaresToolsetBlueprintLibrary.h"
#include "FlaresToolsetSettings.h"
#include "FlaresPreset.h"
#include "Curves/CurveLinearColor.h"
#include "Materials/MaterialParameterCollection.h"

void UFlaresToolsetBlueprintLibrary::SetFlares(bool Enabled, EFlaresToolsetFlareType Type, float Intensity, float Threshold, float Length, float Rotation, FLinearColor Tint)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->bFlaresEnabled = Enabled;
    S->FlareType = Type;
    S->FlareIntensity = FMath::Clamp(Intensity, 0.0f, 10.0f);
    S->FlareThreshold = FMath::Clamp(Threshold, 0.0f, 100000.0f);
    S->FlareLength = FMath::Clamp(Length, 0.02f, 0.75f);
    S->FlareRotation = FMath::Clamp(Rotation, -90.0f, 90.0f);
    S->FlareTint = Tint;
}

void UFlaresToolsetBlueprintLibrary::SetFlareSpectralDispersion(float ChromaticDispersion, float SoftKnee)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->FlareChromaticDispersion = FMath::Clamp(ChromaticDispersion, 0.0f, 1.0f);
    S->FlareSoftKnee = FMath::Clamp(SoftKnee, 0.0f, 1.0f);
}

void UFlaresToolsetBlueprintLibrary::SetSensorSmear(bool Enabled, EFlaresToolsetSmearOrientation Orientation, float Intensity, float Threshold, float Length, float Width, FLinearColor Tint, float Falloff)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->bSensorSmearEnabled = Enabled;
    S->SmearOrientation = Orientation;
    S->SmearIntensity = FMath::Clamp(Intensity, 0.0f, 10.0f);
    S->SmearThreshold = FMath::Clamp(Threshold, 1.0f, 100000.0f);
    S->SmearLength = FMath::Clamp(Length, 0.05f, 1.0f);
    S->SmearWidth = FMath::Clamp(Width, 0.5f, 16.0f);
    S->SmearTint = Tint;
    S->SmearFalloff = FMath::Clamp(Falloff, 0.1f, 5.0f);
}

void UFlaresToolsetBlueprintLibrary::SetLensDistortion(bool Enabled, float Strength, float AnamorphicSqueeze, float CurvatureAxisRatio, float Scale)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->bLensDistortionEnabled = Enabled;
    S->DistortionStrength = FMath::Clamp(Strength, -1.0f, 1.0f);
    S->AnamorphicSqueeze = FMath::Clamp(AnamorphicSqueeze, 1.0f, 3.0f);
    S->CurvatureAxisRatio = FMath::Clamp(CurvatureAxisRatio, 0.0f, 2.0f);
    S->DistortionScale = FMath::Clamp(Scale, 0.8f, 1.5f);
}

void UFlaresToolsetBlueprintLibrary::SetAdvancedChromaticAberration(bool Enabled, float NormalizedStrength, float RadialExponent, float EdgeThreshold, float EdgeSensitivity)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->bAdvancedChromaticAberration = Enabled;
    S->AberrationStrength = FMath::Clamp(NormalizedStrength, 0.0f, 1.0f);
    S->AberrationRadialExponent = FMath::Clamp(RadialExponent, 1.0f, 6.0f);
    S->AberrationEdgeThreshold = FMath::Clamp(EdgeThreshold, 0.01f, 1.0f);
    S->AberrationEdgeSensitivity = FMath::Clamp(EdgeSensitivity, 0.0f, 1.0f);
}

void UFlaresToolsetBlueprintLibrary::SetAberrationMPC(UMaterialParameterCollection* Collection, FName ParameterName)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->AberrationMPC = Collection;
    S->AberrationParameterName = ParameterName.IsNone() ? FName(TEXT("AberrationStrength")) : ParameterName;
}

void UFlaresToolsetBlueprintLibrary::SetCelShading(bool Enabled, int32 Bands, float Strength, float ShadowBrightness, float WhitePoint, float EmissiveThreshold)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->bCelShading = Enabled;
    S->CelBands = FMath::Clamp(Bands, 2, 8);
    S->CelStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
    S->CelShadowBrightness = FMath::Clamp(ShadowBrightness, 0.0f, 1.0f);
    S->CelWhitePoint = FMath::Clamp(WhitePoint, 0.05f, 20.0f);
    S->CelEmissiveThreshold = FMath::Clamp(EmissiveThreshold, 0.1f, 100.0f);
}

void UFlaresToolsetBlueprintLibrary::SetHalftoneShading(bool Enabled, EFlaresToolsetHalftoneStyle Style, float Density, float Strength, float Threshold, float Softness, FLinearColor Tint)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->bHalftoneEnabled = Enabled;
    S->HalftoneStyle = Style;
    S->HalftoneDensity = FMath::Clamp(Density, 4.0f, 64.0f);
    S->HalftoneStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
    S->HalftoneThreshold = FMath::Clamp(Threshold, 0.05f, 1.0f);
    S->HalftoneSoftness = FMath::Clamp(Softness, 0.01f, 0.5f);
    S->HalftoneTint = Tint;
}

void UFlaresToolsetBlueprintLibrary::SetInkOutlines(bool Enabled, float Width, float Strength, FLinearColor Color)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->bInkOutlines = Enabled;
    S->OutlineWidth = FMath::Clamp(Width, 0.5f, 4.0f);
    S->OutlineStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
    S->OutlineColor = Color;
}

void UFlaresToolsetBlueprintLibrary::SetAdaptiveInking(bool Enabled, float Strength, float Darkness)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->bAdaptiveInkColor = Enabled;
    S->AdaptiveInkStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
    S->AdaptiveInkDarkness = FMath::Clamp(Darkness, 0.0f, 1.0f);
}

void UFlaresToolsetBlueprintLibrary::SetOutlineDistanceFade(bool Enabled, float StartDistance, float EndDistance)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->bOutlineDistanceFade = Enabled;
    S->OutlineFadeStartDistance = FMath::Max(10.0f, StartDistance);
    S->OutlineFadeEndDistance = FMath::Max(S->OutlineFadeStartDistance + 10.0f, EndDistance);
}

void UFlaresToolsetBlueprintLibrary::SetOutlineStyles(bool Silhouettes, bool Creases, float SilhouetteWidth, float CreaseWidth, float SilhouetteStrength, float CreaseStrength, FLinearColor SilhouetteColor, FLinearColor CreaseColor)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->bInkOutlines = true;
    S->bSeparateOutlineStyles = true;
    S->bSilhouetteOutlines = Silhouettes;
    S->bCreaseOutlines = Creases;
    S->SilhouetteWidth = FMath::Clamp(SilhouetteWidth, 0.5f, 8.0f);
    S->CreaseWidth = FMath::Clamp(CreaseWidth, 0.5f, 8.0f);
    S->SilhouetteStrength = FMath::Clamp(SilhouetteStrength, 0.0f, 1.0f);
    S->CreaseStrength = FMath::Clamp(CreaseStrength, 0.0f, 1.0f);
    S->SilhouetteColor = SilhouetteColor;
    S->CreaseColor = CreaseColor;
}

void UFlaresToolsetBlueprintLibrary::SetColorRamp(bool Enabled, FLinearColor ShadowColor, FLinearColor HighlightColor, float Strength, UCurveLinearColor* Curve, bool ContinuousInput, float LightColorInfluence)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->bCelColorRamp = Enabled;
    S->CelShadowColor = ShadowColor;
    S->CelHighlightColor = HighlightColor;
    S->CelColorRampStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
    S->CelColorRamp = Curve;
    S->bCelContinuousRamp = ContinuousInput;
    S->CelLightColorInfluence = FMath::Clamp(LightColorInfluence, 0.0f, 1.0f);
}

void UFlaresToolsetBlueprintLibrary::ApplyPreset(const UFlaresPreset* Preset)
{
    check(IsInGameThread());
    if (Preset)
    {
        Preset->ApplyToSettings(GetMutableDefault<UFlaresToolsetSettings>());
    }
}

void UFlaresToolsetBlueprintLibrary::ApplyBuiltinPreset(FName PresetName)
{
    check(IsInGameThread());
    UFlaresPreset::ApplyBuiltinPresetByName(PresetName, GetMutableDefault<UFlaresToolsetSettings>());
}

TArray<FName> UFlaresToolsetBlueprintLibrary::GetBuiltinPresets()
{
    return UFlaresPreset::GetBuiltinPresetList();
}

void UFlaresToolsetBlueprintLibrary::SetDebugViewMode(EFlaresToolsetDebugViewMode Mode)
{
    check(IsInGameThread());
    UFlaresToolsetSettings* S = GetMutableDefault<UFlaresToolsetSettings>();
    S->DebugViewMode = Mode;
}
