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

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "ags/lib/aastr-0.1.1/aastr.h"
#include "ags/shared/gfx/allegro_bitmap.h"
#include "ags/shared/gfx/image.h"

namespace AGS3 {

extern void __my_setcolor(int *ctset, int newcol, int wantColDep);

namespace AGS {
namespace Shared {

Bitmap::Bitmap()
	: _alBitmap(nullptr)
	, _isDataOwner(false) {
}

Bitmap::Bitmap(int width, int height, int color_depth)
	: _alBitmap(nullptr)
	, _isDataOwner(false) {
	Create(width, height, color_depth);
}

Bitmap::Bitmap(Bitmap *src, const Rect &rc)
	: _alBitmap(nullptr)
	, _isDataOwner(false) {
	CreateSubBitmap(src, rc);
}

Bitmap::Bitmap(BITMAP *al_bmp, bool shared_data)
	: _alBitmap(nullptr)
	, _isDataOwner(false) {
	WrapAllegroBitmap(al_bmp, shared_data);
}

Bitmap::~Bitmap() {
	Destroy();
}

//=============================================================================
// Creation and destruction
//=============================================================================

bool Bitmap::Create(int width, int height, int color_depth) {
	Destroy();
	if (color_depth) {
		_alBitmap = create_bitmap_ex(color_depth, width, height);
	} else {
		_alBitmap = create_bitmap(width, height);
	}
	_isDataOwner = true;
	return _alBitmap != nullptr;
}

bool Bitmap::CreateTransparent(int width, int height, int color_depth) {
	if (Create(width, height, color_depth)) {
		clear_to_color(_alBitmap, bitmap_mask_color(_alBitmap));
		return true;
	}
	return false;
}

bool Bitmap::CreateSubBitmap(Bitmap *src, const Rect &rc) {
	Destroy();
	_alBitmap = create_sub_bitmap(src->_alBitmap, rc.Left, rc.Top, rc.GetWidth(), rc.GetHeight());
	_isDataOwner = true;
	return _alBitmap != nullptr;
}

bool Bitmap::ResizeSubBitmap(int width, int height) {
	if (!isSubBitmap())
		return false;
	// TODO: can't clamp to parent size, because subs do not keep parent ref;
	// might require amending allegro bitmap struct
	_alBitmap->w = _alBitmap->cr = width;
	_alBitmap->h = _alBitmap->cb = height;
	return true;
}

bool Bitmap::CreateCopy(Bitmap *src, int color_depth) {
	if (Create(src->_alBitmap->w, src->_alBitmap->h, color_depth ? color_depth : bitmap_color_depth(src->_alBitmap))) {
		blit(src->_alBitmap, _alBitmap, 0, 0, 0, 0, _alBitmap->w, _alBitmap->h);
		return true;
	}
	return false;
}

bool Bitmap::WrapAllegroBitmap(BITMAP *al_bmp, bool shared_data) {
	Destroy();
	_alBitmap = al_bmp;
	_isDataOwner = !shared_data;
	return _alBitmap != nullptr;
}

void Bitmap::Destroy() {
	if (_isDataOwner && _alBitmap) {
		destroy_bitmap(_alBitmap);
	}
	_alBitmap = nullptr;
	_isDataOwner = false;
}

bool Bitmap::LoadFromFile(const char *filename) {
	Destroy();

	BITMAP *al_bmp = load_bitmap(filename, nullptr);
	if (al_bmp) {
		_alBitmap = al_bmp;
		_isDataOwner = true;
	}
	return _alBitmap != nullptr;
}

bool Bitmap::LoadFromFile(PACKFILE *pf) {
	Destroy();

	BITMAP *al_bmp = load_bitmap(pf, nullptr);
	if (al_bmp) {
		_alBitmap = al_bmp;
		_isDataOwner = true;
	}
	return _alBitmap != nullptr;
}

bool Bitmap::SaveToFile(Common::WriteStream &out, const void *palette) {
	return save_bitmap(out, _alBitmap, (const RGB *)palette) == 0;
}

bool Bitmap::SaveToFile(const char *filename, const void *palette) {
	// Only keeps the file name and add the game target as prefix.
	Common::String name = filename;
	size_t lastSlash = name.findLastOf('/');
	if (lastSlash != Common::String::npos)
		name = name.substr(lastSlash + 1);
	Common::String gameTarget = ConfMan.getActiveDomainName();
	if (!name.hasPrefixIgnoreCase(gameTarget))
		name = gameTarget + "-" + name;

	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(name, false);
	assert(out);
	bool result = SaveToFile(*out, palette);
	out->finalize();
	delete out;

	return result;
}

color_t Bitmap::GetCompatibleColor(color_t color) {
	color_t compat_color = 0;
	__my_setcolor(&compat_color, color, bitmap_color_depth(_alBitmap));
	return compat_color;
}

//=============================================================================
// Clipping
//=============================================================================

void Bitmap::SetClip(const Rect &rc) {
	set_clip_rect(_alBitmap, rc.Left, rc.Top, rc.Right, rc.Bottom);
}

void Bitmap::ResetClip() {
	set_clip_rect(_alBitmap, 0, 0, _alBitmap->w - 1, _alBitmap->h - 1);
}

Rect Bitmap::GetClip() const {
	Rect temp;
	get_clip_rect(_alBitmap, &temp.Left, &temp.Top, &temp.Right, &temp.Bottom);
	return temp;
}

//=============================================================================
// Blitting operations (drawing one bitmap over another)
//=============================================================================

void Bitmap::Blit(Bitmap *src, int dst_x, int dst_y, BitmapMaskOption mask) {
	BITMAP *al_src_bmp = src->_alBitmap;
	// WARNING: For some evil reason Allegro expects dest and src bitmaps in different order for blit and draw_sprite
	if (mask == kBitmap_Transparency) {
		draw_sprite(_alBitmap, al_src_bmp, dst_x, dst_y);
	} else {
		blit(al_src_bmp, _alBitmap, 0, 0, dst_x, dst_y, al_src_bmp->w, al_src_bmp->h);
	}
}

void Bitmap::Blit(Bitmap *src, int src_x, int src_y, int dst_x, int dst_y, int width, int height, BitmapMaskOption mask) {
	BITMAP *al_src_bmp = src->_alBitmap;
	if (mask == kBitmap_Transparency) {
		masked_blit(al_src_bmp, _alBitmap, src_x, src_y, dst_x, dst_y, width, height);
	} else {
		blit(al_src_bmp, _alBitmap, src_x, src_y, dst_x, dst_y, width, height);
	}
}

void Bitmap::MaskedBlit(Bitmap *src, int dst_x, int dst_y) {
	draw_sprite(_alBitmap, src->_alBitmap, dst_x, dst_y);
}

void Bitmap::StretchBlt(Bitmap *src, const Rect &dst_rc, BitmapMaskOption mask) {
	BITMAP *al_src_bmp = src->_alBitmap;
	// WARNING: For some evil reason Allegro expects dest and src bitmaps in different order for blit and draw_sprite
	if (mask == kBitmap_Transparency) {
		stretch_sprite(_alBitmap, al_src_bmp,
		               dst_rc.Left, dst_rc.Top, dst_rc.GetWidth(), dst_rc.GetHeight());
	} else {
		stretch_blit(al_src_bmp, _alBitmap,
		             0, 0, al_src_bmp->w, al_src_bmp->h,
		             dst_rc.Left, dst_rc.Top, dst_rc.GetWidth(), dst_rc.GetHeight());
	}
}

void Bitmap::StretchBlt(Bitmap *src, const Rect &src_rc, const Rect &dst_rc, BitmapMaskOption mask) {
	BITMAP *al_src_bmp = src->_alBitmap;
	if (mask == kBitmap_Transparency) {
		masked_stretch_blit(al_src_bmp, _alBitmap,
		                    src_rc.Left, src_rc.Top, src_rc.GetWidth(), src_rc.GetHeight(),
		                    dst_rc.Left, dst_rc.Top, dst_rc.GetWidth(), dst_rc.GetHeight());
	} else {
		stretch_blit(al_src_bmp, _alBitmap,
		             src_rc.Left, src_rc.Top, src_rc.GetWidth(), src_rc.GetHeight(),
		             dst_rc.Left, dst_rc.Top, dst_rc.GetWidth(), dst_rc.GetHeight());
	}
}

void Bitmap::AAStretchBlt(Bitmap *src, const Rect &dst_rc, BitmapMaskOption mask) {
	BITMAP *al_src_bmp = src->_alBitmap;
	// WARNING: For some evil reason Allegro expects dest and src bitmaps in different order for blit and draw_sprite
	if (mask == kBitmap_Transparency) {
		aa_stretch_sprite(_alBitmap, al_src_bmp,
		                  dst_rc.Left, dst_rc.Top, dst_rc.GetWidth(), dst_rc.GetHeight());
	} else {
		aa_stretch_blit(al_src_bmp, _alBitmap,
		                0, 0, al_src_bmp->w, al_src_bmp->h,
		                dst_rc.Left, dst_rc.Top, dst_rc.GetWidth(), dst_rc.GetHeight());
	}
}

void Bitmap::AAStretchBlt(Bitmap *src, const Rect &src_rc, const Rect &dst_rc, BitmapMaskOption mask) {
	BITMAP *al_src_bmp = src->_alBitmap;
	if (mask == kBitmap_Transparency) {
		// TODO: aastr lib does not expose method for masked stretch blit; should do that at some point since
		// the source code is a gift-ware anyway
		// aa_masked_blit(_alBitmap, al_src_bmp, src_rc.Left, src_rc.Top, src_rc.GetWidth(), src_rc.GetHeight(), dst_rc.Left, dst_rc.Top, dst_rc.GetWidth(), dst_rc.GetHeight());
		error("aa_masked_blit is not yet supported!");
	} else {
		aa_stretch_blit(al_src_bmp, _alBitmap,
		                src_rc.Left, src_rc.Top, src_rc.GetWidth(), src_rc.GetHeight(),
		                dst_rc.Left, dst_rc.Top, dst_rc.GetWidth(), dst_rc.GetHeight());
	}
}

void Bitmap::TransBlendBlt(Bitmap *src, int dst_x, int dst_y) {
	BITMAP *al_src_bmp = src->_alBitmap;
	draw_trans_sprite(_alBitmap, al_src_bmp, dst_x, dst_y);
}

void Bitmap::LitBlendBlt(Bitmap *src, int dst_x, int dst_y, int light_amount) {
	BITMAP *al_src_bmp = src->_alBitmap;
	draw_lit_sprite(_alBitmap, al_src_bmp, dst_x, dst_y, light_amount);
}

void Bitmap::FlipBlt(Bitmap *src, int dst_x, int dst_y, GraphicFlip flip) {
	BITMAP *al_src_bmp = src->_alBitmap;
	switch (flip) {
	case kFlip_Horizontal:
		draw_sprite_h_flip(_alBitmap, al_src_bmp, dst_x, dst_y);
		break;
	case kFlip_Vertical:
		draw_sprite_v_flip(_alBitmap, al_src_bmp, dst_x, dst_y);
		break;
	case kFlip_Both:
		draw_sprite_vh_flip(_alBitmap, al_src_bmp, dst_x, dst_y);
		break;
	default: // blit with no transform
		Blit(src, dst_x, dst_y);
		break;
	}
}

void Bitmap::RotateBlt(Bitmap *src, int dst_x, int dst_y, fixed_t angle) {
	BITMAP *al_src_bmp = src->_alBitmap;
	rotate_sprite(_alBitmap, al_src_bmp, dst_x, dst_y, angle);
}

void Bitmap::RotateBlt(Bitmap *src, int dst_x, int dst_y, int pivot_x, int pivot_y, fixed_t angle) {
	BITMAP *al_src_bmp = src->_alBitmap;
	pivot_sprite(_alBitmap, al_src_bmp, dst_x, dst_y, pivot_x, pivot_y, angle);
}

//=============================================================================
// Pixel operations
//=============================================================================

void Bitmap::Clear(color_t color) {
	if (color) {
		clear_to_color(_alBitmap, color);
	} else {
		clear_bitmap(_alBitmap);
	}
}

void Bitmap::ClearTransparent() {
	clear_to_color(_alBitmap, bitmap_mask_color(_alBitmap));
}

void Bitmap::PutPixel(int x, int y, color_t color) {
	if (x < 0 || x >= _alBitmap->w || y < 0 || y >= _alBitmap->h) {
		return;
	}

	switch (bitmap_color_depth(_alBitmap)) {
	case 8:
		return _putpixel(_alBitmap, x, y, color);
	case 15:
		return _putpixel15(_alBitmap, x, y, color);
	case 16:
		return _putpixel16(_alBitmap, x, y, color);
	case 24:
		return _putpixel24(_alBitmap, x, y, color);
	case 32:
		return _putpixel32(_alBitmap, x, y, color);
	}
	assert(0); // this should not normally happen
	return putpixel(_alBitmap, x, y, color);
}

int Bitmap::GetPixel(int x, int y) const {
	if (x < 0 || x >= _alBitmap->w || y < 0 || y >= _alBitmap->h) {
		return -1; // Allegros getpixel() implementation returns -1 in this case
	}

	switch (bitmap_color_depth(_alBitmap)) {
	case 8:
		return _getpixel(_alBitmap, x, y);
	case 15:
		return _getpixel15(_alBitmap, x, y);
	case 16:
		return _getpixel16(_alBitmap, x, y);
	case 24:
		return _getpixel24(_alBitmap, x, y);
	case 32:
		return _getpixel32(_alBitmap, x, y);
	}
	assert(0); // this should not normally happen
	return getpixel(_alBitmap, x, y);
}

//=============================================================================
// Vector drawing operations
//=============================================================================

void Bitmap::DrawLine(const Line &ln, color_t color) {
	line(_alBitmap, ln.X1, ln.Y1, ln.X2, ln.Y2, color);
}

void Bitmap::DrawTriangle(const Triangle &tr, color_t color) {
	triangle(_alBitmap,
	         tr.X1, tr.Y1, tr.X2, tr.Y2, tr.X3, tr.Y3, color);
}

void Bitmap::DrawRect(const Rect &rc, color_t color) {
	rect(_alBitmap, rc.Left, rc.Top, rc.Right, rc.Bottom, color);
}

void Bitmap::FillRect(const Rect &rc, color_t color) {
	rectfill(_alBitmap, rc.Left, rc.Top, rc.Right, rc.Bottom, color);
}

void Bitmap::FillCircle(const Circle &circle, color_t color) {
	circlefill(_alBitmap, circle.X, circle.Y, circle.Radius, color);
}

void Bitmap::Fill(color_t color) {
	if (color) {
		clear_to_color(_alBitmap, color);
	} else {
		clear_bitmap(_alBitmap);
	}
}

void Bitmap::FillTransparent() {
	clear_to_color(_alBitmap, bitmap_mask_color(_alBitmap));
}

void Bitmap::FloodFill(int x, int y, color_t color) {
	_alBitmap->floodfill(x, y, color);
}

//=============================================================================
// Direct access operations
//=============================================================================

void Bitmap::SetScanLine(int index, unsigned char *data, int data_size) {
	if (index < 0 || index >= GetHeight()) {
		return;
	}

	int copy_length = data_size;
	if (copy_length < 0) {
		copy_length = GetLineLength();
	} else // TODO: use Math namespace here
		if (copy_length > GetLineLength()) {
			copy_length = GetLineLength();
		}

	memcpy(_alBitmap->line[index], data, copy_length);
}

namespace BitmapHelper {

Bitmap *CreateRawBitmapOwner(BITMAP *al_bmp) {
	Bitmap *bitmap = new Bitmap();
	if (!bitmap->WrapAllegroBitmap(al_bmp, false)) {
		delete bitmap;
		bitmap = nullptr;
	}
	return bitmap;
}

Bitmap *CreateRawBitmapWrapper(BITMAP *al_bmp) {
	Bitmap *bitmap = new Bitmap();
	if (!bitmap->WrapAllegroBitmap(al_bmp, true)) {
		delete bitmap;
		bitmap = nullptr;
	}
	return bitmap;
}

} // namespace BitmapHelper


} // namespace Shared
} // namespace AGS
} // namespace AGS3
