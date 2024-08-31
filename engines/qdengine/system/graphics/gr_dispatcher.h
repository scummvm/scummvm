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

#ifndef QDENGINE_SYSTEM_GRAPHICS_GR_DISPATCHER_H
#define QDENGINE_SYSTEM_GRAPHICS_GR_DISPATCHER_H

#include "common/std/vector.h"
#include "qdengine/xmath.h"
#include "qdengine/system/graphics/gr_screen_region.h"

namespace Graphics {
class ManagedSurface;
}

namespace QDEngine {

// Directions for LineTo()
enum GR_LINEDIR {
	GR_LEFT,
	GR_TOP,
	GR_RIGHT,
	GR_BOTTOM
};

// Modes for putSpr()
const int GR_BLACK_FON      = 0x01;
const int GR_CLIPPED        = 0x02;
const int GR_NOCLIP     = 0x04;
const int GR_FLIP_HORIZONTAL    = 0x08;
const int GR_FLIP_VERTICAL  = 0x10;
const int GR_IGNORE_ALPHA   = 0x20;

// Modes for Rectangle()
const int GR_FILLED     = 0x00;
const int  GR_OUTLINED      = 0x01;

// grDispatcher::_flags
const int GR_INITED     = 0x01;
const int GR_PALETTE        = 0x02;
const int GR_REINIT     = 0x04;

#ifdef _GR_ENABLE_ZBUFFER
typedef int16 zbuf_t;
const GR_ZBUFFFER_MASK  = 0xFFFF;
const GR_ZBUFFER_MAX_Z = 30000;
#endif

enum grTextAlign {
	GR_ALIGN_LEFT,
	GR_ALIGN_CENTER,
	GR_ALIGN_RIGHT
};

enum grPixelFormat {
	GR_RGB565 = 0,
	GR_ARGB1555,
	GR_RGB888,
	GR_ARGB8888
};

class grFont;
class grTileSprite;
class RLEBuffer;
class UI_TextParser;

class grDispatcher {
public:
	grDispatcher();
	virtual ~grDispatcher();

	static bool sys_init();

	bool init(int sx, int sy, grPixelFormat pixel_format);

	void toggle_reinit() {
		_flags |= GR_REINIT;
	}
	bool is_in_reinit_mode() const {
		return _flags & GR_REINIT;
	}

	void set_flag(int fl) {
		_flags |= fl;
	}
	void drop_flag(int fl) {
		_flags &= ~fl;
	}
	bool check_flag(int fl) {
		if (_flags & fl) return true;
		return false;
	}

	bool finit();

	void *Get_hWnd() const {
		return _hWnd;
	}
	int get_SizeX() const {
		return _sizeX;
	}
	int get_SizeY() const {
		return _sizeY;
	}

	void setClipMode(int m) {
		_clipMode = m;
	}
	int getClipMode() const {
		return _clipMode;
	}

	void setClip() {
		setClip(0, 0, _sizeX, _sizeY);
	}

	void getClip(int &l, int &t, int &r, int &b) const {
		l = _clipCoords[GR_LEFT];
		t = _clipCoords[GR_TOP];
		r = _clipCoords[GR_RIGHT];
		b = _clipCoords[GR_BOTTOM];
	}

	void setClip(int l, int t, int r, int b) {
		if (l < 0) l = 0;
		if (r > _sizeX) r = _sizeX;

		if (t < 0) t = 0;
		if (b > _sizeY) b = _sizeY;

		_clipCoords[GR_LEFT] = l;
		_clipCoords[GR_TOP] = t;
		_clipCoords[GR_RIGHT] = r;
		_clipCoords[GR_BOTTOM] = b;
	}

	void limitClip(int l, int t, int r, int b) {
		if (_clipCoords[GR_LEFT] < l) _clipCoords[GR_LEFT] = l;
		if (_clipCoords[GR_TOP] < t) _clipCoords[GR_TOP] = t;
		if (_clipCoords[GR_RIGHT] > r) _clipCoords[GR_RIGHT] = r;
		if (_clipCoords[GR_BOTTOM] > b) _clipCoords[GR_BOTTOM] = b;
	}

	int clipCheck(int x, int y) {
		if (x >= _clipCoords[GR_LEFT] && x < _clipCoords[GR_RIGHT] && y >= _clipCoords[GR_TOP] && y < _clipCoords[GR_BOTTOM])
			return 1;

		return 0;
	}

	int clipCheck(int x, int y, int sx, int sy) {
		if (x - sx >= _clipCoords[GR_LEFT] && x + sx < _clipCoords[GR_RIGHT] && y - sy >= _clipCoords[GR_TOP] && y + sy < _clipCoords[GR_BOTTOM])
			return 1;

		return 0;
	}


	bool flush(int x, int y, int sx, int sy);
	bool flush();
	bool flushChanges();

	void fill(int val);

	void putSpr(int x, int y, int sx, int sy, const byte *p, int mode, int spriteFormat);
	void putSpr(int x, int y, int sx, int sy, const byte *p, int mode, int spriteFormat, float scale);
	void putSpr_rle(int x, int y, int sx, int sy, const RLEBuffer *p, int mode, bool alpha_flag);
	void putSpr_rle(int x, int y, int sx, int sy, const RLEBuffer *p, int mode, float scale, bool alpha_flag);
	void putSpr_a(int x, int y, int sx, int sy, const byte *p, int mode);
	void putSpr_a(int x, int y, int sx, int sy, const byte *p, int mode, float scale);

	void putSpr_rot(const Vect2i &pos, const Vect2i &size, const byte *data, bool has_alpha, int mode, float angle);
	void putSpr_rot(const Vect2i &pos, const Vect2i &size, const byte *data, bool has_alpha, int mode, float angle, const Vect2f &scale);
	void putSpr_rle_rot(const Vect2i &pos, const Vect2i &size, const RLEBuffer *data, bool has_alpha, int mode, float angle);
	void putSpr_rle_rot(const Vect2i &pos, const Vect2i &size, const RLEBuffer *data, bool has_alpha, int mode, float angle, const Vect2f &scale);

	void putSprMask_rot(const Vect2i &pos, const Vect2i &size, const byte *data, bool has_alpha, uint32 mask_color, int mask_alpha, int mode, float angle);
	void putSprMask_rot(const Vect2i &pos, const Vect2i &size, const byte *data, bool has_alpha, uint32 mask_color, int mask_alpha, int mode, float angle, const Vect2f &scale);
	void putSprMask_rle_rot(const Vect2i &pos, const Vect2i &size, const RLEBuffer *data, bool has_alpha, uint32 mask_color, int mask_alpha, int mode, float angle);
	void putSprMask_rle_rot(const Vect2i &pos, const Vect2i &size, const RLEBuffer *data, bool has_alpha, uint32 mask_color, int mask_alpha, int mode, float angle, const Vect2f &scale);

	void putSprMask(int x, int y, int sx, int sy, const byte *p, uint32 mask_color, int mask_alpha, int mode);
	void putSprMask(int x, int y, int sx, int sy, const byte *p, uint32 mask_color, int mask_alpha, int mode, float scale);
	void putSprMask_rle(int x, int y, int sx, int sy, const RLEBuffer *p, uint32 mask_color, int mask_alpha, int mode, bool alpha_flag);
	void putSprMask_rle(int x, int y, int sx, int sy, const RLEBuffer *p, uint32 mask_color, int mask_alpha, int mode, float scale, bool alpha_flag);
	void putSprMask_a(int x, int y, int sx, int sy, const byte *p, uint32 mask_color, int mask_alpha, int mode);
	void putSprMask_a(int x, int y, int sx, int sy, const byte *p, uint32 mask_color, int mask_alpha, int mode, float scale);

	void putTileSpr(int x, int y, const grTileSprite &sprite, bool has_alpha, int mode);

	void putChar(int x, int y, uint32 color, int font_sx, int font_sy, const byte *font_alpha, const grScreenRegion &char_region);

	void drawSprContour_a(int x, int y, int sx, int sy, const byte *p, int contour_color, int mode);
	void drawSprContour_a(int x, int y, int sx, int sy, const byte *p, int contour_color, int mode, float scale);
	void drawSprContour(int x, int y, int sx, int sy, const byte *p, int contour_color, int mode);
	void drawSprContour(int x, int y, int sx, int sy, const byte *p, int contour_color, int mode, float scale);
	void drawSprContour(int x, int y, int sx, int sy, const RLEBuffer *p, int contour_color, int mode, bool alpha_flag);
	void drawSprContour(int x, int y, int sx, int sy, const RLEBuffer *p, int contour_color, int mode, float scale, bool alpha_flag);

	bool drawText(int x, int y, uint32 color, const char *str, int hspace = 0, int vspace = 0, const grFont *font = NULL);
	bool drawAlignedText(int x, int y, int sx, int sy, uint32 color, const char *str, grTextAlign align = GR_ALIGN_LEFT, int hspace = 0, int vspace = 0, const grFont *font = NULL);
	bool drawParsedText(int x, int y, int sx, int sy, uint32 color, const UI_TextParser *parser, grTextAlign align = GR_ALIGN_LEFT, const grFont *font = NULL);
	int textWidth(const char *str, int hspace = 0, const grFont *font = NULL, bool first_string_only = false) const;
	int textHeight(const char *str, int vspace = 0, const grFont *font = NULL) const;

#ifdef _GR_ENABLE_ZBUFFER
	void putSpr_z(int x, int y, int z, int sx, int sy, const byte *p, int mode);
	void putSpr_z(int x, int y, int z, int sx, int sy, const byte *p, int mode, float scale);
	void putSpr_rle_z(int x, int y, int z, int sx, int sy, const RLEBuffer *p, int mode, bool alpha_flag);
	void putSpr_rle_z(int x, int y, int z, int sx, int sy, const RLEBuffer *p, int mode, float scale, bool alpha_flag);
	void putSpr_a_z(int x, int y, int z, int sx, int sy, const byte *p, int mode);
	void putSpr_a_z(int x, int y, int z, int sx, int sy, const byte *p, int mode, float scale);
#endif

	void erase(int x, int y, int sx, int sy, int col);
	void erase(int x, int y, int sx, int sy, int r, int g, int b);

	void setPixel(int x, int y, int col);
	void setPixelFast(int x, int y, int col);
	void setPixelFast(int x, int y, int r, int g, int b);

	void setPixel(int x, int y, int r, int g, int b);

	void getPixel(int x, int y, uint16 &col);
	void getPixel(int x, int y, byte &r, byte &g, byte &b);

	void line(int x1, int y1, int x2, int y2, int col, int line_style = 0, bool inverse_col = false);

#ifdef _GR_ENABLE_ZBUFFER
	void line_z(int x1, int y1, int z1, int x2, int y2, int z2, int col, int line_style = 0);
#endif

	void lineTo(int x, int y, int len, int dir, int col, int line_style = 0);
	void rectangle(int x, int y, int sx, int sy, int outcol, int incol, int mode, int line_style = 0);
	void rectangleAlpha(int x, int y, int sx, int sy, uint32 color, int alpha);

	int PalettedMode() const {
		return (_flags & GR_PALETTE);
	}

	grPixelFormat pixel_format() const {
		return _pixel_format;
	}
	void set_pixel_format(grPixelFormat mode) {
		_pixel_format = GR_RGB565;
	}

	inline int bytes_per_pixel() const {
		return 2;
	}

	enum { // маски для high color режимов
		mask_565_r = 0xFFFF & (0x001F << 11),
		mask_565_g = 0xFFFF & (0x003F << 5),
		mask_565_b = 0xFFFF & (0x001F << 0),

		mask_555_r = 0xFFFF & (0x001F << 10),
		mask_555_g = 0xFFFF & (0x001F << 5),
		mask_555_b = 0xFFFF & (0x001F << 0)
	};

	inline uint32 make_rgb(uint32 color) const {
		switch (_pixel_format) {
		case GR_RGB565:
			return make_rgb565u((color >> 0) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
		case GR_ARGB1555:
			return make_rgb555u((color >> 0) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
		case GR_RGB888:
		case GR_ARGB8888:
			return color;
		}

		return 0;
	}

	inline uint32 make_rgb(uint32 r, uint32 g, uint32 b) const {
		switch (_pixel_format) {
		case GR_RGB565:
			return make_rgb565u(r, g, b);
		case GR_ARGB1555:
			return make_rgb555u(r, g, b);
		case GR_RGB888:
		case GR_ARGB8888:
			return ((b << 16) | (g << 8) | r);
		}

		return 0;
	}


	/// Обработчик ввода символа
	typedef bool (*char_input_hanler_t)(int input);
	static char_input_hanler_t set_input_handler(char_input_hanler_t h) {
		char_input_hanler_t old_h = _input_handler;
		_input_handler = h;
		return old_h;
	}

	static bool handle_char_input(int input) {
		if (_input_handler) return (*_input_handler)(input);
		return false;
	}

	static inline uint32 make_rgb888(uint32 r, uint32 g, uint32 b) {
		return ((b << 16) | (g << 8) | r);
	}

	static inline uint16 make_rgb565u(uint32 r, uint32 g, uint32 b) {
		return (((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3) << 0));
	}
	static inline uint16 make_rgb555u(uint32 r, uint32 g, uint32 b) {
		return (((r >> 3) << 10) | ((g >> 3) << 5) | ((b >> 3) << 0));
	}

	static inline void split_rgb565u(uint32 col, byte &r, byte &g, byte &b) {
		r = ((col & mask_565_r) >> 11) << 3;
		g = ((col & mask_565_g) >> 5) << 2;
		b = ((col & mask_565_b) >> 0) << 3;
	}
	static inline void split_rgb555u(uint32 col, byte &r, byte &g, byte &b) {
		r = ((col & mask_555_r) >> 10) << 3;
		g = ((col & mask_555_g) >> 5) << 3;
		b = ((col & mask_555_b) >> 0) << 3;
	}

	static inline void split_rgb888(uint32 col, uint32 &r, uint32 &g, uint32 &b) {
		r = (col >>  0) & 0xFF;
		g = (col >>  8) & 0xFF;
		b = (col >> 16) & 0xFF;
	}

	static inline uint16 make_rgb565(uint32 r, uint32 g, uint32 b) {
		return ((r << 11) | (g << 5) | (b << 0));
	}
	static inline uint16 make_rgb555(uint32 r, uint32 g, uint32 b) {
		return ((r << 10) | (g << 5) | (b << 0));
	}

	static inline uint16 alpha_blend_565(uint16 pic_col, uint16 scr_col, uint32 a) {
		if (a != 255) {
			if (a)
				return pic_col + (((((scr_col & mask_565_r) * a) >> 8) & mask_565_r) |
				                  ((((scr_col & mask_565_g) * a) >> 8) & mask_565_g) |
				                  ((((scr_col & mask_565_b) * a) >> 8) & mask_565_b));
			else
				return pic_col;
		} else
			return scr_col;
	}

	static inline uint16 alpha_blend_555(uint16 pic_col, uint16 scr_col, uint32 a) {
		if (a != 255) {
			if (a)
				return pic_col + (((((scr_col & mask_555_r) * a) >> 8) & mask_555_r) |
				                  ((((scr_col & mask_555_g) * a) >> 8) & mask_555_g) |
				                  ((((scr_col & mask_555_b) * a) >> 8) & mask_555_b));
			else
				return pic_col;
		} else
			return scr_col;
	}

	const void *mouse_cursor() const {
		return _mouse_cursor;
	}
	void set_default_mouse_cursor() {
		_mouse_cursor = _default_mouse_cursor;
	}
	void set_null_mouse_cursor() {
		_mouse_cursor = NULL;
	}

	static grDispatcher *instance(void *hwnd);

	bool is_mouse_hidden() const {
		return _hide_mouse;
	}
	void hideMouse() {
		_hide_mouse = true;
	}
	void showMouse() {
		_hide_mouse = false;
	}

	bool clip_line(int &x0, int &y0, int &x1, int &y1) const;
	bool clip_line(int &x0, int &y0, int &z0, int &x1, int &y1, int &z1) const;
	bool clip_rectangle(int &x, int &y, int &pic_x, int &pic_y, int &pic_sx, int &pic_sy) const;

	bool is_rectangle_visible(int x, int y, int sx, int sy) const {
		if (x + sx < 0 || x >= _sizeX || y + sy < 0 || y >= _sizeY) return false;
		return true;
	}

	bool clip_rectangle(int &x, int &y, int &sx, int &sy) const {
		int x1 = x + sx;
		int y1 = y + sy;

		if (x < _clipCoords[0]) x = _clipCoords[0];
		if (x1 >= _clipCoords[2]) x1 = _clipCoords[2] - 1;

		if (y < _clipCoords[1]) y = _clipCoords[1];
		if (y1 >= _clipCoords[3]) y1 = _clipCoords[3] - 1;

		sx = x1 - x;
		sy = y1 - y;

		if (sx <= 0 || sy <= 0)
			return false;

		return true;
	}

	void clear_changes_mask();

	typedef Std::vector<grScreenRegion> regions_container_t;
	typedef regions_container_t::const_iterator region_iterator;

	const regions_container_t &changed_regions() const {
		return _changed_regions;
	}
	void build_changed_regions();
	bool invalidate_region(const grScreenRegion &reg);

	static inline grDispatcher *instance() {
		return _dispatcher_ptr;
	}
	static inline grDispatcher *set_instance(grDispatcher *p) {
		grDispatcher *old_p = _dispatcher_ptr;
		_dispatcher_ptr = p;
		return old_p;
	}

	static inline const char *wnd_class_name() {
		return _wnd_class_name;
	}

	typedef void (*restore_handler_t)();
	static restore_handler_t set_restore_handler(restore_handler_t h) {
		restore_handler_t old_h = _restore_handler;
		_restore_handler = h;
		return old_h;
	}

	static bool is_active() {
		return _is_active;
	}
	static void activate(bool state) {
		if (state && !_is_active) {
			if (_restore_handler)
				(*_restore_handler)();
		}
		_is_active = state;
	}

	char *temp_buffer(int size);

	static bool convert_sprite(grPixelFormat src_fmt, grPixelFormat &dest_fmt, int sx, int sy, byte *data, bool &has_alpha);

	static grFont *load_font(const char *file_name);
	static void set_default_font(grFont *p) {
		_default_font = p;
	}
	static grFont *get_default_font() {
		return _default_font;
	}
protected:

	int _flags;

	int _wndPosX;
	int _wndPosY;
	int _wndSizeX;
	int _wndSizeY;

	int _sizeX;
	int _sizeY;

	grPixelFormat _pixel_format;
	void *_hWnd;

	Graphics::ManagedSurface *_screenBuf = nullptr;

	int *_yTable;

	char *_temp_buffer;
	int _temp_buffer_size;

private:

	int _clipMode;
	int _clipCoords[4];

	bool _hide_mouse;
	void *_mouse_cursor;
	static void *_default_mouse_cursor;

	enum {
		clLEFT   = 1,
		clRIGHT  = 2,
		clBOTTOM = 4,
		clTOP    = 8
	};

	inline int clip_out_code(int x, int y) const {
		int code = 0;
		if (y >= _clipCoords[3])
			code |= clTOP;
		else if (y < _clipCoords[1])
			code |= clBOTTOM;
		if (x >= _clipCoords[2])
			code |= clRIGHT;
		else if (x < _clipCoords[0])
			code |= clLEFT;

		return code;
	}

#ifdef _GR_ENABLE_ZBUFFER
	zbuf_t *zbuffer_;

	bool alloc_zbuffer(int sx, int sy);
	bool free_zbuffer();
	bool clear_zbuffer();

	zbuf_t get_z(int x, int y) {
		return zbuffer_[x + y * _sizeX];
	}
	void put_z(int x, int y, int z) {
		zbuffer_[x + y * _sizeX] = z;
	}
#endif

	typedef Std::vector<char> changes_mask_t;

	enum {
		kChangesMaskTile = 16,
		kChangesMaskTileShift = 4
	};

	int _changes_mask_size_x;
	int _changes_mask_size_y;

	changes_mask_t _changes_mask;

	regions_container_t _changed_regions;

	static char_input_hanler_t _input_handler;

	static grFont *_default_font;

	static bool _is_active;
	static restore_handler_t _restore_handler;

	static grDispatcher *_dispatcher_ptr;
	static char *_wnd_class_name;

	void putSpr_rot90(const Vect2i &pos, const Vect2i &size, const byte *data, bool has_alpha, int mode, float angle);
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_GRAPHICS_GR_DISPATCHER_H
