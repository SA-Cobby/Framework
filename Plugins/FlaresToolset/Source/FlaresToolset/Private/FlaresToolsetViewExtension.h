#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "FlaresToolsetFlare.h"
#include "FlaresToolsetCel.h"
#include "FlaresToolsetSmear.h"
#include "FlaresToolsetOptics.h"
#include "UObject/StrongObjectPtr.h"

class UCurveLinearColor;

struct FFlaresToolsetRenderOptions
{
    FFlaresToolsetFlareOptions Flare;
    FFlaresToolsetCelOptions Cel;
    FFlaresToolsetSmearOptions Smear;
    FFlaresToolsetOpticsOptions Optics;
};

class FFlaresToolsetViewExtension : public FSceneViewExtensionBase
{
public:
    FFlaresToolsetViewExtension(const FAutoRegister& AutoRegister);
    virtual ~FFlaresToolsetViewExtension() override = default;

    virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
    virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
    virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;
    virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;

private:
    TStrongObjectPtr<UCurveLinearColor> LoadedRamp;
    FSoftObjectPath LoadedRampPath;
    FFlaresToolsetRenderOptions Options;
};
