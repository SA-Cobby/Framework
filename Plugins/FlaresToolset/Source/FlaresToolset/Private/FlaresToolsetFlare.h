#pragma once

#include "CoreMinimal.h"
#include "RenderGraphFwd.h"
#include "FlaresToolsetTypes.h"

class FSceneView;

struct FFlaresToolsetFlareOptions
{
    bool Enabled = false;
    bool HighQuality = false;
    EFlaresToolsetFlareType Type = EFlaresToolsetFlareType::Anamorphic;
    float Intensity = 0.4f;
    float Threshold = 4.0f;
    float SoftKnee = 0.2f;
    float Length = 0.25f;
    float Rotation = 0.0f;
    FVector3f Tint = FVector3f(0.35f, 0.65f, 1.0f);
    float ChromaticDispersion = 0.25f;
    EFlaresToolsetDebugViewMode DebugViewMode = EFlaresToolsetDebugViewMode::None;
};

void AddFlaresToolsetFlarePasses(
    FRDGBuilder& GraphBuilder,
    const FSceneView& View,
    FRDGTextureRef SceneColor,
    const FFlaresToolsetFlareOptions& Options);
