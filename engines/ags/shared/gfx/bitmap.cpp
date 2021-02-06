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

#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/util/memory.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

// TODO: revise this construction later
namespace BitmapHelper {

Bitmap *CreateBitmap(int width, int height, int color_depth) {
	Bitmap *bitmap = new Bitmap();
	if (!bitmap->Create(width, height, color_depth)) {
		delete bitmap;
		bitmap = nullptr;
	}
	return bitmap;
}

Bitmap *CreateTransparentBitmap(int width, int height, int color_depth) {
	Bitmap *bitmap = new Bitmap();
	if (!bitmap->CreateTransparent(width, height, color_depth)) {
		delete bitmap;
		bitmap = nullptr;
	}
	return bitmap;
}

Bitmap *CreateSubBitmap(Bitmap *src, const Rect &rc) {
	Bitmap *bitmap = new Bitmap();
	if (!bitmap->CreateSubBitmap(src, rc)) {
		delete bitmap;
		bitmap = nullptr;
	}
	return bitmap;
}

Bitmap *CreateBitmapCopy(Bitmap *src, int color_depth) {
	Bitmap *bitmap = new Bitmap();
	if (!bitmap->CreateCopy(src, color_depth)) {
		delete bitmap;
		bitmap = nullptr;
	}
	return bitmap;
}

Bitmap *LoadFromFile(const char *filename) {
	Bitmap *bitmap = new Bitmap();
	if (!bitmap->LoadFromFile(filename)) {
		delete bitmap;
		bitmap = nullptr;
	}
	return bitmap;
}

Bitmap *AdjustBitmapSize(Bitmap *src, int width, int height) {
	int oldw = src->GetWidth(), oldh = src->GetHeight();
	if ((oldw == width) && (oldh == height))
		return src;
	Bitmap *bmp = BitmapHelper::CreateBitmap(width, height, src->GetColorDepth());
	bmp->StretchBlt(src, RectWH(0, 0, oldw, oldh), RectWH(0, 0, width, height));
	return bmp;
}

template <class TPx, size_t BPP_>
struct PixelTransCpy {
	static const size_t BPP = BPP_;
	inline void operator()(uint8_t *dst, const uint8_t *src, color_t mask_color, bool use_alpha) const {
		if (*(const TPx *)src == mask_color)
			*(TPx *)dst = mask_color;
	}
};

struct PixelNoSkip {
	inline bool operator()(uint8_t *data, color_t mask_color, bool use_alpha) const {
		return false;
	}
};

typedef PixelTransCpy<uint8_t, 1> PixelTransCpy8;
typedef PixelTransCpy<uint16_t, 2> PixelTransCpy16;

struct PixelTransCpy24 {
	static const size_t BPP = 3;
	inline void operator()(uint8_t *dst, const uint8_t *src, color_t mask_color, bool use_alpha) const {
		const uint8_t *mcol_ptr = (const uint8_t *)&mask_color;
		if (src[0] == mcol_ptr[0] && src[1] == mcol_ptr[1] && src[2] == mcol_ptr[2]) {
			dst[0] = mcol_ptr[0];
			dst[1] = mcol_ptr[1];
			dst[2] = mcol_ptr[2];
		}
	}
};

struct PixelTransCpy32 {
	static const size_t BPP = 4;
	inline void operator()(uint8_t *dst, const uint8_t *src, color_t mask_color, bool use_alpha) const {
		if (*(const uint32_t *)src == (uint32_t)mask_color)
			*(uint32_t *)dst = mask_color;
		else if (use_alpha)
			dst[3] = src[3]; // copy alpha channel
		else
			dst[3] = 0xFF; // set the alpha channel byte to opaque
	}
};

struct PixelTransSkip32 {
	inline bool operator()(uint8_t *data, color_t mask_color, bool use_alpha) const {
		return *(uint32_t *)data == (uint32_t)mask_color || (use_alpha && data[3] == 0);
	}
};

template <class FnPxProc, class FnSkip>
void ApplyMask(uint8_t *dst, const uint8_t *src, size_t pitch, size_t height, FnPxProc proc, FnSkip skip, color_t mask_color, bool dst_has_alpha, bool mask_has_alpha) {
	for (size_t y = 0; y < height; ++y) {
		for (size_t x = 0; x < pitch; x += FnPxProc::BPP, src += FnPxProc::BPP, dst += FnPxProc::BPP) {
			if (!skip(dst, mask_color, dst_has_alpha))
				proc(dst, src, mask_color, mask_has_alpha);
		}
	}
}

void CopyTransparency(Bitmap *dst, const Bitmap *mask, bool dst_has_alpha, bool mask_has_alpha) {
	color_t mask_color = mask->GetMaskColor();
	uint8_t *dst_ptr = dst->GetDataForWriting();
	const uint8_t *src_ptr = mask->GetData();
	const size_t bpp = mask->GetBPP();
	const size_t pitch = mask->GetLineLength();
	const size_t height = mask->GetHeight();

	if (bpp == 1)
		ApplyMask(dst_ptr, src_ptr, pitch, height, PixelTransCpy8(), PixelNoSkip(), mask_color, dst_has_alpha, mask_has_alpha);
	else if (bpp == 2)
		ApplyMask(dst_ptr, src_ptr, pitch, height, PixelTransCpy16(), PixelNoSkip(), mask_color, dst_has_alpha, mask_has_alpha);
	else if (bpp == 3)
		ApplyMask(dst_ptr, src_ptr, pitch, height, PixelTransCpy24(), PixelNoSkip(), mask_color, dst_has_alpha, mask_has_alpha);
	else
		ApplyMask(dst_ptr, src_ptr, pitch, height, PixelTransCpy32(), PixelTransSkip32(), mask_color, dst_has_alpha, mask_has_alpha);
}

void ReadPixelsFromMemory(Bitmap *dst, const uint8_t *src_buffer, const size_t src_pitch, const size_t src_px_offset) {
	const size_t bpp = dst->GetBPP();
	const size_t src_px_pitch = src_pitch / bpp;
	if (src_px_offset >= src_px_pitch)
		return; // nothing to copy
	Memory::BlockCopy(dst->GetDataForWriting(), dst->GetLineLength(), 0, src_buffer, src_pitch, src_px_offset * bpp, dst->GetHeight());
}

} // namespace BitmapHelper

} // namespace Shared
} // namespace AGS
} // namespace AGS3
