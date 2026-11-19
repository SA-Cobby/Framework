#pragma once

#include "CoreMinimal.h"
#include "RenderGraphFwd.h"
#include "FlaresToolsetTypes.h"

class FSceneView;

struct FFlaresToolsetSmearOptions
{
    bool Enabled = false;
    EFlaresToolsetSmearOrientation Orientation = EFlaresToolsetSmearOrientation::Vertical;
    float Intensity = 0.6f;
    float Threshold = 15.0f;
    float Length = 0.85f;
    float Width = 1.5f;
    FVector3f Tint = FVector3f(0.92f, 0.88f, 1.0f);
    float Falloff = 1.5f;
    EFlaresToolsetDebugViewMode DebugViewMode = EFlaresToolsetDebugViewMode::None;
};

void AddFlaresToolsetSmearPasses(
    FRDGBuilder& GraphBuilder,
    const FSceneView& View,
    FRDGTextureRef SceneColor,
    const FFlaresToolsetSmearOptions& Options);
