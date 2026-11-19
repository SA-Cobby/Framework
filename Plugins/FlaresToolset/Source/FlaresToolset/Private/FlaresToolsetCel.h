#pragma once

#include "CoreMinimal.h"
#include "Containers/StaticArray.h"
#include "RenderGraphFwd.h"
#include "FlaresToolsetTypes.h"

class FSceneView;
class FSceneTextureUniformParameters;

struct FFlaresToolsetCelOptions
{
    bool Enabled = false;
    bool Outlines = false;
    int32 Bands = 3;
    float Strength = 1.0f;
    float Shadow = 0.12f;
    float WhitePoint = 1.0f;
    float EmissiveThreshold = 4.0f;
    float Width = 1.0f;
    float OutlineStrength = 1.0f;
    float DepthThreshold = 0.015f;
    float NormalThreshold = 0.25f;
    FVector3f OutlineColor = FVector3f(0.015f, 0.012f, 0.01f);
    bool Silhouettes = true;
    bool Creases = true;
    bool SeparateStyles = false;
    bool RampEnabled = false;
    bool ContinuousRamp = false;
    float SilhouetteWidth = 1.0f;
    float SilhouetteStrength = 1.0f;
    float CreaseWidth = 1.0f;
    float CreaseStrength = 1.0f;
    FVector3f SilhouetteColor = FVector3f(0.015f, 0.012f, 0.01f);
    FVector3f CreaseColor = FVector3f(0.015f, 0.012f, 0.01f);
    float RampStrength = 1.0f;
    float LightColorInfluence = 1.0f;
    TStaticArray<FVector4f, 64> RampSamples;

    // Adaptive Inking & Distance Fade
    bool bAdaptiveInkColor = false;
    float AdaptiveInkStrength = 0.7f;
    float AdaptiveInkDarkness = 0.25f;
    bool bDistanceFade = false;
    float FadeStartDistance = 1000.0f;
    float FadeEndDistance = 5000.0f;

    // Halftone & Manga Hatching
    bool bHalftoneEnabled = false;
    EFlaresToolsetHalftoneStyle HalftoneStyle = EFlaresToolsetHalftoneStyle::Dots;
    float HalftoneDensity = 12.0f;
    float HalftoneStrength = 0.85f;
    float HalftoneThreshold = 0.35f;
    float HalftoneSoftness = 0.15f;
    FVector3f HalftoneTint = FVector3f(0.02f, 0.02f, 0.02f);

    EFlaresToolsetDebugViewMode DebugViewMode = EFlaresToolsetDebugViewMode::None;
};

void AddFlaresToolsetCelPass(
    FRDGBuilder& GraphBuilder,
    const FSceneView& View,
    const FSceneTextureUniformParameters& Textures,
    const FFlaresToolsetCelOptions& Options);
