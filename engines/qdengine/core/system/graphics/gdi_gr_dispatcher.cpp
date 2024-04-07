/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "gdi_gr_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

GDI_grDispatcher::GDI_grDispatcher() : dibPtr(0),
	dibHandle(0)
{
	palette_mode_ = false;

	sys_palette_ = NULL;
	sys_pal_handle_ = NULL;
	palette_ = NULL;
	pal_handle_ = NULL;
}

GDI_grDispatcher::~GDI_grDispatcher()
{
}

bool GDI_grDispatcher::init(int sx,int sy,grPixelFormat pixel_format,void* hwnd,bool fullscreen)
{
	grDispatcher::init(sx,sy,pixel_format,hwnd,fullscreen);

	HDC hdc = GetDC(NULL);

	char* ptr;
	BITMAPINFO* bi;

	switch(pixel_format_){
		case GR_RGB565:
			ptr = new char[sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 3];
			memset(ptr,0,sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 3);
			bi = (BITMAPINFO*)ptr;
			bi -> bmiHeader.biCompression = BI_BITFIELDS;

			*(DWORD*)&(bi -> bmiColors[0]) = mask_565_r;
			*(DWORD*)&(bi -> bmiColors[1]) = mask_565_g;
			*(DWORD*)&(bi -> bmiColors[2]) = mask_565_b;
			break;
		case GR_ARGB1555:
			ptr = new char[sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 3];
			memset(ptr,0,sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 3);
			bi = (BITMAPINFO*)ptr;
			bi -> bmiHeader.biCompression = BI_BITFIELDS;

			*(DWORD*)&(bi -> bmiColors[0]) = mask_555_r;
			*(DWORD*)&(bi -> bmiColors[1]) = mask_555_g;
			*(DWORD*)&(bi -> bmiColors[2]) = mask_555_b;
			break;
		case GR_RGB888:
		case GR_ARGB8888:
			ptr = new char[sizeof(BITMAPINFO)];
			memset(ptr,0,sizeof(BITMAPINFO));
			bi = (BITMAPINFO*)ptr;
			bi -> bmiHeader.biCompression = BI_RGB;
			break;
	}

	dibPtr = ptr;

	int dsx = SizeX * bytes_per_pixel();
	if(dsx % 4) dsx += 4 - (dsx % 4);

	bi -> bmiHeader.biBitCount = bytes_per_pixel() * 8;
	bi -> bmiHeader.biClrUsed = 0;

	bi -> bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi -> bmiHeader.biWidth = SizeX;
	bi -> bmiHeader.biHeight = -SizeY;
	bi -> bmiHeader.biPlanes = 1;
	bi -> bmiHeader.biSizeImage = dsx * SizeY;
	bi -> bmiHeader.biXPelsPerMeter = SizeX;
	bi -> bmiHeader.biYPelsPerMeter = SizeY;
	bi -> bmiHeader.biClrImportant = 0;

	dibHandle = CreateDIBSection(hdc,bi,DIB_RGB_COLORS,(void**)&(screenBuf),NULL,NULL);

	delete yTable;
	yTable = new int[SizeY + 1];
	for(int i = 0; i < SizeY; i ++)
		yTable[i] = i * dsx;

	int caps = GetDeviceCaps(hdc,RASTERCAPS);

	if(caps & RC_PALETTE){
		palette_mode_ = true;

		char* ptr = new char[sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256];
		memset(ptr,0,sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256);

		sys_palette_ = (LOGPALETTE*)ptr;

		sys_palette_ -> palVersion = 0x300;
		sys_palette_ -> palNumEntries = 256;

		sys_pal_handle_ = CreatePalette(sys_palette_);

		ptr = new char[sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256];
		memset(ptr,0,sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256);

		palette_ = (LOGPALETTE*)ptr;

		palette_ -> palVersion = 0x300;
		palette_ -> palNumEntries = 256;

		pal_handle_ = CreatePalette(palette_);
		GetSystemPaletteEntries(hdc,0,256,palette_ -> palPalEntry);

		for(int i = 0; i < 256; i ++){
			palette_ -> palPalEntry[i].peRed = (unsigned)(i);
			palette_ -> palPalEntry[i].peGreen = (unsigned)(i);
			palette_ -> palPalEntry[i].peBlue = (unsigned)(i);
			palette_ -> palPalEntry[i].peFlags = PC_NOCOLLAPSE;
		}

		SetPaletteEntries(pal_handle_,0,256,palette_ -> palPalEntry);

		GetSystemPaletteEntries(hdc,0,256,sys_palette_ -> palPalEntry);
		SetPaletteEntries(sys_pal_handle_,0,256,sys_palette_ -> palPalEntry);
	}

	ReleaseDC(NULL,hdc);

	SetClip();

	flags |= GR_INITED;

	return true;
}

bool GDI_grDispatcher::Flush(int x,int y,int sx,int sy)
{
	if(flags & GR_INITED && hWnd){
		int x_src = wndPosX + x;
		int y_src = wndPosY + y;

		HDC hdc = GetDC((HWND)hWnd);

		if(palette_mode_){
			SelectPalette(hdc,pal_handle_,1);
			RealizePalette(hdc);
		}

		HDC hbmp = CreateCompatibleDC(hdc);
		HGDIOBJ holdbmp = SelectObject(hbmp,dibHandle);

		BitBlt(hdc,x,y,sx,sy,hbmp,x_src,y_src,SRCCOPY);
		SelectObject(hbmp,holdbmp);
		DeleteDC(hbmp);

		ReleaseDC((HWND)hWnd,hdc);

		return true;
	}

	return false;
}

bool GDI_grDispatcher::StretchFlush(int x_dest,int y_dest,int sx_dest,int sy_dest,int x_src,int y_src,int sx_src,int sy_src)
{
	if(flags & GR_INITED && hWnd){
		HDC hdc = GetDC((HWND)hWnd);

		if(palette_mode_){
			SelectPalette(hdc,pal_handle_,1);
			RealizePalette(hdc);
		}

		HDC hbmp = CreateCompatibleDC(hdc);
		HGDIOBJ holdbmp = SelectObject(hbmp,dibHandle);

		StretchBlt(hdc,x_dest,y_dest,sx_dest,sy_dest,hbmp,x_src,y_src,sx_src,sy_src,SRCCOPY);
		SelectObject(hbmp,holdbmp);
		DeleteDC(hbmp);

		ReleaseDC((HWND)hWnd,hdc);

		return true;
	}

	return false;
}

bool GDI_grDispatcher::Finit()
{
	grDispatcher::Finit();

	if(dibHandle)
		DeleteObject(dibHandle);

	if(dibPtr){
		char* p = (char*)dibPtr;
		delete p;
	}

	dibPtr = NULL;
	dibHandle = NULL;

	if(palette_mode_){
		HDC hdc = GetDC(NULL);
		SelectPalette(hdc,sys_pal_handle_,1);
		RealizePalette(hdc);
		ReleaseDC(NULL,hdc);

		delete sys_palette_;
		sys_palette_ = NULL;
		delete palette_;
		palette_ = NULL;

		sys_pal_handle_ = NULL;
		pal_handle_ = NULL;
		
		palette_mode_ = false;
	}

	return true;
}

bool GDI_grDispatcher::set_palette(const char* pal,int start_col,int pal_size)
{
	if(!palette_mode_)
		return false;

	for(int i = 0; i < pal_size; i ++){
		palette_ -> palPalEntry[start_col + i].peRed = (unsigned)(pal[i * 3 + 0]) * 4;
		palette_ -> palPalEntry[start_col + i].peGreen = (unsigned)(pal[i * 3 + 1]) * 4;
		palette_ -> palPalEntry[start_col + i].peBlue = (unsigned)(pal[i * 3 + 2]) * 4;
		palette_ -> palPalEntry[start_col + i].peFlags = PC_NOCOLLAPSE;
	}

	SetPaletteEntries(pal_handle_,start_col,pal_size,palette_ -> palPalEntry);
	return true;
}
