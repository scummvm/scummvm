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

#include "gfx/bitmap.h"
#include "gfx/gfxfilter_hqx.h"
#include "gfx/hq2x3x.h"

namespace AGS3 {
namespace AGS {
namespace Engine {
namespace ALSW {

using namespace Shared;

const GfxFilterInfo HqxGfxFilter::FilterInfo = GfxFilterInfo("Hqx", "Hqx (High Quality)", 2, 3);

HqxGfxFilter::HqxGfxFilter()
	: _pfnHqx(nullptr)
	, _hqxScalingBuffer(nullptr) {
}

HqxGfxFilter::~HqxGfxFilter() {
	delete _hqxScalingBuffer;
}

const GfxFilterInfo &HqxGfxFilter::GetInfo() const {
	return FilterInfo;
}

bool HqxGfxFilter::Initialize(const int color_depth, String &err_str) {
	if (color_depth < 32) {
		err_str = "Only supports 32-bit colour games";
		return false;
	}
	return AllegroGfxFilter::Initialize(color_depth, err_str);
}

Bitmap *HqxGfxFilter::InitVirtualScreen(Bitmap *screen, const Size src_size, const Rect dst_rect) {
	Bitmap *virtual_screen = AllegroGfxFilter::InitVirtualScreen(screen, src_size, dst_rect);

	// Choose used algorithm depending on minimal required integer scaling
	int min_scaling = Math::Min(dst_rect.GetWidth() / src_size.Width, dst_rect.GetHeight() / src_size.Height);
	min_scaling = Math::Clamp(min_scaling, 2, 3);
	if (min_scaling == 2)
		_pfnHqx = hq2x_32;
	else
		_pfnHqx = hq3x_32;
	_hqxScalingBuffer = BitmapHelper::CreateBitmap(src_size.Width * min_scaling, src_size.Height * min_scaling);

	InitLUTs();
	return virtual_screen;
}

Bitmap *HqxGfxFilter::ShutdownAndReturnRealScreen() {
	Bitmap *real_screen = AllegroGfxFilter::ShutdownAndReturnRealScreen();
	delete _hqxScalingBuffer;
	_hqxScalingBuffer = nullptr;
	return real_screen;
}

Bitmap *HqxGfxFilter::PreRenderPass(Bitmap *toRender) {
	_hqxScalingBuffer->Acquire();
	_pfnHqx(toRender->GetDataForWriting(), _hqxScalingBuffer->GetDataForWriting(),
		toRender->GetWidth(), toRender->GetHeight(), _hqxScalingBuffer->GetLineLength());
	_hqxScalingBuffer->Release();
	return _hqxScalingBuffer;
}

} // namespace ALSW
} // namespace Engine
} // namespace AGS
} // namespace AGS3
