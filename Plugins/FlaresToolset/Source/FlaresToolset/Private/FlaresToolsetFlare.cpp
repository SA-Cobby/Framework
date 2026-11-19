#include "FlaresToolsetFlare.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "SceneRendering.h"
#include "ScreenPass.h"
#include "RenderGraphUtils.h"

class FFlaresToolsetFlarePS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FFlaresToolsetFlarePS);
    SHADER_USE_PARAMETER_STRUCT(FFlaresToolsetFlarePS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SourceColor)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, StreakTexture)
        SHADER_PARAMETER_SAMPLER(SamplerState, LinearSampler)
        SHADER_PARAMETER(FVector4f, Rect)
        SHADER_PARAMETER(FVector2f, SceneSize)
        SHADER_PARAMETER(FVector2f, WorkSize)
        SHADER_PARAMETER(FVector2f, DirectionStep)
        SHADER_PARAMETER(FVector3f, Tint)
        SHADER_PARAMETER(float, Threshold)
        SHADER_PARAMETER(float, SoftKnee)
        SHADER_PARAMETER(float, Intensity)
        SHADER_PARAMETER(float, ChromaticDispersion)
        SHADER_PARAMETER(int32, DebugViewMode)
        SHADER_PARAMETER(int32, Mode)
        RENDER_TARGET_BINDING_SLOTS()
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

IMPLEMENT_GLOBAL_SHADER(FFlaresToolsetFlarePS, "/Plugin/FlaresToolset/Private/FlaresToolsetFlare.usf", "MainPS", SF_Pixel);

void AddFlaresToolsetFlarePasses(FRDGBuilder& GraphBuilder, const FSceneView& View, FRDGTextureRef SceneColor, const FFlaresToolsetFlareOptions& Options)
{
    const bool bIsBrightMaskMode = (Options.DebugViewMode == EFlaresToolsetDebugViewMode::BrightMask);
    if (!Options.Enabled && !bIsBrightMaskMode)
    {
        return;
    }
    if (Options.Intensity <= 0.0f && !bIsBrightMaskMode)
    {
        return;
    }

    const FIntRect R = static_cast<const FViewInfo&>(View).ViewRect;
    if (R.IsEmpty())
    {
        return;
    }

    const int32 Divider = Options.HighQuality ? 2 : 4;
    const FIntPoint Size(FMath::DivideAndRoundUp(R.Width(), Divider), FMath::DivideAndRoundUp(R.Height(), Divider));
    const FIntRect WorkRect(FIntPoint::ZeroValue, Size);

    const auto Desc = FRDGTextureDesc::Create2D(Size, PF_FloatRGBA, FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_ShaderResource);
    auto NewWork = [&]()
    {
        return GraphBuilder.CreateTexture(Desc, TEXT("FlaresToolset.FlareWork"));
    };

    TShaderMapRef<FFlaresToolsetFlarePS> Shader(GetGlobalShaderMap(View.GetFeatureLevel()));

    auto Params = [&](int32 Mode, FRDGTextureRef SourceTex, FRDGTextureRef StreakTex)
    {
        auto* P = GraphBuilder.AllocParameters<FFlaresToolsetFlarePS::FParameters>();
        P->View = View.ViewUniformBuffer;
        P->SourceColor = SourceTex;
        P->StreakTexture = StreakTex;
        P->LinearSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
        P->Rect = FVector4f(R.Min.X, R.Min.Y, R.Width(), R.Height());
        P->SceneSize = FVector2f(SceneColor->Desc.Extent);
        P->WorkSize = FVector2f(Size);
        P->DirectionStep = FVector2f::ZeroVector;
        P->Tint = Options.Tint;
        P->Threshold = Options.Threshold;
        P->SoftKnee = Options.SoftKnee;
        P->Intensity = Options.Intensity;
        P->ChromaticDispersion = Options.ChromaticDispersion;
        P->DebugViewMode = (int32)Options.DebugViewMode;
        P->Mode = Mode;
        return P;
    };

    // 1. Extract bright regions
    FRDGTextureRef Bright = NewWork();
    auto* ExtractParams = Params(0, SceneColor, SceneColor);
    ExtractParams->RenderTargets[0] = FRenderTargetBinding(Bright, ERenderTargetLoadAction::ENoAction);
    AddDrawScreenPass(GraphBuilder, RDG_EVENT_NAME("FlaresToolset Flare Bright Extract"), View,
        FScreenPassTextureViewport(Bright, WorkRect), FScreenPassTextureViewport(SceneColor, R), Shader, ExtractParams);

    // If BrightMask debug mode is active, output directly
    if (bIsBrightMaskMode)
    {
        FScreenPassTexture Input(SceneColor, R);
        auto Output = FScreenPassRenderTarget::CreateFromInput(GraphBuilder, Input, ERenderTargetLoadAction::ENoAction, TEXT("FlaresToolset.BrightMaskOutput"));
        auto* MaskParams = Params(4, SceneColor, Bright);
        MaskParams->RenderTargets[0] = Output.GetRenderTargetBinding();
        AddDrawScreenPass(GraphBuilder, RDG_EVENT_NAME("FlaresToolset Debug Bright Mask"), View,
            FScreenPassTextureViewport(Output), FScreenPassTextureViewport(Input), Shader, MaskParams);

        FRHICopyTextureInfo CopyInfo;
        CopyInfo.SourcePosition = FIntVector(R.Min.X, R.Min.Y, 0);
        CopyInfo.DestPosition = CopyInfo.SourcePosition;
        CopyInfo.Size = FIntVector(R.Width(), R.Height(), 1);
        AddCopyTexturePass(GraphBuilder, Output.Texture, SceneColor, CopyInfo);
        return;
    }

    // Determine axes based on flare type
    TArray<float, TInlineAllocator<4>> AxisAngles;
    switch (Options.Type)
    {
    case EFlaresToolsetFlareType::VerticalAnamorphic:
        AxisAngles.Add(Options.Rotation + 90.0f);
        break;
    case EFlaresToolsetFlareType::Cross:
        AxisAngles.Add(Options.Rotation);
        AxisAngles.Add(Options.Rotation + 90.0f);
        break;
    case EFlaresToolsetFlareType::Star:
        AxisAngles.Add(Options.Rotation);
        AxisAngles.Add(Options.Rotation + 60.0f);
        AxisAngles.Add(Options.Rotation + 120.0f);
        break;
    case EFlaresToolsetFlareType::Anamorphic:
    default:
        AxisAngles.Add(Options.Rotation);
        break;
    }

    const float BaseScale = Options.Length * R.Width() / Divider / 126.0f;
    FRDGTextureRef AccumulatedStreak = nullptr;
    FVector2f PrimaryDirection = FVector2f::ZeroVector;

    for (int32 AxisIndex = 0; AxisIndex < AxisAngles.Num(); ++AxisIndex)
    {
        const float AngleRad = FMath::DegreesToRadians(AxisAngles[AxisIndex]);
        const FVector2f Direction(FMath::Cos(AngleRad), FMath::Sin(AngleRad));
        if (AxisIndex == 0)
        {
            PrimaryDirection = Direction;
        }

        FRDGTextureRef CurrentStreak = Bright;
        for (int32 I = 0; I < 6; ++I)
        {
            FRDGTextureRef Next = NewWork();
            auto* P = Params(1, SceneColor, CurrentStreak);
            P->DirectionStep = Direction * (BaseScale * (1 << I)) / FVector2f(Size);
            P->RenderTargets[0] = FRenderTargetBinding(Next, ERenderTargetLoadAction::ENoAction);
            AddDrawScreenPass(GraphBuilder, RDG_EVENT_NAME("FlaresToolset Flare Axis %d Filter %d", AxisIndex, I), View,
                FScreenPassTextureViewport(Next, WorkRect), FScreenPassTextureViewport(CurrentStreak, WorkRect), Shader, P);
            CurrentStreak = Next;
        }

        if (AccumulatedStreak == nullptr)
        {
            AccumulatedStreak = CurrentStreak;
        }
        else
        {
            FRDGTextureRef Combined = NewWork();
            auto* SumParams = Params(3, AccumulatedStreak, CurrentStreak);
            SumParams->RenderTargets[0] = FRenderTargetBinding(Combined, ERenderTargetLoadAction::ENoAction);
            AddDrawScreenPass(GraphBuilder, RDG_EVENT_NAME("FlaresToolset Flare Accumulate Axis %d", AxisIndex), View,
                FScreenPassTextureViewport(Combined, WorkRect), FScreenPassTextureViewport(AccumulatedStreak, WorkRect), Shader, SumParams);
            AccumulatedStreak = Combined;
        }
    }

    // 3. Composite onto scene color
    FScreenPassTexture Input(SceneColor, R);
    auto Output = FScreenPassRenderTarget::CreateFromInput(GraphBuilder, Input, ERenderTargetLoadAction::ENoAction, TEXT("FlaresToolset.FlareComposite"));
    auto* CompositeParams = Params(2, SceneColor, AccumulatedStreak);
    // Provide primary streak step for spectral dispersion
    CompositeParams->DirectionStep = PrimaryDirection * (BaseScale * 8.0f) / FVector2f(Size);
    CompositeParams->RenderTargets[0] = Output.GetRenderTargetBinding();
    AddDrawScreenPass(GraphBuilder, RDG_EVENT_NAME("FlaresToolset Flare Composite"), View,
        FScreenPassTextureViewport(Output), FScreenPassTextureViewport(Input), Shader, CompositeParams);

    // Copy composite result back into SceneColor
    FRHICopyTextureInfo CopyInfo;
    CopyInfo.SourcePosition = FIntVector(R.Min.X, R.Min.Y, 0);
    CopyInfo.DestPosition = CopyInfo.SourcePosition;
    CopyInfo.Size = FIntVector(R.Width(), R.Height(), 1);
    AddCopyTexturePass(GraphBuilder, Output.Texture, SceneColor, CopyInfo);
}
