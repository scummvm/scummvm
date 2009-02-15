#ifndef __cplusplus
#error NOTE: This file MUST be compiled as C++. In Visual C++, use the /Tp command line option.
#endif

// for WINNT 4.0 (only DirectDraw 3)
#ifdef HAVE_DDRAW

#define   DIRECTDRAW_VERSION 0x0300

#define INITGUID

#include <windows.h>
#include <ddraw.h>
#include <stdarg.h>

extern "C" {
#include <gfx_system.h>
#include <gfx_driver.h>
#include <gfx_tools.h>
#include <assert.h>
#include <uinput.h>
#include <ctype.h>
#include <console.h> // for sciprintf
#include <sci_memory.h>
};

#include "dd_driver.h"

#define DD_DRIVER_VERSION "0.1"

#define GFX_DD_DEBUG


#ifdef GFX_DD_DEBUG
#define POSMSG sciprintf("%s L%d:", __FILE__, __LINE__)
#define DEBUG_PTR (!(drv->debug_flags & GFX_DEBUG_POINTER))? 0 : POSMSG && sciprintf
#define DEBUG_UPDATES (!(drv->debug_flags & GFX_DEBUG_UPDATES))? 0 : POSMSG && sciprintf
#define DEBUG_PIXMAPS (!(drv->debug_flags & GFX_DEBUG_PIXMAPS))? 0 : POSMSG && sciprintf
#define DEBUG_BASIC (!(drv->debug_flags & GFX_DEBUG_BASIC))? 0 : POSMSG , sciprintf
#else /* !GFX_DD_DEBUG */
#define DEBUG_PTR (1)? 0 :
#define DEBUG_UPDATES (1)? 0 :
#define DEBUG_PIXMAPS (1)? 0 :
#define DEBUG_BASIC (1)? 0 :
#endif /* !GFX_DD_DEBUG */

BOOL	g_bFullScreen = FALSE;

#define DD_BUFFER_BACK		0
#define DD_BUFFER_STATIC	1

static long FAR PASCAL WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static int process_messages(void);

struct gfx_dd_struct_t
{
	LPDIRECTDRAW	pDD;
	HWND hMainWnd;
	LPDIRECTDRAWSURFACE pPrimary;
	LPDIRECTDRAWSURFACE pBack;
	LPDIRECTDRAWSURFACE pStatic;
	LPDIRECTDRAWCLIPPER pClipper; // used in windowed (not fullscreen) mode
	int		win_xpos,win_ypos;	// current position of the window (windowed mode only)
	BOOL		bShowMouse;
	gfx_pixmap_t *priority_maps[2];
	// event queue
	int queue_size,queue_first,queue_last;
	sci_event_t * event_queue;
};

static void init_event_queue(gfx_dd_struct_t *ctx);
static void free_event_queue(gfx_dd_struct_t *ctx);

static __inline DWORD MakeRGB(gfx_mode_t *mode,unsigned int r,unsigned int g,unsigned int b)
{
	return ((r << mode->red_shift) & mode->red_mask) |
			((g << mode->green_shift) & mode->green_mask) |
			((b << mode->blue_shift) & mode->blue_mask);
};

static int
dd_set_param(gfx_driver_t *drv, char *attribute, char *value)
{
	DEBUG_BASIC("dd_set_param('%s' to '%s')\n", attribute, value);
	return GFX_OK;
}

static int
ShiftCount(DWORD mask)
{
	int cnt;

	if(mask==0)
		return 0; // error !!!

	cnt=0;

	while((mask & 1)==0)
	{
		mask >>=1;
		cnt++;
	}
	return cnt;
}

static int
dd_init_specific(gfx_driver_t *drv, int xres, int yres, int bpp)
{
	DDSURFACEDESC dd_desc,ddsd;
	HRESULT hr;
	WNDCLASS wc;
	RECT rc;
	POINT pnt;
	gfx_dd_struct_t *ctx;
	int dd_bpp,r_sh,g_sh,b_sh,dd_bpp2,tbytes;
//	const ggi_pixelformat *pixelformat;
	int frames = 3;

	// force ....
	xres=1; yres=1; bpp=1;

	if(g_bFullScreen)
	{
		// force the best mode
		xres=1; yres=1; bpp=1;
	}
	switch(bpp) // bpp of 8,16,24,32
	{
		case 1:
			dd_bpp=8; break;
		case 2:
			dd_bpp=16; break;
		case 3:
			dd_bpp=24; break;
		case 4:
			dd_bpp=32; break;
		default:
			sciprintf("GFXDD: Error: Invalid bytes per pixel value: %d\n", bpp);
			return GFX_ERROR;
	}

	drv->state = NULL;

	ctx = (struct gfx_dd_struct_t *) sci_malloc(sizeof(gfx_dd_struct_t));
	if(ctx == NULL)
		return GFX_FATAL;

	memset(ctx,0,sizeof(gfx_dd_struct_t));

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void *);
	wc.hInstance     = NULL;
	wc.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
	wc.hCursor       = NULL; /*LoadCursor (NULL, IDC_ARROW)*/;
	wc.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "freesci.WndClass";
	RegisterClass (&wc);

	SetRect (&rc, 0, 0, xres*320-1, yres*200-1);
	if(!g_bFullScreen)
	{
		AdjustWindowRectEx (&rc, WS_OVERLAPPEDWINDOW, FALSE, 0);
	}

	init_event_queue(ctx);

	ctx->hMainWnd = CreateWindowEx (0,"freesci.WndClass","FreeSCI",
		g_bFullScreen ? WS_POPUP : (WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU),
		0,0,rc.right-rc.left,rc.bottom-rc.top,NULL,NULL,NULL,NULL);

	SetWindowLong(ctx->hMainWnd,0,(long) drv);

	hr = DirectDrawCreate(NULL,&(ctx->pDD),NULL);
	if ( FAILED(hr))
	{
		DestroyWindow(ctx->hMainWnd);
		free(ctx);
		return GFX_FATAL;
	}


	if(g_bFullScreen)
	{
		// fulscreen mode, change to the desired mode
		hr = ctx->pDD->SetCooperativeLevel ( ctx->hMainWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
		hr = ctx->pDD->SetDisplayMode ( xres*320, yres*200, dd_bpp);
	}
	else
	{
		// windowed mode, accept whatever mode is current
		hr = ctx->pDD->SetCooperativeLevel (ctx->hMainWnd, DDSCL_NORMAL);
	}

	drv->state = ctx;

	ShowWindow(ctx->hMainWnd,SW_NORMAL);
	UpdateWindow(ctx->hMainWnd);

	pnt.x = 0; pnt.y = 0;
	ClientToScreen (ctx->hMainWnd, &pnt);
	ctx->win_xpos = pnt.x;
	ctx->win_ypos = pnt.y;

	// create surface - primary
	memset(&dd_desc,0,sizeof(dd_desc));
	dd_desc.dwSize = sizeof(dd_desc);
	dd_desc.dwFlags = DDSD_CAPS;
	dd_desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	hr = ctx->pDD->CreateSurface(&dd_desc,&ctx->pPrimary,NULL);
	if(FAILED(hr))
	{
		if(g_bFullScreen)
		{
			hr = ctx->pDD->RestoreDisplayMode ();
			hr = ctx->pDD->SetCooperativeLevel (ctx->hMainWnd, DDSCL_NORMAL);
		}
		ctx->pDD->Release();
		free(ctx);
		return GFX_FATAL;
	}

	// get the current mode; in windowed mode it can be different than the one we want
	memset(&dd_desc,0,sizeof(dd_desc));
	dd_desc.dwSize = sizeof(dd_desc);
	hr = ctx->pDD->GetDisplayMode(&dd_desc);

	r_sh = ShiftCount(dd_desc.ddpfPixelFormat.dwRBitMask);
	g_sh = ShiftCount(dd_desc.ddpfPixelFormat.dwGBitMask);
	b_sh = ShiftCount(dd_desc.ddpfPixelFormat.dwBBitMask);
	dd_bpp2=0;
	if(dd_desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED1)
		dd_bpp2=2;
	if(dd_desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED2)
		dd_bpp2=4;
	if(dd_desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
		dd_bpp2=16;
	if(dd_desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
		dd_bpp2=256;

	drv->mode = gfx_new_mode(xres,yres,dd_desc.ddpfPixelFormat.dwRGBBitCount/8,
		dd_desc.ddpfPixelFormat.dwRBitMask,dd_desc.ddpfPixelFormat.dwGBitMask,dd_desc.ddpfPixelFormat.dwBBitMask,0,
		r_sh,g_sh,b_sh,0, dd_bpp2, 0);

	tbytes = 320*xres*200*yres* (dd_desc.ddpfPixelFormat.dwRGBBitCount/8);

	// create a secondary surfaces
	memset (&ddsd, 0, sizeof (DDSURFACEDESC));
	ddsd.dwSize = sizeof (DDSURFACEDESC);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = 320 * drv->mode->xfact;
	ddsd.dwHeight = 200 * drv->mode->yfact;
	hr = ctx->pDD->CreateSurface(&ddsd,&ctx->pBack,NULL);
	if(FAILED(hr))
	{
		return GFX_FATAL;
	}
	memset (&ddsd, 0, sizeof (DDSURFACEDESC));
	ddsd.dwSize = sizeof (DDSURFACEDESC);
	hr = ctx->pBack->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
	if(!FAILED(hr))
	{
		memset(ddsd.lpSurface,0,tbytes);
		hr = ctx->pBack->Unlock(NULL);
	}

	memset (&ddsd, 0, sizeof (DDSURFACEDESC));
	ddsd.dwSize = sizeof (DDSURFACEDESC);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = 320 * drv->mode->xfact;
	ddsd.dwHeight = 200 * drv->mode->yfact;
	hr = ctx->pDD->CreateSurface(&ddsd,&ctx->pStatic,NULL);
	if(FAILED(hr))
	{
		return GFX_FATAL;
	}
	memset (&ddsd, 0, sizeof (DDSURFACEDESC));
	ddsd.dwSize = sizeof (DDSURFACEDESC);
	hr = ctx->pStatic->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
	if(!FAILED(hr))
	{
		memset(ddsd.lpSurface,0,tbytes);
		hr = ctx->pStatic->Unlock(NULL);
	}

	// if not in full-screen mode, set the clipper
	if (!g_bFullScreen)
	{
		hr = ctx->pDD->CreateClipper (0, &ctx->pClipper, NULL);
		hr = ctx->pClipper->SetHWnd (0, ctx->hMainWnd);
		hr = ctx->pPrimary->SetClipper (ctx->pClipper);
	}

	ctx->priority_maps[DD_BUFFER_BACK] =
		gfx_pixmap_alloc_index_data(gfx_new_pixmap(320 * xres, 200 * yres, GFX_RESID_NONE, 0, 0));
	ctx->priority_maps[DD_BUFFER_STATIC] =
		gfx_pixmap_alloc_index_data(gfx_new_pixmap(320 * xres, 200 * yres, GFX_RESID_NONE, 0, 0));

	ctx->priority_maps[DD_BUFFER_BACK]->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;
	ctx->priority_maps[DD_BUFFER_STATIC]->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;

/*
	if (_open_meta_visuals(drv)) {
		free(meta);
		gfx_free_pixmap(drv, meta->priority_maps[GGI_BUFFER_BACK]);
		gfx_free_pixmap(drv, meta->priority_maps[GGI_BUFFER_STATIC]);
		ggiClose(meta->vis);
		ggiExit();
		return GFX_ERROR;
	}

	if (frames < 2) {
		meta->alt_back_buffer = sci_malloc(bpp * 320 * 200 * xres * yres);
		meta->back_vis = ggiOpen("memory:pointer", meta->alt_back_buffer, NULL);
		if (ggiSetSimpleMode(meta->back_vis, xres * 320, yres * 200, 1, GT_8BIT)) {
			sciprintf("GFXGGI: Warning: Setting mode for memory visual failed\n");
		}
	} else meta->alt_back_buffer = NULL;

	if (frames < 3) {
		meta->static_buffer = sci_malloc(bpp * 320 * 200 * xres * yres);
		meta->static_vis = ggiOpen("memory:pointer", meta->static_buffer, NULL);
		if (ggiSetSimpleMode(meta->static_vis, xres * 320, yres * 200, 1, GT_8BIT)) {
			sciprintf("GFXGGI: Warning: Setting mode for memory visual #2 failed\n");
		}
	} else meta->static_buffer = NULL;

	init_input_ggi();
*/
	return GFX_OK;
}

int
dd_init(gfx_driver_t *drv)
{
	return dd_init_specific(drv,1,1,1);
}

void
dd_exit(gfx_driver_t *drv)
{
	HRESULT hr;
	gfx_dd_struct_t *ctx;

	if(drv->state == NULL)
		return;

	ctx = (gfx_dd_struct_t *) drv->state;

	if(g_bFullScreen)
	{
		hr = ctx->pDD->RestoreDisplayMode ();
		hr = ctx->pDD->SetCooperativeLevel (ctx->hMainWnd, DDSCL_NORMAL);
	}

	free_event_queue(ctx);

	gfx_free_pixmap(drv, ctx->priority_maps[0]);
	gfx_free_pixmap(drv, ctx->priority_maps[1]);

	if(ctx->hMainWnd)
	{
		DestroyWindow(ctx->hMainWnd);
		ctx->hMainWnd = NULL;
	}

	if(ctx->pBack)
	{
		ctx->pBack->Release();
		ctx->pBack=NULL;
	}
	if(ctx->pStatic)
	{
		ctx->pStatic->Release();
		ctx->pStatic=NULL;
	}
	if(ctx->pPrimary)
	{
		ctx->pPrimary->Release();
		ctx->pPrimary=NULL;
	}
	if(ctx->pClipper)
	{
		ctx->pClipper->Release();
		ctx->pClipper=NULL;
	}
	if(ctx->pDD)
	{
		ctx->pDD->Release();
		ctx->pDD = NULL;
	}

	drv->state = NULL;

	free(ctx);
}

int
dd_draw_line(gfx_driver_t *drv,
	     point_t start, point_t end,
	     gfx_color_t color,
	     gfx_line_mode_t line_mode, gfx_line_style_t line_style)
{
	rect_t line = gfx_rect(start.x, start.y,
			       end.x - start.x, end.y - start.y);
	HRESULT hr;
	DDSURFACEDESC ddsc;
	gfx_dd_struct_t *ctx;

	if(drv->state == NULL)
		return GFX_ERROR;

	ctx = (gfx_dd_struct_t *) drv->state;

	memset(&ddsc,0,sizeof(ddsc));
	ddsc.dwSize = sizeof(ddsc);
	hr = ctx->pBack->Lock(NULL,&ddsc,DDLOCK_WAIT,NULL);
	if(FAILED(hr))
	{
		return GFX_ERROR;
	}

	_dd_draw_line(drv->mode, start, end, (byte *) ddsc.lpSurface, ddsc.lPitch,color);

//	gfx_crossblit_pixmap(drv->mode, pxm, priority, src, dest, (byte *) ddsc.lpSurface,
//			     ddsc.lPitch,pri_map, drv->mode->xfact * 320, 1);

	hr = ctx->pBack->Unlock(NULL);
	return GFX_OK;
}

int
dd_draw_filled_rect(gfx_driver_t *drv, rect_t box, gfx_color_t color1, gfx_color_t color2,
		     gfx_rectangle_fill_t shade_mode)
{
	HRESULT hr;
	RECT rcDest;
	DDBLTFX ddblt;
	gfx_dd_struct_t *ctx;
	byte *pri;
	int i;

	if(drv->state == NULL)
		return GFX_ERROR;

	ctx = (gfx_dd_struct_t *) drv->state;

	rcDest.left   = box.x;
	rcDest.top    = box.y;
	rcDest.right  = box.x+box.xl;
	rcDest.bottom = box.y+box.yl;

	if (color1.mask & GFX_MASK_VISUAL)
	{
		memset(&ddblt,0,sizeof(ddblt));
		ddblt.dwSize = sizeof(ddblt);
		if(drv->mode->palette!=NULL)
			ddblt.dwFillColor = color1.visual.global_index;
		else
			ddblt.dwFillColor = MakeRGB(drv->mode,color1.visual.r,color1.visual.g,color1.visual.b);
		hr = ctx->pBack->Blt(&rcDest,NULL,NULL,DDBLT_COLORFILL,&ddblt);
	}

	if (color1.mask & GFX_MASK_PRIORITY)
	{
		pri = ctx->priority_maps[DD_BUFFER_BACK]->index_data + box.x + box.y*(drv->mode->xfact * 320);
		for(i=0; i<box.yl; i++)
		{
			memset(pri,color1.priority,box.xl);
			pri += drv->mode->xfact * 320;
		}
	}

	return GFX_OK;
}

int
dd_draw_pixmap(gfx_driver_t *drv, gfx_pixmap_t *pxm, int priority,
		rect_t src, rect_t dest, gfx_buffer_t buffer)
{
	HRESULT hr;
	gfx_dd_struct_t *ctx;
	LPDIRECTDRAWSURFACE dst;
	DDSURFACEDESC ddsc;
	byte *pri_map = NULL;

	if(drv->state == NULL)
		return GFX_ERROR;

	ctx = (gfx_dd_struct_t *) drv->state;

	if (src.xl != dest.xl || src.yl != dest.yl)
	{
		GFXERROR("Attempt to draw scaled view- not supported by this driver!\n");
		return GFX_FATAL; /* Scaling not supported! */
	}

	switch(buffer)
	{
		case GFX_BUFFER_FRONT:
			GFXERROR("Attempt to draw pixmap to front buffer\n");
			return GFX_ERROR;
		case GFX_BUFFER_BACK:
			dst = ctx->pBack;
			pri_map = ctx->priority_maps[DD_BUFFER_BACK]->index_data;
			break;
		case GFX_BUFFER_STATIC:
			dst = ctx->pStatic;
			pri_map = ctx->priority_maps[DD_BUFFER_STATIC]->index_data;
			break;
		default:
			GFXERROR("Unexpected buffer ID %d\n", buffer);
			return GFX_ERROR;
	}

	memset(&ddsc,0,sizeof(ddsc));
	ddsc.dwSize = sizeof(ddsc);
	hr = dst->Lock(NULL,&ddsc,DDLOCK_WAIT,NULL);
	if(FAILED(hr))
	{
		return GFX_ERROR;
	}

	gfx_crossblit_pixmap(drv->mode, pxm, priority, src, dest, (byte *) ddsc.lpSurface,
			     ddsc.lPitch,pri_map, drv->mode->xfact * 320, 1, 0);

	hr = dst->Unlock(NULL);

	return GFX_OK;
}

int
dd_grab_pixmap(gfx_driver_t *drv, rect_t src, gfx_pixmap_t *pxm, gfx_map_mask_t map)
{
	HRESULT hr;
	gfx_dd_struct_t *ctx;
	DDSURFACEDESC ddsc;
	int i,x,y,xlb,bpp;
	BYTE *s,*d;

	if(drv->state == NULL)
		return GFX_ERROR;

	ctx = (gfx_dd_struct_t *) drv->state;

	if (src.x < 0 || src.y < 0)
	{
		GFXERROR("Attempt to grab pixmap from invalid coordinates (%d,%d)\n", src.x, src.y);
		return GFX_ERROR;
	}

	if (!pxm->data)
	{
		GFXERROR("Attempt to grab pixmap to unallocated memory\n");
		return GFX_ERROR;
	}

	pxm->xl = src.xl;
	pxm->yl = src.yl;

	memset(&ddsc,0,sizeof(ddsc));
	ddsc.dwSize = sizeof(ddsc);
	hr = ctx->pBack->Lock(NULL,&ddsc,DDLOCK_WAIT,NULL);
	if(FAILED(hr))
		return GFX_ERROR;

	bpp = ddsc.ddpfPixelFormat.dwRGBBitCount / 8;
	x = src.x*drv->mode->xfact;
	y = src.y*drv->mode->yfact;
/*
	if(!g_bFullScreen)
	{
		x += ctx->win_xpos;
		y += ctx->win_ypos;
	}
*/
	s = (BYTE *) ddsc.lpSurface + x*bpp + y*ddsc.lPitch;
	d = pxm->data;
	xlb = src.xl * bpp;

	for(i=0; i<src.yl; i++)
	{
		memcpy(d,s,xlb);
		d += xlb;
		s += ddsc.lPitch;
	}

	hr = ctx->pBack->Unlock(NULL);

	return GFX_OK;
}

static int
dd_update(gfx_driver_t *drv, rect_t src, point_t dest, gfx_buffer_t buffer)
{
	gfx_dd_struct_t *ctx;
	RECT rcDst,rcSrc;
	HRESULT hr;
	LPDIRECTDRAWSURFACE p_dst,p_src;

	if(drv->state == NULL)
		return GFX_ERROR;

	ctx = (gfx_dd_struct_t *) drv->state;

	rcSrc.left   = src.x;
	rcSrc.right  = src.x + src.xl;
	rcSrc.top    = src.y;
	rcSrc.bottom = src.y + src.yl;

	rcDst.left   = dest.x;
	rcDst.right  = dest.x + src.xl;
	rcDst.top    = dest.y;
	rcDst.bottom = dest.y + src.yl;

	switch(buffer)
	{
		case GFX_BUFFER_FRONT:
			rcDst.left   += ctx->win_xpos;
			rcDst.right  += ctx->win_xpos;
			rcDst.top    += ctx->win_ypos;
			rcDst.bottom += ctx->win_ypos;
			p_dst = ctx->pPrimary;
			p_src = ctx->pBack;
			break;
		case GFX_BUFFER_BACK:
			p_dst = ctx->pBack;
			p_src = ctx->pStatic;
			break;
		default:
			return GFX_ERROR;
	}

	hr = p_dst->Blt(&rcDst,p_src,&rcSrc,DDBLT_WAIT,NULL);
	if(FAILED(hr))
		return GFX_ERROR;

	return GFX_OK;
}

static int
dd_set_static_buffer(gfx_driver_t *drv, gfx_pixmap_t *pic, gfx_pixmap_t *priority)
{
	gfx_dd_struct_t *ctx;
	DDSURFACEDESC ddsc;
	HRESULT hr;
	int i,xs;
	byte *s,*d;
//	ggi_visual_t vis = get_writeable_static_visual(drv);

	if(drv->state == NULL)
		return GFX_ERROR;

	ctx = (gfx_dd_struct_t *) drv->state;

	/* First, check if the priority map is sane */
	if (priority->index_xl != ctx->priority_maps[DD_BUFFER_STATIC]->index_xl
	    || priority->index_yl != ctx->priority_maps[DD_BUFFER_STATIC]->index_yl)
	{
		GFXERROR("Invalid priority map: (%dx%d) vs expected (%dx%d)\n",
			 priority->index_xl, priority->index_yl,
			 ctx->priority_maps[DD_BUFFER_STATIC]->index_xl,
			 ctx->priority_maps[DD_BUFFER_STATIC]->index_yl);
		return GFX_ERROR;
	}

//	ggiPutBox(vis, 0, 0, pic->xl, pic->yl, pic->data);
	memset(&ddsc,0,sizeof(ddsc));
	ddsc.dwSize = sizeof(ddsc);
	hr = ctx->pStatic->Lock(NULL,&ddsc,DDLOCK_WAIT,NULL);
	if(FAILED(hr))
	{
		return GFX_ERROR;
	}

	s = pic->data;
	d = (byte *) ddsc.lpSurface;
	xs = drv->mode->bytespp * pic->xl;

	for(i=0; i<pic->yl; i++)
	{
		memcpy(d,s,xs);
		s+= xs;
		d+= ddsc.lPitch;
	}

	hr = ctx->pStatic->Unlock(NULL);

	memcpy(ctx->priority_maps[DD_BUFFER_STATIC]->index_data, priority->index_data,
	       priority->index_xl * priority->index_yl);

	return GFX_OK;
}

static int
dd_set_palette(gfx_driver_t *drv, int index, byte red, byte green, byte blue)
{
	//!! implement me !!
	return GFX_ERROR;
}

// ------------------------------------- non-drawing functions ------------------------------------
sci_event_t get_queue_event(gfx_dd_struct_t *ctx);

static int process_messages(void)
{
	MSG msg;

	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage (&msg, NULL, 0, 0))
			return 0;
		TranslateMessage (&msg);
		DispatchMessage(&msg);
	}
	return 1;
}

static sci_event_t
dd_get_event(gfx_driver_t *drv)
{
	gfx_dd_struct_t *ctx;

	process_messages();

	assert(drv->state!=NULL);

	ctx = (gfx_dd_struct_t *) drv->state;

	return get_queue_event(ctx);
}

static void
MsgWait (long WaitTime)
{
	DWORD dwRet=0;
	long dwWait;
	DWORD StartTime=timeGetTime();

	dwWait = WaitTime;

	if (!process_messages())
		return;

	if (dwWait <= 0)
		return;
	do
	{
		dwWait=WaitTime-(timeGetTime()-StartTime);
		if (dwWait <= 0)
			break;

		dwRet=MsgWaitForMultipleObjects (0, NULL, FALSE, dwWait, QS_ALLINPUT);

		if(dwRet == WAIT_TIMEOUT)
			return;

		if (!process_messages())
			return;

	} while(1);
}

int
dd_usleep(gfx_driver_t* drv, long usec)
{
	if (usec < 1000)
		Sleep(usec / 1000);
	else
		MsgWait (usec / 1000);

	return GFX_OK;
}
/* --------------------------------------------------------------- */

static void init_event_queue(gfx_dd_struct_t *ctx)
{
	ctx->queue_first = 0;
	ctx->queue_last  = 0;
	ctx->queue_size  = 256;
	ctx->event_queue = (sci_event_t *) sci_malloc (ctx->queue_size * sizeof (sci_event_t));
}

static void free_event_queue(gfx_dd_struct_t *ctx)
{
	if (ctx->event_queue) sci_free(ctx->event_queue);
	ctx->queue_size  = 0;
	ctx->queue_first =0;
	ctx->queue_last  =0;
}

static void add_queue_event(gfx_dd_struct_t *ctx,int type, int data, int buckybits)
{
	if ((ctx->queue_last+1) % ctx->queue_size == ctx->queue_first)
	{
		/* Reallocate queue */
		int i, event_count;
		sci_event_t *new_queue;

		new_queue = (sci_event_t *) sci_malloc (ctx->queue_size * 2 * sizeof (sci_event_t));
		event_count = (ctx->queue_last - ctx->queue_first) % ctx->queue_size;
		for (i=0; i<event_count; i++)
			new_queue [i] = ctx->event_queue [(ctx->queue_first+i) % ctx->queue_size];
		free (ctx->event_queue);
		ctx->event_queue = new_queue;
		ctx->queue_size *= 2;
		ctx->queue_first = 0;
		ctx->queue_last  = event_count;
	}

	ctx->event_queue [ctx->queue_last].data = data;
	ctx->event_queue [ctx->queue_last].type = type;
	ctx->event_queue [ctx->queue_last++].buckybits = buckybits;
	if (ctx->queue_last == ctx->queue_size)
		ctx->queue_last=0;
}

static void add_mouse_event (gfx_dd_struct_t *ctx,int type, int data, WPARAM wParam)
{
	int buckybits = 0;

	if (wParam & MK_SHIFT)
		buckybits |= SCI_EVM_LSHIFT | SCI_EVM_RSHIFT;
	if (wParam & MK_CONTROL)
		buckybits |= SCI_EVM_CTRL;

	add_queue_event (ctx,type, data, buckybits);
}

static void add_key_event (gfx_dd_struct_t *ctx,int data)
{
	int buckybits = 0;

	/* FIXME: If anyone cares, on Windows NT we can distinguish left and right shift */
	if (GetAsyncKeyState (VK_SHIFT))
		buckybits |= SCI_EVM_LSHIFT | SCI_EVM_RSHIFT;
	if (GetAsyncKeyState (VK_CONTROL))
		buckybits |= SCI_EVM_CTRL;
	if (GetAsyncKeyState (VK_MENU))
		buckybits |= SCI_EVM_ALT;
	if (GetKeyState (VK_CAPITAL) & 1)
		buckybits |= SCI_EVM_CAPSLOCK;

	add_queue_event (ctx,SCI_EVT_KEYBOARD, data, buckybits);
}

sci_event_t get_queue_event(gfx_dd_struct_t *ctx)
{
	if (ctx->queue_first == ctx->queue_size)
		ctx->queue_first = 0;

	if (ctx->queue_first == ctx->queue_last)
	{
		sci_event_t noevent;
		noevent.data = 0;
		noevent.type = SCI_EVT_NONE;
		noevent.buckybits = 0;
		return noevent;
	}
	else
		return ctx->event_queue [ctx->queue_first++];
}

#define MAP_KEY(x,y) case x: add_key_event (ctx, y); break

static long FAR PASCAL WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;
	gfx_dd_struct_t *ctx;
	gfx_driver_t* drv;

	drv = (gfx_driver_t *) GetWindowLong(hWnd,0);
	if(drv == NULL)
		return DefWindowProc(hWnd,message,wParam,lParam);
	ctx = (gfx_dd_struct_t *) drv->state;
	if(ctx == NULL)
		return DefWindowProc(hWnd,message,wParam,lParam);

	switch(message)
	{
		case WM_SIZE:
		case WM_MOVE:
			{
				POINT pnt;
				pnt.x = 0; pnt.y = 0;
				ClientToScreen (hWnd, &pnt);
				ctx->win_xpos = pnt.x;
				ctx->win_ypos = pnt.y;
			}
			break;
		case WM_PAINT:
			{
				RECT rcSrc,rcDst;
				if (ctx->pPrimary!=NULL && ctx->pBack!=NULL && GetUpdateRect (hWnd, &rcSrc, FALSE))
				{
					if(rcSrc.right>320)
						rcSrc.right=320;
					if(rcSrc.bottom>200)
						rcSrc.bottom=200;
					rcDst.left   = rcSrc.left   + ctx->win_xpos;
					rcDst.right  = rcSrc.right  + ctx->win_xpos;
					rcDst.top    = rcSrc.top    + ctx->win_ypos;
					rcDst.bottom = rcSrc.bottom + ctx->win_ypos;
					hr = ctx->pPrimary->Blt(&rcDst,ctx->pBack,&rcSrc,DDBLT_WAIT,NULL);
					ValidateRect(hWnd,&rcSrc);
				}
			}
			break;
		case WM_SETCURSOR:
			if(IsIconic(hWnd) || ctx->bShowMouse)
			{
				SetCursor(LoadCursor (NULL, IDC_ARROW));
			}
			else
			{
				SetCursor(NULL);
			}
			return TRUE;
		case WM_NCMOUSEMOVE:
			ctx->bShowMouse = TRUE;
			break;
		case WM_MOUSEMOVE:
			ctx->bShowMouse = FALSE;
			if( ((lParam & 0xFFFF) >= 320*drv->mode->xfact) || ((lParam >> 16) >= 200*drv->mode->yfact))
				break;
			drv->pointer_x = lParam & 0xFFFF;
			drv->pointer_y = lParam >> 16;
			break;

		case WM_LBUTTONDOWN: add_mouse_event (ctx, SCI_EVT_MOUSE_PRESS, 1, wParam);   break;
		case WM_RBUTTONDOWN: add_mouse_event (ctx, SCI_EVT_MOUSE_PRESS, 2, wParam);   break;
		case WM_MBUTTONDOWN: add_mouse_event (ctx, SCI_EVT_MOUSE_PRESS, 3, wParam);   break;
		case WM_LBUTTONUP:   add_mouse_event (ctx, SCI_EVT_MOUSE_RELEASE, 1, wParam); break;
		case WM_RBUTTONUP:   add_mouse_event (ctx, SCI_EVT_MOUSE_RELEASE, 2, wParam); break;
		case WM_MBUTTONUP:   add_mouse_event (ctx, SCI_EVT_MOUSE_RELEASE, 3, wParam); break;

		case WM_KEYDOWN:
			switch (wParam)
			{
				MAP_KEY (VK_ESCAPE, SCI_K_ESC);
				MAP_KEY (VK_END,    SCI_K_END);
				MAP_KEY (VK_DOWN,   SCI_K_DOWN);
				MAP_KEY (VK_NEXT,   SCI_K_PGDOWN);
				MAP_KEY (VK_LEFT,   SCI_K_LEFT);
				MAP_KEY (VK_RIGHT,  SCI_K_RIGHT);
				MAP_KEY (VK_HOME,   SCI_K_HOME);
				MAP_KEY (VK_UP,     SCI_K_UP);
				MAP_KEY (VK_PRIOR,  SCI_K_PGUP);
				MAP_KEY (VK_INSERT, SCI_K_INSERT);
				MAP_KEY (VK_DELETE, SCI_K_DELETE);
				MAP_KEY (VK_BACK,   SCI_K_BACKSPACE);
				MAP_KEY (VK_TAB,    '\t');
				MAP_KEY (VK_RETURN, '\r');

			default:
				if (wParam >= VK_F1 && wParam <= VK_F10)
					add_key_event (ctx, wParam - VK_F1 + SCI_K_F1);
				else if (wParam >= 'A' && wParam <= 'Z')
					add_key_event (ctx, wParam - 'A' + 97);
				else if (wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD9)
				{
					if (GetKeyState (VK_NUMLOCK) & 1)
						add_key_event (ctx, wParam - VK_NUMPAD0 + '0');
					else
					switch (wParam)
					{
						MAP_KEY (VK_NUMPAD0, SCI_K_INSERT);
						MAP_KEY (VK_NUMPAD1, SCI_K_END);
						MAP_KEY (VK_NUMPAD2, SCI_K_DOWN);
						MAP_KEY (VK_NUMPAD3, SCI_K_PGDOWN);
						MAP_KEY (VK_NUMPAD4, SCI_K_LEFT);
						MAP_KEY (VK_NUMPAD6, SCI_K_RIGHT);
						MAP_KEY (VK_NUMPAD7, SCI_K_HOME);
						MAP_KEY (VK_NUMPAD8, SCI_K_UP);
						MAP_KEY (VK_NUMPAD9, SCI_K_PGUP);
					}
				}
				else if (wParam == 0xC0)  /* tilde key - used for invoking console */
					add_key_event (ctx, '`');
				else
					add_key_event (ctx, wParam);
				break;
			}
	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}

/* --------------------------------------------------------------- */
extern "C"
gfx_driver_t gfx_driver_dd = {
	"ddraw",
	DD_DRIVER_VERSION,
	SCI_GFX_DRIVER_MAGIC,
	SCI_GFX_DRIVER_VERSION,
	NULL,
	0,0,
	0,
	GFX_DEBUG_POINTER | GFX_DEBUG_UPDATES | GFX_DEBUG_PIXMAPS | GFX_DEBUG_BASIC,
	dd_set_param,
	dd_init_specific,
	dd_init,
	dd_exit,
	dd_draw_line,
	dd_draw_filled_rect,
	NULL,
	NULL,
	dd_draw_pixmap,
	dd_grab_pixmap,
	dd_update,
	dd_set_static_buffer,
	NULL,
	dd_set_palette,
	dd_get_event,
	dd_usleep
};

#endif // HAVE_DDRAW
