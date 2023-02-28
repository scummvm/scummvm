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

#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/flood.h"
#include "ags/lib/allegro/rotate.h"
#include "ags/ags.h"
#include "ags/globals.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/screen.h"

namespace AGS3 {

int color_conversion;

/*-------------------------------------------------------------------*/

void set_color_conversion(int mode) {
	color_conversion = mode;
}

int get_color_conversion() {
	return color_conversion;
}

int set_gfx_mode(int card, int w, int h, int depth) {
	// Graphics shutdown can be ignored
	if (card != -1) {
		assert(card == SCUMMVM_ID);
		::AGS::g_vm->setGraphicsMode(w, h, depth);
	}
	return 0;
}

void set_clip_rect(BITMAP *bitmap, int x1, int y1, int x2, int y2) {
	// The rect passed to the function in inclusive-inclusive, but
	// internally the clipping rect in BITMAP is inclusive-exclusive.
	bitmap->cl = CLIP(x1, 0, (int)bitmap->w - 1);
	bitmap->ct = CLIP(y1, 0, (int)bitmap->h - 1);
	bitmap->cr = CLIP(x2 + 1, 0, (int)bitmap->w);
	bitmap->cb = CLIP(y2 + 1, 0, (int)bitmap->h);
}

void get_clip_rect(BITMAP *bitmap, int *x1, int *y1, int *x2, int *y2) {
	if (x1)
		*x1 = bitmap->cl;
	if (y1)
		*y1 = bitmap->ct;
	if (x2)
		*x2 = bitmap->cr - 1;
	if (y2)
		*y2 = bitmap->cb - 1;
}

void acquire_bitmap(BITMAP *bitmap) {
	// No implementation needed
}

void release_bitmap(BITMAP *bitmap) {
	// No implementation needed
}

void clear_to_color(BITMAP *bitmap, int color) {
	Graphics::ManagedSurface &surf = **bitmap;

	surf.clear(color);
}

int bitmap_color_depth(BITMAP *bmp) {
	Graphics::ManagedSurface &surf = **bmp;

	return (surf.format.bytesPerPixel == 1) ? 8 : surf.format.bpp();
}

int bitmap_mask_color(BITMAP *bmp) {
	// For paletted sprites this is 0.
	// For other color depths this is bright pink (RGB 255, 0, 255).
	// The alpha chanel should be 0.
	//if (bmp-format.bytesPerPixel == 1)
	//  return 0;
	//return bmp->format.AGRBToColor(0, 255, 0, 255);
	return bmp->getTransparentColor();
}

void blit(const BITMAP *src, BITMAP *dest, int src_x, int src_y, int dst_x, int dst_y, int width, int height) {
	dest->draw(src, Common::Rect(src_x, src_y, src_x + width, src_y + height),
	           dst_x, dst_y, false, false, false, -1);
}

void stretch_blit(const BITMAP *src, BITMAP *dest,
                  int source_x, int source_y, int source_width, int source_height,
                  int dest_x, int dest_y, int dest_width, int dest_height) {
	dest->stretchDraw(src,
	                  Common::Rect(source_x, source_y, source_x + source_width, source_y + source_height),
	                  Common::Rect(dest_x, dest_y, dest_x + dest_width, dest_y + dest_height),
	                  false, -1);
}

void masked_blit(const BITMAP *src, BITMAP *dest, int src_x, int src_y, int dst_x, int dst_y, int width, int height) {
	assert(src->format == dest->format);

	dest->draw(src, Common::Rect(src_x, src_y, src_x + width, src_y + height),
	           dst_x, dst_y, false, false, true, -1);
}

void masked_stretch_blit(const BITMAP *src, BITMAP *dest,
                         int source_x, int source_y, int source_width, int source_height,
                         int dest_x, int dest_y, int dest_width, int dest_height) {
	dest->stretchDraw(src,
	                  Common::Rect(source_x, source_y, source_x + source_width, source_y + source_height),
	                  Common::Rect(dest_x, dest_y, dest_x + dest_width, dest_y + dest_height),
	                  true, -1);
}

void draw_sprite(BITMAP *bmp, const BITMAP *sprite, int x, int y) {
	bmp->draw(sprite, Common::Rect(0, 0, sprite->w, sprite->h),
	          x, y, false, false, true, -1);
}

void stretch_sprite(BITMAP *bmp, const BITMAP *sprite, int x, int y, int w, int h) {
	bmp->stretchDraw(sprite, Common::Rect(0, 0, sprite->w, sprite->h),
	                 Common::Rect(x, y, x + w, y + h),
	                 true, -1);
}

void draw_trans_sprite(BITMAP *bmp, const BITMAP *sprite, int x, int y) {
	bmp->draw(sprite, Common::Rect(0, 0, sprite->w, sprite->h),
	          x, y, false, false, true, _G(trans_blend_alpha));
}

void draw_lit_sprite(BITMAP *bmp, const BITMAP *sprite, int x, int y, int color) {
	bmp->draw(sprite, Common::Rect(0, 0, sprite->w, sprite->h),
	          x, y, false, false, true, color,
	          _G(trans_blend_red), _G(trans_blend_green), _G(trans_blend_blue));
}

void draw_sprite_h_flip(BITMAP *bmp, const BITMAP *sprite, int x, int y) {
	bmp->draw(sprite, Common::Rect(0, 0, sprite->w, sprite->h),
	          x, y, true, false, true, -1);
}

void draw_sprite_v_flip(BITMAP *bmp, const BITMAP *sprite, int x, int y) {
	bmp->draw(sprite, Common::Rect(0, 0, sprite->w, sprite->h),
	          x, y, false, true, true, -1);
}

void draw_sprite_vh_flip(BITMAP *bmp, const BITMAP *sprite, int x, int y) {
	bmp->draw(sprite, Common::Rect(0, 0, sprite->w, sprite->h),
	          x, y, true, true, true, -1);
}

void rotate_sprite(BITMAP *bmp, const BITMAP *sprite, int x, int y, fixed angle) {
	pivot_scaled_sprite(bmp, sprite, (x<<16) + (sprite->w * 0x10000) / 2, (y<<16) + (sprite->h * 0x10000) / 2, sprite->w << 15, sprite->h << 15, angle, 0x10000);
}

void pivot_sprite(BITMAP *bmp, const BITMAP *sprite, int x, int y, int cx, int cy, fixed angle) {
	pivot_scaled_sprite(bmp, sprite, x<<16, y<<16, cx<<16, cy<<16, angle, 0x10000);
}


bool is_screen_bitmap(BITMAP *bmp) {
	return dynamic_cast<Graphics::Screen *>(bmp) != nullptr;
}

bool is_video_bitmap(BITMAP *bmp) {
	return dynamic_cast<Graphics::Screen *>(bmp) != nullptr;
}

bool is_planar_bitmap(BITMAP *bmp) {
	return false;
}

bool is_linear_bitmap(BITMAP *bmp) {
	return true;
}

void bmp_select(BITMAP *bmp) {
	// No implementation needed
}

byte *bmp_write_line(BITMAP *bmp, int line) {
	return bmp->line[line];
}

void bmp_write8(byte *addr, int color) {
	*addr = color;
}

void bmp_write15(byte *addr, int color) {
	*((uint16 *)addr) = color;
}

void bmp_write16(byte *addr, int color) {
	*((uint16 *)addr) = color;
}

void bmp_write24(byte *addr, int color) {
	*addr = (color & 0xff);
	*(addr + 1) = ((color >> 8) & 0xff);
	*(addr + 2) = ((color >> 16) & 0xff);
}

void bmp_write32(byte *addr, int color) {
	*((uint32 *)addr) = color;
}

void memory_putpixel(BITMAP *bmp, int x, int y, int color) {
	putpixel(bmp, x, y, color);
}

void putpixel(BITMAP *bmp, int x, int y, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	if (x < 0 || x >= surf.w || y < 0 || y >= surf.h)
		return;
	void *p = surf.getBasePtr(x, y);

	switch (surf.format.bytesPerPixel) {
	case 1:
		*((uint8 *)p) = color;
		break;
	case 2:
		*((uint16 *)p) = color;
		break;
	case 4:
		*((uint32 *)p) = color;
		break;
	default:
		break;
	}
}

void _putpixel(BITMAP *bmp, int x, int y, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	if (x < 0 || x >= surf.w || y < 0 || y >= surf.h)
		return;
	void *p = surf.getBasePtr(x, y);
	*((uint8 *)p) = color;
}

void _putpixel15(BITMAP *bmp, int x, int y, int color) {
	error("Unsupported bpp");
}

void _putpixel16(BITMAP *bmp, int x, int y, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	if (x < 0 || x >= surf.w || y < 0 || y >= surf.h)
		return;
	void *p = surf.getBasePtr(x, y);
	*((uint16 *)p) = color;
}

void _putpixel24(BITMAP *bmp, int x, int y, int color) {
	error("Unsupported bpp");
}

void _putpixel32(BITMAP *bmp, int x, int y, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	if (x < 0 || x >= surf.w || y < 0 || y >= surf.h)
		return;
	void *p = surf.getBasePtr(x, y);
	*((uint32 *)p) = color;
}

int getpixel(const BITMAP *bmp, int x, int y) {
	Graphics::ManagedSurface &surf = **bmp;

	// Allegro returns -1 if the pixel lies outside the bitmap
	if (x < 0 || y < 0 || x >= surf.w || y >= surf.h)
		return -1;

	void *p = surf.getBasePtr(x, y);

	switch (surf.format.bytesPerPixel) {
	case 1:
		return *((uint8 *)p);
	case 2:
		return *((uint16 *)p);
	case 4:
		return *((uint32 *)p);
	default:
		break;
	}

	error("Unsupported bpp");
}

int _getpixel(const BITMAP *bmp, int x, int y) {
	Graphics::ManagedSurface &surf = **bmp;
	if (x < 0 || y < 0 || x >= surf.w || y >= surf.h)
		return -1;
	void *p = surf.getBasePtr(x, y);
	return *((uint8 *)p);
}

int _getpixel15(const BITMAP *bmp, int x, int y) {
	error("Unsupported bpp");
}

int _getpixel16(const BITMAP *bmp, int x, int y) {
	Graphics::ManagedSurface &surf = **bmp;
	if (x < 0 || y < 0 || x >= surf.w || y >= surf.h)
		return -1;
	void *p = surf.getBasePtr(x, y);
	return *((uint16 *)p);
}

int _getpixel24(const BITMAP *bmp, int x, int y) {
	error("Unsupported bpp");
}

int _getpixel32(const BITMAP *bmp, int x, int y) {
	Graphics::ManagedSurface &surf = **bmp;
	if (x < 0 || y < 0 || x >= surf.w || y >= surf.h)
		return -1;
	void *p = surf.getBasePtr(x, y);
	return *((uint32 *)p);
}

void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	surf.drawLine(x1, y1, x2, y2, color);
}

void rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	if (x1 > x2)
		SWAP(x1, x2);
	if (y1 > y2)
		SWAP(y1, y2);
	surf.frameRect(Common::Rect(x1, y1, x2 + 1, y2 + 1), color);
}

void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	if (x1 > x2)
		SWAP(x1, x2);
	if (y1 > y2)
		SWAP(y1, y2);
	surf.fillRect(Common::Rect(x1, y1, x2 + 1, y2 + 1), color);
}

void triangle(BITMAP *bmp, int x1, int y1, int x2, int y2, int x3, int y3, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	surf.drawLine(x1, y1, x2, y2, color);
	surf.drawLine(x2, y2, x3, y3, color);
	surf.drawLine(x3, y3, x1, y1, color);
}

void circlefill(BITMAP *bmp, int x, int y, int radius, int color) {
	bmp->circlefill(x, y, radius, color);
}

void clear_bitmap(BITMAP *bmp) {
	bmp->clear();
}

} // namespace AGS3
