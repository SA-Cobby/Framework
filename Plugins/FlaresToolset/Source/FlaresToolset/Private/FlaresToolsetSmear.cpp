#include "FlaresToolsetSmear.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "SceneRendering.h"
#include "ScreenPass.h"
#include "RenderGraphUtils.h"

class FFlaresToolsetSmearPS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FFlaresToolsetSmearPS);
    SHADER_USE_PARAMETER_STRUCT(FFlaresToolsetSmearPS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SourceColor)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SmearTexture)
        SHADER_PARAMETER_SAMPLER(SamplerState, LinearSampler)
        SHADER_PARAMETER(FVector4f, Rect)
        SHADER_PARAMETER(FVector2f, SceneSize)
        SHADER_PARAMETER(FVector2f, WorkSize)
        SHADER_PARAMETER(FVector2f, DirectionStep)
        SHADER_PARAMETER(FVector3f, Tint)
        SHADER_PARAMETER(float, Threshold)
        SHADER_PARAMETER(float, Intensity)
        SHADER_PARAMETER(float, Falloff)
        SHADER_PARAMETER(int32, DebugViewMode)
        SHADER_PARAMETER(int32, Mode)
        RENDER_TARGET_BINDING_SLOTS()
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

IMPLEMENT_GLOBAL_SHADER(FFlaresToolsetSmearPS, "/Plugin/FlaresToolset/Private/FlaresToolsetSmear.usf", "MainPS", SF_Pixel);

void AddFlaresToolsetSmearPasses(
    FRDGBuilder& GraphBuilder,
    const FSceneView& View,
    FRDGTextureRef SceneColor,
    const FFlaresToolsetSmearOptions& Options)
{
    const bool bIsSmearDebug = (Options.DebugViewMode == EFlaresToolsetDebugViewMode::SmearOnly);
    if (!Options.Enabled && !bIsSmearDebug)
    {
        return;
    }
    if (Options.Intensity <= 0.0f && !bIsSmearDebug)
    {
        return;
    }

    const FIntRect R = static_cast<const FViewInfo&>(View).ViewRect;
    if (R.IsEmpty())
    {
        return;
    }

    const int32 Divider = 4;
    const FIntPoint Size(FMath::DivideAndRoundUp(R.Width(), Divider), FMath::DivideAndRoundUp(R.Height(), Divider));
    const FIntRect WorkRect(FIntPoint::ZeroValue, Size);

    const auto Desc = FRDGTextureDesc::Create2D(Size, PF_FloatRGBA, FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_ShaderResource);
    auto NewWork = [&]()
    {
        return GraphBuilder.CreateTexture(Desc, TEXT("FlaresToolset.SmearWork"));
    };

    TShaderMapRef<FFlaresToolsetSmearPS> Shader(GetGlobalShaderMap(View.GetFeatureLevel()));

    auto Params = [&](int32 Mode, FRDGTextureRef SourceTex, FRDGTextureRef SmearTex)
    {
        auto* P = GraphBuilder.AllocParameters<FFlaresToolsetSmearPS::FParameters>();
        P->View = View.ViewUniformBuffer;
        P->SourceColor = SourceTex;
        P->SmearTexture = SmearTex;
        P->LinearSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
        P->Rect = FVector4f(R.Min.X, R.Min.Y, R.Width(), R.Height());
        P->SceneSize = FVector2f(SceneColor->Desc.Extent);
        P->WorkSize = FVector2f(Size);
        P->DirectionStep = FVector2f::ZeroVector;
        P->Tint = Options.Tint;
        P->Threshold = Options.Threshold;
        P->Intensity = Options.Intensity;
        P->Falloff = Options.Falloff;
        P->DebugViewMode = (int32)Options.DebugViewMode;
        P->Mode = Mode;
        return P;
    };

    // 1. Extract extreme overload regions
    FRDGTextureRef Bright = NewWork();
    auto* ExtractParams = Params(0, SceneColor, SceneColor);
    ExtractParams->RenderTargets[0] = FRenderTargetBinding(Bright, ERenderTargetLoadAction::ENoAction);
    AddDrawScreenPass(GraphBuilder, RDG_EVENT_NAME("FlaresToolset Smear Overload Extract"), View,
        FScreenPassTextureViewport(Bright, WorkRect), FScreenPassTextureViewport(SceneColor, R), Shader, ExtractParams);

    // 2. 1D Directional Dilation
    const FVector2f Direction = (Options.Orientation == EFlaresToolsetSmearOrientation::Vertical)
        ? FVector2f(0.0f, 1.0f)
        : FVector2f(1.0f, 0.0f);

    const float Dimension = (Options.Orientation == EFlaresToolsetSmearOrientation::Vertical) ? (float)R.Height() : (float)R.Width();
    const float BaseScale = Options.Length * Dimension / (float)Divider / 255.0f;

    FRDGTextureRef CurrentSmear = Bright;
    for (int32 I = 0; I < 8; ++I)
    {
        FRDGTextureRef Next = NewWork();
        auto* P = Params(1, SceneColor, CurrentSmear);
        P->DirectionStep = Direction * (BaseScale * (1 << I)) / FVector2f(Size);
        P->RenderTargets[0] = FRenderTargetBinding(Next, ERenderTargetLoadAction::ENoAction);
        AddDrawScreenPass(GraphBuilder, RDG_EVENT_NAME("FlaresToolset Smear Dilation %d", I), View,
            FScreenPassTextureViewport(Next, WorkRect), FScreenPassTextureViewport(CurrentSmear, WorkRect), Shader, P);
        CurrentSmear = Next;
    }

    // 3. Composite onto scene color
    FScreenPassTexture Input(SceneColor, R);
    auto Output = FScreenPassRenderTarget::CreateFromInput(GraphBuilder, Input, ERenderTargetLoadAction::ENoAction, TEXT("FlaresToolset.SmearComposite"));
    auto* CompositeParams = Params(2, SceneColor, CurrentSmear);
    CompositeParams->RenderTargets[0] = Output.GetRenderTargetBinding();
    AddDrawScreenPass(GraphBuilder, RDG_EVENT_NAME("FlaresToolset Smear Composite"), View,
        FScreenPassTextureViewport(Output), FScreenPassTextureViewport(Input), Shader, CompositeParams);

    FRHICopyTextureInfo CopyInfo;
    CopyInfo.SourcePosition = FIntVector(R.Min.X, R.Min.Y, 0);
    CopyInfo.DestPosition = CopyInfo.SourcePosition;
    CopyInfo.Size = FIntVector(R.Width(), R.Height(), 1);
    AddCopyTexturePass(GraphBuilder, Output.Texture, SceneColor, CopyInfo);
}
