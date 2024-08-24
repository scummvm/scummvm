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

#include "common/textconsole.h"
#include "common/system.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/graphics/gr_font.h"
#include "qdengine/system/graphics/UI_TextParser.h"


namespace QDEngine {

void *grDispatcher::_default_mouse_cursor = nullptr;

grDispatcher::char_input_hanler_t grDispatcher::_input_handler = 0;

bool grDispatcher::_is_active = true; // We have system always active
grDispatcher::restore_handler_t grDispatcher::_restore_handler = 0;

grDispatcher *grDispatcher::_dispatcher_ptr;
grFont *grDispatcher::_default_font;

grDispatcher::grDispatcher() : _screenBuf(NULL),
#ifdef _GR_ENABLE_ZBUFFER
	zbuffer_(NULL),
#endif
	_hWnd(NULL),
	_yTable(NULL),
	_temp_buffer(0) {
	_flags = 0;

	_temp_buffer_size = 0;

	_clipMode = 0;

	_sizeX = _sizeY = 0;
	_wndSizeX = _wndSizeY = 0;
	_wndPosX = _wndPosY = 0;

	_changes_mask_size_x = _changes_mask_size_y = 0;

	_hide_mouse = false;
	_mouse_cursor = NULL;

	_pixel_format = GR_RGB565;

	setClip();

	if (!_dispatcher_ptr) _dispatcher_ptr = this;
}

grDispatcher::~grDispatcher() {
	finit();

	if (_dispatcher_ptr == this) _dispatcher_ptr = 0;
}

bool grDispatcher::finit() {
#ifdef _GR_ENABLE_ZBUFFER
	free_zbuffer();
#endif

	_flags &= ~GR_INITED;
	_sizeX = _sizeY = 0;
	_wndPosX = _wndPosY = 0;
	delete _screenBuf;
	_screenBuf = nullptr;
	delete  _yTable;
	_yTable = NULL;

	return true;
}

bool grDispatcher::init(int sx, int sy, grPixelFormat pixel_format) {
	finit();

	_pixel_format = pixel_format;

	initGraphics(sx, sy, &g_engine->_pixelformat);
	_screenBuf = new Graphics::ManagedSurface(sx, sy, g_engine->_pixelformat);

	_sizeX = sx;
	_sizeY = sy;

	_changes_mask_size_x = _sizeX >> kChangesMaskTileShift;
	if (_sizeX % kChangesMaskTile) _changes_mask_size_x++;
	_changes_mask_size_y = _sizeY >> kChangesMaskTileShift;
	if (_sizeY % kChangesMaskTile) _changes_mask_size_y++;

	_changes_mask.resize(_changes_mask_size_x * _changes_mask_size_y);

	_flags &= ~GR_REINIT;

#ifdef _GR_ENABLE_ZBUFFER
	alloc_zbuffer(_sizeX, _sizeY);
#endif

	return true;
}

void grDispatcher::fill(int val) {
	_screenBuf->clear(val);
}

bool grDispatcher::flush(int x, int y, int sx, int sy) {
	int x1 = x + sx;
	int y1 = y + sy;

	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	if (x1 > _sizeX)
		x1 = _sizeX;

	if (y1 > _sizeY)
		y1 = _sizeY;

	debugC(8, kDebugGraphics, "grDispatcher::flush(%d, %d, %d, %d)", x, y, x1 - x, y1 - y);

	g_system->copyRectToScreen(_screenBuf->getBasePtr(x, y), _screenBuf->pitch, x, y, x1 - x, y1 - y);

	return true;
}

bool grDispatcher::flush() {
	return flush(0, 0, _sizeX, _sizeY);
}

void grDispatcher::line(int x1, int y1, int x2, int y2, int col, int line_style, bool inverse_col) {
	const int F_PREC = 16;

	if (!clip_line(x1, y1, x2, y2)) return;

	if (!inverse_col) {
		if (x1 == x2 && y1 == y2) {
			setPixelFast(x1, y1, col);
			return;
		}

		if (abs(x2 - x1) > abs(y2 - y1)) {
			int a = x2 - x1;
			int b = y2 - y1;
			int x = x1;
			int y = (y1 << F_PREC) + (1 << (F_PREC - 1));
			int incr = 1;
			int k = (b << F_PREC) / a;
			int v = 0;
			if (x1 > x2) {
				incr = -1;
				k = -k;
				a = -a;
			}
			do {
				if (++v > line_style) {
					setPixelFast(x, y >> F_PREC, col);
					if (v >= line_style * 2)
						v = 0;
				}
				x += incr;
				y += k;
			} while (--a >= 0);
		} else {
			int a = x2 - x1;
			int b = y2 - y1;
			int x = (x1 << F_PREC) + (1 << (F_PREC - 1));
			int y = y1;
			int incr = 1;
			int k = (a << F_PREC) / b;
			int v = 0;
			if (y1 > y2) {
				incr = -1;
				k = -k;
				b = -b;
			}
			do {
				if (++v > line_style) {
					setPixelFast(x >> F_PREC, y, col);
					if (v >= line_style * 2)
						v = 0;
				}
				y += incr;
				x += k;
			} while (--b >= 0);
		}
	} else {
		if (x1 == x2 && y1 == y2) {
			setPixelFast(x1, y1, col);
			return;
		}

		if (abs(x2 - x1) > abs(y2 - y1)) {
			int a = x2 - x1;
			int b = y2 - y1;
			int x = x1;
			int y = (y1 << F_PREC) + (1 << (F_PREC - 1));
			int incr = 1;
			int k = (b << F_PREC) / a;
			int v = 0;
			if (x1 > x2) {
				incr = -1;
				k = -k;
				a = -a;
			}
			do {
				if (++v > line_style) {
					setPixelFast(x, y >> F_PREC, col);
					if (v >= line_style * 2)
						v = 0;
				} else
					setPixelFast(x, y >> F_PREC, ~col);

				x += incr;
				y += k;
			} while (--a >= 0);
		} else {
			int a = x2 - x1;
			int b = y2 - y1;
			int x = (x1 << F_PREC) + (1 << (F_PREC - 1));
			int y = y1;
			int incr = 1;
			int k = (a << F_PREC) / b;
			int v = 0;
			if (y1 > y2) {
				incr = -1;
				k = -k;
				b = -b;
			}
			do {
				if (++v > line_style) {
					setPixelFast(x >> F_PREC, y, col);
					if (v >= line_style * 2)
						v = 0;
				} else
					setPixelFast(x >> F_PREC, y, ~col);

				y += incr;
				x += k;
			} while (--b >= 0);
		}
	}
}

void grDispatcher::lineTo(int x, int y, int len, int dir, int col, int line_style) {
	int v;

	switch (dir) {
	case GR_LEFT:
		v = x - len;
		if (!clip_line(x, y, v, y)) return;
		_screenBuf->vLine(x, y, y + len, col);
		break;
	case GR_TOP:
		v = y - len;
		if (!clip_line(x, y, x, v)) return;
		_screenBuf->hLine(x, y, x + len, col);
		break;
	case GR_RIGHT:
		v = x + len;
		if (!clip_line(x, y, v, y)) return;
		_screenBuf->vLine(x, y, y + len, col);
		break;
	case GR_BOTTOM:
		v = y + len;
		if (!clip_line(x, y, x, v)) return;
		_screenBuf->hLine(x, y, x + len, col);
		break;
	}
}

void grDispatcher::rectangle(int x, int y, int sx, int sy, int outcol, int incol, int mode, int line_style) {
	if (!sx || !sy) return;

	lineTo(x, y, sx, GR_RIGHT, outcol, line_style);
	lineTo(x, y, sy, GR_BOTTOM, outcol, line_style);
	lineTo(x + sx - 1, y, sy, GR_BOTTOM, outcol, line_style);
	lineTo(x, y + sy - 1, sx, GR_RIGHT, outcol, line_style);

	if (mode == GR_FILLED) {
		if (sx < 3) return;
		erase(x + 1, y + 1, sx - 2, sy - 2, incol);
	}
}

void grDispatcher::rectangleAlpha(int x, int y, int sx, int sy, uint32 color, int alpha) {
	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if (!clip_rectangle(x, y, px, py, psx, psy)) return;
	int dx = 1;
	int dy = 1;

	byte mr, mg, mb;
	split_rgb565u(color, mr, mg, mb);

	mr = (mr * (255 - alpha)) >> 8;
	mg = (mg * (255 - alpha)) >> 8;
	mb = (mb * (255 - alpha)) >> 8;

	uint32 mcl = make_rgb565u(mr, mg, mb);

	for (int i = 0; i < psy; i++) {
		uint16 *scr_buf = reinterpret_cast<uint16 *>(_screenBuf->getBasePtr(x, y));

		for (int j = 0; j < psx; j++) {
			*scr_buf = alpha_blend_565(mcl, *scr_buf, alpha);
			scr_buf += dx;
		}

		y += dy;
	}
}

void grDispatcher::erase(int x, int y, int sx, int sy, int col) {
	if (_clipMode)
		if (!clip_rectangle(x, y, sx, sy))
			return;

	_screenBuf->fillRect(Common::Rect(x, y, x + sx, y + sy), col);
	return;
}

void grDispatcher::setPixel(int x, int y, int col) {
	if (_clipMode && !clipCheck(x, y)) return;

	uint16 *p = (uint16 *)(_screenBuf->getBasePtr(x, y));
	*p = col;
}

void grDispatcher::setPixelFast(int x, int y, int col) {
	uint16 *p = (uint16 *)(_screenBuf->getBasePtr(x, y));
	*p = col;
}

void grDispatcher::setPixelFast(int x, int y, int r, int g, int b) {
	uint16 *p = (uint16 *)(_screenBuf->getBasePtr(x, y));
	*p = (((r >> 3) << 11) + ((g >> 2) << 5) + ((b >> 3) << 0));
}

void grDispatcher::setPixel(int x, int y, int r, int g, int b) {
	if (_clipMode && !clipCheck(x, y)) return;

	uint16 *p = (uint16 *)(_screenBuf->getBasePtr(x * 2, y));
	*p = (((r >> 3) << 11) + ((g >> 2) << 5) + ((b >> 3) << 0));
}

void grDispatcher::getPixel(int x, int y, uint16 &col) {
	col = *(uint16 *)(_screenBuf->getBasePtr(x, y));
}

void grDispatcher::getPixel(int x, int y, byte &r, byte &g, byte &b) {
	uint16 col = *(uint16 *)(_screenBuf->getBasePtr(x, y));
	split_rgb565u(col, r, g, b);
}

bool grDispatcher::clip_line(int &x0, int &y0, int &x1, int &y1) const {
	int x = 0, y = 0;
	bool accept = false, done = false;

	int outcodeOut;
	int outcode0 = clip_out_code(x0, y0);
	int outcode1 = clip_out_code(x1, y1);

	do {
		if (outcode0 == 0 && outcode1 == 0) {
			accept = true;
			done = true;
		} else {
			if ((outcode0 & outcode1) != 0)
				done = true;
			else {
				if (outcode0)
					outcodeOut = outcode0;
				else
					outcodeOut = outcode1;

				if (clTOP & outcodeOut) {
					x = x0 + (x1 - x0) * (_clipCoords[3] - y0 - 1) / (y1 - y0);
					y = _clipCoords[3] - 1;
				} else if (clBOTTOM & outcodeOut) {
					x = x0 + (x1 - x0) * (_clipCoords[1] - y0) / (y1 - y0);
					y = _clipCoords[1];
				}
				if (clRIGHT & outcodeOut) {
					y = y0 + (y1 - y0) * (_clipCoords[2] - x0 - 1) / (x1 - x0);
					x = _clipCoords[2] - 1;
				} else if (clLEFT & outcodeOut) {
					y = y0 + (y1 - y0) * (_clipCoords[0] - x0) / (x1 - x0);
					x = _clipCoords[0];
				}

				if (outcodeOut == outcode0) {
					x0 = x;
					y0 = y;

					outcode0 = clip_out_code(x, y);
				} else {
					x1 = x;
					y1 = y;

					outcode1 = clip_out_code(x, y);
				}
			}
		}
	} while (!done);

	return accept;
}

bool grDispatcher::clip_line(int &x0, int &y0, int &z0, int &x1, int &y1, int &z1) const {
	int x = 0, y = 0, z = 0;
	bool accept = false, done = false;

	int outcodeOut;
	int outcode0 = clip_out_code(x0, y0);
	int outcode1 = clip_out_code(x1, y1);

	do {
		if (outcode0 == 0 && outcode1 == 0) {
			accept = true;
			done = true;
		} else {
			if ((outcode0 & outcode1) != 0)
				done = true;
			else {
				if (outcode0)
					outcodeOut = outcode0;
				else
					outcodeOut = outcode1;

				if (clTOP & outcodeOut) {
					x = x0 + (x1 - x0) * (_clipCoords[3] - y0 - 1) / (y1 - y0);
					z = z0 + (z1 - z0) * (_clipCoords[3] - y0 - 1) / (y1 - y0);
					y = _clipCoords[3] - 1;
				} else if (clBOTTOM & outcodeOut) {
					x = x0 + (x1 - x0) * (_clipCoords[1] - y0) / (y1 - y0);
					z = z0 + (z1 - z0) * (_clipCoords[1] - y0) / (y1 - y0);
					y = _clipCoords[1];
				}
				if (clRIGHT & outcodeOut) {
					y = y0 + (y1 - y0) * (_clipCoords[2] - x0 - 1) / (x1 - x0);
					z = z0 + (z1 - z0) * (_clipCoords[2] - x0 - 1) / (x1 - x0);
					x = _clipCoords[2] - 1;
				} else if (clLEFT & outcodeOut) {
					y = y0 + (y1 - y0) * (_clipCoords[0] - x0) / (x1 - x0);
					z = z0 + (z1 - z0) * (_clipCoords[0] - x0) / (x1 - x0);
					x = _clipCoords[0];
				}

				if (outcodeOut == outcode0) {
					x0 = x;
					y0 = y;
					z0 = z;

					outcode0 = clip_out_code(x, y);
				} else {
					x1 = x;
					y1 = y;
					z1 = z;

					outcode1 = clip_out_code(x, y);
				}
			}
		}
	} while (!done);

	return accept;
}

#ifdef _GR_ENABLE_ZBUFFER
void grDispatcher::Line_z(int x1, int y1, int z1, int x2, int y2, int z2, int col, int line_style) {
	const F_PREC = 16;

	if (!clip_line(x1, y1, z1, x2, y2, z2)) return;

	if (x1 == x2 && y1 == y2) {
		int z = (z1 < z2) ? z1 : z2;
		if (get_z(x1, y1) > z)
			setPixelFast(x1, y1, col);
		return;
	}

	if (abs(x2 - x1) > abs(y2 - y1)) {
		int a = x2 - x1;
		int x = x1;
		int y = (y1 << F_PREC) + (1 << (F_PREC - 1));
		int incr = 1;
		int k = ((y2 - y1) << F_PREC) / a;
		int v = 0;

		int z = z1 << F_PREC;
		int dz = ((z2 - z1) << F_PREC) / a;

		if (x1 > x2) {
			incr = -1;
			k = -k;
			a = -a;
			dz = -dz;
		}

		do {
			if (++v > line_style) {
				if (get_z(x, y >> F_PREC) >= (z >> F_PREC))
					setPixelFast(x, y >> F_PREC, col);
				v = 0;
			}
			x += incr;
			y += k;
			z += dz;
		} while (--a >= 0);
	} else {
		int b = y2 - y1;
		int x = (x1 << F_PREC) + (1 << (F_PREC - 1));
		int y = y1;
		int incr = 1;
		int k = ((x2 - x1) << F_PREC) / b;
		int v = 0;

		int z = z1 << F_PREC;
		int dz = ((z2 - z1) << F_PREC) / b;

		if (y1 > y2) {
			incr = -1;
			k = -k;
			b = -b;
			dz = -dz;
		}
		do {
			if (++v > line_style) {
				if (get_z(x >> F_PREC, y) >= (z >> F_PREC))
					setPixelFast(x >> F_PREC, y, col);
				v = 0;
			}
			y += incr;
			x += k;
			z += dz;
		} while (--b >= 0);
	}
}
#endif

bool grDispatcher::clip_rectangle(int &x, int &y, int &pic_x, int &pic_y, int &pic_sx, int &pic_sy) const {
	if (x < _clipCoords[0]) {
		pic_x += _clipCoords[0] - x;
		pic_sx += x - _clipCoords[0];

		x = _clipCoords[0];
	}
	if (x + pic_sx >= _clipCoords[2])
		pic_sx += _clipCoords[2] - (x + pic_sx);
//		pic_sx += _clipCoords[2] - 1 - (x + pic_sx);

	if (y < _clipCoords[1]) {
		pic_y += _clipCoords[1] - y;
		pic_sy += y - _clipCoords[1];

		y = _clipCoords[1];
	}
	if (y + pic_sy >= _clipCoords[3])
		pic_sy += _clipCoords[3] - (y + pic_sy);
//		pic_sy += _clipCoords[3] - 1 - (y + pic_sy);

	if (pic_x >= 0 && pic_y >= 0 && pic_sx > 0 && pic_sy > 0)
		return true;

	return false;
}

#ifdef _GR_ENABLE_ZBUFFER
bool grDispatcher::alloc_zbuffer(int sx, int sy) {
	if (zbuffer_) delete zbuffer_;
	zbuffer_ = new zbuf_t[sx * sy];
	return true;
}

bool grDispatcher::free_zbuffer() {
	if (zbuffer_) delete zbuffer_;
	zbuffer_ = 0;

	return true;
}

bool grDispatcher::clear_zbuffer() {
	zbuf_t *p = zbuffer_;
	for (int i = 0; i < _sizeY; i++) {
		for (int j = 0; j < _sizeX; j++) {
			*p++ = GR_ZBUFFER_MAX_Z;
		}
	}
	return true;
}
#endif

void grDispatcher::clear_changes_mask() {
	Common::fill(_changes_mask.begin(), _changes_mask.end(), 0);
}

void grDispatcher::build_changed_regions() {
	_changed_regions.clear();

	bool flag = true;

	while (flag) {
		flag = false;

		changes_mask_t::iterator it = Common::find(_changes_mask.begin(), _changes_mask.end(), 1);
		if (it != _changes_mask.end()) {
			int x = (it - _changes_mask.begin()) % _changes_mask_size_x;
			int y = (it - _changes_mask.begin()) / _changes_mask_size_x;

			changes_mask_t::iterator it1 = Common::find(it, it + (_changes_mask_size_x - x), 0);

			int sx = it1 - it;
			int sy = 0;

			for (int i = 0; i < _changes_mask_size_y - y; i++, sy++) {
				changes_mask_t::iterator it2 = Common::find(it, it1, 0);
				if (it2 == it1)
					Common::fill(it, it1, 0);
				else
					break;

				if (i < _changes_mask_size_y - y - 1) {
					it += _changes_mask_size_x;
					it1 += _changes_mask_size_x;
				}
			}

			x <<= kChangesMaskTileShift;
			y <<= kChangesMaskTileShift;

			sx <<= kChangesMaskTileShift;
			sy <<= kChangesMaskTileShift;

			_changed_regions.push_back(grScreenRegion(x + sx / 2, y + sy / 2, sx, sy));

			flag = true;
		}
	}
}

bool grDispatcher::invalidate_region(const grScreenRegion &reg) {
	int x = reg.min_x();
	int y = reg.min_y();

	int sx = reg.size_x();
	int sy = reg.size_y();

	if (clip_rectangle(x, y, sx, sy)) {
		sx = ((x + sx) >> kChangesMaskTileShift) - (x >> kChangesMaskTileShift) + 1;
		sy = ((y + sy) >> kChangesMaskTileShift) - (y >> kChangesMaskTileShift) + 1;

		x >>= kChangesMaskTileShift;
		y >>= kChangesMaskTileShift;

		if (x + sx > _changes_mask_size_x) sx = _changes_mask_size_x - x;
		if (y + sy > _changes_mask_size_y) sy = _changes_mask_size_y - y;

		if (sx <= 0 || sy <= 0) return false;

		changes_mask_t::iterator it = _changes_mask.begin() + (x + y * _changes_mask_size_x);

		for (int i = 0; i < sy; i++) {
			auto it1 = it;
			for (int j = 0; j < sx; j++) {
				*it1 = true;
				++it1;
			}
			if (i < sy - 1)
				it += _changes_mask_size_x;
		}
	}

	return true;
}

bool grDispatcher::flushChanges() {
	for (regions_container_t::const_iterator it = _changed_regions.begin(); it != _changed_regions.end(); ++it)
		flush(it->min_x() - 1, it->min_y() - 1, it->size_x() + 2, it->size_y() + 2);

	return true;
}

// TODO
bool grDispatcher::convert_sprite(grPixelFormat src_fmt, grPixelFormat &dest_fmt, int sx, int sy, byte *data, bool &has_alpha) {
	if (dest_fmt == GR_RGB888 && (src_fmt == GR_ARGB1555 || src_fmt == GR_RGB565)) {
		if (has_alpha)
			dest_fmt = GR_ARGB8888;
	} else if (dest_fmt == GR_ARGB8888 && (src_fmt == GR_ARGB1555 || src_fmt == GR_RGB565)) {
		if (!has_alpha)
			dest_fmt = GR_RGB888;
	}

	switch (dest_fmt) {
	case GR_ARGB1555:
		if (src_fmt == GR_RGB565) {
			uint16 *p = reinterpret_cast<uint16 *>(data);
			for (int i = 0; i < sx * sy; i++) {
				byte r, g, b;
				split_rgb565u(*p, r, g, b);
				*p++ = make_rgb555u(r, g, b);
				if (has_alpha) p++;
			}
		} else if (src_fmt == GR_RGB888) {
			byte *dp = data;
			uint16 *p = reinterpret_cast<uint16 *>(data);
			for (int i = 0; i < sx * sy; i++) {
				*p++ = make_rgb555u(dp[2], dp[1], dp[0]);
				dp += 3;
			}
			has_alpha = false;
		} else if (src_fmt == GR_ARGB8888) {
			byte *dp = data;
			uint16 *p = reinterpret_cast<uint16 *>(data);
			for (int i = 0; i < sx * sy; i++) {
				*p++ = make_rgb555u(dp[2], dp[1], dp[0]);
				*p++ >>= 8;
				dp += 4;
			}
			has_alpha = true;
		}
		return true;
	case GR_RGB565:
		if (src_fmt == GR_ARGB1555) {
			uint16 *p = reinterpret_cast<uint16 *>(data);
			for (int i = 0; i < sx * sy; i++) {
				byte r, g, b;
				split_rgb555u(*p, r, g, b);
				*p++ = make_rgb565u(r, g, b);

				if (has_alpha) p++;
			}
		} else if (src_fmt == GR_RGB888) {
			byte *dp = data;
			uint16 *p = reinterpret_cast<uint16 *>(data);
			for (int i = 0; i < sx * sy; i++) {
				*p++ = make_rgb565u(dp[2], dp[1], dp[0]);
				dp += 3;
			}
			has_alpha = false;
		} else if (src_fmt == GR_ARGB8888) {
			byte *dp = data;
			uint16 *p = reinterpret_cast<uint16 *>(data);
			for (int i = 0; i < sx * sy; i++) {
				*p++ = make_rgb565u(dp[2], dp[1], dp[0]);
				*p++ >>= 8;
				dp += 4;
			}
			has_alpha = true;
		}
		return true;
	case GR_RGB888:
		if (src_fmt == GR_ARGB1555 || src_fmt == GR_RGB565) {
			uint16 *p = reinterpret_cast<uint16 *>(data) + sx * sy - 1;
			byte *dp = data + sx * sy * 3 - 1;

			for (int i = 0; i < sx * sy; i++) {
				byte r, g, b;

				if (src_fmt == GR_ARGB1555)
					split_rgb555u(*p--, r, g, b);
				else
					split_rgb565u(*p--, r, g, b);

				*dp-- = r;
				*dp-- = g;
				*dp-- = b;
			}
			return true;
		}
		break;
	case GR_ARGB8888:
		if (src_fmt == GR_ARGB1555 || src_fmt == GR_RGB565) {
			uint16 *p = reinterpret_cast<uint16 *>(data);
			for (int i = 0; i < sx * sy; i++) {
				p++;
				*p++ <<= 8;
			}

			p = reinterpret_cast<uint16 *>(data);
			byte *dp = data;
			for (int i = 0; i < sx * sy; i++) {
				byte r, g, b;

				if (src_fmt == GR_ARGB1555)
					split_rgb555u(*p++, r, g, b);
				else
					split_rgb565u(*p++, r, g, b);

				p++;

				dp[0] = b;
				dp[1] = g;
				dp[2] = r;

				dp += 4;
			}
			return true;
		}
		break;
	}

	return false;
}

grFont *grDispatcher::load_font(const char *file_name) {
	grFont *p = new grFont;

	if (!p->load(file_name)) {
		delete p;
		p = 0;
	}

	return p;
}

char *grDispatcher::temp_buffer(int size) {
	if (size <= 0) size = 1;

	if (size > _temp_buffer_size) {
		delete[] _temp_buffer;
		_temp_buffer = new char[size];
		_temp_buffer_size = size;
	}

	return _temp_buffer;
}

bool grDispatcher::drawText(int x, int y, uint32 color, const char *str, int hspace, int vspace, const grFont *font) {
	if (!font)
		font = _default_font;

	if (!font || !font->alpha_buffer())
		return false;

	const byte *str_buf = reinterpret_cast<const byte *>(str);

	int x0 = x;
	int sz = strlen(str);

	for (int i = 0; i < sz; i++) {
		if (str_buf[i] != '\n') {
			const grScreenRegion &rg = font->find_char(str_buf[i]);
			int dx = rg.size_x();

			if (str_buf[i] == ' ')
				x += font->size_x() / 2 + hspace;
			else if (dx) {
				putChar(x, y, color, font->alpha_buffer_size_x(), font->alpha_buffer_size_y(), font->alpha_buffer(), rg);
				x += dx + hspace;
			}
		} else {
			x = x0;
			y += font->size_y() + vspace;
		}
	}

	return true;
}

bool grDispatcher::drawAlignedText(int x, int y, int sx, int sy, uint32 color, const char *str, grTextAlign align, int hspace, int vspace, const grFont *font) {
	if (!font)
		font = _default_font;

	if (!font || !font->alpha_buffer())
		return false;

	const byte *str_buf = reinterpret_cast<const byte *>(str);

	if (!sx)
		sx = textWidth(str, hspace, font);

	int x0 = x;
	int delta_x = 0;
	int sz = strlen(str);

	switch (align) {
	case GR_ALIGN_CENTER:
		delta_x = (sx - textWidth(str, hspace, font, true)) / 2;
		break;
	case GR_ALIGN_RIGHT:
		delta_x = sx - textWidth(str, hspace, font, true);
		break;
	default:
		break;
	}

	for (int i = 0; i < sz; i++) {
		if (str_buf[i] != '\n') {
			const grScreenRegion &rg = font->find_char(str_buf[i]);
			int dx = rg.size_x();

			if (str_buf[i] == ' ') {
				x += font->size_x() / 2 + hspace;
			} else if (dx) {
				putChar(x + delta_x, y, color, font->alpha_buffer_size_x(), font->alpha_buffer_size_y(), font->alpha_buffer(), rg);
				x += dx + hspace;
			}
		} else {
			x = x0;
			y += font->size_y() + vspace;

			switch (align) {
			case GR_ALIGN_CENTER:
				delta_x = (sx - textWidth(str + i + 1, hspace, font, true)) / 2;
				break;
			case GR_ALIGN_RIGHT:
				delta_x = sx - textWidth(str + i + 1, hspace, font, true);
				break;
			default:
				break;
			}
		}
	}

	return true;
}

bool grDispatcher::drawParsedText(int x, int y, int sx, int sy, uint32 color, const UI_TextParser *parser, grTextAlign align, const grFont *font) {
	if (!font)
		font = _default_font;

	int hSize = parser->fontHeight();
	y -= hSize;

	bool skipToNextLine = false;
	int cur_x = x;
	for (OutNodes::const_iterator it = parser->outNodes().begin(); it != parser->outNodes().end(); ++it) {
		switch (it->type) {
		case OutNode::NEW_LINE:
			skipToNextLine = false;
			y += hSize;
			switch (align) {
			case GR_ALIGN_LEFT:
				cur_x = x;
				break;
			case GR_ALIGN_CENTER:
				cur_x = x + (sx - it->width) / 2;
				break;
			case GR_ALIGN_RIGHT:
				cur_x = x + sx - it->width - 1;
				break;
			default:
				break;
			}
			break;

		case OutNode::TEXT: {
			if (skipToNextLine)
				break;

			Common::String str(it->nl.begin, it->nl.end - it->nl.begin);
			drawText(cur_x, y, color, str.c_str(), 0, 0, font);

			//int ssx = textWidth(str.c_str(), 0, font);
			//int ssx1 = it->width;
			cur_x += it->width;
		}
		break;

		case OutNode::COLOR:
			color = it->color;
			break;

		}
	}

	return true;
}

int grDispatcher::textWidth(const char *str, int hspace, const grFont *font, bool first_string_only) const {
	if (!font)
		font = _default_font;

	if (!font)
		return false;

	const byte *str_buf = (const byte *)str;

	int sx = 0, sx_max = 0;
	int sz = strlen(str);
	for (int i = 0; i < sz; i++) {
		if (str_buf[i] != '\n') {
			const grScreenRegion &rg = font->find_char(str_buf[i]);
			int dx = rg.size_x();

			if (str_buf[i] == ' ')
				sx += font->size_x() / 2;
			else
				sx += dx + hspace;
		} else {
			if (first_string_only)
				return sx;

			if (sx_max < sx) sx_max = sx;
			sx = 0;
		}
	}

	if (sx_max < sx) sx_max = sx;

	return sx_max;
}

int grDispatcher::textHeight(const char *str, int vspace, const grFont *font) const {
	if (!font)
		font = _default_font;

	if (!font)
		return false;

	int sy = font->size_y() + vspace;

	for (uint i = 0; i < strlen(str); i++) {
		if (str[i] == '\n')
			sy += font->size_y() + vspace;
	}

	return sy;
}

} // namespace QDEngine
