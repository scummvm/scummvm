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
//
// Graphic definitions and type/unit conversions.
//
//=============================================================================
#ifndef __AGS_CN_GFX__GFXDEF_H
#define __AGS_CN_GFX__GFXDEF_H

namespace AGS
{
namespace Common
{

enum BlendMode
{
    // free blending (ARGB -> ARGB) modes
    kBlendMode_NoAlpha        = 0, // ignore alpha channel
    kBlendMode_Alpha,              // alpha-blend src to dest, combining src & dest alphas
    // NOTE: add new modes here

    kNumBlendModes
};

namespace GfxDef
{
    inline int Trans100ToAlpha255(int transparency)
    {
        return ((100 - transparency) * 255) / 100;
    }

    inline int Alpha255ToTrans100(int alpha)
    {
        return 100 - ((alpha * 100) / 255);
    }

    // Special formulae to reduce precision loss and support flawless forth &
    // reverse conversion for multiplies of 10%
    inline int Trans100ToAlpha250(int transparency)
    {
        return ((100 - transparency) * 25) / 10;
    }

    inline int Alpha250ToTrans100(int alpha)
    {
        return 100 - ((alpha * 10) / 25);
    }

    // Convert correct 100-ranged transparency into legacy 255-ranged
    // transparency; legacy inconsistent transparency value range:
    // 0   = opaque,
    // 255 = invisible,
    // 1 -to- 254 = barely visible -to- mostly visible (as proper alpha)
    inline int Trans100ToLegacyTrans255(int transparency)
    {
        if (transparency == 0)
        {
            return 0; // this means opaque
        }
        else if (transparency == 100)
        {
            return 255; // this means invisible
        }
        // the rest of the range works as alpha
        return Trans100ToAlpha250(transparency);
    }

    // Convert legacy 255-ranged "incorrect" transparency into proper
    // 100-ranged transparency.
    inline int LegacyTrans255ToTrans100(int legacy_transparency)
    {
        if (legacy_transparency == 0)
        {
            return 0; // this means opaque
        }
        else if (legacy_transparency == 255)
        {
            return 100; // this means invisible
        }
        // the rest of the range works as alpha
        return Alpha250ToTrans100(legacy_transparency);
    }

    // Convert legacy 100-ranged transparency into proper 255-ranged alpha
    // 0      => alpha 255
    // 100    => alpha 0
    // 1 - 99 => alpha 1 - 244
    inline int LegacyTrans100ToAlpha255(int legacy_transparency)
    {
        if (legacy_transparency == 0)
        {
            return 255; // this means opaque
        }
        else if (legacy_transparency == 100)
        {
            return 0; // this means invisible
        }
        // the rest of the range works as alpha (only 100-ranged)
        return legacy_transparency * 255 / 100;
    }
} // namespace GfxDef

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_GFX__GFXDEF_H
