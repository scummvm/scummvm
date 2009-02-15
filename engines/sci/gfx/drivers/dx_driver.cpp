/***************************************************************************
 dx_driver.cpp Copyright (C) 2008 Alexander R Angas,
               Some portions Copyright (C) 1999 Dmitry Jemerov

 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.

 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

		Alexander R Angas (Alex) <arangas AT internode dot on dot net>

 History:
	20051106 (AAngas) - Rewrite
	20060201 (AAngas) - Fixed wrong format for texture priority maps
	20060205 (AAngas) - Changed pointer to use D3DXSprite
	20060208 (AAngas) - Fixed pointer alpha blending bug
	20060307 (AAngas) - Fixed crash on exit
	20080118 (AAngas) - Fixed pointer scaling and window size

 Notes:
    DirectX handles all scaling. All drawing functions assume an unscaled
	resolution.

TODO:
	P1 - Corrupt cursor at different resolutions
	P1 - Lost devices
	P1 - Problems moving and activating window
	P1 - Occasional pause when starting FreeSCI and poor sound resulting (?)
	P1 - Mouse pointer corruption at bottom and why need to scale vertical by 4?
	P2 - Full screen
	P3 - Draw lines as Direct3D vertices
	P3 - Fine line mode
	P3 - Allow user to specify hardware or software vertex processing
	P3 - Fancies

***************************************************************************/

#ifdef HAVE_DIRECTX

#ifndef __cplusplus
#error NOTE: This file MUST be compiled as C++. In Visual C++, use the /Tp command line option.
#endif

#include "dx_driver.h"

#define TO_STRING2(x) #x
#define TO_STRING(x) TO_STRING2(x)
#if (DIRECT3D_VERSION < 0x0800)
#	error The DirectX 8 SDK (or higher) is required for this driver.
#elif (DIRECT3D_VERSION > 0x0800)
#	pragma message (" ")
#	pragma message ("*** This driver has been developed against version 8 of the DirectX SDK and may not work against your version " TO_STRING(DIRECT3D_VERSION))
#	pragma message (" ")
#endif


// Stores driver flags
static int flags = 0;


// Windows message processing
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	return DefWindowProc( hWnd, msg, wParam, lParam );
}


///// RENDERING

// Render the scene to screen
static gfx_return_value_t
RenderD3D(struct _gfx_driver *drv)
{
	HRESULT hr;

	// Check we haven't lost the device (i.e. window focus)
	if (CheckDevice(drv))
	{
		// Begin scene
		DODX( (dx_state->pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0, 0)), RenderD3D );
		DODX( (dx_state->pDevice->BeginScene()), RenderD3D );

		// Set texture
		DODX( (dx_state->pDevice->SetTexture( 0, dx_state->pTexVisuals[PRIMARY_VIS] )), RenderD3D );	// Scene image

		// Set texture states for scene
		DODX( (dx_state->pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE )), RenderD3D );
		DODX( (dx_state->pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE )), RenderD3D );
		DODX( (dx_state->pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE )), RenderD3D );

		// Set vertices and how to draw
		DODX( (dx_state->pDevice->SetStreamSource(0, dx_state->pVertBuff, sizeof(CUSTOMVERTEX))), RenderD3D );
		DODX( (dx_state->pDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX )), RenderD3D );
		DODX( (dx_state->pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)), RenderD3D );

		// Draw the pointer sprite
		if (dx_state->pTexPointer)
		{
			D3DXVECTOR2 pointerPos((float)drv->pointer_x, (float)drv->pointer_y);
			DODX( (dx_state->pSPointer->Begin()), RenderD3D );
			DODX( (dx_state->pSPointer->Draw(dx_state->pTexPointer, NULL, NULL, NULL, 0.0, &pointerPos, 0xFFFFFFFF)), RenderD3D );
			DODX( (dx_state->pSPointer->End()), RenderD3D );
		}

		// Present scene
		DODX( (dx_state->pDevice->EndScene()), RenderD3D );
		DODX( (dx_state->pDevice->Present(NULL, NULL, NULL, NULL)), RenderD3D );
	}

	return GFX_OK;
}


// Check device hasn't been lost
static int
CheckDevice(struct _gfx_driver *drv)
{
	HRESULT hr;
	switch ( dx_state->pDevice->TestCooperativeLevel() )
	{
		case D3DERR_DEVICELOST: return false;	// Lost window focus

		case D3DERR_DEVICENOTRESET:				// We're back!
		{
			// Reset with our presentation parameters and reinitialise the scene
			DODX( (dx_state->pDevice->Reset(&dx_state->presParams)), CheckDevice );
			if (hr != D3D_OK)
			   return false;

			InitScene(drv);
			return true;
		}

		default: return true;
	}
}


///// INITIALIZATION

// Create window to draw to
static gfx_return_value_t
InitWindow(struct _gfx_driver *drv, UINT width, UINT height)
{
	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW;
	RECT clientSize = { 0, 0, width, height };
	sciprintf("Window %d x %d\n", width, height);

	// Register the window class
	ZeroMemory( &(dx_state->wc), sizeof(WNDCLASSEX) );
	dx_state->wc.cbSize = sizeof(WNDCLASSEX);
	dx_state->wc.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
	dx_state->wc.lpfnWndProc = MsgProc;
	dx_state->wc.hInstance = GetModuleHandle(NULL);
	dx_state->wc.lpszClassName = DX_CLASS_NAME;
	if ( RegisterClassEx( &dx_state->wc ) == 0 )
	{
		sciprintf("InitWindow(): RegisterClassEx failed (%u)\n", GetLastError());
		return GFX_FATAL;
	}

	// Get correct size of window to pass to CreateWindow (considering window decorations)
	if ( AdjustWindowRectEx( &clientSize, dwStyle, false, dwExStyle ) == 0 )
	{
		sciprintf("InitWindow(): Window size calculation failed (%u)\n", GetLastError());
		return GFX_FATAL;
	}

	// Create the application's window
	dx_state->hWnd = CreateWindowEx(
		dwExStyle,
		DX_CLASS_NAME, DX_APP_NAME,
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT,
		clientSize.right - clientSize.left, clientSize.bottom - clientSize.top,
		NULL/*GetDesktopWindow()*/, NULL, dx_state->wc.hInstance, NULL );

	if ( dx_state->hWnd == NULL )
	{
		sciprintf("InitWindow(): CreateWindow failed (%u)\n", GetLastError());
		return GFX_FATAL;
	}

	// Show the window
	ShowWindow( dx_state->hWnd, SW_SHOWDEFAULT );
	UpdateWindow( dx_state->hWnd );

	return GFX_OK;
}


// Initialize Direct3D
static gfx_return_value_t
InitD3D(struct _gfx_driver *drv)
{
	HRESULT hr;

	sciprintf("Initializing Direct3D\n");

	// Set our colour format
	dx_state->d3dFormat = D3DFMT_A8R8G8B8;
	dx_state->vertexProcessing = D3DCREATE_MIXED_VERTEXPROCESSING;

	// Create Direct3D object
	dx_state->pD3d = Direct3DCreate8( D3D_SDK_VERSION );
	if ( FAILED( dx_state->pD3d ) ) {
		sciprintf("InitD3D(): Direct3DCreate8 failed\n");
		return GFX_FATAL;
	}

	// Look for adapters
	for ( UINT adapterLoop = 0; adapterLoop < dx_state->pD3d->GetAdapterCount(); adapterLoop++ )
	{
		D3DADAPTER_IDENTIFIER8 adapterId;
		DODX( (dx_state->pD3d->GetAdapterIdentifier(adapterLoop, D3DENUM_NO_WHQL_LEVEL, &adapterId)), InitD3D );
		if ( FAILED( hr ) )
			break;
		if (adapterId.Driver[0] == '\0')
			break;
		sciprintf("   Adapter %u: %s\n", adapterLoop++, adapterId.Description);
	}
	if (dx_state->adapterId == -1)
		dx_state->adapterId = D3DADAPTER_DEFAULT;

	// Get device caps
	DODX( (dx_state->pD3d->GetDeviceCaps(dx_state->adapterId, D3DDEVTYPE_HAL, &(dx_state->deviceCaps))), InitD3D );
	if ( FAILED( hr ) ) {
		sciprintf("Sorry, this adapter does not have a 3D accelerated video driver installed.\n");
		return GFX_FATAL;
	}

	// Define presentation parameters
	ZeroMemory( &dx_state->presParams, sizeof(D3DPRESENT_PARAMETERS) );
	dx_state->presParams.Windowed = TRUE;							// We want windowed by default
	dx_state->presParams.SwapEffect = D3DSWAPEFFECT_DISCARD;		// Throw away last frame
	dx_state->presParams.hDeviceWindow = dx_state->hWnd;			// Window handle
	dx_state->presParams.BackBufferWidth  = 320;					// Back buffer dimensions
	dx_state->presParams.BackBufferHeight = 200;					// 
	dx_state->presParams.BackBufferFormat = dx_state->d3dFormat;	// Colour format

	// Check if user requested full screen
	if (flags & DX_FLAGS_FULLSCREEN)
	{
		if (dx_state->deviceCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED) {
			dx_state->presParams.Windowed = FALSE;
			sciprintf("Full screen mode");
		} else {
			sciprintf("Sorry, DirectX will not render in full screen with your video card\n");
		}
	}

	// Get current display mode
	DODX( (dx_state->pD3d->GetAdapterDisplayMode( dx_state->adapterId, &dx_state->displayMode )), InitD3D );
	sciprintf("Chosen adapter %u\n", dx_state->adapterId);

	// Turn off Windows mouse pointer
	ShowCursor(FALSE);

	return GFX_OK;
}


// Initialize scene
static gfx_return_value_t
InitScene(struct _gfx_driver *drv)
{
	HRESULT hr;

	sciprintf("Creating scene\n");

	// Describe how scene will be rendered
	DODX((dx_state->pDevice->SetRenderState( D3DRS_AMBIENT, RGB(255,255,255) )), InitScene);	// Maximum ambient light
	DODX((dx_state->pDevice->SetRenderState( D3DRS_LIGHTING, FALSE )), InitScene);				// Disable lighting features
	DODX((dx_state->pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE )), InitScene);		// Don't cull back side of polygons
	DODX((dx_state->pDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE )), InitScene);		 	// No depth buffering

	return GFX_OK;
}


// For user to set a driver-specific parameter
static int
dx_set_param(struct _gfx_driver *drv, char *attribute, char *value)
{
/*	// Full screen
	if (!strncmp(attribute, "fullscreen", 11)) {
		if (string_truep(value))
			flags |= DX_FLAGS_FULLSCREEN;
		else
			flags &= ~DX_FLAGS_FULLSCREEN;

		return GFX_OK;
	}
*/
	// Adapter ID
	if (!strncmp(attribute, "adapterid", 11)) {
		int aid = D3DADAPTER_DEFAULT;
		dx_state->adapterId = atoi(value);

		return GFX_OK;
	}

	sciprintf("Unrecognised attempt to set DirectX parameter \"%s\" to \"%s\"\n", attribute, value);
	return GFX_ERROR;
}


// Initialize a specific graphics mode
static int
dx_init_specific(struct _gfx_driver *drv,
				 int xfact, int yfact,	/* horizontal and vertical scaling */
				 int bytespp)			/* must be value 4 */
{
	HRESULT hr;
	int red_shift = 8, green_shift = 16, blue_shift = 24, alpha_shift = 32;
	int alpha_mask = 0x00000000, red_mask = 0x00ff0000, green_mask = 0x0000ff00, blue_mask = 0x000000ff;
	gfx_return_value_t d3dret;

	// Error checking
	if (xfact < 1 || yfact < 1 || bytespp < 1 || bytespp > 4) {
		sciprintf("Attempt to open window with invalid scale factors (%d,%d) and bpp=%d!\n",
		 xfact, yfact, bytespp);
		return GFX_ERROR;
	}

	// Prepare memory for gfx_dx_struct_t
	drv->state = (struct gfx_dx_struct_t *) sci_malloc(sizeof(gfx_dx_struct_t));
	ZeroMemory(drv->state, sizeof(gfx_dx_struct_t));
	dx_state->adapterId = -1;	// we will set this later

	// Set window size factor
	dx_state->xfact = xfact;
	dx_state->yfact = yfact;
	dx_state->bpp = bytespp;
	sciprintf("Window scaling %d x %d @ %d bpp\n", dx_state->xfact, dx_state->yfact, dx_state->bpp << 3);

	// Set up Direct3D
	d3dret = InitD3D(drv);
	if (d3dret != GFX_OK)
		return d3dret;

	// Create window
	InitWindow(drv, dx_state->xfact * 320, dx_state->yfact * 200);

	// Create D3D device
	DODX( (dx_state->pD3d->CreateDevice(dx_state->adapterId, D3DDEVTYPE_HAL, dx_state->hWnd,
		dx_state->vertexProcessing, &dx_state->presParams, &dx_state->pDevice)), dx_init_specific );

	// Create the scene
	d3dret = InitScene(drv);
	if (d3dret != GFX_OK)
		return d3dret;

	// Define and populate vertex buffers
	DODX((dx_state->pDevice->CreateVertexBuffer( 4 * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX,
												D3DPOOL_MANAGED, &dx_state->pVertBuff )), dx_init_specific);

	dx_state->pvData[0].p = D3DXVECTOR4(                  0.0f,                   0.0f, 0.0f, 1.0f);
    dx_state->pvData[1].p = D3DXVECTOR4(320.0f,                   0.0f, 0.0f, 1.0f);
    dx_state->pvData[2].p = D3DXVECTOR4(                  0.0f, 200.0f, 0.0f, 1.0f);
    dx_state->pvData[3].p = D3DXVECTOR4(320.0f, 200.0f, 0.0f, 1.0f);
	dx_state->pvData[0].colour = dx_state->pvData[1].colour = dx_state->pvData[2].colour = dx_state->pvData[3].colour = 0xffffffff;
	dx_state->pvData[0].t = D3DXVECTOR2(0.0f, 0.0f);
	dx_state->pvData[1].t = D3DXVECTOR2(1.0f, 0.0f);
	dx_state->pvData[2].t = D3DXVECTOR2(0.0f, 1.0f);
	dx_state->pvData[3].t = D3DXVECTOR2(1.0f, 1.0f);

	VOID *ptr;
	DODX((dx_state->pVertBuff->Lock(0, 0, (BYTE**)&ptr, 0)), dx_init_specific);
	memcpy(ptr, dx_state->pvData, sizeof(dx_state->pvData));
	DODX((dx_state->pVertBuff->Unlock()), dx_init_specific);

	// Create textures
	int i;
	for (i = 0; i < NUM_VISUAL_BUFFERS; i++)
	{
		DODX((dx_state->pDevice->CreateTexture(320, 200,
			1, 0,
			dx_state->d3dFormat,
			D3DPOOL_MANAGED,
			&dx_state->pTexVisuals[i])), dx_init_specific);
	}
	for (i = 0; i < NUM_PRIORITY_BUFFERS; i++)
	{
		DODX((dx_state->pDevice->CreateTexture(320, 200,
			1, 0,
			dx_state->d3dFormat,
			D3DPOOL_MANAGED,
			&dx_state->pTexPrioritys[i])), dx_init_specific);
	}

	// Create sprite for pointer
	DODX( (D3DXCreateSprite(dx_state->pDevice, &dx_state->pSPointer)), dx_init_specific );

	// Allocate priority maps
	for (int i = 0; i < NUM_PRIORITY_BUFFERS; i++)
	{
		dx_state->priority_maps[i] = gfx_pixmap_alloc_index_data(gfx_new_pixmap(320, 200, GFX_RESID_NONE, -i, -777));
		if (!dx_state->priority_maps[i]) {
			GFXERROR("Out of memory: Could not allocate priority maps! (%dx%d)\n", 320, 200);
			return GFX_FATAL;
		}
		dx_state->priority_maps[i]->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;
	}

	// Set up the event queue
	dx_state->queue_first = 0;
	dx_state->queue_last  = 0;
	dx_state->queue_size  = 256;
	dx_state->event_queue = (sci_event_t *) sci_malloc (dx_state->queue_size * sizeof (sci_event_t));

	// Set up graphics mode for primary vis
	drv->mode = gfx_new_mode(1, 1, dx_state->bpp,
			   red_mask, green_mask, blue_mask, alpha_mask,
			   red_shift, green_shift, blue_shift, alpha_shift,
			   (dx_state->bpp == 1) ? 256 : 0, 0);

	// Set up graphics mode for pointer
	dx_state->pointerMode = gfx_new_mode(1, 4, dx_state->bpp,
    									0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000,
										        24,         16,          8,          0,
										0, GFX_MODE_FLAG_REVERSE_ALPHA);

	return GFX_OK;
}


// Initialize 'most natural' graphics mode
static int
dx_init(struct _gfx_driver *drv)
{
	return dx_init_specific(drv, 2, 2, 4);
}


// Uninitialize the current graphics mode
static void
dx_exit(struct _gfx_driver *drv)
{
	int i;

	if(drv->state == NULL)
		return;

	for (i = 0; i < NUM_PRIORITY_BUFFERS; i++)
		SAFE_RELEASE( dx_state->pTexPrioritys[i] );
	for (i = 0; i < NUM_VISUAL_BUFFERS; i++)
		SAFE_RELEASE( dx_state->pTexVisuals[i] );
	SAFE_RELEASE( dx_state->pTexPointer );
	SAFE_RELEASE( dx_state->pSPointer );
	SAFE_RELEASE( dx_state->pVertBuff );
    SAFE_RELEASE( dx_state->pDevice );
    SAFE_RELEASE( dx_state->pD3d );

	if ( dx_state->pointerMode )
		gfx_free_mode(dx_state->pointerMode);
	if ( drv->mode )
		gfx_free_mode(drv->mode);

	if ( dx_state->event_queue )
		sci_free(dx_state->event_queue);
	dx_state->queue_size = 0;

	for (i = 0; i < NUM_PRIORITY_BUFFERS; i++)
		gfx_free_pixmap(drv, dx_state->priority_maps[i]);

    UnregisterClass( DX_CLASS_NAME, dx_state->wc.hInstance );
	DestroyWindow(dx_state->hWnd);

	sci_free(dx_state);
}


///// DRAWING

// Draws a single filled and possibly shaded rectangle to the back buffer
static int
dx_draw_filled_rect(struct _gfx_driver *drv, rect_t box,
			   gfx_color_t color1, gfx_color_t color2,
			   gfx_rectangle_fill_t shade_mode)
{
	if (color1.mask & GFX_MASK_VISUAL)
	{
		HRESULT hr;
		D3DLOCKED_RECT lockedRect;

		// Calculate colour value for line pixel
		UINT lineColor = (color1.alpha << 24) | (color1.visual.r << 16) | (color1.visual.g << 8) | color1.visual.b;
		RECT r = { box.x, box.y, box.x + box.xl, box.y + box.yl };
		RECT lr = r;

		// Fix bounds
		if (lr.left == lr.right)
			lr.right++;
		if (lr.top == lr.bottom)
			lr.bottom++;
		if ((UINT)r.right > 320)
			lr.right = r.right = 320;
		if ((UINT)r.bottom > 200)
			lr.bottom = r.bottom = 200;

		sciprintf("dx_draw_filled_rect(): %08X  %i,%i -> %i,%i\n", lineColor, r.left, r.top, r.right, r.bottom);

		DODX( (dx_state->pTexVisuals[BACK_VIS]->LockRect(0, &lockedRect, &lr, 0)), dx_draw_filled_rect );
		UINT *rectPtr = (UINT*)lockedRect.pBits;

		// Going along x axis
		for (int y_pixel = r.top; y_pixel < r.bottom; y_pixel++)
		{
			UINT *startLine = rectPtr;
			for (int x_pixel = r.left; x_pixel < r.right; x_pixel++)
			{
				*rectPtr = lineColor;
				rectPtr++;
			}
			rectPtr = startLine;
			rectPtr += 320;
		}

		DODX( (dx_state->pTexVisuals[BACK_VIS]->UnlockRect(0)), dx_draw_filled_rect );
	}

	if (color1.mask & GFX_MASK_PRIORITY)
	{
		byte *pri;
		pri = dx_state->priority_maps[BACK_PRI]->index_data + box.x + box.y*(320);
		for(int i=0; i<box.yl; i++)
		{
			memset(pri,color1.priority,box.xl);
			pri += 320;
		}
	}

	return GFX_OK;
}


// Draws a single line to the back buffer
static int
dx_draw_line(struct _gfx_driver *drv,
			point_t start, point_t end,
			gfx_color_t color,
			gfx_line_mode_t line_mode, gfx_line_style_t line_style)
{
	if (color.mask & GFX_MASK_VISUAL) {

		rect_t line_rect = { start.x, start.y, end.x - start.x + 1, end.y - start.y + 1 };

		sciprintf("dx_draw_line(): %i,%i -> %i,%i\n", line_rect.x, line_rect.y, line_rect.xl + line_rect.x, line_rect.yl + line_rect.y);

		// Make sure priorities are not updated in dx_draw_filled_rect()
		gfx_color_t col = color;
		col.mask = GFX_MASK_VISUAL;

		dx_draw_filled_rect(drv, line_rect, col, col, GFX_SHADE_FLAT);
	}

	if (color.mask & GFX_MASK_PRIORITY) {
		gfx_draw_line_pixmap_i(dx_state->priority_maps[BACK_PRI], start, end, color.priority);
	}

	return GFX_OK;
}


///// PIXMAPS

// Register the pixmap as a texture
static int
dx_register_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm)
{
	HRESULT hr;

	int i, xs;
	byte *s, *d;
	D3DLOCKED_RECT lockedRect;
	LPDIRECT3DTEXTURE8 newTex;
	DODX( (dx_state->pDevice->CreateTexture(pxm->xl, pxm->yl, 1, 0, dx_state->d3dFormat, D3DPOOL_MANAGED, &newTex )), dx_register_pixmap );

	// Do gfx crossblit
	DODX( (newTex->LockRect(0, &lockedRect, NULL, 0)), dx_register_pixmap );
	s = pxm->data;
	d = (byte *) lockedRect.pBits;
	xs = drv->mode->bytespp * pxm->xl;

	for(i = 0; i < pxm->yl; i++)
	{
		memcpy(d, s, xs);
		s += xs;
		d += lockedRect.Pitch;
	}
	DODX( (newTex->UnlockRect(0)), dx_register_pixmap );

	pxm->internal.info = newTex;
	pxm->internal.handle = SCI_DX_HANDLE_NORMAL;

	return GFX_OK;
}


// Unregister the pixmap
static int
dx_unregister_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm)
{
	SAFE_RELEASE((LPDIRECT3DTEXTURE8) (pxm->internal.info));
	pxm->internal.info = NULL;

	return GFX_OK;
}


// Draws part of a pixmap to the static or back buffer
static int
dx_draw_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm, int priority,
		      rect_t src, rect_t dest, gfx_buffer_t buffer)
{
	HRESULT hr;
	int bufnr = (buffer == GFX_BUFFER_STATIC) ? 2 : 1;
	int pribufnr = bufnr - 1;
	LPDIRECT3DTEXTURE8 srct, dstt;
	LPDIRECT3DSURFACE8 sbuf, dbuf;
	D3DLOCKED_RECT lockedRect;
	byte *pri_map = NULL;

	if (pxm->internal.handle == SCI_DX_HANDLE_GRABBED) {
		// copy from pxm->internal.info to visual[bufnr]
		RECT srcRect = RECT_T_TO_RECT(src);
		POINT dstPoint = { dest.x, dest.y };

		srct = (LPDIRECT3DTEXTURE8) (pxm->internal.info);
		dstt = dx_state->pTexVisuals[bufnr];

		DODX( (srct->GetSurfaceLevel(0, &sbuf)), dx_draw_pixmap );
		DODX( (dstt->GetSurfaceLevel(0, &dbuf)), dx_draw_pixmap );
		DODX( (dx_state->pDevice->CopyRects(sbuf, &srcRect, 1, dbuf, &dstPoint)), dx_draw_pixmap );

		SAFE_RELEASE(sbuf);
		SAFE_RELEASE(dbuf);

		return GFX_OK;
	}


	// Create texture to temporarily hold visuals[bufnr]
	LPDIRECT3DTEXTURE8 temp;
	DODX( (dx_state->pDevice->CreateTexture(pxm->xl, pxm->yl, 1, 0, dx_state->d3dFormat, D3DPOOL_MANAGED, &temp)), dx_draw_pixmap );
	RECT srcRect = RECT_T_TO_RECT(dest);
	RECT destRect = { 0, 0, dest.xl, dest.yl };
	POINT dstPoint = { destRect.left, destRect.top };

	// Copy from visuals[bufnr] to temp
	srct = dx_state->pTexVisuals[bufnr];
	dstt = temp;
	DODX( (srct->GetSurfaceLevel(0, &sbuf)), dx_draw_pixmap );
	DODX( (dstt->GetSurfaceLevel(0, &dbuf)), dx_draw_pixmap );
	DODX( (dx_state->pDevice->CopyRects(sbuf, &srcRect, 1, dbuf, &dstPoint)), dx_draw_pixmap );

	// Copy from given pixmap to temp
	DODX( (dbuf->LockRect(&lockedRect, &destRect, 0)), dx_draw_pixmap );
	gfx_crossblit_pixmap(drv->mode, pxm, priority, src, dest,
		       (byte *) lockedRect.pBits, lockedRect.Pitch,
		       dx_state->priority_maps[pribufnr]->index_data,
		       dx_state->priority_maps[pribufnr]->index_xl, 1,
		       GFX_CROSSBLIT_FLAG_DATA_IS_HOMED);
	DODX( (dbuf->UnlockRect()), dx_draw_pixmap );

	SAFE_RELEASE(sbuf);
	SAFE_RELEASE(dbuf);


	// Copy from temp to visuals[bufnr]
	RECT src2Rect = { 0, 0, dest.xl, dest.yl };
	POINT dst2Point = { dest.x, dest.y };

	srct = temp;
	dstt = dx_state->pTexVisuals[bufnr];
	DODX( (srct->GetSurfaceLevel(0, &sbuf)), dx_draw_pixmap );
	DODX( (dstt->GetSurfaceLevel(0, &dbuf)), dx_draw_pixmap );
	DODX( (dx_state->pDevice->CopyRects(sbuf, &src2Rect, 1, dbuf, &dst2Point)), dx_draw_pixmap );

	SAFE_RELEASE(sbuf);
	SAFE_RELEASE(dbuf);
	SAFE_RELEASE(temp);

	return GFX_OK;
}


// Grabs an image from the visual or priority back buffer
static int
dx_grab_pixmap(struct _gfx_driver *drv, rect_t src, gfx_pixmap_t *pxm,
		      gfx_map_mask_t map)
{
	HRESULT hr;

	if (src.x < 0 || src.y < 0) {
		GFXERROR("Attempt to grab pixmap from invalid coordinates (%d,%d)\n", src.x, src.y);
		return GFX_ERROR;
	}

	if (!pxm->data) {
		GFXERROR("Attempt to grab pixmap to unallocated memory\n");
		return GFX_ERROR;
	}

	// Choose map to grab from
	switch (map) {

	case GFX_MASK_VISUAL: {
		LPDIRECT3DTEXTURE8 temp;
		LPDIRECT3DSURFACE8 tempSrf, backSrf;
		CONST RECT srcRect = RECT_T_TO_RECT(src);
		CONST POINT dstPoint = { 0, 0 };

		pxm->xl = src.xl;
		pxm->yl = src.yl;

		DODX( (dx_state->pDevice->CreateTexture(pxm->xl, pxm->yl, 1, 0, dx_state->d3dFormat, D3DPOOL_MANAGED, &temp)), dx_grab_pixmap );

		DODX( (dx_state->pTexVisuals[BACK_VIS]->GetSurfaceLevel(0, &backSrf)), dx_grab_pixmap );
		DODX( (temp->GetSurfaceLevel(0, &tempSrf)), dx_grab_pixmap );
		DODX( (dx_state->pDevice->CopyRects(backSrf, &srcRect, 1, tempSrf, &dstPoint)), dx_grab_pixmap );

		pxm->internal.info = temp;
		pxm->internal.handle = SCI_DX_HANDLE_GRABBED;
		pxm->flags |= GFX_PIXMAP_FLAG_INSTALLED | GFX_PIXMAP_FLAG_EXTERNAL_PALETTE | GFX_PIXMAP_FLAG_PALETTE_SET;

		SAFE_RELEASE(backSrf);
		SAFE_RELEASE(tempSrf);

		break;
	}

	case GFX_MASK_PRIORITY:
		sciprintf("FIXME: priority map grab not implemented yet!\n");
		break;

	default:
		sciprintf("Attempt to grab pixmap from invalid map 0x%02x\n", map);
		return GFX_ERROR;
	}

	return GFX_OK;
}


///// BUFFERING

// Updates the front buffer or the back buffers
static int
dx_update(struct _gfx_driver *drv, rect_t src, point_t dest, gfx_buffer_t buffer)
{
	HRESULT hr;
	LPDIRECT3DTEXTURE8 srct, dstt;
	LPDIRECT3DSURFACE8 sbuf, dbuf;
	CONST RECT srcRect = RECT_T_TO_RECT(src);
	CONST POINT dstPoint = { dest.x, dest.y };

	switch (buffer) {

	case GFX_BUFFER_FRONT:
		srct = dx_state->pTexVisuals[BACK_VIS];
		dstt = dx_state->pTexVisuals[PRIMARY_VIS];

		DODX( (srct->GetSurfaceLevel(0, &sbuf)), dx_update );
		DODX( (dstt->GetSurfaceLevel(0, &dbuf)), dx_update );

		DODX( (dx_state->pDevice->CopyRects(sbuf, &srcRect, 1, dbuf, &dstPoint)), dx_update );

		SAFE_RELEASE(sbuf);
		SAFE_RELEASE(dbuf);

		RenderD3D(drv);
		break;

	case GFX_BUFFER_BACK:
		if (src.x == dest.x && src.y == dest.y)
			gfx_copy_pixmap_box_i(dx_state->priority_maps[BACK_PRI], dx_state->priority_maps[STATIC_PRI], src);

		srct = dx_state->pTexVisuals[STATIC_VIS];
		dstt = dx_state->pTexVisuals[BACK_VIS];

		DODX( (srct->GetSurfaceLevel(0, &sbuf)), dx_update );
		DODX( (dstt->GetSurfaceLevel(0, &dbuf)), dx_update );

		DODX( (dx_state->pDevice->CopyRects(sbuf, &srcRect, 1, dbuf, &dstPoint)), dx_update );

		SAFE_RELEASE(sbuf);
		SAFE_RELEASE(dbuf);

		break;

	default:
		GFXERROR("Invalid buffer %d in update!\n", buffer);
		return GFX_ERROR;
	}

	return GFX_OK;
}


// Sets the contents of the static visual and priority buffers
static int
dx_set_static_buffer(struct _gfx_driver *drv, gfx_pixmap_t *pic,
			    gfx_pixmap_t *priority)
{
	if (!pic->internal.info) {
		GFXERROR("Attempt to set static buffer with unregistered pixmap!\n");
		return GFX_ERROR;
	}

	HRESULT hr;
	LPDIRECT3DTEXTURE8 pii = (LPDIRECT3DTEXTURE8) (pic->internal.info);
	LPDIRECT3DSURFACE8 pbf;
	LPDIRECT3DTEXTURE8 vis = dx_state->pTexVisuals[STATIC_VIS];
	LPDIRECT3DSURFACE8 vbf;

	// Copy from pic to visual[static]
	DODX( (pii->GetSurfaceLevel(0, &pbf)), dx_set_static_buffer );
	DODX( (vis->GetSurfaceLevel(0, &vbf)), dx_set_static_buffer );
	DODX( (dx_state->pDevice->CopyRects(pbf, NULL, 0, vbf, NULL)), dx_set_static_buffer );

	SAFE_RELEASE(pbf);
	SAFE_RELEASE(vbf);

	// Copy priority map
	gfx_copy_pixmap_box_i(dx_state->priority_maps[STATIC_PRI], priority, gfx_rect(0, 0, 320, 200));

	return GFX_OK;
}


///// MOUSE

// Sets a new mouse pointer
static int
dx_set_pointer(struct _gfx_driver *drv, gfx_pixmap_t *pointer)
{
	HRESULT hr;

	if (pointer->data == NULL)
		return GFX_OK;
	pointer->yl /= 2;	// Scale the pointer to compensate for mode scale

	LPDIRECT3DTEXTURE8 pntTex;
	LPDIRECT3DSURFACE8 pntSrf;

	// Get pointer dimensions and init
	POINTS pDims = {pointer->xl, pointer->yl};
	dx_state->pointerDims = pDims;
	RECT r = {0, 0, pointer->xl, pointer->yl};

	// Recreate pointer data according to the graphics mode we need
	gfx_pixmap_free_data(pointer);
	gfx_xlate_pixmap(pointer, dx_state->pointerMode, GFX_XLATE_FILTER_NONE);

	// Create texture and fill with pointer data
	DODX( (dx_state->pDevice->CreateTexture(pointer->xl, pointer->yl, 1, 0, dx_state->d3dFormat, D3DPOOL_MANAGED, &pntTex )), dx_set_pointer );
	DODX( (pntTex->GetSurfaceLevel(0, &pntSrf)), dx_set_pointer );

	DODX( (D3DXLoadSurfaceFromMemory(pntSrf, NULL, &r, pointer->data, dx_state->d3dFormat, 256, NULL, &r, D3DX_FILTER_NONE, 0)), dx_set_pointer);

	SAFE_RELEASE(pntSrf);

	// Assign as current pointer texture
	if (dx_state->pTexPointer)
		SAFE_RELEASE(dx_state->pTexPointer);
	dx_state->pTexPointer = pntTex;

	return GFX_OK;
}


// Display mouse pointer
static int
show_pointer(struct _gfx_driver *drv, LPARAM pos)
{
	POINTS mousePos;	// mouse coordinates

	// Sort out coordinates
	mousePos = MAKEPOINTS(pos);

	// Update pos
	drv->pointer_x = mousePos.x;
	drv->pointer_y = mousePos.y;

	RenderD3D(drv);

	return GFX_OK;
}


///// EVENTS

// Get event from the queue
static sci_event_t
get_queue_event(gfx_dx_struct_t *ctx)
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


// Add event to the queue
static void add_queue_event(gfx_dx_struct_t *ctx, int type, int data, short buckybits)
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


// Add keystroke event to queue
static void add_key_event (gfx_dx_struct_t *ctx, int data)
{
	short buckybits = 0;

	if (GetAsyncKeyState (VK_RSHIFT))
		buckybits |= SCI_EVM_RSHIFT;
	if (GetAsyncKeyState (VK_LSHIFT))
		buckybits |= SCI_EVM_LSHIFT;
	if (GetAsyncKeyState (VK_CONTROL))
		buckybits |= SCI_EVM_CTRL;
	if (GetAsyncKeyState (VK_MENU))
		buckybits |= SCI_EVM_ALT;
	if (GetKeyState (VK_SCROLL) & 1)
		buckybits |= SCI_EVM_SCRLOCK;
	if (GetKeyState (VK_NUMLOCK) & 1)
		buckybits |= SCI_EVM_NUMLOCK;
	if (GetKeyState (VK_CAPITAL) & 1)
		buckybits |= SCI_EVM_CAPSLOCK;

	add_queue_event (ctx, SCI_EVT_KEYBOARD, data, buckybits);
}


// Add mouse event to queue
static void add_mouse_event(gfx_dx_struct_t *ctx, int type, int data, WPARAM wParam)
{
	short buckybits = 0;
	if (wParam & MK_SHIFT)
		buckybits |= SCI_EVM_LSHIFT | SCI_EVM_RSHIFT;
	if (wParam & MK_CONTROL)
		buckybits |= SCI_EVM_CTRL;

	add_queue_event (ctx, type, data, buckybits);
}


// Returns the next event in the event queue for this driver
static sci_event_t
dx_get_event(struct _gfx_driver *drv)
{
	assert(drv->state != NULL);
	return get_queue_event(dx_state);
}


// Sleeps the specified amount of microseconds, or until the mouse moves
static int
dx_usleep(struct _gfx_driver *drv, long usecs)
{
	if (usecs < 1000)
	{
		sleep(0);
	}
	else
	{
		sleep(usecs/1000);
	}
	ProcessMessages(drv);

	return GFX_OK;
}


// Process any Windows messages
static int
ProcessMessages(struct _gfx_driver *drv)
{
	MSG msg;

	while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
		switch( msg.message )
		{
			case WM_PAINT:
				ValidateRect( dx_state->hWnd, NULL );
				RenderD3D(drv);
				break;

			case WM_KEYDOWN:
				switch (msg.wParam)
				{
					MAP_KEY (VK_ESCAPE,		SCI_K_ESC);
					MAP_KEY (VK_BACK,		SCI_K_BACKSPACE);
					MAP_KEY (VK_RETURN,		SCI_K_ENTER);
					MAP_KEY (VK_TAB,		SCI_K_TAB);
					MAP_KEY (VK_END,		SCI_K_END);
					MAP_KEY (VK_DOWN,		SCI_K_DOWN);
					MAP_KEY (VK_NEXT,		SCI_K_PGDOWN);
					MAP_KEY (VK_LEFT,		SCI_K_LEFT);
					MAP_KEY (VK_RIGHT,		SCI_K_RIGHT);
					MAP_KEY (VK_HOME,		SCI_K_HOME);
					MAP_KEY (VK_UP,			SCI_K_UP);
					MAP_KEY (VK_PRIOR,		SCI_K_PGUP);
					MAP_KEY (VK_INSERT,		SCI_K_INSERT);
					MAP_KEY (VK_DELETE,		SCI_K_DELETE);
					MAP_KEY (VK_DECIMAL,	SCI_K_DELETE);
					/* TODO: Glutton no longer had SCI_K etc... declared
					MAP_KEY (VK_ADD,		SCI_K_PLUS);
					MAP_KEY (VK_OEM_PLUS,	SCI_K_EQUALS);
					MAP_KEY (VK_SUBTRACT,	SCI_K_MINUS);
					MAP_KEY (VK_OEM_MINUS,	SCI_K_MINUS);
					MAP_KEY (VK_MULTIPLY,	SCI_K_MULTIPLY);
					MAP_KEY (VK_DIVIDE,		SCI_K_DIVIDE);
					*/
					MAP_KEY (VK_OEM_COMMA,	',');
					MAP_KEY (VK_OEM_PERIOD,	'.');
					MAP_KEY (VK_OEM_1,		';');	// US keyboards only
					MAP_KEY (VK_OEM_2,		'/');
					MAP_KEY (VK_OEM_3,		'`');
					MAP_KEY (VK_OEM_4,		'[');
					MAP_KEY (VK_OEM_5,		'\\');
					MAP_KEY (VK_OEM_6,		']');
					MAP_KEY (VK_OEM_7,		'\'');
					MAP_KEY (VK_F1,			SCI_K_F1);
					MAP_KEY (VK_F2,			SCI_K_F2);
					MAP_KEY (VK_F3,			SCI_K_F3);
					MAP_KEY (VK_F4,			SCI_K_F4);
					MAP_KEY (VK_F5,			SCI_K_F5);
					MAP_KEY (VK_F6,			SCI_K_F6);
					MAP_KEY (VK_F7,			SCI_K_F7);
					MAP_KEY (VK_F8,			SCI_K_F8);
					MAP_KEY (VK_F9,			SCI_K_F9);
					MAP_KEY (VK_F10,		SCI_K_F10);

					case VK_RSHIFT:
					case VK_LSHIFT:
					case VK_CONTROL:
					case VK_MENU:
					case VK_SCROLL:
					case VK_NUMLOCK:
					case VK_CAPITAL:
						break;	// ignore

				default:
					if (msg.wParam >= 'A' && msg.wParam <= 'Z')
						add_key_event (dx_state, msg.wParam - 'A' + 97);
					else if (msg.wParam >= VK_NUMPAD0 && msg.wParam <= VK_NUMPAD9)
					{
						if (GetKeyState (VK_NUMLOCK) & 1)
							add_key_event (dx_state, msg.wParam - VK_NUMPAD0 + '0');
						else
						switch (msg.wParam)
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
					else if (msg.wParam == 0xC0)  // tilde key - used for invoking console
						add_key_event (dx_state, '`');
					else
						add_key_event (dx_state, msg.wParam);
					break;
				}
				break;

			case WM_MOUSEMOVE:
				// Turn off mouse cursor
				ShowCursor(FALSE);
				show_pointer(drv, msg.lParam);
				break;

			case WM_MOUSELEAVE:
				// Turn on mouse cursor
				ShowCursor(TRUE);
				break;

			case WM_LBUTTONDOWN: add_mouse_event (dx_state, SCI_EVT_MOUSE_PRESS, 1, msg.wParam);   break;
			case WM_RBUTTONDOWN: add_mouse_event (dx_state, SCI_EVT_MOUSE_PRESS, 2, msg.wParam);   break;
			case WM_MBUTTONDOWN: add_mouse_event (dx_state, SCI_EVT_MOUSE_PRESS, 3, msg.wParam);   break;
			case WM_LBUTTONUP:   add_mouse_event (dx_state, SCI_EVT_MOUSE_RELEASE, 1, msg.wParam); break;
			case WM_RBUTTONUP:   add_mouse_event (dx_state, SCI_EVT_MOUSE_RELEASE, 2, msg.wParam); break;
			case WM_MBUTTONUP:   add_mouse_event (dx_state, SCI_EVT_MOUSE_RELEASE, 3, msg.wParam); break;

			case WM_DESTROY:
				PostQuitMessage( 0 );
				drv->exit(drv);
				exit(-1);
				break;

		}
	}

	return 0;
}


extern "C"
gfx_driver_t gfx_driver_dx = {
	"directx",
	"0.4.2",
	SCI_GFX_DRIVER_MAGIC,
	SCI_GFX_DRIVER_VERSION,
	NULL,	/* mode */
	0, 0,	/* mouse pointer position */
	GFX_CAPABILITY_MOUSE_SUPPORT | GFX_CAPABILITY_MOUSE_POINTER | GFX_CAPABILITY_COLOR_MOUSE_POINTER | GFX_CAPABILITY_PIXMAP_REGISTRY | GFX_CAPABILITY_WINDOWED,
	0,
	dx_set_param,
	dx_init_specific,
	dx_init,
	dx_exit,
	dx_draw_line,
	dx_draw_filled_rect,
	dx_register_pixmap,
	dx_unregister_pixmap,
	dx_draw_pixmap,
	dx_grab_pixmap,
	dx_update,
	dx_set_static_buffer,
	dx_set_pointer,
	NULL,
	dx_get_event,
	dx_usleep,
	NULL
};

#endif
