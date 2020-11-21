/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

//=============================================================================
//
// Standard software scaling filter
//
//=============================================================================

#ifndef __AGS_EE_GFX__ALLEGROGFXFILTER_H
#define __AGS_EE_GFX__ALLEGROGFXFILTER_H

#include "gfx/bitmap.h"
#include "gfx/gfxfilter_scaling.h"
#include "gfx/gfxdefines.h"

namespace AGS
{
namespace Engine
{
namespace ALSW
{

using Common::Bitmap;

class AllegroGfxFilter : public ScalingGfxFilter
{
public:
    AllegroGfxFilter();

    const GfxFilterInfo &GetInfo() const override;
    
    virtual Bitmap *InitVirtualScreen(Bitmap *screen, const Size src_size, const Rect dst_rect);
    virtual Bitmap *ShutdownAndReturnRealScreen();
    virtual void RenderScreen(Bitmap *toRender, int x, int y);
    virtual void RenderScreenFlipped(Bitmap *toRender, int x, int y, GlobalFlipType flipType);
    virtual void ClearRect(int x1, int y1, int x2, int y2, int color);
    virtual void GetCopyOfScreenIntoBitmap(Bitmap *copyBitmap);
    virtual void GetCopyOfScreenIntoBitmap(Bitmap *copyBitmap, bool copy_with_yoffset);

    static const GfxFilterInfo FilterInfo;

protected:
    virtual Bitmap *PreRenderPass(Bitmap *toRender);

    // pointer to real screen bitmap
    Bitmap *realScreen;
    // bitmap the size of game resolution
    Bitmap *virtualScreen;
    // buffer for making a copy of video memory before stretching
    // for screen capture
    Bitmap *realScreenSizedBuffer;
    Bitmap *lastBlitFrom;
    int     lastBlitX;
    int     lastBlitY;
};

} // namespace ALSW
} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GFX__ALLEGROGFXFILTER_H
