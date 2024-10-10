/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/std/memory.h"
#include "ags/shared/core/platform.h"
#include "ags/engine/gfx/gfx_util.h"
#include "ags/engine/gfx/blender.h"

namespace AGS3 {

namespace AGS {
namespace Engine {

using namespace Shared;

namespace GfxUtil {

Bitmap *ConvertBitmap(Bitmap *src, int dst_color_depth) {
	int src_col_depth = src->GetColorDepth();
	if (src_col_depth != dst_color_depth) {
		int old_conv = get_color_conversion();
		// TODO: find out what is this, and why do we need to call this every time (do we?)
		set_color_conversion(COLORCONV_KEEP_TRANS | COLORCONV_TOTAL);
		Bitmap *dst = BitmapHelper::CreateBitmapCopy(src, dst_color_depth);
		set_color_conversion(old_conv);
		return dst;
	}
	return src;
}


struct BlendModeSetter {
	// Blender setter for destination with and without alpha channel;
	// assign kRgbToRgbBlender if not supported
	BlenderMode AllAlpha;       // src w alpha   -> dst w alpha
	BlenderMode AlphaToOpaque;  // src w alpha   -> dst w/o alpha
	BlenderMode OpaqueToAlpha;  // src w/o alpha -> dst w alpha
	BlenderMode OpaqueToAlphaNoTrans; // src w/o alpha -> dst w alpha (opt-ed for no transparency)
	BlenderMode AllOpaque;      // src w/o alpha -> dst w/o alpha
};

// Array of blender descriptions
// NOTE: set kRgbToRgbBlender to fallback to common image blitting
static const BlendModeSetter BlendModeSets[kNumBlendModes] = {
	{ kRgbToRgbBlender, kRgbToRgbBlender, kRgbToRgbBlender, kRgbToRgbBlender, kRgbToRgbBlender }, // kBlendMode_NoAlpha
	{ kArgbToArgbBlender, kArgbToRgbBlender, kRgbToArgbBlender, kOpaqueBlenderMode, kRgbToRgbBlender }, // kBlendMode_Alpha
	// NOTE: add new modes here
};

bool SetBlender(BlendMode blend_mode, bool dst_has_alpha, bool src_has_alpha, int blend_alpha) {
	if (blend_mode < 0 || blend_mode >= kNumBlendModes)
		return false;
	const BlendModeSetter &set = BlendModeSets[blend_mode];
	BlenderMode blender;
	if (dst_has_alpha)
		blender = src_has_alpha ? set.AllAlpha :
		          (blend_alpha == 0xFF ? set.OpaqueToAlphaNoTrans : set.OpaqueToAlpha);
	else
		blender = src_has_alpha ? set.AlphaToOpaque : set.AllOpaque;

	set_blender_mode(blender, 0, 0, 0, blend_alpha);
	return true;
}

void DrawSpriteBlend(Bitmap *ds, const Point &ds_at, Bitmap *sprite,
                     BlendMode blend_mode, bool dst_has_alpha, bool src_has_alpha, int blend_alpha) {
	if (blend_alpha <= 0)
		return; // do not draw 100% transparent image

	if (// support only 32-bit blending at the moment
	    ds->GetColorDepth() == 32 && sprite->GetColorDepth() == 32 &&
	    // set blenders if applicable and tell if succeeded
	    SetBlender(blend_mode, dst_has_alpha, src_has_alpha, blend_alpha)) {
		ds->TransBlendBlt(sprite, ds_at.X, ds_at.Y);
	} else {
		GfxUtil::DrawSpriteWithTransparency(ds, sprite, ds_at.X, ds_at.Y, blend_alpha);
	}
}

void DrawSpriteWithTransparency(Bitmap *ds, Bitmap *sprite, int x, int y, int alpha) {
	if (alpha <= 0) {
		// fully transparent, don't draw it at all
		return;
	}

	const int surface_depth = ds->GetColorDepth();
	const int sprite_depth = sprite->GetColorDepth();

	// Allegro does not support masked blit or blend between different formats
	// *except* when drawing 8-bit sprites onto a higher dest.
	std::unique_ptr<Bitmap> conv_bm;
	if ((surface_depth != sprite_depth) && (sprite_depth > 8)) {
		// use ConvertBitmap in order to keep mask pixels
		conv_bm.reset(ConvertBitmap(sprite, surface_depth));
		sprite = conv_bm.get();
	}

	if ((alpha < 0xFF) && (surface_depth > 8) && (sprite_depth > 8)) {
		set_trans_blender(0, 0, 0, alpha);
		ds->TransBlendBlt(sprite, x, y);
	} else {
		ds->Blit(sprite, x, y, kBitmap_Transparency);
	}
}

} // namespace GfxUtil

} // namespace Engine
} // namespace AGS
} // namespace AGS3
