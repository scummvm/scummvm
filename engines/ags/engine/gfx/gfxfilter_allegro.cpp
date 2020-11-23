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

#include "ags/shared/gfx/gfxfilter_allegro.h"

namespace AGS3 {
namespace AGS {
namespace Engine {
namespace ALSW {

using namespace Shared;

const GfxFilterInfo AllegroGfxFilter::FilterInfo = GfxFilterInfo("StdScale", "Nearest-neighbour");

AllegroGfxFilter::AllegroGfxFilter()
	: realScreen(nullptr)
	, virtualScreen(nullptr)
	, realScreenSizedBuffer(nullptr)
	, lastBlitFrom(nullptr)
	, lastBlitX(0)
	, lastBlitY(0) {
}

const GfxFilterInfo &AllegroGfxFilter::GetInfo() const {
	return FilterInfo;
}

Bitmap *AllegroGfxFilter::InitVirtualScreen(Bitmap *screen, const Size src_size, const Rect dst_rect) {
	ShutdownAndReturnRealScreen();

	realScreen = screen;
	SetTranslation(src_size, dst_rect);

	if (src_size == dst_rect.GetSize() && dst_rect.Top == 0 && dst_rect.Left == 0) {
		// Speed up software rendering if no scaling is performed
		realScreenSizedBuffer = nullptr;
		virtualScreen = realScreen;
	} else {
		realScreenSizedBuffer = BitmapHelper::CreateBitmap(screen->GetWidth(), screen->GetHeight(), screen->GetColorDepth());
		virtualScreen = BitmapHelper::CreateBitmap(src_size.Width, src_size.Height, screen->GetColorDepth());
	}
	return virtualScreen;
}

Bitmap *AllegroGfxFilter::ShutdownAndReturnRealScreen() {
	if (virtualScreen != realScreen)
		delete virtualScreen;
	delete realScreenSizedBuffer;
	virtualScreen = nullptr;
	realScreenSizedBuffer = nullptr;
	Bitmap *real_scr = realScreen;
	realScreen = nullptr;
	return real_scr;
}

void AllegroGfxFilter::RenderScreen(Bitmap *toRender, int x, int y) {

	if (toRender != realScreen) {
		x = _scaling.X.ScalePt(x);
		y = _scaling.Y.ScalePt(y);
		const int width = _scaling.X.ScaleDistance(toRender->GetWidth());
		const int height = _scaling.Y.ScaleDistance(toRender->GetHeight());
		Bitmap *render_src = PreRenderPass(toRender);
		if (render_src->GetSize() == _dstRect.GetSize())
			realScreen->Blit(render_src, 0, 0, x, y, width, height);
		else {
			realScreen->StretchBlt(render_src, RectWH(x, y, width, height));
		}
	}
	lastBlitFrom = toRender;
	lastBlitX = x;
	lastBlitY = y;
}

void AllegroGfxFilter::RenderScreenFlipped(Bitmap *toRender, int x, int y, GlobalFlipType flipType) {

	if (toRender == virtualScreen)
		return;

	switch (flipType) {
	case kFlip_Horizontal:
		virtualScreen->FlipBlt(toRender, 0, 0, Common::kBitmap_HFlip);
		break;
	case kFlip_Vertical:
		virtualScreen->FlipBlt(toRender, 0, 0, Common::kBitmap_VFlip);
		break;
	case kFlip_Both:
		virtualScreen->FlipBlt(toRender, 0, 0, Common::kBitmap_HVFlip);
		break;
	default:
		virtualScreen->Blit(toRender, 0, 0);
		break;
	}

	RenderScreen(virtualScreen, x, y);
}

void AllegroGfxFilter::ClearRect(int x1, int y1, int x2, int y2, int color) {
	if (!realScreen) return;
	Rect r = _scaling.ScaleRange(Rect(x1, y1, x2, y2));
	realScreen->FillRect(r, color);
}

void AllegroGfxFilter::GetCopyOfScreenIntoBitmap(Bitmap *copyBitmap) {
	GetCopyOfScreenIntoBitmap(copyBitmap, true);
}

void AllegroGfxFilter::GetCopyOfScreenIntoBitmap(Bitmap *copyBitmap, bool copy_with_yoffset) {
	if (copyBitmap == realScreen)
		return;

	if (!copy_with_yoffset) {
		if (copyBitmap->GetSize() == _dstRect.GetSize())
			copyBitmap->Blit(realScreen, _dstRect.Left, _dstRect.Top, 0, 0, _dstRect.GetWidth(), _dstRect.GetHeight());
		else {
			// Can't stretch_blit from Video Memory to normal memory,
			// so copy the screen to a buffer first.
			realScreenSizedBuffer->Blit(realScreen, 0, 0);
			copyBitmap->StretchBlt(realScreenSizedBuffer,
				_dstRect,
				RectWH(0, 0, copyBitmap->GetWidth(), copyBitmap->GetHeight()));
		}
	} else if (!lastBlitFrom)
		copyBitmap->Fill(0);
	else if (copyBitmap->GetSize() == _dstRect.GetSize())
		copyBitmap->Blit(realScreen, lastBlitX, lastBlitY, 0, 0, copyBitmap->GetWidth(), copyBitmap->GetHeight());
	else {
		copyBitmap->StretchBlt(lastBlitFrom,
			RectWH(0, 0, lastBlitFrom->GetWidth(), lastBlitFrom->GetHeight()),
			RectWH(0, 0, copyBitmap->GetWidth(), copyBitmap->GetHeight()));
	}
}

Bitmap *AllegroGfxFilter::PreRenderPass(Bitmap *toRender) {
	// do nothing by default
	return toRender;
}

} // namespace ALSW
} // namespace Engine
} // namespace AGS
} // namespace AGS3
