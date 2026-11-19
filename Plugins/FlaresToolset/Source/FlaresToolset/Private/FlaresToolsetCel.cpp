#include "FlaresToolsetCel.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "SceneTexturesConfig.h"
#include "SceneRendering.h"
#include "ScreenPass.h"
#include "RenderGraphUtils.h"

class FFlaresToolsetCelPS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FFlaresToolsetCelPS);
    SHADER_USE_PARAMETER_STRUCT(FFlaresToolsetCelPS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SourceColor)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, DepthTexture)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, NormalTexture)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, AlbedoTexture)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, MaterialTexture)
        SHADER_PARAMETER(FVector4f, Rect)
        SHADER_PARAMETER(int32, CelEnabled)
        SHADER_PARAMETER(int32, InkEnabled)
        SHADER_PARAMETER(int32, Bands)
        SHADER_PARAMETER(float, Strength)
        SHADER_PARAMETER(float, ShadowBrightness)
        SHADER_PARAMETER(float, WhitePoint)
        SHADER_PARAMETER(float, EmissiveThreshold)
        SHADER_PARAMETER(float, OutlineWidth)
        SHADER_PARAMETER(float, OutlineStrength)
        SHADER_PARAMETER(float, DepthThreshold)
        SHADER_PARAMETER(float, NormalThreshold)
        SHADER_PARAMETER(FVector3f, InkColor)
        SHADER_PARAMETER(int32, Silhouettes)
        SHADER_PARAMETER(int32, Creases)
        SHADER_PARAMETER(int32, SeparateStyles)
        SHADER_PARAMETER(float, SilhouetteWidth)
        SHADER_PARAMETER(float, SilhouetteStrength)
        SHADER_PARAMETER(float, CreaseWidth)
        SHADER_PARAMETER(float, CreaseStrength)
        SHADER_PARAMETER(FVector3f, SilhouetteColor)
        SHADER_PARAMETER(FVector3f, CreaseColor)
        SHADER_PARAMETER(int32, RampEnabled)
        SHADER_PARAMETER(int32, ContinuousRamp)
        SHADER_PARAMETER(float, RampStrength)
        SHADER_PARAMETER(float, LightColorInfluence)
        SHADER_PARAMETER_ARRAY(FVector4f, RampSamples, [64])
        SHADER_PARAMETER(int32, AdaptiveInkColor)
        SHADER_PARAMETER(float, AdaptiveInkStrength)
        SHADER_PARAMETER(float, AdaptiveInkDarkness)
        SHADER_PARAMETER(int32, DistanceFadeEnabled)
        SHADER_PARAMETER(float, FadeStartDistance)
        SHADER_PARAMETER(float, FadeEndDistance)
        SHADER_PARAMETER(int32, HalftoneEnabled)
        SHADER_PARAMETER(int32, HalftoneStyle)
        SHADER_PARAMETER(float, HalftoneDensity)
        SHADER_PARAMETER(float, HalftoneStrength)
        SHADER_PARAMETER(float, HalftoneThreshold)
        SHADER_PARAMETER(float, HalftoneSoftness)
        SHADER_PARAMETER(FVector3f, HalftoneTint)
        SHADER_PARAMETER(int32, DebugViewMode)
        RENDER_TARGET_BINDING_SLOTS()
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

IMPLEMENT_GLOBAL_SHADER(FFlaresToolsetCelPS, "/Plugin/FlaresToolset/Private/FlaresToolsetCel.usf", "MainPS", SF_Pixel);

void AddFlaresToolsetCelPass(
    FRDGBuilder& GraphBuilder,
    const FSceneView& View,
    const FSceneTextureUniformParameters& T,
    const FFlaresToolsetCelOptions& O)
{
    const bool bIsCelDebug = (O.DebugViewMode == EFlaresToolsetDebugViewMode::OutlinesOnly || O.DebugViewMode == EFlaresToolsetDebugViewMode::CelLightingOnly);
    if ((!O.Enabled && !O.Outlines && !bIsCelDebug) || !T.GBufferBTexture)
    {
        return;
    }

    const FIntRect R = static_cast<const FViewInfo&>(View).ViewRect;
    FScreenPassTexture Input(T.SceneColorTexture, R);
    auto Output = FScreenPassRenderTarget::CreateFromInput(GraphBuilder, Input, ERenderTargetLoadAction::ENoAction, TEXT("FlaresToolset.Cel"));

    auto* P = GraphBuilder.AllocParameters<FFlaresToolsetCelPS::FParameters>();
    P->View = View.ViewUniformBuffer;
    P->SourceColor = T.SceneColorTexture;
    P->DepthTexture = T.SceneDepthTexture;
    P->NormalTexture = T.GBufferATexture;
    P->AlbedoTexture = T.GBufferCTexture;
    P->MaterialTexture = T.GBufferBTexture;
    P->Rect = FVector4f(R.Min.X, R.Min.Y, R.Max.X - 1, R.Max.Y - 1);
    P->CelEnabled = (O.Enabled || O.DebugViewMode == EFlaresToolsetDebugViewMode::CelLightingOnly) ? 1 : 0;
    P->InkEnabled = (O.Outlines || O.DebugViewMode == EFlaresToolsetDebugViewMode::OutlinesOnly) ? 1 : 0;
    P->Bands = O.Bands;
    P->Strength = O.Strength;
    P->ShadowBrightness = O.Shadow;
    P->WhitePoint = O.WhitePoint;
    P->EmissiveThreshold = O.EmissiveThreshold;
    P->OutlineWidth = O.Width;
    P->OutlineStrength = O.OutlineStrength;
    P->DepthThreshold = O.DepthThreshold;
    P->NormalThreshold = O.NormalThreshold;
    P->InkColor = O.OutlineColor;
    P->Silhouettes = O.Silhouettes ? 1 : 0;
    P->Creases = O.Creases ? 1 : 0;
    P->SeparateStyles = O.SeparateStyles ? 1 : 0;
    P->SilhouetteWidth = O.SilhouetteWidth;
    P->SilhouetteStrength = O.SilhouetteStrength;
    P->CreaseWidth = O.CreaseWidth;
    P->CreaseStrength = O.CreaseStrength;
    P->SilhouetteColor = O.SilhouetteColor;
    P->CreaseColor = O.CreaseColor;
    P->RampEnabled = O.RampEnabled ? 1 : 0;
    P->ContinuousRamp = O.ContinuousRamp ? 1 : 0;
    P->RampStrength = O.RampStrength;
    P->LightColorInfluence = O.LightColorInfluence;
    for (int32 I = 0; I < 64; ++I)
    {
        P->RampSamples[I] = O.RampSamples[I];
    }
    P->AdaptiveInkColor = O.bAdaptiveInkColor ? 1 : 0;
    P->AdaptiveInkStrength = O.AdaptiveInkStrength;
    P->AdaptiveInkDarkness = O.AdaptiveInkDarkness;
    P->DistanceFadeEnabled = O.bDistanceFade ? 1 : 0;
    P->FadeStartDistance = O.FadeStartDistance;
    P->FadeEndDistance = O.FadeEndDistance;
    P->HalftoneEnabled = O.bHalftoneEnabled ? 1 : 0;
    P->HalftoneStyle = (int32)O.HalftoneStyle;
    P->HalftoneDensity = O.HalftoneDensity;
    P->HalftoneStrength = O.HalftoneStrength;
    P->HalftoneThreshold = O.HalftoneThreshold;
    P->HalftoneSoftness = O.HalftoneSoftness;
    P->HalftoneTint = O.HalftoneTint;
    P->DebugViewMode = (int32)O.DebugViewMode;
    P->RenderTargets[0] = Output.GetRenderTargetBinding();

    TShaderMapRef<FFlaresToolsetCelPS> Shader(GetGlobalShaderMap(View.GetFeatureLevel()));
    AddDrawScreenPass(GraphBuilder, RDG_EVENT_NAME("FlaresToolset Cel Shading and Outlines"), View,
        FScreenPassTextureViewport(Output), FScreenPassTextureViewport(Input), Shader, P);

    FRHICopyTextureInfo Copy;
    Copy.SourcePosition = FIntVector(R.Min.X, R.Min.Y, 0);
    Copy.DestPosition = Copy.SourcePosition;
    Copy.Size = FIntVector(R.Width(), R.Height(), 1);
    AddCopyTexturePass(GraphBuilder, Output.Texture, T.SceneColorTexture, Copy);
}
