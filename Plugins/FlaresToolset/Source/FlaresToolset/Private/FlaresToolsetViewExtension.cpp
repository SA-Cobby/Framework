#include "FlaresToolsetViewExtension.h"
#include "FlaresToolsetSettings.h"
#include "SceneTexturesConfig.h"
#include "PostProcess/PostProcessInputs.h"
#include "Curves/CurveLinearColor.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "SceneRendering.h"

FFlaresToolsetViewExtension::FFlaresToolsetViewExtension(const FAutoRegister& AutoRegister)
    : FSceneViewExtensionBase(AutoRegister)
{
}

void FFlaresToolsetViewExtension::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
    const UFlaresToolsetSettings* S = GetDefault<UFlaresToolsetSettings>();
    FFlaresToolsetRenderOptions NewOptions;

    // Flare options
    NewOptions.Flare.Enabled = S->bFlaresEnabled;
    NewOptions.Flare.HighQuality = S->bHighQualityFlares;
    NewOptions.Flare.Type = S->FlareType;
    NewOptions.Flare.Intensity = FMath::Clamp(S->FlareIntensity, 0.0f, 10.0f);
    NewOptions.Flare.Threshold = FMath::Clamp(S->FlareThreshold, 0.0f, 100000.0f);
    NewOptions.Flare.SoftKnee = FMath::Clamp(S->FlareSoftKnee, 0.0f, 1.0f);
    NewOptions.Flare.Length = FMath::Clamp(S->FlareLength, 0.02f, 0.75f);
    NewOptions.Flare.Rotation = FMath::Clamp(S->FlareRotation, -90.0f, 90.0f);
    NewOptions.Flare.Tint = FVector3f(
        FMath::Clamp(S->FlareTint.R, 0.0f, 64.0f),
        FMath::Clamp(S->FlareTint.G, 0.0f, 64.0f),
        FMath::Clamp(S->FlareTint.B, 0.0f, 64.0f));
    NewOptions.Flare.ChromaticDispersion = FMath::Clamp(S->FlareChromaticDispersion, 0.0f, 1.0f);
    NewOptions.Flare.DebugViewMode = S->DebugViewMode;

    // Sensor Smear options
    NewOptions.Smear.Enabled = S->bSensorSmearEnabled;
    NewOptions.Smear.Orientation = S->SmearOrientation;
    NewOptions.Smear.Intensity = FMath::Clamp(S->SmearIntensity, 0.0f, 10.0f);
    NewOptions.Smear.Threshold = FMath::Clamp(S->SmearThreshold, 1.0f, 100000.0f);
    NewOptions.Smear.Length = FMath::Clamp(S->SmearLength, 0.05f, 1.0f);
    NewOptions.Smear.Width = FMath::Clamp(S->SmearWidth, 0.5f, 16.0f);
    NewOptions.Smear.Tint = FVector3f(S->SmearTint.R, S->SmearTint.G, S->SmearTint.B);
    NewOptions.Smear.Falloff = FMath::Clamp(S->SmearFalloff, 0.1f, 5.0f);
    NewOptions.Smear.DebugViewMode = S->DebugViewMode;

    // Lens Distortion options
    NewOptions.Optics.bDistortionEnabled = S->bLensDistortionEnabled;
    NewOptions.Optics.DistortionStrength = FMath::Clamp(S->DistortionStrength, -1.0f, 1.0f);
    NewOptions.Optics.AnamorphicSqueeze = FMath::Clamp(S->AnamorphicSqueeze, 1.0f, 3.0f);
    NewOptions.Optics.CurvatureAxisRatio = FMath::Clamp(S->CurvatureAxisRatio, 0.0f, 2.0f);
    NewOptions.Optics.DistortionScale = FMath::Clamp(S->DistortionScale, 0.8f, 1.5f);

    // Radial Chromatic Aberration with Material Parameter Collection modulation
    NewOptions.Optics.bAberrationEnabled = S->bAdvancedChromaticAberration;
    float ModulatedAberration = FMath::Clamp(S->AberrationStrength, 0.0f, 1.0f);
    if (S->bAdvancedChromaticAberration && !S->AberrationMPC.IsNull())
    {
        UMaterialParameterCollection* MPC = S->AberrationMPC.LoadSynchronous();
        if (MPC && InViewFamily.Scene && InViewFamily.Scene->GetWorld())
        {
            UMaterialParameterCollectionInstance* Instance = InViewFamily.Scene->GetWorld()->GetParameterCollectionInstance(MPC);
            if (Instance)
            {
                float MPCValue = 1.0f;
                if (Instance->GetScalarParameterValue(S->AberrationParameterName, MPCValue))
                {
                    ModulatedAberration *= FMath::Clamp(MPCValue, 0.0f, 1.0f);
                }
            }
        }
    }
    NewOptions.Optics.AberrationStrength = ModulatedAberration;
    NewOptions.Optics.AberrationRadialExponent = FMath::Clamp(S->AberrationRadialExponent, 1.0f, 6.0f);
    NewOptions.Optics.AberrationEdgeThreshold = FMath::Clamp(S->AberrationEdgeThreshold, 0.01f, 1.0f);
    NewOptions.Optics.AberrationEdgeSensitivity = FMath::Clamp(S->AberrationEdgeSensitivity, 0.0f, 1.0f);
    NewOptions.Optics.DebugViewMode = S->DebugViewMode;

    // Cel shading options
    NewOptions.Cel.Enabled = S->bCelShading;
    NewOptions.Cel.Outlines = S->bInkOutlines;
    NewOptions.Cel.Bands = FMath::Clamp(S->CelBands, 2, 8);
    NewOptions.Cel.Strength = FMath::Clamp(S->CelStrength, 0.0f, 1.0f);
    NewOptions.Cel.Shadow = FMath::Clamp(S->CelShadowBrightness, 0.0f, 1.0f);
    NewOptions.Cel.WhitePoint = FMath::Clamp(S->CelWhitePoint, 0.05f, 20.0f);
    NewOptions.Cel.EmissiveThreshold = FMath::Clamp(S->CelEmissiveThreshold, 0.1f, 100.0f);

    // Halftone & Manga Hatching
    NewOptions.Cel.bHalftoneEnabled = S->bHalftoneEnabled;
    NewOptions.Cel.HalftoneStyle = S->HalftoneStyle;
    NewOptions.Cel.HalftoneDensity = FMath::Clamp(S->HalftoneDensity, 4.0f, 64.0f);
    NewOptions.Cel.HalftoneStrength = FMath::Clamp(S->HalftoneStrength, 0.0f, 1.0f);
    NewOptions.Cel.HalftoneThreshold = FMath::Clamp(S->HalftoneThreshold, 0.05f, 1.0f);
    NewOptions.Cel.HalftoneSoftness = FMath::Clamp(S->HalftoneSoftness, 0.01f, 0.5f);
    NewOptions.Cel.HalftoneTint = FVector3f(S->HalftoneTint.R, S->HalftoneTint.G, S->HalftoneTint.B);

    // Outline options
    NewOptions.Cel.Width = FMath::Clamp(S->OutlineWidth, 0.5f, 4.0f);
    NewOptions.Cel.OutlineStrength = FMath::Clamp(S->OutlineStrength, 0.0f, 1.0f);
    NewOptions.Cel.OutlineColor = FVector3f(S->OutlineColor.R, S->OutlineColor.G, S->OutlineColor.B);
    NewOptions.Cel.DepthThreshold = FMath::Clamp(S->OutlineDepthThreshold, 0.001f, 0.2f);
    NewOptions.Cel.NormalThreshold = FMath::Clamp(S->OutlineNormalThreshold, 0.01f, 1.0f);

    NewOptions.Cel.Silhouettes = S->bSilhouetteOutlines;
    NewOptions.Cel.Creases = S->bCreaseOutlines;
    NewOptions.Cel.SeparateStyles = S->bSeparateOutlineStyles;
    NewOptions.Cel.SilhouetteWidth = FMath::Clamp(S->SilhouetteWidth, 0.5f, 8.0f);
    NewOptions.Cel.SilhouetteStrength = FMath::Clamp(S->SilhouetteStrength, 0.0f, 1.0f);
    NewOptions.Cel.CreaseWidth = FMath::Clamp(S->CreaseWidth, 0.5f, 8.0f);
    NewOptions.Cel.CreaseStrength = FMath::Clamp(S->CreaseStrength, 0.0f, 1.0f);
    NewOptions.Cel.SilhouetteColor = FVector3f(S->SilhouetteColor.R, S->SilhouetteColor.G, S->SilhouetteColor.B);
    NewOptions.Cel.CreaseColor = FVector3f(S->CreaseColor.R, S->CreaseColor.G, S->CreaseColor.B);

    // Adaptive Inking & Distance Fade
    NewOptions.Cel.bAdaptiveInkColor = S->bAdaptiveInkColor;
    NewOptions.Cel.AdaptiveInkStrength = FMath::Clamp(S->AdaptiveInkStrength, 0.0f, 1.0f);
    NewOptions.Cel.AdaptiveInkDarkness = FMath::Clamp(S->AdaptiveInkDarkness, 0.0f, 1.0f);
    NewOptions.Cel.bDistanceFade = S->bOutlineDistanceFade;
    NewOptions.Cel.FadeStartDistance = FMath::Max(10.0f, S->OutlineFadeStartDistance);
    NewOptions.Cel.FadeEndDistance = FMath::Max(NewOptions.Cel.FadeStartDistance + 10.0f, S->OutlineFadeEndDistance);
    NewOptions.Cel.DebugViewMode = S->DebugViewMode;

    // Color ramp options
    NewOptions.Cel.RampEnabled = S->bCelColorRamp;
    NewOptions.Cel.ContinuousRamp = S->bCelContinuousRamp;
    NewOptions.Cel.RampStrength = FMath::Clamp(S->CelColorRampStrength, 0.0f, 1.0f);
    NewOptions.Cel.LightColorInfluence = FMath::Clamp(S->CelLightColorInfluence, 0.0f, 1.0f);

    const FSoftObjectPath DesiredRamp = (NewOptions.Cel.Enabled && NewOptions.Cel.RampEnabled) ? S->CelColorRamp.ToSoftObjectPath() : FSoftObjectPath();
    if (DesiredRamp != LoadedRampPath)
    {
        LoadedRampPath = DesiredRamp;
        LoadedRamp.Reset(DesiredRamp.IsNull() ? nullptr : S->CelColorRamp.LoadSynchronous());
    }

    for (int32 I = 0; I < 64; ++I)
    {
        const float T = I / 63.0f;
        const FLinearColor Endpoints = FMath::Lerp(S->CelShadowColor, S->CelHighlightColor, T);
        const FLinearColor Sample = LoadedRamp.IsValid() ? LoadedRamp->GetLinearColorValue(T) : FLinearColor::White;
        const FLinearColor Tint = Endpoints * Sample;
        auto Safe = [](float V) { return FMath::IsFinite(V) ? FMath::Clamp(V, 0.0f, 64.0f) : 1.0f; };
        NewOptions.Cel.RampSamples[I] = FVector4f(Safe(Tint.R), Safe(Tint.G), Safe(Tint.B), 1.0f);
    }

    ENQUEUE_RENDER_COMMAND(FlaresToolsetOptions)(
        [this, EnqueuedOptions = MoveTemp(NewOptions)](FRHICommandListImmediate&) mutable
        {
            Options = MoveTemp(EnqueuedOptions);
        });
}

void FFlaresToolsetViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs)
{
    const bool bHasActiveDebug = (Options.Flare.DebugViewMode != EFlaresToolsetDebugViewMode::None);
    const bool bHasActivePasses = Options.Flare.Enabled || Options.Cel.Enabled || Options.Cel.Outlines
        || Options.Smear.Enabled || Options.Optics.bDistortionEnabled || Options.Optics.bAberrationEnabled;

    if ((!bHasActivePasses && !bHasActiveDebug)
        || View.GetFeatureLevel() < ERHIFeatureLevel::SM5
        || !Inputs.SceneTextures
        || View.bIsSceneCapture
        || View.bIsReflectionCapture
        || !View.Family->EngineShowFlags.Lighting
        || View.Family->EngineShowFlags.PathTracing
        || View.StereoPass != EStereoscopicPass::eSSP_FULL
        || IsForwardShadingEnabled(View.GetShaderPlatform())
        || Substrate::IsSubstrateEnabled())
    {
        return;
    }

    const auto& T = *Inputs.SceneTextures->GetContents();
    if (!T.SceneColorTexture)
    {
        return;
    }

    // 1. Diagnostic View Modes: BrightMask or FlaresOnly
    if (Options.Flare.DebugViewMode == EFlaresToolsetDebugViewMode::BrightMask || Options.Flare.DebugViewMode == EFlaresToolsetDebugViewMode::FlaresOnly)
    {
        AddFlaresToolsetFlarePasses(GraphBuilder, View, T.SceneColorTexture, Options.Flare);
        return;
    }

    // 2. Diagnostic View Modes: SmearOnly
    if (Options.Smear.DebugViewMode == EFlaresToolsetDebugViewMode::SmearOnly)
    {
        AddFlaresToolsetSmearPasses(GraphBuilder, View, T.SceneColorTexture, Options.Smear);
        return;
    }

    // 3. Diagnostic View Modes: OutlinesOnly or CelLightingOnly
    if (Options.Cel.DebugViewMode == EFlaresToolsetDebugViewMode::OutlinesOnly || Options.Cel.DebugViewMode == EFlaresToolsetDebugViewMode::CelLightingOnly)
    {
        if (T.SceneDepthTexture && T.GBufferATexture && T.GBufferBTexture && T.GBufferCTexture)
        {
            if (T.GBufferATexture->Desc.Extent == T.SceneDepthTexture->Desc.Extent)
            {
                AddFlaresToolsetCelPass(GraphBuilder, View, T, Options.Cel);
            }
        }
        return;
    }

    // 4. Diagnostic View Modes: DistortionGrid
    if (Options.Optics.DebugViewMode == EFlaresToolsetDebugViewMode::DistortionGrid)
    {
        AddFlaresToolsetOpticsPass(GraphBuilder, View, T.SceneColorTexture, Options.Optics);
        return;
    }

    // 5. Standard Pipeline:
    // a) Cel Shading, Halftone & Ink Outlines
    if (Options.Cel.Enabled || Options.Cel.Outlines)
    {
        if (T.SceneDepthTexture && T.GBufferATexture && T.GBufferBTexture && T.GBufferCTexture)
        {
            if (T.GBufferATexture->Desc.Extent == T.SceneDepthTexture->Desc.Extent)
            {
                AddFlaresToolsetCelPass(GraphBuilder, View, T, Options.Cel);
            }
        }
    }

    // b) Lens Flares (Anamorphic / Multi-Axis streaks)
    if (Options.Flare.Enabled)
    {
        AddFlaresToolsetFlarePasses(GraphBuilder, View, T.SceneColorTexture, Options.Flare);
    }

    // c) Sensor Smear (CCD column overflow or CMOS row overload)
    if (Options.Smear.Enabled)
    {
        AddFlaresToolsetSmearPasses(GraphBuilder, View, T.SceneColorTexture, Options.Smear);
    }

    // d) Anamorphic Lens Optics (Barrel/Pincushion warp + Radial Chromatic Aberration)
    if (Options.Optics.bDistortionEnabled || Options.Optics.bAberrationEnabled)
    {
        AddFlaresToolsetOpticsPass(GraphBuilder, View, T.SceneColorTexture, Options.Optics);
    }
}
