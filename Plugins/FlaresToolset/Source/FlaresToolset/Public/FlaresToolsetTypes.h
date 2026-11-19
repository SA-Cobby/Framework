#pragma once

#include "CoreMinimal.h"
#include "FlaresToolsetTypes.generated.h"

UENUM(BlueprintType)
enum class EFlaresToolsetFlareType : uint8
{
    Anamorphic          UMETA(DisplayName = "Horizontal Anamorphic"),
    VerticalAnamorphic  UMETA(DisplayName = "Vertical Anamorphic"),
    Cross               UMETA(DisplayName = "Cross (Horizontal + Vertical)"),
    Star                UMETA(DisplayName = "Star (Three-Ray / 120°)")
};

UENUM(BlueprintType)
enum class EFlaresToolsetSmearOrientation : uint8
{
    Vertical    UMETA(DisplayName = "Vertical (CCD Column Register Overload)"),
    Horizontal  UMETA(DisplayName = "Horizontal (CMOS Row Saturation Smear)")
};

UENUM(BlueprintType)
enum class EFlaresToolsetHalftoneStyle : uint8
{
    Dots            UMETA(DisplayName = "Ben-Day / Comic Halftone Dots (45° Print Grid)"),
    Crosshatch      UMETA(DisplayName = "Manga Dual Crosshatch Lines"),
    DiagonalLines   UMETA(DisplayName = "Single-Direction Hatch Lines")
};

UENUM(BlueprintType)
enum class EFlaresToolsetDebugViewMode : uint8
{
    None                UMETA(DisplayName = "None (Normal Final Composite)"),
    FlaresOnly          UMETA(DisplayName = "Isolated Flares (Over Black)"),
    OutlinesOnly        UMETA(DisplayName = "Isolated Outlines (Lines Over White)"),
    BrightMask          UMETA(DisplayName = "Bright Extraction Threshold Mask"),
    CelLightingOnly     UMETA(DisplayName = "Cel Lighting Bands Only (No Albedo)"),
    SmearOnly           UMETA(DisplayName = "Isolated Sensor Smear (Over Black)"),
    DistortionGrid      UMETA(DisplayName = "Lens Distortion Alignment Grid")
};
