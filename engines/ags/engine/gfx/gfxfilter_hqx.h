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
// High quality x2 scaling filter
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_HQ2XGFXFILTER_H
#define AGS_ENGINE_GFX_HQ2XGFXFILTER_H

#include "gfx/gfxfilter_allegro.h"

namespace AGS {
namespace Engine {
namespace ALSW {

class HqxGfxFilter : public AllegroGfxFilter {
public:
	HqxGfxFilter();
	~HqxGfxFilter() override;

	const GfxFilterInfo &GetInfo() const override;

	bool Initialize(const int color_depth, String &err_str) override;
	Bitmap *InitVirtualScreen(Bitmap *screen, const Size src_size, const Rect dst_rect) override;
	Bitmap *ShutdownAndReturnRealScreen() override;

	static const GfxFilterInfo FilterInfo;

protected:
	Bitmap *PreRenderPass(Bitmap *toRender) override;

	typedef void (*PfnHqx)(unsigned char *in, unsigned char *out, int src_w, int src_h, int bpl);

	PfnHqx  _pfnHqx;
	Bitmap *_hqxScalingBuffer;
};

} // namespace ALSW
} // namespace Engine
} // namespace AGS

#endif
