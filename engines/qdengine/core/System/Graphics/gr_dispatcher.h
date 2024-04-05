
#ifndef __GR_DISPATCHER_H__
#define __GR_DISPATCHER_H__

#include "gr_screen_region.h"

// Directions for LineTo()
enum GR_LINEDIR
{
	GR_LEFT,
	GR_TOP,
	GR_RIGHT,
	GR_BOTTOM
};

// Modes for PutSpr()
const int GR_BLACK_FON		= 0x01;
const int GR_CLIPPED		= 0x02;
const int GR_NOCLIP		= 0x04;
const int GR_FLIP_HORIZONTAL	= 0x08;
const int GR_FLIP_VERTICAL	= 0x10;
const int GR_IGNORE_ALPHA	= 0x20;

// Modes for Rectangle()
const int GR_FILLED		= 0x00;
const int  GR_OUTLINED		= 0x01;

// grDispatcher::flags
const int GR_INITED		= 0x01;
const int GR_PALETTE		= 0x02;
const int GR_REINIT		= 0x04;

#ifdef _GR_ENABLE_ZBUFFER
typedef short zbuf_t;
const GR_ZBUFFFER_MASK	= 0xFFFF;
const GR_ZBUFFER_MAX_Z = 30000;
#endif

enum grTextAlign
{
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
class rleBuffer;
class UI_TextParser;

class grDispatcher
{
public:
	grDispatcher();
	virtual ~grDispatcher();

	static bool sys_init();
	static bool sys_finit();

	virtual bool is_in_fullscreen_mode() const = 0;
	virtual bool is_mode_supported(int sx,int sy,grPixelFormat pixel_format) const = 0;
	virtual bool is_mode_supported(grPixelFormat pixel_format) const = 0;
	grPixelFormat adjust_mode(grPixelFormat pixel_format) const {
		if(!is_mode_supported(pixel_format)){
			switch(pixel_format){
				case GR_RGB565:
					return GR_ARGB1555;
				case GR_ARGB1555:
					return GR_RGB565;
				case GR_RGB888:
					return GR_ARGB8888;
				case GR_ARGB8888:
					return GR_RGB888;
			}
		}

		return pixel_format;
	}

	virtual bool init(int sx,int sy,grPixelFormat pixel_format,void* hwnd,bool fullscreen = false);

	void toggle_reinit(){ flags |= GR_REINIT; }
	bool is_in_reinit_mode() const { return flags & GR_REINIT; }

	bool create_window(int sx,int sy);
	bool resize_window(int sx,int sy);
	bool resize_window(){ return resize_window(SizeX,SizeY); }

	bool destroy_window();

	void set_flag(int fl){ flags |= fl; }
	void drop_flag(int fl){ flags &= ~fl; }
	bool check_flag(int fl){ if(flags & fl) return true; return false; }

	bool set_window(void* hwnd);

	virtual bool Finit();

	void* Get_hWnd() const { return hWnd; }
	int Get_SizeX() const { return SizeX; }
	int Get_SizeY() const { return SizeY; }

	void SetClipMode(int m){ clipMode = m; }
	int GetClipMode() const { return clipMode; }

	void SetClip(){ SetClip(0,0,SizeX,SizeY); }

	void GetClip(int& l,int& t,int& r,int& b) const { 
		l = clipCoords[GR_LEFT];
		t = clipCoords[GR_TOP];
		r = clipCoords[GR_RIGHT];
		b = clipCoords[GR_BOTTOM];
	}

	void SetClip(int l,int t,int r,int b){
		if(l < 0) l = 0;
		if(r > SizeX) r = SizeX;

		if(t < 0) t = 0;
		if(b > SizeY) b = SizeY;

		clipCoords[GR_LEFT] = l; clipCoords[GR_TOP] = t;
		clipCoords[GR_RIGHT] = r; clipCoords[GR_BOTTOM] = b;
	}

	void LimitClip(int l,int t,int r,int b){
		if(clipCoords[GR_LEFT] < l) clipCoords[GR_LEFT] = l;
		if(clipCoords[GR_TOP] < t) clipCoords[GR_TOP] = t;
		if(clipCoords[GR_RIGHT] > r) clipCoords[GR_RIGHT] = r;
		if(clipCoords[GR_BOTTOM] > b) clipCoords[GR_BOTTOM] = b;
	}

	int ClipCheck(int x,int y){ 
		if(x >= clipCoords[GR_LEFT] && x < clipCoords[GR_RIGHT] && y >= clipCoords[GR_TOP] && y < clipCoords[GR_BOTTOM])
			return 1;

		return 0;
	}

	int ClipCheck(int x,int y,int sx,int sy){ 
		if(x - sx >= clipCoords[GR_LEFT] && x + sx < clipCoords[GR_RIGHT] && y - sy >= clipCoords[GR_TOP] && y + sy < clipCoords[GR_BOTTOM])
			return 1;

		return 0;
	}

	virtual bool Flush(int x,int y,int sx,int sy) = 0;
	virtual bool StretchFlush(int x_dest,int y_dest,int sx_dest,int sy_dest,int x_src,int y_src,int sx_src,int sy_src) = 0;

	bool Flush(){ return Flush(0,0,SizeX,SizeY); }
	bool FlushChanges();

	void Fill(int val);

	void PutSpr(int x,int y,int sx,int sy,const unsigned char* p,int mode);
	void PutSpr(int x,int y,int sx,int sy,const unsigned char* p,int mode,float scale);
	void PutSpr_rle(int x,int y,int sx,int sy,const rleBuffer* p,int mode,bool alpha_flag);
	void PutSpr_rle(int x,int y,int sx,int sy,const rleBuffer* p,int mode,float scale,bool alpha_flag);
	void PutSpr_a(int x,int y,int sx,int sy,const unsigned char* p,int mode);
	void PutSpr_a(int x,int y,int sx,int sy,const unsigned char* p,int mode,float scale);

	void PutSpr_rot(const Vect2i& pos, const Vect2i& size, const unsigned char* data, bool has_alpha, int mode, float angle);
	void PutSpr_rot(const Vect2i& pos, const Vect2i& size, const unsigned char* data, bool has_alpha, int mode, float angle, const Vect2f& scale);
	void PutSpr_rle_rot(const Vect2i& pos, const Vect2i& size, const rleBuffer* data, bool has_alpha, int mode, float angle);
	void PutSpr_rle_rot(const Vect2i& pos, const Vect2i& size, const rleBuffer* data, bool has_alpha, int mode, float angle, const Vect2f& scale);

	void PutSprMask_rot(const Vect2i& pos, const Vect2i& size, const unsigned char* data, bool has_alpha, unsigned mask_color, int mask_alpha, int mode, float angle);
	void PutSprMask_rot(const Vect2i& pos, const Vect2i& size, const unsigned char* data, bool has_alpha, unsigned mask_color, int mask_alpha, int mode, float angle, const Vect2f& scale);
	void PutSprMask_rle_rot(const Vect2i& pos, const Vect2i& size, const rleBuffer* data, bool has_alpha, unsigned mask_color, int mask_alpha, int mode, float angle);
	void PutSprMask_rle_rot(const Vect2i& pos, const Vect2i& size, const rleBuffer* data, bool has_alpha, unsigned mask_color, int mask_alpha, int mode, float angle, const Vect2f& scale);

	void PutSprMask(int x,int y,int sx,int sy,const unsigned char* p,unsigned mask_color,int mask_alpha,int mode);
	void PutSprMask(int x,int y,int sx,int sy,const unsigned char* p,unsigned mask_color,int mask_alpha,int mode,float scale);
	void PutSprMask_rle(int x,int y,int sx,int sy,const rleBuffer* p,unsigned mask_color,int mask_alpha,int mode,bool alpha_flag);
	void PutSprMask_rle(int x,int y,int sx,int sy,const rleBuffer* p,unsigned mask_color,int mask_alpha,int mode,float scale,bool alpha_flag);
	void PutSprMask_a(int x,int y,int sx,int sy,const unsigned char* p,unsigned mask_color,int mask_alpha,int mode);
	void PutSprMask_a(int x,int y,int sx,int sy,const unsigned char* p,unsigned mask_color,int mask_alpha,int mode,float scale);

	void PutTileSpr(int x, int y, const grTileSprite& sprite, bool has_alpha, int mode);

	void PutChar(int x,int y,unsigned color,int font_sx,int font_sy,const unsigned char* font_alpha,const grScreenRegion& char_region);

	void DrawSprContour_a(int x,int y,int sx,int sy,const unsigned char* p,int contour_color,int mode);
	void DrawSprContour_a(int x,int y,int sx,int sy,const unsigned char* p,int contour_color,int mode,float scale);
	void DrawSprContour(int x,int y,int sx,int sy,const unsigned char* p,int contour_color,int mode);
	void DrawSprContour(int x,int y,int sx,int sy,const unsigned char* p,int contour_color,int mode,float scale);
	void DrawSprContour(int x,int y,int sx,int sy,const rleBuffer* p,int contour_color,int mode,bool alpha_flag);
	void DrawSprContour(int x,int y,int sx,int sy,const rleBuffer* p,int contour_color,int mode,float scale,bool alpha_flag);

	bool DrawText(int x,int y,unsigned color,const char* str,int hspace = 0,int vspace = 0,const grFont* font = NULL);
	bool DrawAlignedText(int x,int y,int sx,int sy,unsigned color,const char* str,grTextAlign align = GR_ALIGN_LEFT,int hspace = 0,int vspace = 0,const grFont* font = NULL);
	bool DrawParsedText(int x,int y,int sx,int sy,unsigned color,const UI_TextParser* parser,grTextAlign align = GR_ALIGN_LEFT,const grFont* font = NULL);
	int TextWidth(const char* str,int hspace = 0,const grFont* font = NULL,bool first_string_only = false) const;
	int TextHeight(const char* str,int vspace = 0,const grFont* font = NULL) const;

#ifdef _GR_ENABLE_ZBUFFER
	void PutSpr_z(int x,int y,int z,int sx,int sy,const unsigned char* p,int mode);
	void PutSpr_z(int x,int y,int z,int sx,int sy,const unsigned char* p,int mode,float scale);
	void PutSpr_rle_z(int x,int y,int z,int sx,int sy,const rleBuffer* p,int mode,bool alpha_flag);
	void PutSpr_rle_z(int x,int y,int z,int sx,int sy,const rleBuffer* p,int mode,float scale,bool alpha_flag);
	void PutSpr_a_z(int x,int y,int z,int sx,int sy,const unsigned char* p,int mode);
	void PutSpr_a_z(int x,int y,int z,int sx,int sy,const unsigned char* p,int mode,float scale);
#endif

	void Erase(int x,int y,int sx,int sy,int col);
	void Erase(int x,int y,int sx,int sy,int r,int g,int b);

	void SetPixel(int x,int y,int col);
	void SetPixelFast(int x,int y,int col);
	void SetPixelFast(int x,int y,int r,int g,int b);

	void SetPixel(int x,int y,int r,int g,int b);

	void GetPixel(int x,int y,unsigned& col);
	void GetPixel(int x,int y,unsigned& r,unsigned& g,unsigned& b);

	void Line(int x1,int y1,int x2,int y2,int col,int line_style = 0,bool inverse_col = false);

#ifdef _GR_ENABLE_ZBUFFER
	void Line_z(int x1,int y1,int z1,int x2,int y2,int z2,int col,int line_style = 0);
#endif

	void LineTo(int x,int y,int len,int dir,int col,int line_style = 0);
	void Rectangle(int x,int y,int sx,int sy,int outcol,int incol,int mode,int line_style = 0);
	void RectangleAlpha(int x,int y,int sx,int sy,unsigned color,int alpha);

	int PalettedMode() const { return (flags & GR_PALETTE); }

	grPixelFormat pixel_format() const { return pixel_format_; }
	void set_pixel_format(grPixelFormat mode){ pixel_format_ = mode; }

	inline int bytes_per_pixel() const {
		switch(pixel_format_){
		case GR_RGB565:
		case GR_ARGB1555:
			return 2;
		case GR_RGB888:
			return 3;
		case GR_ARGB8888:
			return 4;
		}

		return 0;
	}

	enum { // маски для high color режимов
		mask_565_r = 0xFFFF & (0x001F << 11),
		mask_565_g = 0xFFFF & (0x003F << 5),
		mask_565_b = 0xFFFF & (0x001F << 0),

		mask_555_r = 0xFFFF & (0x001F << 10),
		mask_555_g = 0xFFFF & (0x001F << 5),
		mask_555_b = 0xFFFF & (0x001F << 0)
	};

	inline unsigned make_rgb(unsigned color) const {
		switch(pixel_format_){
		case GR_RGB565:
			return make_rgb565u((color >> 0) & 0xFF,(color >> 8) & 0xFF,(color >> 16) & 0xFF);
		case GR_ARGB1555:
			return make_rgb555u((color >> 0) & 0xFF,(color >> 8) & 0xFF,(color >> 16) & 0xFF);
		case GR_RGB888:
		case GR_ARGB8888:
			return color;
		}

		return 0;
	}

	inline unsigned make_rgb(unsigned r,unsigned g,unsigned b) const {
		switch(pixel_format_){
		case GR_RGB565:
			return make_rgb565u(r,g,b);
		case GR_ARGB1555:
			return make_rgb555u(r,g,b);
		case GR_RGB888:
		case GR_ARGB8888:
			return ((b << 16) | (g << 8) | r);
		}

		return 0;
	}


	/// Обработчик ввода символа
	typedef bool (*char_input_hanler_t)(int input);
	static char_input_hanler_t set_input_handler(char_input_hanler_t h){
		char_input_hanler_t old_h = input_handler_;
		input_handler_ = h;
		return old_h;
	}

	static bool handle_char_input(int input){
		if(input_handler_) return (*input_handler_)(input);
		return false;
	}

	static inline unsigned make_rgb888(unsigned r,unsigned g,unsigned b){ return ((b << 16) | (g << 8) | r); }

	static inline unsigned short make_rgb565u(unsigned r,unsigned g,unsigned b){ return (((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3) << 0)); }
	static inline unsigned short make_rgb555u(unsigned r,unsigned g,unsigned b){ return (((r >> 3) << 10) | ((g >> 3) << 5) | ((b >> 3) << 0)); }

	static inline void split_rgb565u(unsigned col,unsigned& r,unsigned& g,unsigned& b){
		r = ((col & mask_565_r) >> 11) << 3;
		g = ((col & mask_565_g) >> 5) << 2;
		b = ((col & mask_565_b) >> 0) << 3;
	}
	static inline void split_rgb555u(unsigned col,unsigned& r,unsigned& g,unsigned& b){
		r = ((col & mask_555_r) >> 10) << 3;
		g = ((col & mask_555_g) >> 5) << 3;
		b = ((col & mask_555_b) >> 0) << 3;
	}

	static inline void split_rgb888(unsigned col,unsigned& r,unsigned& g,unsigned& b){
		r = (col >>  0) & 0xFF;
		g = (col >>  8) & 0xFF;
		b = (col >> 16) & 0xFF;
	}
	
	static inline unsigned short make_rgb565(unsigned r,unsigned g,unsigned b){ return ((r << 11) | (g << 5) | (b << 0)); }
	static inline unsigned short make_rgb555(unsigned r,unsigned g,unsigned b){ return ((r << 10) | (g << 5) | (b << 0)); }

	static inline unsigned short alpha_blend_565(unsigned short pic_col,unsigned short scr_col,unsigned a){
		if(a != 255){
			if(a)
				return pic_col + (((((scr_col & mask_565_r) * a) >> 8) & mask_565_r) |
					((((scr_col & mask_565_g) * a) >> 8) & mask_565_g) |
					((((scr_col & mask_565_b) * a) >> 8) & mask_565_b));
			else
				return pic_col;
		}
		else
			return scr_col;
	}

	static inline unsigned short alpha_blend_555(unsigned short pic_col,unsigned short scr_col,unsigned a){
		if(a != 255){
			if(a)
				return pic_col + (((((scr_col & mask_555_r) * a) >> 8) & mask_555_r) |
					 ((((scr_col & mask_555_g) * a) >> 8) & mask_555_g) |
					 ((((scr_col & mask_555_b) * a) >> 8) & mask_555_b));
			else
				return pic_col;
		}
		else
			return scr_col;
	}

	const void* mouse_cursor() const { return mouse_cursor_; }
	void set_default_mouse_cursor(){ mouse_cursor_ = default_mouse_cursor_; }
	void set_null_mouse_cursor(){ mouse_cursor_ = NULL;	}

	static grDispatcher* instance(void* hwnd);

	bool is_mouse_hidden() const { return hide_mouse_; }
	void HideMouse(){ hide_mouse_ = true; }
	void ShowMouse(){ hide_mouse_ = false; }

	bool clip_line(int& x0,int& y0,int& x1,int& y1) const;
	bool clip_line(int& x0,int& y0,int& z0,int& x1,int& y1,int& z1) const;
	bool clip_rectangle(int& x,int& y,int& pic_x,int& pic_y,int& pic_sx,int& pic_sy) const;

	bool is_rectangle_visible(int x,int y,int sx,int sy) const {
		if(x + sx < 0 || x >= SizeX || y + sy < 0 || y >= SizeY) return false;
		return true;
	}

	bool clip_rectangle(int& x,int& y,int& sx,int& sy) const {
		int x1 = x + sx;
		int y1 = y + sy;

		if(x < clipCoords[0]) x = clipCoords[0];
		if(x1 >= clipCoords[2]) x1 = clipCoords[2] - 1;

		if(y < clipCoords[1]) y = clipCoords[1];
		if(y1 >= clipCoords[3]) y1 = clipCoords[3] - 1;

		sx = x1 - x;
		sy = y1 - y;

		if(sx <= 0 || sy <= 0)
			return false;

		return true;
	}

	void clear_changes_mask();

	typedef std::vector<grScreenRegion> regions_container_t;
	typedef regions_container_t::const_iterator region_iterator;

	const regions_container_t& changed_regions() const { return changed_regions_; }
	void build_changed_regions();
	bool invalidate_region(const grScreenRegion& reg);

	static inline grDispatcher* instance(){ return dispatcher_ptr_; }
	static inline grDispatcher* set_instance(grDispatcher* p){ grDispatcher* old_p = dispatcher_ptr_; dispatcher_ptr_ = p; return old_p; }

	static inline const char* wnd_class_name(){ return wnd_class_name_; }

	typedef void (*restore_handler_t)();
	static restore_handler_t set_restore_handler(restore_handler_t h){
		restore_handler_t old_h = restore_handler_;
		restore_handler_ = h;
		return old_h;
	}

	static bool is_active(){ return is_active_; }
	static void activate(bool state){ 
		if(state && !is_active_){
			if(restore_handler_)
				(*restore_handler_)();
		}
		is_active_ = state;
	}

	typedef void (*maximize_handler_t)();
	maximize_handler_t set_maximize_handler(maximize_handler_t p)
	{
		maximize_handler_t old_handler = maximize_handler_;
		maximize_handler_ = p;
		return old_handler;
	}
	maximize_handler_t maximize_handler() const { return maximize_handler_; }

	char* temp_buffer(int size);

	static bool convert_sprite(grPixelFormat src_fmt,grPixelFormat& dest_fmt,int sx,int sy,unsigned char* data,bool& has_alpha);

	static grFont* load_font(const char* file_name);
	static void set_default_font(grFont* p){ default_font_ = p; }
	static grFont* get_default_font() { return default_font_; }
protected:

	int flags;

	int wndPosX;
	int wndPosY;
	int wndSizeX;
	int wndSizeY;

	int SizeX;
	int SizeY;

	grPixelFormat pixel_format_;
	void* hWnd;

	char* screenBuf;
	int* yTable;

	char* temp_buffer_;
	int temp_buffer_size_;

private:

	int clipMode;
	int clipCoords[4];

	bool hide_mouse_;
	void* mouse_cursor_;
	static void* default_mouse_cursor_;

	enum
	{
		clLEFT	 = 1,
		clRIGHT	 = 2,
		clBOTTOM = 4,
		clTOP	 = 8
	};

	inline int clip_out_code(int x,int y) const
	{
		int code = 0;
		if(y >= clipCoords[3])
			code |= clTOP;
		else if(y < clipCoords[1])
			code |= clBOTTOM;
		if(x >= clipCoords[2])
			code |= clRIGHT;
		else if(x < clipCoords[0])
			code |= clLEFT;

		return code;
	}

#ifdef _GR_ENABLE_ZBUFFER
	zbuf_t* zbuffer_;

	bool alloc_zbuffer(int sx,int sy);
	bool free_zbuffer();
	bool clear_zbuffer();

	zbuf_t get_z(int x,int y){ return zbuffer_[x + y * SizeX]; }
	void put_z(int x,int y,int z){ zbuffer_[x + y * SizeX] = z; }
#endif

	typedef std::vector<char> changes_mask_t;

	enum {
		changes_mask_tile_ = 16,
		changes_mask_tile_shift_ = 4
	};

	int changes_mask_size_x_;
	int changes_mask_size_y_;

	changes_mask_t changes_mask_;

	regions_container_t changed_regions_;

	maximize_handler_t maximize_handler_;

	static char_input_hanler_t input_handler_;

	static grFont* default_font_;

	static bool is_active_;
	static restore_handler_t restore_handler_;

	static grDispatcher* dispatcher_ptr_;
	static char* wnd_class_name_;

	void PutSpr_rot90(const Vect2i& pos, const Vect2i& size, const unsigned char* data, bool has_alpha, int mode, float angle);
};

#endif /* __GR_DISPATCHER_H__ */
