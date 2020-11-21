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

#include "core/platform.h"
#include "gfx/gfx_util.h"
#include "gfx/blender.h"

// CHECKME: is this hack still relevant?
#if AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_ANDROID
extern int psp_gfx_renderer;
#endif

namespace AGS
{
namespace Engine
{

using namespace Common;

namespace GfxUtil
{

Bitmap *ConvertBitmap(Bitmap *src, int dst_color_depth)
{
    int src_col_depth = src->GetColorDepth();
    if (src_col_depth != dst_color_depth)
    {
        int old_conv = get_color_conversion();
        // TODO: find out what is this, and why do we need to call this every time (do we?)
        set_color_conversion(COLORCONV_KEEP_TRANS | COLORCONV_TOTAL);
        Bitmap *dst = BitmapHelper::CreateBitmapCopy(src, dst_color_depth);
        set_color_conversion(old_conv);
        return dst;
    }
    return src;
}


typedef BLENDER_FUNC PfnBlenderCb;

struct BlendModeSetter
{
    // Blender setter for destination with and without alpha channel;
    // assign null pointer if not supported
    PfnBlenderCb AllAlpha;       // src w alpha   -> dst w alpha
    PfnBlenderCb AlphaToOpaque;  // src w alpha   -> dst w/o alpha
    PfnBlenderCb OpaqueToAlpha;  // src w/o alpha -> dst w alpha
    PfnBlenderCb OpaqueToAlphaNoTrans; // src w/o alpha -> dst w alpha (opt-ed for no transparency)
    PfnBlenderCb AllOpaque;      // src w/o alpha -> dst w/o alpha
};

// Array of blender descriptions
// NOTE: set NULL function pointer to fallback to common image blitting
static const BlendModeSetter BlendModeSets[kNumBlendModes] =
{
    { nullptr, nullptr, nullptr, nullptr, nullptr }, // kBlendMode_NoAlpha
    { _argb2argb_blender, _argb2rgb_blender, _rgb2argb_blender, _opaque_alpha_blender, nullptr }, // kBlendMode_Alpha
    // NOTE: add new modes here
};

bool SetBlender(BlendMode blend_mode, bool dst_has_alpha, bool src_has_alpha, int blend_alpha)
{
    if (blend_mode < 0 || blend_mode > kNumBlendModes)
        return false;
    const BlendModeSetter &set = BlendModeSets[blend_mode];
    PfnBlenderCb blender;
    if (dst_has_alpha)
        blender = src_has_alpha ? set.AllAlpha :
            (blend_alpha == 0xFF ? set.OpaqueToAlphaNoTrans : set.OpaqueToAlpha);
    else
        blender = src_has_alpha ? set.AlphaToOpaque : set.AllOpaque;

    if (blender)
    {
        set_blender_mode(nullptr, nullptr, blender, 0, 0, 0, blend_alpha);
        return true;
    }
    return false;
}

void DrawSpriteBlend(Bitmap *ds, const Point &ds_at, Bitmap *sprite,
                       BlendMode blend_mode,  bool dst_has_alpha, bool src_has_alpha, int blend_alpha)
{
    if (blend_alpha <= 0)
        return; // do not draw 100% transparent image

    if (// support only 32-bit blending at the moment
        ds->GetColorDepth() == 32 && sprite->GetColorDepth() == 32 &&
        // set blenders if applicable and tell if succeeded
        SetBlender(blend_mode, dst_has_alpha, src_has_alpha, blend_alpha))
    {
        ds->TransBlendBlt(sprite, ds_at.X, ds_at.Y);
    }
    else
    {
        GfxUtil::DrawSpriteWithTransparency(ds, sprite, ds_at.X, ds_at.Y, blend_alpha);
    }
}

void DrawSpriteWithTransparency(Bitmap *ds, Bitmap *sprite, int x, int y, int alpha)
{
    if (alpha <= 0)
    {
        // fully transparent, don't draw it at all
        return;
    }

    int surface_depth = ds->GetColorDepth();
    int sprite_depth  = sprite->GetColorDepth();

    if (sprite_depth < surface_depth
        // CHECKME: what is the purpose of this hack and is this still relevant?
#if AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_ANDROID
        || (ds->GetBPP() < surface_depth && psp_gfx_renderer > 0) // Fix for corrupted speechbox outlines with the OGL driver
#endif
        )
    {
        // If sprite is lower color depth than destination surface, e.g.
        // 8-bit sprites drawn on 16/32-bit surfaces.
        if (sprite_depth == 8 && surface_depth >= 24)
        {
            // 256-col sprite -> truecolor background
            // this is automatically supported by allegro, no twiddling needed
            ds->Blit(sprite, x, y, kBitmap_Transparency);
            return;
        }

        // 256-col sprite -> hi-color background, or
        // 16-bit sprite -> 32-bit background
        Bitmap hctemp;
        hctemp.CreateCopy(sprite, surface_depth);
        if (sprite_depth == 8)
        {
            // only do this for 256-col -> hi-color, cos the Blit call converts
            // transparency for 16->32 bit
            color_t mask_color = hctemp.GetMaskColor();
            for (int scan_y = 0; scan_y < hctemp.GetHeight(); ++scan_y)
            {
                // we know this must be 1 bpp source and 2 bpp pixel destination
                const uint8_t *src_scanline = sprite->GetScanLine(scan_y);
                uint16_t      *dst_scanline = (uint16_t*)hctemp.GetScanLineForWriting(scan_y);
                for (int scan_x = 0; scan_x < hctemp.GetWidth(); ++scan_x)
                {
                    if (src_scanline[scan_x] == 0)
                    {
                        dst_scanline[scan_x] = mask_color;
                    }
                }
            }
        }

        if (alpha < 0xFF) 
        {
            set_trans_blender(0, 0, 0, alpha);
            ds->TransBlendBlt(&hctemp, x, y);
        }
        else
        {
            ds->Blit(&hctemp, x, y, kBitmap_Transparency);
        }
    }
    else
    {
        if (alpha < 0xFF && surface_depth > 8 && sprite_depth > 8) 
        {
            set_trans_blender(0, 0, 0, alpha);
            ds->TransBlendBlt(sprite, x, y);
        }
        else
        {
            ds->Blit(sprite, x, y, kBitmap_Transparency);
        }
    }
}

} // namespace GfxUtil

} // namespace Engine
} // namespace AGS
