#pragma once

#include "CoreMinimal.h"
#include "RenderGraphFwd.h"
#include "FlaresToolsetTypes.h"

class FSceneView;

struct FFlaresToolsetOpticsOptions
{
    bool bDistortionEnabled = false;
    float DistortionStrength = 0.15f;
    float AnamorphicSqueeze = 2.0f;
    float CurvatureAxisRatio = 0.35f;
    float DistortionScale = 1.02f;

    bool bAberrationEnabled = false;
    float AberrationStrength = 0.5f;
    float AberrationRadialExponent = 2.5f;
    float AberrationEdgeThreshold = 0.15f;
    float AberrationEdgeSensitivity = 0.85f;

    EFlaresToolsetDebugViewMode DebugViewMode = EFlaresToolsetDebugViewMode::None;
};

void AddFlaresToolsetOpticsPass(
    FRDGBuilder& GraphBuilder,
    const FSceneView& View,
    FRDGTextureRef SceneColor,
    const FFlaresToolsetOpticsOptions& Options);
