/***************************************************************************
 graphics_directx.h Copyright (C) 2008 Alexander R Angas,
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

***************************************************************************/

#include <windows.h>
#include <d3d8.h>
#include <d3dx8math.h>
#include <dxerr8.h>

extern "C" {
#include <gfx_system.h>
#include <gfx_driver.h>
#include <gfx_tools.h>
#include <assert.h>
#include <uinput.h>
#include <ctype.h>
#include <console.h> // for sciprintf
#include <sci_win32.h>
#include <sci_memory.h>
};

// Error trapping, every DirectX call should use this
#define DODX(cmd, proc)																	\
		hr = cmd;																		\
		if (hr != S_OK)	{																\
			sciprintf("%s, %i, %i, %s from %s\n", __FILE__, __LINE__, hr, #cmd, #proc);	\
			DXTrace(__FILE__, __LINE__, hr, #cmd" from "#proc, 1);						\
		}


// Easily release only allocated objects
#define SAFE_RELEASE(p)  \
	if (p)               \
		(p)->Release();


// Make it simple to access drv->state
#define dx_state ((struct gfx_dx_struct_t *)(drv->state))


// Simply map a key using add_key_event()
#define MAP_KEY(x,y) case x: add_key_event ((struct gfx_dx_struct_t *)(drv->state), y); break


#define DX_CLASS_NAME "FreeSCI DirectX Graphics"
#define DX_APP_NAME "FreeSCI"

// Vertex format
#define D3DFVF_CUSTOMVERTEX ( D3DFVF_DIFFUSE | D3DFVF_XYZRHW | D3DFVF_TEX1 )

// Vertex structure
struct CUSTOMVERTEX
{
	D3DXVECTOR4 p;	// Vertex coordinates
	DWORD colour;	// Colour
	D3DXVECTOR2 t;	// Texture coordinates
};

#define SCI_DX_HANDLE_NORMAL 0
#define SCI_DX_HANDLE_GRABBED 1

// Number of buffers for each type of texture
#define NUM_VISUAL_BUFFERS		3
#define NUM_PRIORITY_BUFFERS	2

// What each buffer references
#define PRIMARY_VIS	0
#define BACK_VIS	1
#define STATIC_VIS	2

#define BACK_PRI	0
#define STATIC_PRI	1

// Struct that holds everything
struct gfx_dx_struct_t
{
	D3DFORMAT d3dFormat;	// Colour format
	UINT adapterId;			// Adapter ID to use
	DWORD vertexProcessing;	// Hardware or software vertex processing

	LPDIRECT3D8 pD3d;					// D3D object
	D3DCAPS8 deviceCaps;				// Capabilities of device
	D3DDISPLAYMODE displayMode;			// Width and height of screen
	D3DPRESENT_PARAMETERS presParams;	// Presentation parameters
	LPDIRECT3DDEVICE8 pDevice;			// Rendering device

	LPDIRECT3DVERTEXBUFFER8 pVertBuff;	// Buffer to hold pixmap vertices
	CUSTOMVERTEX pvData[4];				// Buffer of pixmap vertex structs

	LPDIRECT3DTEXTURE8 pTexVisuals[NUM_VISUAL_BUFFERS];		// Array of visual textures
	LPDIRECT3DTEXTURE8 pTexPrioritys[NUM_PRIORITY_BUFFERS];	// Array of priority textures
	gfx_pixmap_t *priority_maps[NUM_PRIORITY_BUFFERS];		// Array of SCI priority maps

	gfx_mode_t *pointerMode;			// SCI graphics mode for pointer
	LPDIRECT3DTEXTURE8 pTexPointer;	// Mouse pointer texture
	LPD3DXSPRITE pSPointer;			// Mouse pointer sprite
	POINTS pointerDims;				// Pointer dimensions

	WNDCLASSEX wc;		// Window class
	HWND hWnd;			// Window
	UINT xfact, yfact;	// Scaling factors
	UINT bpp;			// Bits per pixel

	// Event queue
	int queue_size, queue_first, queue_last;
	sci_event_t *event_queue;
};

// Flags that may be set in the driver
#define DX_FLAGS_FULLSCREEN  1

// Initialization functions
static int
ProcessMessages(struct _gfx_driver *drv);

static gfx_return_value_t
RenderD3D(struct _gfx_driver *drv);

static int
CheckDevice(struct _gfx_driver *drv);

static gfx_return_value_t
InitWindow(struct _gfx_driver *drv, UINT width, UINT height);

static gfx_return_value_t
InitD3D(struct _gfx_driver *drv);

static gfx_return_value_t
InitScene(struct _gfx_driver *drv);
