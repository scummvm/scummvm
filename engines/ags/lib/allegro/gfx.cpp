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

#include "ags/lib/allegro/gfx.h"
#include "common/textconsole.h"
#include "graphics/screen.h"

namespace AGS3 {

int color_conversion;

/*-------------------------------------------------------------------*/

BITMAP::BITMAP(Graphics::ManagedSurface *owner) : _owner(owner),
		w(owner->w), h(owner->h), format(owner->format),
		clip(false), ct(0), cb(0), cl(0), cr(0) {
	line.resize(h);
	for (uint y = 0; y < h; ++y)
		line[y] = (byte *)_owner->getBasePtr(0, y);
}

int BITMAP::getpixel(int x, int y) const {
	if (x < 0 || y < 0 || x >= w || y >= h)
		return -1;

	const byte *pixel = (const byte *)getBasePtr(x, y);
	if (format.bytesPerPixel == 1)
		return *pixel;
	else if (format.bytesPerPixel == 2)
		return *(const uint16 *)pixel;
	else
		return *(const uint32 *)pixel;
}

/*-------------------------------------------------------------------*/

/**
 * Dervied screen surface
 */
class Screen : public Graphics::Screen, public BITMAP {
public:
	Screen() : Graphics::Screen(), BITMAP(this) {}
	Screen(int width, int height) : Graphics::Screen(width, height), BITMAP(this) {}
	Screen(int width, int height, const Graphics::PixelFormat &pixelFormat) :
		Graphics::Screen(width, height, pixelFormat), BITMAP(this) {}
	~Screen() override {}
};


/*-------------------------------------------------------------------*/

void set_color_conversion(int mode) {
	color_conversion = mode;
}

int get_color_conversion() {
	return color_conversion;
}

int set_gfx_mode(int card, int w, int h, int v_w, int v_h) {
	error("TODO: set_gfx_mode");
}

BITMAP *create_bitmap(int width, int height) {
	return new Surface(width, height);
}

BITMAP *create_bitmap_ex(int color_depth, int width, int height) {
	Graphics::PixelFormat format;

	switch (color_depth) {
	case 16:
		format = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
		break;
	case 32:
		format = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
		break;
	default:
		error("Invalid color depth");
	}

	return new Surface(width, height, format);
}

BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height) {
	Graphics::ManagedSurface &surf = **parent;
	return new Surface(surf, Common::Rect(x, y, x + width, y + height));
}

BITMAP *create_video_bitmap(int width, int height) {
	return new Screen(width, height);
}

BITMAP *create_system_bitmap(int width, int height) {
	return create_bitmap(width, height);
}

void destroy_bitmap(BITMAP *bitmap) {
	delete bitmap;
}

void set_clip_rect(BITMAP *bitmap, int x1, int y1, int x2, int y2) {
	warning("TODO: set_clip_rect");
}

void add_clip_rect(BITMAP *bitmap, int x1, int y1, int x2, int y2) {
	warning("TODO: add_clip_rect");
}

void get_clip_rect(BITMAP *bitmap, int *x1, int *y1, int *x2, int *y2) {
	warning("TODO: get_clip_rect");
	*x1 = *y1 = *x2 = *y2 = 0;
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

	return surf.format.bpp();
}

int bitmap_mask_color(BITMAP *bmp) {
	Graphics::ManagedSurface &surf = **bmp;
	return surf.getTransparentColor();
}

void blit(const BITMAP *src, BITMAP *dest, int src_x, int src_y, int dst_x, int dst_y, int width, int height) {
	Graphics::ManagedSurface &srcS = **src;
	Graphics::ManagedSurface &destS = **dest;

	destS.blitFrom(srcS, Common::Rect(src_x, src_y, src_x + width, src_y + height), Common::Point(dst_x, dst_y));
}

void stretch_blit(const BITMAP *src, BITMAP *dest, int source_x, int source_y, int source_width, int source_height,
		int dest_x, int dest_y, int dest_width, int dest_height) {
	Graphics::ManagedSurface &srcS = **src;
	Graphics::ManagedSurface &destS = **dest;

	destS.transBlitFrom(srcS, Common::Rect(source_x, source_y, source_x + source_width, source_y + source_height),
		Common::Rect(dest_x, dest_y, dest_x + dest_width, dest_y + dest_height));
}

void masked_blit(const BITMAP *src, BITMAP *dest, int src_x, int src_y, int dst_x, int dst_y, int width, int height) {
	Graphics::ManagedSurface &srcS = **src;
	Graphics::ManagedSurface &destS = **dest;

	destS.blitFrom(srcS, Common::Rect(src_x, src_y, src_x + width, src_y + height), Common::Point(dst_x, dst_y));
}

void masked_stretch_blit(const BITMAP *src, BITMAP *dest, int source_x, int source_y, int source_width, int source_height,
	int dest_x, int dest_y, int dest_width, int dest_height) {
	Graphics::ManagedSurface &srcS = **src;
	Graphics::ManagedSurface &destS = **dest;

	destS.transBlitFrom(srcS, Common::Rect(source_x, source_y, source_x + source_width, source_y + source_height),
		Common::Rect(dest_x, dest_y, dest_x + dest_width, dest_y + dest_height));
}

void draw_sprite(BITMAP *bmp, const BITMAP *sprite, int x, int y) {
	Graphics::ManagedSurface &bmpS = **bmp;
	Graphics::ManagedSurface &spriteS = **sprite;

	bmpS.blitFrom(spriteS, Common::Point(x, y));
}

void stretch_sprite(BITMAP *bmp, const BITMAP *sprite, int x, int y, int w, int h) {
	Graphics::ManagedSurface &bmpS = **bmp;
	Graphics::ManagedSurface &spriteS = **sprite;

	bmpS.transBlitFrom(spriteS, Common::Rect(0, 0, sprite->w, sprite->h),
		Common::Rect(x, y, x + w, y + h));
}

void draw_trans_sprite(BITMAP *bmp, const BITMAP *sprite, int x, int y) {
	error("TODO: draw_trans_sprite");
}

void draw_lit_sprite(BITMAP *bmp, const BITMAP *sprite, int x, int y, int color) {
	error("TODO: draw_lit_sprite");
}

void draw_sprite_h_flip(BITMAP *bmp, const BITMAP *sprite, int x, int y) {
	error("TODO: draw_sprite_h_flip");
}

void draw_sprite_v_flip(BITMAP *bmp, const BITMAP *sprite, int x, int y) {
	error("TODO: draw_sprite_v_flip");
}

void draw_sprite_vh_flip(BITMAP *bmp, const BITMAP *sprite, int x, int y) {
	error("TODO: draw_sprite_vh_flip");
}

void rotate_sprite(BITMAP *bmp, const BITMAP *sprite, int x, int y, fixed angle) {
	error("TODO: rotate_sprite");
}

void pivot_sprite(BITMAP *bmp, const BITMAP *sprite, int x, int y, int cx, int cy, fixed angle) {
	error("TODO: pivot_sprite");
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

long bmp_write_line(BITMAP *bmp, int line) {
	return (long)bmp->line[line];
}

void bmp_unwrite_line(BITMAP *bmp) {
	// No implementation needed
}

void bmp_write8(unsigned long addr, int color) {
	*((byte *)addr) = color;
}

void memory_putpixel(BITMAP *bmp, int x, int y, int color) {
	putpixel(bmp, x, y, color);
}

void putpixel(BITMAP *bmp, int x, int y, int color) {
	Graphics::ManagedSurface &surf = **bmp;
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
	void *p = surf.getBasePtr(x, y);
	*((uint8 *)p) = color;
}

void _putpixel15(BITMAP *bmp, int x, int y, int color) {
	error("Unsupported bpp");
}

void _putpixel16(BITMAP *bmp, int x, int y, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	void *p = surf.getBasePtr(x, y);
	*((uint16 *)p) = color;
}

void _putpixel24(BITMAP *bmp, int x, int y, int color) {
	error("Unsupported bpp");
}

void _putpixel32(BITMAP *bmp, int x, int y, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	void *p = surf.getBasePtr(x, y);
	*((uint32 *)p) = color;
}

int getpixel(BITMAP *bmp, int x, int y) {
	Graphics::ManagedSurface &surf = **bmp;
	void *p = surf.getBasePtr(x, y);

	switch (surf.format.bytesPerPixel) {
	case 1:
		return *((uint8 *)p);
		break;
	case 2:
		return *((uint16 *)p);
		break;
	case 4:
		return *((uint32 *)p);
		break;
	default:
		break;
	}

	error("Unsupported bpp");
}

int _getpixel(BITMAP *bmp, int x, int y) {
	Graphics::ManagedSurface &surf = **bmp;
	void *p = surf.getBasePtr(x, y);
	return *((uint8 *)p);
}

int _getpixel15(BITMAP *bmp, int x, int y) {
	error("Unsupported bpp");
}

int _getpixel16(BITMAP *bmp, int x, int y) {
	Graphics::ManagedSurface &surf = **bmp;
	void *p = surf.getBasePtr(x, y);
	return *((uint16 *)p);
}

int _getpixel24(BITMAP *bmp, int x, int y) {
	error("Unsupported bpp");
}

int _getpixel32(BITMAP *bmp, int x, int y) {
	Graphics::ManagedSurface &surf = **bmp;
	void *p = surf.getBasePtr(x, y);
	return *((uint32 *)p);
}

void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	surf.drawLine(x1, y1, x2, y2, color);
}

void rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	surf.frameRect(Common::Rect(x1, y1, x2, y2), color);
}

void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	surf.fillRect(Common::Rect(x1, y1, x2, y2), color);
}

void triangle(BITMAP *bmp, int x1, int y1, int x2, int y2, int x3, int y3, int color) {
	Graphics::ManagedSurface &surf = **bmp;
	surf.drawLine(x1, y1, x2, y2, color);
	surf.drawLine(x2, y2, x3, y3, color);
	surf.drawLine(x3, y3, x1, y1, color);
}

void floodfill(BITMAP *bmp, int x, int y, int color) {
	error("TODO: floodfill");
}

void circlefill(BITMAP *bmp, int x, int y, int radius, int color) {
	error("TODO: circlefill");
}

void clear_bitmap(BITMAP *bmp) {
	bmp->clear();
}


} // namespace AGS3
