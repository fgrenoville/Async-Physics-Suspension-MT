// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "CoreMinimal.h"
#include "SWPDebugDrawCategory.h"
#include "SWPDebugDrawShape.h"

struct FSWPDebugDrawCommand
{
	ESWPDebugDrawShape Shape = ESWPDebugDrawShape::Line;
    ESWPDebugDrawCategory Category = ESWPDebugDrawCategory::Misc;

    FColor Color = FColor::White;
    float  Duration = 0.f;
    float  Thickness = 1.f;
    uint8  bDepthTest : 1 = 1;
    uint8  bPersistent: 1 = 0;

    FVector P0 = FVector::ZeroVector;  // Line/Arrow start, Sphere/Box center, Point
    FVector P1 = FVector::ZeroVector;  // Line/Arrow end
    FRotator Rot = FRotator::ZeroRotator; // Box orientation

    FVector Extents = FVector::ZeroVector; // Box half-extent
    float   Radius  = 0.f;                 // Sphere radius
    float   ArrowSize  = 0.f;              // Arrow cone size
    
    static FSWPDebugDrawCommand MakeLine(const FVector& A, const FVector& B,
                                         FColor Col, float Thick=1.f, float Dur=0.f,
                                         ESWPDebugDrawCategory Cat=ESWPDebugDrawCategory::Misc, bool bDepth=true)
    {
        FSWPDebugDrawCommand C;
        C.Shape = ESWPDebugDrawShape::Line; C.Category = Cat;
        C.Color = Col; C.Thickness = Thick; C.Duration = Dur; C.bDepthTest = bDepth;
        C.P0 = A; C.P1 = B;
        return C;
    }

    static FSWPDebugDrawCommand MakeArrow(const FVector& A, const FVector& B,
                                          float Arrow=20.f, FColor Col=FColor::Yellow,
                                          float Thick=1.f, float Dur=0.f,
                                          ESWPDebugDrawCategory Cat=ESWPDebugDrawCategory::Misc, bool bDepth=true)
    {
        FSWPDebugDrawCommand C;
        C.Shape = ESWPDebugDrawShape::Arrow; C.Category = Cat;
        C.Color = Col; C.Thickness = Thick; C.Duration = Dur; C.bDepthTest = bDepth;
        C.P0 = A; C.P1 = B; C.ArrowSize = Arrow;
        return C;
    }

    static FSWPDebugDrawCommand MakeSphere(const FVector& Center, float Rad,
                                           FColor Col, float Dur=0.f,
                                           ESWPDebugDrawCategory Cat=ESWPDebugDrawCategory::Misc, bool bDepth=true)
    {
        FSWPDebugDrawCommand C;
        C.Shape = ESWPDebugDrawShape::Sphere; C.Category = Cat;
        C.Color = Col; C.Duration = Dur; C.bDepthTest = bDepth;
        C.P0 = Center; C.Radius = Rad;
        return C;
    }

    static FSWPDebugDrawCommand MakeBox(const FVector& Center, const FVector& HalfExt, const FRotator& R,
                                        FColor Col, float Dur=0.f,
                                        ESWPDebugDrawCategory Cat=ESWPDebugDrawCategory::Misc, bool bDepth=true)
    {
        FSWPDebugDrawCommand C;
        C.Shape = ESWPDebugDrawShape::Box; C.Category = Cat;
        C.Color = Col; C.Duration = Dur; C.bDepthTest = bDepth;
        C.P0 = Center; C.Extents = HalfExt; C.Rot = R;
        return C;
    }

    static FSWPDebugDrawCommand MakePoint(const FVector& P, FColor Col,
                                          float Size=8.f, float Dur=0.f,
                                          ESWPDebugDrawCategory Cat=ESWPDebugDrawCategory::Misc, bool bDepth=true)
    {
        FSWPDebugDrawCommand C;
        C.Shape = ESWPDebugDrawShape::Point; C.Category = Cat;
        C.Color = Col; C.Duration = Dur; C.bDepthTest = bDepth;
        C.P0 = P; C.Radius = Size;
        return C;
    }
};
