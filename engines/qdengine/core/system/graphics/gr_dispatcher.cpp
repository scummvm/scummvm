/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "gr_dispatcher.h"
#include "gr_font.h"

#include "UI_TextParser.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

grDispatcher::char_input_hanler_t grDispatcher::input_handler_ = 0;

bool grDispatcher::is_active_ = false;
grDispatcher::restore_handler_t grDispatcher::restore_handler_ = 0;

grDispatcher* grDispatcher::dispatcher_ptr_;
grFont* grDispatcher::default_font_;

grDispatcher::grDispatcher() : screenBuf(NULL),
#ifdef _GR_ENABLE_ZBUFFER
	zbuffer_(NULL),
#endif
	hWnd(NULL),
	yTable(NULL),
	temp_buffer_(0),
	maximize_handler_(0)
{
	flags = 0;

	temp_buffer_size_ = 0;

	clipMode = 0;

	SizeX = SizeY = 0;
	wndSizeX = wndSizeY = 0;
	wndPosX = wndPosY = 0;

	changes_mask_size_x_ = changes_mask_size_y_ = 0;

	hide_mouse_ = false;
	mouse_cursor_ = NULL;

	pixel_format_ = GR_RGB888;

	if(!dispatcher_ptr_) dispatcher_ptr_ = this;
}

grDispatcher::~grDispatcher()
{
	Finit();

	if(dispatcher_ptr_ == this) dispatcher_ptr_ = 0;
}

bool grDispatcher::Finit()
{
#ifdef _GR_ENABLE_ZBUFFER
	free_zbuffer();
#endif

	flags &= ~GR_INITED;
	SizeX = SizeY = 0;
	wndPosX = wndPosY = 0;
	screenBuf = NULL;
	delete  yTable;
	yTable = NULL;

	return true;
}

bool grDispatcher::init(int sx,int sy,grPixelFormat pixel_format,void* hwnd,bool fullscreen)
{ 
	Finit();

	pixel_format_ = pixel_format;

	if(!is_mode_supported(sx,sy,pixel_format_))
		pixel_format_ = adjust_mode(pixel_format);

	if(!hwnd){
		if(!Get_hWnd()){
			if(!create_window(sx,sy)) return false;
		}
		else
			resize_window(sx,sy);

		SizeX = sx;
		SizeY = sy;
	}
	else {
		set_window(hwnd);

		if(sx && sy){
			SizeX = sx;
			SizeY = sy;
		}
	}

	changes_mask_size_x_ = SizeX >> changes_mask_tile_shift_;
	if(SizeX % changes_mask_tile_) changes_mask_size_x_++;
	changes_mask_size_y_ = SizeY >> changes_mask_tile_shift_;
	if(SizeY % changes_mask_tile_) changes_mask_size_y_++;

	changes_mask_.resize(changes_mask_size_x_ * changes_mask_size_y_);
	
	SetWindowLong((HWND)Get_hWnd(),GWL_USERDATA,int(this));

	flags &= ~GR_REINIT;

#ifdef _GR_ENABLE_ZBUFFER
	alloc_zbuffer(SizeX,SizeY);
#endif

	return true;
}

void grDispatcher::Fill(int val)
{
	if(val){
		if(bytes_per_pixel() == 2){
			for(int y = 0; y < SizeY; y ++){
				unsigned short* p = (unsigned short*)(screenBuf + yTable[y]);
				for(int x = 0; x < SizeX; x ++)
					*p++ = val;
			}
			return;
		}
		if(bytes_per_pixel() == 3){
			unsigned char* vp = (unsigned char*)(&val);

			for(int y = 0; y < SizeY; y ++){
				unsigned char* p = (unsigned char*)(screenBuf + yTable[y]);
				for(int x = 0; x < SizeX; x ++){
					p[0] = vp[2];
					p[1] = vp[1];
					p[2] = vp[0];
					p += 3;
				}
			}
			return;
		}
		if(bytes_per_pixel() == 4){
			for(int y = 0; y < SizeY; y ++){
				unsigned* p = (unsigned*)(screenBuf + yTable[y]);
				for(int x = 0; x < SizeX; x ++)
					*p++ = val;
			}
			return;
		}
	}
	else {
		for(int y = 0; y < SizeY; y ++)
			memset(screenBuf + yTable[y],0,SizeX * bytes_per_pixel());

#ifdef _GR_ENABLE_ZBUFFER
		clear_zbuffer();
#endif
	}
}

void grDispatcher::Line(int x1,int y1,int x2,int y2,int col,int line_style,bool inverse_col)
{
	const F_PREC = 16;

	if(!clip_line(x1,y1,x2,y2)) return;

	if(!inverse_col){
		if(x1 == x2 && y1 == y2){
			SetPixelFast(x1,y1,col);
			return;
		}

		if(abs(x2 - x1) > abs(y2 - y1)){
			int a = x2 - x1;
			int b = y2 - y1;
			int x = x1;
			int y = (y1 << F_PREC) + (1 << (F_PREC - 1));
			int incr = 1;
			int k = (b << F_PREC)/a;
			int v = 0;
			if(x1 > x2){
				incr = -1;
				k = -k;
				a = -a;
			}
			do {
				if(++v > line_style){
					SetPixelFast(x,y >> F_PREC,col);
					if(v >= line_style * 2)
						v = 0;
				}
				x += incr;
				y += k;
			} while(--a >= 0);
		}
		else {
			int a = x2 - x1;
			int b = y2 - y1;
			int x = (x1 << F_PREC) + (1 << (F_PREC - 1));
			int y = y1;
			int incr = 1;
			int k = (a << F_PREC)/b;
			int v = 0;
			if(y1 > y2){
				incr = -1;
				k = -k;
				b = -b;
			}
			do {
				if(++v > line_style){
					SetPixelFast(x >> F_PREC,y,col);
					if(v >= line_style * 2)
						v = 0;
				}
				y += incr;
				x += k;
			} while(--b >= 0);
		}
	}
	else {
		if(x1 == x2 && y1 == y2){
			SetPixelFast(x1,y1,col);
			return;
		}

		if(abs(x2 - x1) > abs(y2 - y1)){
			int a = x2 - x1;
			int b = y2 - y1;
			int x = x1;
			int y = (y1 << F_PREC) + (1 << (F_PREC - 1));
			int incr = 1;
			int k = (b << F_PREC)/a;
			int v = 0;
			if(x1 > x2){
				incr = -1;
				k = -k;
				a = -a;
			}
			do {
				if(++v > line_style){
					SetPixelFast(x,y >> F_PREC,col);
					if(v >= line_style * 2)
						v = 0;
				}
				else
					SetPixelFast(x,y >> F_PREC,~col);

				x += incr;
				y += k;
			} while(--a >= 0);
		}
		else {
			int a = x2 - x1;
			int b = y2 - y1;
			int x = (x1 << F_PREC) + (1 << (F_PREC - 1));
			int y = y1;
			int incr = 1;
			int k = (a << F_PREC)/b;
			int v = 0;
			if(y1 > y2){
				incr = -1;
				k = -k;
				b = -b;
			}
			do {
				if(++v > line_style){
					SetPixelFast(x >> F_PREC,y,col);
					if(v >= line_style * 2)
						v = 0;
				}
				else
					SetPixelFast(x >> F_PREC,y,~col);

				y += incr;
				x += k;
			} while(--b >= 0);
		}
	}
}

void grDispatcher::LineTo(int x,int y,int len,int dir,int col,int line_style)
{
	int i,v,stv = 0;

	switch(dir){
		case GR_LEFT:
			v = x - len;
			if(!clip_line(x,y,v,y)) return;
			for(i = x; i > v; i --){
				if(++stv > line_style){
					SetPixelFast(i,y,col);
					stv = 0;
				}
			}
			break;
		case GR_TOP:
			v = y - len;
			if(!clip_line(x,y,x,v)) return;
			for(i = y; i > v; i --){
				if(++stv > line_style){
					SetPixelFast(x,i,col);
					stv = 0;
				}
			}
			break;
		case GR_RIGHT:
			v = x + len;
			if(!clip_line(x,y,v,y)) return;
			for(i = x; i < v; i ++){
				if(++stv > line_style){
					SetPixelFast(i,y,col);
					stv = 0;
				}
			}
			break;
		case GR_BOTTOM:
			v = y + len;
			if(!clip_line(x,y,x,v)) return;
			for(i = y; i < v; i ++){
				if(++stv > line_style){
					SetPixelFast(x,i,col);
					stv = 0;
				}
			}
			break;
	}
}

void grDispatcher::Rectangle(int x,int y,int sx,int sy,int outcol,int incol,int mode,int line_style)
{
	if(!sx || !sy) return;

	LineTo(x,y,sx,GR_RIGHT,outcol,line_style);
	LineTo(x,y,sy,GR_BOTTOM,outcol,line_style);
	LineTo(x + sx - 1,y,sy,GR_BOTTOM,outcol,line_style);
	LineTo(x,y + sy - 1,sx,GR_RIGHT,outcol,line_style);

	if(mode == GR_FILLED){
		if(sx < 3) return;
		Erase(x + 1,y + 1,sx - 2,sy - 2,incol);
	}
}

void grDispatcher::RectangleAlpha(int x,int y,int sx,int sy,unsigned color,int alpha)
{
	int px = 0;
	int py = 0;

	int psx = sx;
	int psy = sy;

	if(!clip_rectangle(x,y,px,py,psx,psy)) return;

	if(bytes_per_pixel() == 4){
		int dx = 4;
		int dy = 1;

		x *= 4;

		unsigned mr,mg,mb;
		split_rgb888(color,mr,mg,mb);

		mr = (mr * (255 - alpha)) >> 8;
		mg = (mg * (255 - alpha)) >> 8;
		mb = (mb * (255 - alpha)) >> 8;

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);

			for(int j = 0; j < psx; j ++){
				scr_buf[2] = mr + ((alpha * scr_buf[2]) >> 8);
				scr_buf[1] = mg + ((alpha * scr_buf[1]) >> 8);
				scr_buf[0] = mb + ((alpha * scr_buf[0]) >> 8);

				scr_buf += dx;
			}

			y += dy;
		}
		return;
	}
	if(bytes_per_pixel() == 3){
		int dx = 3;
		int dy = 1;

		x *= 3;

		unsigned mr,mg,mb;
		split_rgb888(color,mr,mg,mb);

		mr = (mr * (255 - alpha)) >> 8;
		mg = (mg * (255 - alpha)) >> 8;
		mb = (mb * (255 - alpha)) >> 8;

		for(int i = 0; i < psy; i ++){
			unsigned char* scr_buf = reinterpret_cast<unsigned char*>(screenBuf + yTable[y] + x);

			for(int j = 0; j < psx; j ++){
				scr_buf[2] = mr + ((alpha * scr_buf[2]) >> 8);
				scr_buf[1] = mg + ((alpha * scr_buf[1]) >> 8);
				scr_buf[0] = mb + ((alpha * scr_buf[0]) >> 8);

				scr_buf += dx;
			}

			y += dy;
		}
		return;
	}
	if(bytes_per_pixel() == 2){
		int dx = 1;
		int dy = 1;

		x <<= 1;

		if(pixel_format_ == GR_RGB565){
			unsigned mr,mg,mb;
			split_rgb565u(color,mr,mg,mb);

			mr = (mr * (255 - alpha)) >> 8;
			mg = (mg * (255 - alpha)) >> 8;
			mb = (mb * (255 - alpha)) >> 8;

			unsigned mcl = make_rgb565u(mr,mg,mb);

			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);

				for(int j = 0; j < psx; j ++){
					*scr_buf = alpha_blend_565(mcl,*scr_buf,alpha);
					scr_buf += dx;
				}

				y += dy;
			}
		}
		else {
			unsigned mr,mg,mb;
			split_rgb555u(color,mr,mg,mb);

			mr = (mr * (255 - alpha)) >> 8;
			mg = (mg * (255 - alpha)) >> 8;
			mb = (mb * (255 - alpha)) >> 8;

			unsigned mcl = make_rgb555u(mr,mg,mb);

			for(int i = 0; i < psy; i ++){
				unsigned short* scr_buf = reinterpret_cast<unsigned short*>(screenBuf + yTable[y] + x);

				for(int j = 0; j < psx; j ++){
					*scr_buf = alpha_blend_555(mcl,*scr_buf,alpha);
					scr_buf += dx;
				}

				y += dy;
			}
		}
		return;
	}
}

void grDispatcher::Erase(int x,int y,int sx,int sy,int col)
{
	if(clipMode)
		if(!clip_rectangle(x,y,sx,sy))
			return;

	switch(bytes_per_pixel()){
	case 2: {
			unsigned short* ptr;
			for(int i = y; i <= y + sy; i ++){
				ptr = (unsigned short*)(screenBuf + yTable[i] + (x << 1));
				for(int j = 0; j <= sx; j ++){
					ptr[j] = col;
				}
			}
		}
		break;
	case 3: {
			x *= 3;
			unsigned char* ptr;
			unsigned char* cptr = (unsigned char*)(&col);
			for(int i = y; i <= y + sy; i ++){
				ptr = (unsigned char*)(screenBuf + yTable[i] + x);
				for(int j = 0; j <= sx; j ++){
					ptr[0] = cptr[2];
					ptr[1] = cptr[1];
					ptr[2] = cptr[0];
					ptr += 3;
				}
			}
		}
		break;
	case 4: {
			unsigned col1 = col;

			unsigned char* cptr = (unsigned char*)(&col);
			unsigned char* cptr1 = (unsigned char*)(&col1);
			cptr1[0] = cptr[2];
			cptr1[2] = cptr[0];

			for(int i = y; i <= y + sy; i ++){
				unsigned* ptr = (unsigned*)(screenBuf + yTable[i] + (x << 2));
				for(int j = 0; j <= sx; j ++){
					*ptr++ = col1;
				}
			}
		}
		break;
	}
}

void grDispatcher::SetPixel(int x,int y,int col)
{
	if(clipMode && !ClipCheck(x,y)) return;

	if(bytes_per_pixel() == 2){
		unsigned short* p = (unsigned short*)(screenBuf + yTable[y]);
		p[x] = col;
		return;
	}
	if(bytes_per_pixel() == 3){
		unsigned char* cp = (unsigned char*)(&col);
		unsigned char* p = (unsigned char*)(screenBuf + yTable[y] + x * 3);

		p[0] = cp[2];
		p[1] = cp[1];
		p[2] = cp[0];
		return;
	}
	if(bytes_per_pixel() == 4){
		unsigned char* cp = (unsigned char*)(&col);
		unsigned char* p = (unsigned char*)(screenBuf + yTable[y] + x * 4);

		p[0] = cp[2];
		p[1] = cp[1];
		p[2] = cp[0];
		return;
	}
}

void grDispatcher::SetPixelFast(int x,int y,int col)
{
	if(bytes_per_pixel() == 2){
		unsigned short* p = (unsigned short*)(screenBuf + yTable[y]);
		p[x] = col;
		return;
	}
	if(bytes_per_pixel() == 3){
		unsigned char* cp = (unsigned char*)(&col);
		unsigned char* p = (unsigned char*)(screenBuf + yTable[y] + x * 3);

		p[0] = cp[2];
		p[1] = cp[1];
		p[2] = cp[0];
		return;
	}
	if(bytes_per_pixel() == 4){
		unsigned char* cp = (unsigned char*)(&col);
		unsigned char* p = (unsigned char*)(screenBuf + yTable[y] + x * 4);

		p[0] = cp[2];
		p[1] = cp[1];
		p[2] = cp[0];
		return;
	}
}

void grDispatcher::SetPixelFast(int x,int y,int r,int g,int b)
{
	if(bytes_per_pixel() == 3){
		unsigned char* p = (unsigned char*)(screenBuf + yTable[y] + x * 3);

		p[2] = r;
		p[1] = g;
		p[0] = b;

		return;
	}
	if(bytes_per_pixel() == 4){
		unsigned char* p = (unsigned char*)(screenBuf + yTable[y] + x * 4);

		p[2] = r;
		p[1] = g;
		p[0] = b;

		return;
	}
}

void grDispatcher::SetPixel(int x,int y,int r,int g,int b)
{
	if(clipMode && !ClipCheck(x,y)) return;

	switch(pixel_format_){
		case GR_RGB565: {
				unsigned short* p = (unsigned short*)(screenBuf + yTable[y] + x * 2);
				*p = (((r >> 3) << 11) + ((g >> 2) << 5) + ((b >> 3) << 0));
			}
			return;
		case GR_ARGB1555: {
				unsigned short* p = (unsigned short*)(screenBuf + yTable[y] + x * 2);
				*p = (((r >> 3) << 10) + ((g >> 3) << 5) + ((b >> 3) << 0));
			}
			return;
		case GR_RGB888: {
				unsigned char* cp = (unsigned char*)(screenBuf + yTable[y] + x * 3);

				cp[2] = r;
				cp[1] = g;
				cp[0] = b;
			}
			return;
		case GR_ARGB8888: {
				unsigned char* cp = (unsigned char*)(screenBuf + yTable[y] + x * 4);

				cp[2] = r;
				cp[1] = g;
				cp[0] = b;
			}
			return;
	}
}

void grDispatcher::GetPixel(int x,int y,unsigned& col)
{
	if(bytes_per_pixel() == 2){
		col = *(unsigned short*)(screenBuf + yTable[y] + x * 2);
	}
	if(bytes_per_pixel() == 3){
		unsigned char* p = (unsigned char*)(screenBuf + yTable[y] + x * 3);
		unsigned r = p[2];
		unsigned g = p[1];
		unsigned b = p[0];

		col = (r << 16) + (g << 8) + (b << 0);
	}
	if(bytes_per_pixel() == 4){
		col = *((unsigned*)(screenBuf + yTable[y] + x * 4));
	}
}

void grDispatcher::GetPixel(int x,int y,unsigned& r,unsigned& g,unsigned& b)
{
	if(bytes_per_pixel() == 2){
		unsigned col = *(unsigned short*)(screenBuf + yTable[y] + x * 2);
		if(pixel_format() == GR_RGB565)
			split_rgb565u(col,r,g,b);
		else 
			split_rgb555u(col,r,g,b);
	}
	if(bytes_per_pixel() == 3){
		unsigned char* p = (unsigned char*)(screenBuf + yTable[y] + x * 3);
		r = p[2];
		g = p[1];
		b = p[0];
	}
	if(bytes_per_pixel() == 4){
		unsigned char* p = (unsigned char*)(screenBuf + yTable[y] + x * 4);
		r = p[2];
		g = p[1];
		b = p[0];
	}
}

bool grDispatcher::clip_line(int& x0,int& y0,int& x1,int& y1) const
{
	int x, y;
	bool accept = false, done = false;

	int outcodeOut;
	int outcode0 = clip_out_code(x0,y0);
	int outcode1 = clip_out_code(x1,y1);

	do {
		if(outcode0 == 0 && outcode1 == 0) {
			accept = true; done = true;
		}
		else {
			if((outcode0 & outcode1) != 0)
				done = true;
			else {
				if(outcode0)
					outcodeOut = outcode0;
				else 
					outcodeOut = outcode1;

				if(clTOP & outcodeOut){
					x = x0 + (x1 - x0) * (clipCoords[3] - y0 - 1) / (y1 - y0);
					y = clipCoords[3] - 1;
				}
				else if(clBOTTOM & outcodeOut){
					x = x0 + (x1 - x0) * (clipCoords[1] - y0) / (y1 - y0);
					y = clipCoords[1];
				}
				if(clRIGHT & outcodeOut){
					y = y0 + (y1 - y0) * (clipCoords[2] - x0 - 1) / (x1 - x0);
					x = clipCoords[2] - 1;
				}
				else if(clLEFT & outcodeOut){
					y = y0 + (y1 - y0)*(clipCoords[0] - x0) / (x1 - x0);
					x = clipCoords[0];
				}

				if(outcodeOut = outcode0){
					x0 = x; 
					y0 = y; 

					outcode0 = clip_out_code(x,y);
				}
				else {
					x1 = x; 
					y1 = y; 

					outcode1 = clip_out_code(x,y);
				}
			}
		}
	}
	while(!done);

	return accept;
}

bool grDispatcher::create_window(int sx,int sy)
{
	SizeX = sx;
	SizeY = sy;

	int px = GetSystemMetrics(SM_CXSCREEN);
	int py = GetSystemMetrics(SM_CYSCREEN);

	int wx = sx + GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
	int wy = sy + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

	hWnd = CreateWindow(wnd_class_name(),"",WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN,(px - wx)/2,(py - wy)/2,wx,wy,NULL,NULL,GetModuleHandle(NULL),NULL);

	if(hWnd){
		ShowWindow((HWND)hWnd,SW_SHOWNORMAL);
		UpdateWindow((HWND)hWnd);
		return true;
	}

	return false;
}

bool grDispatcher::destroy_window()
{
	if(hWnd){
		ShowWindow((HWND)hWnd,SW_HIDE);
		DestroyWindow((HWND)hWnd);
		hWnd = NULL;
		return true;
	}

	return false;
}

bool grDispatcher::resize_window(int sx,int sy)
{
	if(!hWnd) return false;

	int px = GetSystemMetrics(SM_CXSCREEN);
	int py = GetSystemMetrics(SM_CYSCREEN);

	if(!is_in_fullscreen_mode()){
		LONG styles = GetWindowLong((HWND)hWnd,GWL_STYLE);
		LONG styles_new = styles | (WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX);

		if(styles != styles_new)
			SetWindowLong((HWND)hWnd,GWL_STYLE,styles_new);

		int wx = sx + GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
		int wy = sy + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

		if(wy <= py)
			SetWindowPos((HWND)hWnd,NULL,(px - wx)/2,(py - wy)/2,wx,wy,SWP_NOZORDER | SWP_NOCOPYBITS);
		else 
			SetWindowPos((HWND)hWnd,NULL,(px - wx)/2,-GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYFIXEDFRAME),wx,wy,SWP_NOZORDER | SWP_NOCOPYBITS);
	}
	else {
		LONG styles = GetWindowLong((HWND)hWnd,GWL_STYLE);
		LONG styles_new = styles & ~(WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX);

		if(styles != styles_new)
			SetWindowLong((HWND)hWnd,GWL_STYLE,styles_new);

		SetWindowPos((HWND)hWnd,NULL,0,0,sx,sy,SWP_NOZORDER | SWP_NOCOPYBITS);
	}

	return true;
}

bool grDispatcher::set_window(void* hwnd)
{
	RECT rc;
	if(GetClientRect((HWND)hwnd,&rc)){
		SizeX = rc.right - rc.left;
		SizeY = rc.bottom - rc.top;
		hWnd = hwnd;

		return true;
	}

	return false;
}

bool grDispatcher::clip_line(int& x0,int& y0,int& z0,int& x1,int& y1,int& z1) const
{
	int x,y,z;
	bool accept = false, done = false;

	int outcodeOut;
	int outcode0 = clip_out_code(x0,y0);
	int outcode1 = clip_out_code(x1,y1);

	do {
		if(outcode0 == 0 && outcode1 == 0) {
			accept = true; done = true;
		}
		else {
			if((outcode0 & outcode1) != 0)
				done = true;
			else {
				if(outcode0)
					outcodeOut = outcode0;
				else 
					outcodeOut = outcode1;

				if(clTOP & outcodeOut){
					x = x0 + (x1 - x0) * (clipCoords[3] - y0 - 1) / (y1 - y0);
					z = z0 + (z1 - z0) * (clipCoords[3] - y0 - 1) / (y1 - y0);
					y = clipCoords[3] - 1;
				}
				else if(clBOTTOM & outcodeOut){
					x = x0 + (x1 - x0) * (clipCoords[1] - y0) / (y1 - y0);
					z = z0 + (z1 - z0) * (clipCoords[1] - y0) / (y1 - y0);
					y = clipCoords[1];
				}
				if(clRIGHT & outcodeOut){
					y = y0 + (y1 - y0) * (clipCoords[2] - x0 - 1) / (x1 - x0);
					z = z0 + (z1 - z0) * (clipCoords[2] - x0 - 1) / (x1 - x0);
					x = clipCoords[2] - 1;
				}
				else if(clLEFT & outcodeOut){
					y = y0 + (y1 - y0)*(clipCoords[0] - x0) / (x1 - x0);
					z = z0 + (z1 - z0)*(clipCoords[0] - x0) / (x1 - x0);
					x = clipCoords[0];
				}

				if(outcodeOut = outcode0){
					x0 = x; 
					y0 = y;
					z0 = z;

					outcode0 = clip_out_code(x,y);
				}
				else {
					x1 = x; 
					y1 = y;
					z1 = z;

					outcode1 = clip_out_code(x,y);
				}
			}
		}
	}
	while(!done);

	return accept;
}

#ifdef _GR_ENABLE_ZBUFFER
void grDispatcher::Line_z(int x1,int y1,int z1,int x2,int y2,int z2,int col,int line_style)
{
	const F_PREC = 16;

	if(!clip_line(x1,y1,z1,x2,y2,z2)) return;

	if(x1 == x2 && y1 == y2){
		int z = (z1 < z2) ? z1 : z2;
		if(get_z(x1,y1) > z)
			SetPixelFast(x1,y1,col);
		return;
	}

	if(abs(x2 - x1) > abs(y2 - y1)){
		int a = x2 - x1;
		int x = x1;
		int y = (y1 << F_PREC) + (1 << (F_PREC - 1));
		int incr = 1;
		int k = ((y2 - y1) << F_PREC)/a;
		int v = 0;

		int z = z1 << F_PREC;
		int dz = ((z2 - z1) << F_PREC)/a;

		if(x1 > x2){
			incr = -1;
			k = -k;
			a = -a;
			dz = -dz;
		}

		do {
			if(++v > line_style){
				if(get_z(x,y >> F_PREC) >= (z >> F_PREC))
					SetPixelFast(x,y >> F_PREC,col);
				v = 0;
			}
			x += incr;
			y += k;
			z += dz;
		} while(--a >= 0);
	}
	else {
		int b = y2 - y1;
		int x = (x1 << F_PREC) + (1 << (F_PREC - 1));
		int y = y1;
		int incr = 1;
		int k = ((x2 - x1) << F_PREC)/b;
		int v = 0;

		int z = z1 << F_PREC;
		int dz = ((z2 - z1) << F_PREC)/b;

		if(y1 > y2){
			incr = -1;
			k = -k;
			b = -b;
			dz = -dz;
		}
		do {
			if(++v > line_style){
				if(get_z(x >> F_PREC,y) >= (z >> F_PREC))
					SetPixelFast(x >> F_PREC,y,col);
				v = 0;
			}
			y += incr;
			x += k;
			z += dz;
		} while(--b >= 0);
	}
}
#endif

bool grDispatcher::clip_rectangle(int& x,int& y,int& pic_x,int& pic_y,int& pic_sx,int& pic_sy) const
{
	if(x < clipCoords[0]){
		pic_x += clipCoords[0] - x;
		pic_sx += x - clipCoords[0];

		x = clipCoords[0];
	}
	if(x + pic_sx >= clipCoords[2])
		pic_sx += clipCoords[2] - (x + pic_sx);
//		pic_sx += clipCoords[2] - 1 - (x + pic_sx);

	if(y < clipCoords[1]){
		pic_y += clipCoords[1] - y;
		pic_sy += y - clipCoords[1];

		y = clipCoords[1];
	}
	if(y + pic_sy >= clipCoords[3])
		pic_sy += clipCoords[3] - (y + pic_sy);
//		pic_sy += clipCoords[3] - 1 - (y + pic_sy);

	if(pic_x >= 0 && pic_y >= 0 && pic_sx > 0 && pic_sy > 0)
		return true;

	return false;
}

#ifdef _GR_ENABLE_ZBUFFER
bool grDispatcher::alloc_zbuffer(int sx,int sy)
{
	if(zbuffer_) delete zbuffer_;
	zbuffer_ = new zbuf_t[sx * sy];
	return true;
}

bool grDispatcher::free_zbuffer()
{
	if(zbuffer_) delete zbuffer_;
	zbuffer_ = 0;

	return true;
}

bool grDispatcher::clear_zbuffer()
{
	zbuf_t* p = zbuffer_;
	for(int i = 0; i < SizeY; i ++){
		for(int j = 0; j < SizeX; j ++){
			*p ++ = GR_ZBUFFER_MAX_Z;
		}
	}
	return true;
}
#endif

void grDispatcher::clear_changes_mask()
{
	std::fill(changes_mask_.begin(),changes_mask_.end(),0);
}

void grDispatcher::build_changed_regions()
{
	changed_regions_.clear();

	bool flag = true;

	while(flag){
		flag = false;

		changes_mask_t::iterator it = std::find(changes_mask_.begin(),changes_mask_.end(),1);
		if(it != changes_mask_.end()){
			int x = (it - changes_mask_.begin()) % changes_mask_size_x_;
			int y = (it - changes_mask_.begin()) / changes_mask_size_x_;

			changes_mask_t::iterator it1 = std::find(it,it + (changes_mask_size_x_ - x),0);

			int sx = it1 - it;
			int sy = 0;

			for(int i = 0; i < changes_mask_size_y_ - y; i++,sy++){
				changes_mask_t::iterator it2 = std::find(it,it1,0);
				if(it2 == it1)
					std::fill(it,it1,0);
				else
					break;

				if(i < changes_mask_size_y_ - y - 1){
					it += changes_mask_size_x_;
					it1 += changes_mask_size_x_;
				}
			}

			x <<= changes_mask_tile_shift_;
			y <<= changes_mask_tile_shift_;

			sx <<= changes_mask_tile_shift_;
			sy <<= changes_mask_tile_shift_;

			changed_regions_.push_back(grScreenRegion(x + sx/2,y + sy/2,sx,sy));

			flag = true;
		}
	}
}

bool grDispatcher::invalidate_region(const grScreenRegion& reg)
{
	int x = reg.min_x();
	int y = reg.min_y();

	int sx = reg.size_x();
	int sy = reg.size_y();

	if(clip_rectangle(x,y,sx,sy)){
		sx = ((x + sx) >> changes_mask_tile_shift_) - (x >> changes_mask_tile_shift_) + 1;
		sy = ((y + sy) >> changes_mask_tile_shift_) - (y >> changes_mask_tile_shift_) + 1;

		x >>= changes_mask_tile_shift_;
		y >>= changes_mask_tile_shift_;

		if(x + sx > changes_mask_size_x_) sx = changes_mask_size_x_ - x;
		if(y + sy > changes_mask_size_y_) sy = changes_mask_size_y_ - y;

		if(sx <= 0 || sy <= 0) return false;

		changes_mask_t::iterator it = changes_mask_.begin() + (x + y * changes_mask_size_x_);

		for(int i = 0; i < sy; i++){
			std::fill_n(it,sx,true);
			if(i < sy - 1)
				it += changes_mask_size_x_;
		}
	}

	return true;
}

bool grDispatcher::FlushChanges()
{
	for(regions_container_t::const_iterator it = changed_regions_.begin(); it != changed_regions_.end(); ++it)
		Flush(it -> min_x() - 1,it -> min_y() - 1,it -> size_x() + 2,it -> size_y() + 2);

	return true;
}

bool grDispatcher::convert_sprite(grPixelFormat src_fmt,grPixelFormat& dest_fmt,int sx,int sy,unsigned char* data,bool& has_alpha)
{
	if(dest_fmt == GR_RGB888 && (src_fmt == GR_ARGB1555 || src_fmt == GR_RGB565)){
		if(has_alpha)
			dest_fmt = GR_ARGB8888;
	}
	else if(dest_fmt == GR_ARGB8888 && (src_fmt == GR_ARGB1555 || src_fmt == GR_RGB565)){
		if(!has_alpha)
			dest_fmt = GR_RGB888;
	}

	switch(dest_fmt){
	case GR_ARGB1555:
		if(src_fmt == GR_RGB565){
			unsigned short* p = reinterpret_cast<unsigned short*>(data);
			for(int i = 0; i < sx * sy; i ++){
				unsigned r,g,b;
				split_rgb565u(*p,r,g,b);
				*p++ = make_rgb555u(r,g,b);
				if(has_alpha) p++;
			}
		}
		else if(src_fmt == GR_RGB888){
			unsigned char* dp = data;
			unsigned short* p = reinterpret_cast<unsigned short*>(data);
			for(int i = 0; i < sx * sy; i ++){
				*p++ = make_rgb555u(dp[2],dp[1],dp[0]);
				dp += 3;
			}
			has_alpha = false;
		}
		else if(src_fmt == GR_ARGB8888){
			unsigned char* dp = data;
			unsigned short* p = reinterpret_cast<unsigned short*>(data);
			for(int i = 0; i < sx * sy; i ++){
				*p++ = make_rgb555u(dp[2],dp[1],dp[0]);
				*p++ >>= 8;
				dp += 4;
			}
			has_alpha = true;
		}
		return true;
	case GR_RGB565:
		if(src_fmt == GR_ARGB1555){
			unsigned short* p = reinterpret_cast<unsigned short*>(data);
			for(int i = 0; i < sx * sy; i ++){
				unsigned r,g,b;
				split_rgb555u(*p,r,g,b);
				*p++ = make_rgb565u(r,g,b);

				if(has_alpha) p++;
			}
		}
		else if(src_fmt == GR_RGB888){
			unsigned char* dp = data;
			unsigned short* p = reinterpret_cast<unsigned short*>(data);
			for(int i = 0; i < sx * sy; i ++){
				*p++ = make_rgb565u(dp[2],dp[1],dp[0]);
				dp += 3;
			}
			has_alpha = false;
		}
		else if(src_fmt == GR_ARGB8888){
			unsigned char* dp = data;
			unsigned short* p = reinterpret_cast<unsigned short*>(data);
			for(int i = 0; i < sx * sy; i ++){
				*p++ = make_rgb565u(dp[2],dp[1],dp[0]);
				*p++ >>= 8;
				dp += 4;
			}
			has_alpha = true;
		}
		return true;
	case GR_RGB888:
		if(src_fmt == GR_ARGB1555 || src_fmt == GR_RGB565){
			unsigned short* p = reinterpret_cast<unsigned short*>(data) + sx * sy - 1;
			unsigned char* dp = data + sx * sy * 3 - 1;
			
			for(int i = 0; i < sx * sy; i++){
				unsigned r,g,b;
				
				if(src_fmt == GR_ARGB1555)
					split_rgb555u(*p--,r,g,b);
				else
					split_rgb565u(*p--,r,g,b);

				*dp-- = r;
				*dp-- = g;
				*dp-- = b;
			}
			return true;
		}
		break;
	case GR_ARGB8888:
		if(src_fmt == GR_ARGB1555 || src_fmt == GR_RGB565){
			unsigned short* p = reinterpret_cast<unsigned short*>(data);
			for(int i = 0; i < sx * sy; i ++){
				p++;
				*p++ <<= 8;
			}

			p = reinterpret_cast<unsigned short*>(data);
			unsigned char* dp = data;
			for(int i = 0; i < sx * sy; i ++){
				unsigned r,g,b;

				if(src_fmt == GR_ARGB1555)
					split_rgb555u(*p++,r,g,b);
				else
					split_rgb565u(*p++,r,g,b);

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

grFont* grDispatcher::load_font(const char* file_name)
{
	grFont* p = new grFont;

	if(!p->load(file_name)){
		delete p;
		p = 0;
	}

	return p;
}

char* grDispatcher::temp_buffer(int size)
{
	if(size <= 0) size = 1;

	if(size > temp_buffer_size_){
		delete temp_buffer_;
		temp_buffer_ = new char[size];
		temp_buffer_size_ = size;
	}

	return temp_buffer_;
}

bool grDispatcher::DrawText(int x,int y,unsigned color,const char* str,int hspace,int vspace,const grFont* font)
{
	if(!font)
		font = default_font_;

	if(!font || !font -> alpha_buffer())
		return false;

	const unsigned char* str_buf = reinterpret_cast<const unsigned char*>(str);

	int x0 = x;
	int sz = strlen(str);

	for(int i = 0; i < sz; i ++){
		if(str_buf[i] != '\n'){
			const grScreenRegion& rg = font -> find_char(str_buf[i]);
			int dx = rg.size_x();

			if(str_buf[i] == ' ')
				x += font -> size_x()/2 + hspace;
			else if(dx){
				PutChar(x,y,color,font -> alpha_buffer_size_x(),font -> alpha_buffer_size_y(),font -> alpha_buffer(),rg);
				x += dx + hspace;
			}
		}
		else {
			x = x0;
			y += font -> size_y() + vspace;
		}
	}

	return true;
}

bool grDispatcher::DrawAlignedText(int x,int y,int sx,int sy,unsigned color,const char* str,grTextAlign align,int hspace,int vspace,const grFont* font)
{
	if(!font)
		font = default_font_;

	if(!font || !font -> alpha_buffer())
		return false;

	const unsigned char* str_buf = reinterpret_cast<const unsigned char*>(str);

	if(!sx)
		sx = TextWidth(str,hspace,font);

	int x0 = x;
	int delta_x = 0;
	int sz = strlen(str);

	switch(align){
		case GR_ALIGN_CENTER:
			delta_x = (sx - TextWidth(str,hspace,font,true))/2;
			break;
		case GR_ALIGN_RIGHT:
			delta_x = sx - TextWidth(str,hspace,font,true);
			break;
	}

	for(int i = 0; i < sz; i ++){
		if(str_buf[i] != '\n'){
			const grScreenRegion& rg = font -> find_char(str_buf[i]);
			int dx = rg.size_x();

			if(str_buf[i] == ' '){
				x += font -> size_x()/2 + hspace;
			}
			else if(dx){
				PutChar(x + delta_x,y,color,font -> alpha_buffer_size_x(),font -> alpha_buffer_size_y(),font -> alpha_buffer(),rg);
				x += dx + hspace;
			}
		}
		else {
			x = x0;
			y += font -> size_y() + vspace;

			switch(align){
				case GR_ALIGN_CENTER:
					delta_x = (sx - TextWidth(str + i + 1,hspace,font,true))/2;
					break;
				case GR_ALIGN_RIGHT:
					delta_x = sx - TextWidth(str + i + 1,hspace,font,true);
					break;
			}
		}
	}

	return true;
}

bool grDispatcher::DrawParsedText(int x,int y,int sx,int sy,unsigned color,const UI_TextParser* parser,grTextAlign align,const grFont* font)
{
	if(!font)
		font = default_font_;

	int hSize = parser->fontHeight();
	y -= hSize;

	bool skipToNextLine = false;
	int cur_x = x;
	for(OutNodes::const_iterator it = parser->outNodes().begin(); it != parser->outNodes().end(); ++it){
		switch(it->type){
			case OutNode::NEW_LINE:
				skipToNextLine = false;
				y += hSize;
				switch(align){
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
						xxassert(false, ":\\");
				}
				break;

			case OutNode::TEXT: {
				if(skipToNextLine)
					break;
					
				std::string str;
				str.assign(it->begin, it->end - it->begin);
				DrawText(cur_x, y, color, str.c_str(), 0, 0, font);

				int ssx = TextWidth(str.c_str(), 0, font);
				int ssx1 = it->width;
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

int grDispatcher::TextWidth(const char* str,int hspace,const grFont* font,bool first_string_only) const
{
	if(!font)
		font = default_font_;

	if(!font)
		return false;

	const unsigned char* str_buf = (const unsigned char*)str;

	int sx = 0,sx_max = 0;
	int sz = strlen(str);
	for(int i = 0; i < sz; i ++){
		if(str_buf[i] != '\n'){
			const grScreenRegion& rg = font -> find_char(str_buf[i]);
			int dx = rg.size_x();

			if(str_buf[i] == ' ')
				sx += font -> size_x()/2;
			else
				sx += dx + hspace;
		}
		else {
			if(first_string_only)
				return sx;

			if(sx_max < sx) sx_max = sx;
			sx = 0;
		}
	}

	if(sx_max < sx) sx_max = sx;

	return sx_max;
}

int grDispatcher::TextHeight(const char* str,int vspace,const grFont* font) const
{
	if(!font)
		font = default_font_;

	if(!font)
		return false;

	int sy = font -> size_y() + vspace;

	for(int i = 0; i < strlen(str); i ++){
		if(str[i] == '\n')
			sy += font -> size_y() + vspace;
	}

	return sy;
}

