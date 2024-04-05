/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "ddraw_gr_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

HRESULT CALLBACK ddEnumDisplayModesCallback(LPDDSURFACEDESC lpDDSurfaceDesc,LPVOID lpContext);

/* --------------------------- DEFINITION SECTION --------------------------- */

DDraw_grDispatcher::DDraw_grDispatcher() : ddobj_(0),
	prim_surface_(0),
	back_surface_(0),
	fullscreen_(false)
{
	DDSURFACEDESC ddSurf;
	ZeroMemory(&ddSurf,sizeof(DDSURFACEDESC));
	ddSurf.dwSize = sizeof(ddSurf);
	ddSurf.dwFlags = 0;

	if(FAILED(DirectDrawCreate(NULL,&ddobj_,NULL))) return;

	video_modes_.reserve(50);
	enum_display_modes();
}

DDraw_grDispatcher::~DDraw_grDispatcher()
{
	if(ddobj_){
		ddobj_ -> Release();
		ddobj_ = NULL;
	}

	video_modes_.clear();
}

bool DDraw_grDispatcher::Flush(int x,int y,int sx,int sy)
{
	if(flags & GR_INITED && hWnd){
		int x1 = x + sx;
		int y1 = y + sy;

		if(x < 0) x = 0;
		if(y < 0) y = 0;
		if(x1 > SizeX) x1 = SizeX;
		if(y1 > SizeY) y1 = SizeY;

		RECT src_rect = { x,y,x1,y1 };
		RECT dest_rect = src_rect;

		if(!fullscreen_){
			POINT pt = { 0,0 };
			ClientToScreen((HWND)Get_hWnd(),&pt);
			OffsetRect(&dest_rect,pt.x,pt.y);
		}

//		if(prim_surface_ -> IsLost() != DD_OK) prim_surface_ -> Restore();
		back_surface_ -> Unlock(&back_surface_obj_);

		while(TRUE){
			HRESULT hr = prim_surface_ -> Blt(&dest_rect,back_surface_,&src_rect,DDBLT_WAIT,NULL);
			if(hr == DD_OK)
				break;
			if(hr == DDERR_SURFACELOST){
				hr = prim_surface_ -> Restore();
				if(hr != DD_OK)
					break;
			}
			if(hr != DDERR_WASSTILLDRAWING)
				break;
		}

		back_surface_ -> Lock(NULL,&back_surface_obj_,DDLOCK_SURFACEMEMORYPTR,NULL);
		screenBuf = static_cast<char*>(back_surface_obj_.lpSurface);
		for(int i = 0; i < SizeY; i ++)
			yTable[i] = i * back_surface_obj_.lPitch;

		return true;
	}

	return false;
}

bool DDraw_grDispatcher::StretchFlush(int x_dest,int y_dest,int sx_dest,int sy_dest,int x_src,int y_src,int sx_src,int sy_src)
{
	if(flags & GR_INITED && hWnd){
		RECT src = { x_src,y_src,x_src + sx_src,y_src + sy_src };
		RECT dst = { x_dest,y_dest,x_dest + sx_dest,y_dest + sy_dest };

		if(!fullscreen_){
			POINT pt = { 0,0 };
			ClientToScreen((HWND)Get_hWnd(),&pt);
			OffsetRect(&dst,pt.x,pt.y);
		}

		if(prim_surface_ -> IsLost() != DD_OK) prim_surface_ -> Restore();
		back_surface_ -> Unlock(&back_surface_obj_);

		while(back_surface_ -> GetBltStatus(DDGBS_ISBLTDONE) == DDERR_WASSTILLDRAWING);
		prim_surface_ -> Blt(&dst,back_surface_,&src,DDBLT_WAIT,NULL);

		back_surface_ -> Lock(NULL,&back_surface_obj_,DDLOCK_SURFACEMEMORYPTR,NULL);
		screenBuf = reinterpret_cast<char*>(back_surface_obj_.lpSurface);
		for(int i = 0; i < SizeY; i ++)
			yTable[i] = i * back_surface_obj_.lPitch;

		return true;
	}

	return false;
}

bool DDraw_grDispatcher::init(int sx,int sy,grPixelFormat pixel_format,void* hwnd,bool fullscreen)
{
	if(!ddobj_) return false;

	fullscreen_ = fullscreen;
	grDispatcher::init(sx,sy,pixel_format,hwnd,fullscreen);

	if(fullscreen_){
		if(ddobj_ -> SetCooperativeLevel((HWND)Get_hWnd(),DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) != DD_OK)
			return false;
		if(ddobj_ -> SetDisplayMode(sx,sy,bytes_per_pixel() * 8) != DD_OK)
			return false;
	}
	else {
		if(ddobj_ -> SetCooperativeLevel((HWND)Get_hWnd(),DDSCL_NORMAL) != DD_OK)
			return false;
	}

	if(!hwnd && sx && sy){
		resize_window(sx,sy);
		SizeX = sx;
		SizeY = sy;
	}

	DDSURFACEDESC ddSurf;
	ZeroMemory(&ddSurf,sizeof(DDSURFACEDESC));
	ddSurf.dwSize = sizeof(ddSurf);
	ddSurf.dwFlags = DDSD_CAPS;
	ddSurf.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if(ddobj_ -> CreateSurface(&ddSurf,&prim_surface_,NULL) != DD_OK) return false;

	if(!fullscreen_){
		LPDIRECTDRAWCLIPPER cp = NULL;
		if(ddobj_ -> CreateClipper(0,&cp,NULL) != DD_OK) return false;
		cp -> SetHWnd(0,(HWND)Get_hWnd());
		prim_surface_ -> SetClipper(cp);
		cp -> Release();
	}

	ZeroMemory(&ddSurf,sizeof(DDSURFACEDESC));
	ddSurf.dwSize = sizeof(ddSurf);
	ddobj_ -> GetDisplayMode(&ddSurf);

	ddSurf.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddSurf.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER | DDSCAPS_SYSTEMMEMORY;
	ddSurf.dwWidth = SizeX;
	ddSurf.dwHeight = SizeY;

	ddSurf.dwFlags |= DDSD_PIXELFORMAT;

	if(ddobj_ -> CreateSurface(&ddSurf,&back_surface_,NULL) != DD_OK) return false;

	switch(ddSurf.ddpfPixelFormat.dwRGBBitCount){
		case 16:
			if(ddSurf.ddpfPixelFormat.dwGBitMask == grDispatcher::mask_565_g)
				pixel_format_ = GR_RGB565;
			else
				pixel_format_ = GR_ARGB1555;
			break;
		case 24:
			pixel_format_ = GR_RGB888;
			break;
		case 32:
			pixel_format_ = GR_ARGB8888;
			break;
		default:
			return false;
	}

	ZeroMemory(&back_surface_obj_,sizeof(DDSURFACEDESC));
	back_surface_obj_.dwSize = sizeof(DDSURFACEDESC);
	back_surface_obj_.dwFlags = DDSD_PITCH;
	if(back_surface_ -> Lock(NULL,&back_surface_obj_,DDLOCK_SURFACEMEMORYPTR,NULL) != DD_OK) return false;
	screenBuf = reinterpret_cast<char*>(back_surface_obj_.lpSurface);

	delete yTable;
	yTable = new int[SizeY + 1];
	for(int i = 0; i < SizeY; i ++)
		yTable[i] = i * back_surface_obj_.lPitch;

	SetClip();

	flags |= GR_INITED;

	return true;
}

bool DDraw_grDispatcher::Finit()
{
	grDispatcher::Finit();

	if(back_surface_){
		while(back_surface_ -> GetBltStatus(DDGBS_ISBLTDONE) == DDERR_WASSTILLDRAWING);
		back_surface_ -> Unlock(&back_surface_obj_);
		ddobj_ -> SetCooperativeLevel((HWND)Get_hWnd(),DDSCL_NORMAL);
		if(fullscreen_ && ddobj_) ddobj_ -> RestoreDisplayMode();
	}
	if(prim_surface_){
		prim_surface_ -> Release();
		prim_surface_ = NULL;
	}
	if(back_surface_){
		back_surface_ -> Release();
		back_surface_ = NULL;
	}

	return true;
}

bool DDraw_grDispatcher::enum_display_modes()
{
	if(!ddobj_) return false;

	video_modes_.clear();

	if(FAILED(ddobj_ -> EnumDisplayModes(DDEDM_STANDARDVGAMODES,NULL,(LPVOID)(&video_modes_),ddEnumDisplayModesCallback))) 
		return false;

	return true;
}

HRESULT CALLBACK ddEnumDisplayModesCallback(LPDDSURFACEDESC lpDDSurfaceDesc,LPVOID lpContext)
{
	grPixelFormat pixel_format;
	switch(lpDDSurfaceDesc -> ddpfPixelFormat.dwRGBBitCount){
		case 16:
			if(lpDDSurfaceDesc -> ddpfPixelFormat.dwGBitMask == grDispatcher::mask_565_g)
				pixel_format = GR_RGB565;
			else
				pixel_format = GR_ARGB1555;
			break;
		case 24:
			pixel_format = GR_RGB888;
			break;
		case 32:
			pixel_format = GR_ARGB8888;
			break;
		default:
			return DDENUMRET_OK;
	}

	DDraw_VideoMode vid_mode(lpDDSurfaceDesc -> dwWidth,lpDDSurfaceDesc -> dwHeight,pixel_format);
	DDraw_VideoModeVector* modes = reinterpret_cast<DDraw_VideoModeVector*>(lpContext);
	modes -> push_back(vid_mode);

	return DDENUMRET_OK;
}

bool DDraw_grDispatcher::is_mode_supported(int sx,int sy,grPixelFormat pixel_format) const 
{
	for(int i = 0; i < video_modes_.size(); i ++){
		if(video_modes_[i].screen_sx() == sx && video_modes_[i].screen_sy() == sy && video_modes_[i].pixel_format() == pixel_format)
			return true;
	}

	return false;
}

bool DDraw_grDispatcher::is_mode_supported(grPixelFormat pixel_format) const 
{
	for(int i = 0; i < video_modes_.size(); i ++){
		if(video_modes_[i].pixel_format() == pixel_format)
			return true;
	}

	return false;
}


bool DDraw_grDispatcher::get_current_mode(int& sx,int& sy,grPixelFormat& pixel_format) const
{
	DDSURFACEDESC ddsd;
	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	HRESULT hRet = ddobj_ -> GetDisplayMode(&ddsd);
	if(hRet != DD_OK)
		return false;

	sx = ddsd.dwWidth;
	sy = abs(float(ddsd.dwHeight));

	switch(ddsd.ddpfPixelFormat.dwRGBBitCount){
		case 16:
			if(ddsd.ddpfPixelFormat.dwGBitMask == grDispatcher::mask_565_g)
				pixel_format = GR_RGB565;
			else
				pixel_format = GR_ARGB1555;
			break;
		case 24:
			pixel_format = GR_RGB888;
			break;
		case 32:
			pixel_format = GR_ARGB8888;
			break;
		default:
			return false;
	}

	return true;
}

