//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#ifndef __AGS_EE_GFX__GFXDEFINES_H
#define __AGS_EE_GFX__GFXDEFINES_H

#include "core/types.h"

namespace AGS
{
namespace Engine
{

// TODO: find the way to merge this with sprite batch transform
enum GlobalFlipType
{
    kFlip_None,
    kFlip_Horizontal, // this means - mirror over horizontal middle line
    kFlip_Vertical,   // this means - mirror over vertical middle line
    kFlip_Both
};

// GraphicResolution struct determines image size and color depth
struct GraphicResolution
{
    int32_t Width;
    int32_t Height;
    int32_t ColorDepth;

    GraphicResolution()
        : Width(0)
        , Height(0)
        , ColorDepth(0)
    {
    }

    GraphicResolution(int32_t width, int32_t height, int32_t color_depth)
    {
        Width = width;
        Height = height;
        ColorDepth = color_depth;
    }

    inline bool IsValid() const { return Width > 0 && Height > 0 && ColorDepth > 0; }
};

// DisplayMode struct provides extended description of display mode
struct DisplayMode : public GraphicResolution
{
    int32_t RefreshRate;
    bool    Vsync;
    bool    Windowed;

    DisplayMode()
        : RefreshRate(0)
        , Vsync(false)
        , Windowed(false)
    {}

    DisplayMode(const GraphicResolution &res, bool windowed = false, int32_t refresh = 0, bool vsync = false)
        : GraphicResolution(res)
        , RefreshRate(refresh)
        , Vsync(vsync)
        , Windowed(windowed)
    {}
};

} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GFX__GFXDEFINES_H
