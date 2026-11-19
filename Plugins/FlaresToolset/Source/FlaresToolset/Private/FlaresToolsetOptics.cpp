#include "FlaresToolsetOptics.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "SceneRendering.h"
#include "ScreenPass.h"
#include "RenderGraphUtils.h"

class FFlaresToolsetOpticsPS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FFlaresToolsetOpticsPS);
    SHADER_USE_PARAMETER_STRUCT(FFlaresToolsetOpticsPS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SourceColor)
        SHADER_PARAMETER_SAMPLER(SamplerState, LinearSampler)
        SHADER_PARAMETER(FVector4f, Rect)
        SHADER_PARAMETER(FVector2f, SceneSize)
        SHADER_PARAMETER(int32, DistortionEnabled)
        SHADER_PARAMETER(float, DistortionStrength)
        SHADER_PARAMETER(float, AnamorphicSqueeze)
        SHADER_PARAMETER(float, CurvatureAxisRatio)
        SHADER_PARAMETER(float, DistortionScale)
        SHADER_PARAMETER(int32, AberrationEnabled)
        SHADER_PARAMETER(float, AberrationStrength)
        SHADER_PARAMETER(float, AberrationRadialExponent)
        SHADER_PARAMETER(float, AberrationEdgeThreshold)
        SHADER_PARAMETER(float, AberrationEdgeSensitivity)
        SHADER_PARAMETER(int32, DebugViewMode)
        RENDER_TARGET_BINDING_SLOTS()
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

IMPLEMENT_GLOBAL_SHADER(FFlaresToolsetOpticsPS, "/Plugin/FlaresToolset/Private/FlaresToolsetOptics.usf", "MainPS", SF_Pixel);

void AddFlaresToolsetOpticsPass(
    FRDGBuilder& GraphBuilder,
    const FSceneView& View,
    FRDGTextureRef SceneColor,
    const FFlaresToolsetOpticsOptions& Options)
{
    const bool bIsGridDebug = (Options.DebugViewMode == EFlaresToolsetDebugViewMode::DistortionGrid);
    if (!Options.bDistortionEnabled && !Options.bAberrationEnabled && !bIsGridDebug)
    {
        return;
    }

    const FIntRect R = static_cast<const FViewInfo&>(View).ViewRect;
    if (R.IsEmpty())
    {
        return;
    }

    FScreenPassTexture Input(SceneColor, R);
    auto Output = FScreenPassRenderTarget::CreateFromInput(GraphBuilder, Input, ERenderTargetLoadAction::ENoAction, TEXT("FlaresToolset.Optics"));

    auto* P = GraphBuilder.AllocParameters<FFlaresToolsetOpticsPS::FParameters>();
    P->View = View.ViewUniformBuffer;
    P->SourceColor = SceneColor;
    P->LinearSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
    P->Rect = FVector4f(R.Min.X, R.Min.Y, R.Width(), R.Height());
    P->SceneSize = FVector2f(SceneColor->Desc.Extent);
    P->DistortionEnabled = Options.bDistortionEnabled ? 1 : 0;
    P->DistortionStrength = Options.DistortionStrength;
    P->AnamorphicSqueeze = Options.AnamorphicSqueeze;
    P->CurvatureAxisRatio = Options.CurvatureAxisRatio;
    P->DistortionScale = Options.DistortionScale;
    P->AberrationEnabled = Options.bAberrationEnabled ? 1 : 0;
    P->AberrationStrength = Options.AberrationStrength;
    P->AberrationRadialExponent = Options.AberrationRadialExponent;
    P->AberrationEdgeThreshold = Options.AberrationEdgeThreshold;
    P->AberrationEdgeSensitivity = Options.AberrationEdgeSensitivity;
    P->DebugViewMode = (int32)Options.DebugViewMode;
    P->RenderTargets[0] = Output.GetRenderTargetBinding();

    TShaderMapRef<FFlaresToolsetOpticsPS> Shader(GetGlobalShaderMap(View.GetFeatureLevel()));
    AddDrawScreenPass(GraphBuilder, RDG_EVENT_NAME("FlaresToolset Anamorphic Optics"), View,
        FScreenPassTextureViewport(Output), FScreenPassTextureViewport(Input), Shader, P);

    FRHICopyTextureInfo CopyInfo;
    CopyInfo.SourcePosition = FIntVector(R.Min.X, R.Min.Y, 0);
    CopyInfo.DestPosition = CopyInfo.SourcePosition;
    CopyInfo.Size = FIntVector(R.Width(), R.Height(), 1);
    AddCopyTexturePass(GraphBuilder, Output.Texture, SceneColor, CopyInfo);
}
