/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

//=============================================================================
//
// AVI/MPG player for AGS
// Adapted from MS DirectX Media example program to work with allegro
// 2002 Chris Jones
//
//=============================================================================

#include "core/platform.h"

#if AGS_PLATFORM_OS_WINDOWS && ! defined (AGS_NO_VIDEO_PLAYER)

//#define ALLEGRO_STATICLINK  // already defined in project settings
#include <allegro.h>
#include <winalleg.h>
#include <windows.h>
#include <stdlib.h>
#include <amstream.h>
#include <mmstream.h>   // Multimedia stream interfaces
#include <ddstream.h>   // DirectDraw multimedia stream interfaces
#include <initguid.h>   // Defines DEFINE_GUID macro and enables GUID initialization
#include "ac/draw.h"
#include "gfx/bitmap.h"
#include "gfx/graphicsdriver.h"
#include "main/game_run.h"
#include "platform/base/agsplatformdriver.h"

using namespace AGS::Shared;
using namespace AGS::Engine;

//link with the following libraries under project/settings/link...
//amstrmid.lib quartz.lib strmbase.lib ddraw.lib

extern void update_audio_system_on_game_loop();
extern void update_polled_stuff_if_runtime();
extern int ags_mgetbutton();
extern volatile char want_exit;
extern IGraphicsDriver *gfxDriver;
//int errno;
char lastError[300];

//Global variables
HWND ghWnd;
BOOL g_bAppactive = FALSE; // The window is active
bool useSound = true;
volatile bool currentlyPlaying = false;
volatile bool currentlyPaused = false;

//DirectDrawEx Global interfaces
extern "C" extern LPDIRECTDRAW2 directdraw;
//extern "C" extern IUnknown* directsound;
extern "C" extern BITMAP *gfx_directx_create_system_bitmap(int width, int height);

//Global MultiMedia streaming interfaces
IMultiMediaStream       *g_pMMStream = NULL;
IMediaStream            *g_pPrimaryVidStream = NULL;
IDirectDrawMediaStream  *g_pDDStream = NULL;
IDirectDrawStreamSample *g_pSample = NULL;

Bitmap *vscreen = NULL;
Bitmap *vsMemory = NULL;

//Function prototypes
HRESULT RenderFileToMMStream(LPCTSTR szFilename);
HRESULT InitRenderToSurface();
void RenderToSurface();

void ExitCode() {
	//Release MultiMedia streaming Objects
	if (g_pMMStream != NULL) {
		g_pMMStream->Release();
		g_pMMStream = NULL;
	}
	if (g_pSample != NULL) {
		g_pSample->Release();
		g_pSample = NULL;
	}
	if (g_pDDStream != NULL) {
		g_pDDStream->Release();
		g_pDDStream = NULL;
	}
	if (g_pPrimaryVidStream != NULL) {
		g_pPrimaryVidStream->Release();
		g_pPrimaryVidStream = NULL;
	}
}

typedef struct BMP_EXTRA_INFO {
	LPDIRECTDRAWSURFACE2 surf;
	struct BMP_EXTRA_INFO *next;
	struct BMP_EXTRA_INFO *prev;
	int flags;
	int lock_nesting;
} BMP_EXTRA_INFO;

LPDIRECTDRAWSURFACE get_bitmap_surface(Bitmap *bmp) {
	BMP_EXTRA_INFO *bei = (BMP_EXTRA_INFO *)((BITMAP *)bmp->GetAllegroBitmap())->extra;

	// convert the DDSurface2 back to a standard DDSurface
	return (LPDIRECTDRAWSURFACE)bei->surf;
}
LPDIRECTDRAWSURFACE2 get_bitmap_surface2(Bitmap *bmp) {
	BMP_EXTRA_INFO *bei = (BMP_EXTRA_INFO *)((BITMAP *)bmp->GetAllegroBitmap())->extra;

	return bei->surf;
}

//Create the stream sample which will be used to call updates on the video
HRESULT InitRenderToSurface() {

	HRESULT hr;
	DDSURFACEDESC ddsd;

	//Use the multimedia stream to get the primary video media stream
	hr = g_pMMStream->GetMediaStream(MSPID_PrimaryVideo, &g_pPrimaryVidStream);
	if (FAILED(hr)) {
		strcpy(lastError, "MMStream::GetMediaStream failed to create the primary video stream.");
		return E_FAIL;
	}

	//Use the media stream to get the IDirectDrawMediaStream
	hr = g_pPrimaryVidStream->QueryInterface(IID_IDirectDrawMediaStream, (void **)&g_pDDStream);
	if (FAILED(hr)) {
		strcpy(lastError, "The video stream does not support the IDirectDrawMediaStream interface; ensure you have the latest DirectX version installed.");
		return E_FAIL;
	}

	//Must set dwSize before calling GetFormat
	ddsd.dwSize = sizeof(ddsd);
	hr = g_pDDStream->GetFormat(&ddsd, NULL, NULL, NULL);
	if (FAILED(hr)) {
		strcpy(lastError, "IDirectDrawMediaStream::GetFormat failed");
		return E_FAIL;
	}

	RECT rect;
	rect.top = rect.left = 0;
	// these are the width and height of the video
	rect.bottom = ddsd.dwHeight;
	rect.right = ddsd.dwWidth;

	if (vscreen == NULL)
		vscreen = BitmapHelper::CreateRawBitmapOwner(gfx_directx_create_system_bitmap(ddsd.dwWidth, ddsd.dwHeight));

	if (vscreen == NULL) {
		strcpy(lastError, "Unable to create the DX Video System Bitmap");
		return E_FAIL;
	}

	vsMemory = BitmapHelper::CreateBitmap(vscreen->GetWidth(), vscreen->GetHeight(), vscreen->GetColorDepth());

	IDirectDrawSurface *g_pDDSOffscreen;
	g_pDDSOffscreen = get_bitmap_surface(vscreen);

	//Create the stream sample
	hr = g_pDDStream->CreateSample(g_pDDSOffscreen, &rect, 0, &g_pSample);
	if (FAILED(hr)) {
		strcpy(lastError, "VideoStream::CreateSample failed");
		return E_FAIL;
	}

	return NOERROR;
}

//Renders a file to a multimedia stream
HRESULT RenderFileToMMStream(LPCTSTR szFilename) {
	HRESULT hr;
	IAMMultiMediaStream *pAMStream = NULL;

	//Convert filename to Unicode
	WCHAR wFile[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, szFilename, -1, wFile,
	                    sizeof(wFile) / sizeof(wFile[0]));

	//Create the AMMultiMediaStream object
	hr = CoCreateInstance(CLSID_AMMultiMediaStream, NULL, CLSCTX_INPROC_SERVER,
	                      IID_IAMMultiMediaStream, (void **)&pAMStream);

	if (FAILED(hr)) {
		strcpy(lastError, "Could not create a CLSID_MultiMediaStream object. "
		       "Make sure you have the latest version of DirectX installed.");
		return E_FAIL;
	}

	//Initialize stream
	hr = pAMStream->Initialize(STREAMTYPE_READ, 0, NULL);
	if (FAILED(hr)) {
		strcpy(lastError, "AMStream::Initialize failed.");
		return E_FAIL;
	}
	//Add primary video stream
	hr = pAMStream->AddMediaStream(directdraw, &MSPID_PrimaryVideo, 0, NULL);
	if (FAILED(hr)) {
		strcpy(lastError, "AddMediaStream failed.");
		return E_FAIL;
	}
	//Add primary audio stream
	if (useSound) {
		//hr = pAMStream->AddMediaStream(directsound, &MSPID_PrimaryAudio, 0, NULL);
		hr = pAMStream->AddMediaStream(NULL, &MSPID_PrimaryAudio, AMMSF_ADDDEFAULTRENDERER, NULL);
		if (FAILED(hr)) {
			strcpy(lastError, "AddMediaStream failed.");
			return E_FAIL;
		}
	}
	//Opens and automatically creates a filter graph for the specified media file
	hr = pAMStream->OpenFile(wFile, 0);
	if (FAILED(hr)) {
		pAMStream->Release();
		sprintf(lastError, "File not found or format not supported: %s", szFilename);
		return E_FAIL;
	}

	//save the local stream to the global variable
	g_pMMStream = pAMStream;
	// Add a reference to the file
	//pAMStream->AddRef();

	return NOERROR;
}

int newWidth, newHeight;

//Perform frame by frame updates and blits. Set the stream
//state to STOP if there are no more frames to update.
void RenderToSurface(Bitmap *vscreen) {
	//update each frame
	if (g_pSample->Update(0, NULL, NULL, 0) != S_OK) {
		g_bAppactive = FALSE;
		g_pMMStream->SetState(STREAMSTATE_STOP);
	} else {
		g_bAppactive = TRUE;
		Bitmap *screen_bmp = gfxDriver->GetMemoryBackBuffer();
		// TODO: don't render on screen bitmap, use gfxDriver->DrawSprite instead!
		screen_bmp->Acquire();
		// Because vscreen is a DX Video Bitmap, it can be stretched
		// onto the screen (also a Video Bmp) but not onto a memory
		// bitmap (which is what "screen" is when using gfx filters)
		if (screen_bmp->IsVideoBitmap()) {
			screen_bmp->StretchBlt(vscreen,
			                       RectWH(0, 0, vscreen->GetWidth(), vscreen->GetHeight()),
			                       RectWH(screen_bmp->GetWidth() / 2 - newWidth / 2,
			                              screen_bmp->GetHeight() / 2 - newHeight / 2,
			                              newWidth, newHeight));
		} else {
			vsMemory->Blit(vscreen, 0, 0, 0, 0, vscreen->GetWidth(), vscreen->GetHeight());
			screen_bmp->StretchBlt(vsMemory,
			                       RectWH(0, 0, vscreen->GetWidth(), vscreen->GetHeight()),
			                       RectWH(screen_bmp->GetWidth() / 2 - newWidth / 2,
			                              screen_bmp->GetHeight() / 2 - newHeight / 2,
			                              newWidth, newHeight));
		}
		screen_bmp->Release();

		// if we're not playing AVI sound, poll the audio system
		if (!useSound)
			update_audio_system_on_game_loop();

		render_to_screen();
	}
}

void dxmedia_pause_video() {

	if (currentlyPlaying) {
		currentlyPaused = true;
		g_pMMStream->SetState(STREAMSTATE_STOP);
	}

}

void dxmedia_resume_video() {

	if (currentlyPlaying) {
		currentlyPaused = false;
		g_pMMStream->SetState(STREAMSTATE_RUN);
	}

}

void dxmedia_abort_video() {

	if (currentlyPlaying) {

		currentlyPlaying = false;
		g_pMMStream->SetState(STREAMSTATE_STOP);

		ExitCode();
		CoUninitialize();
		delete vscreen;
		vscreen = NULL;
		if (vsMemory != NULL) {
			delete vsMemory;
			vsMemory = NULL;
		}
		strcpy(lastError, "Played successfully.");
	}

}

int dxmedia_play_video(const char *filename, bool pUseSound, int canskip, int stretch) {
	HRESULT hr;

	useSound = pUseSound;
	ghWnd = win_get_window();

	CoInitialize(NULL);

	if (!useSound)
		update_polled_stuff_if_runtime();

	hr = RenderFileToMMStream(filename);
	if (FAILED(hr)) {
		ExitCode();
		CoUninitialize();
		return -1;
	}

	if (!useSound)
		update_polled_stuff_if_runtime();

	hr = InitRenderToSurface();
	if (FAILED(hr)) {
		ExitCode();
		CoUninitialize();
		return -1;
	}

	newWidth = vscreen->GetWidth();
	newHeight = vscreen->GetHeight();

	Bitmap *screen_bmp = gfxDriver->GetMemoryBackBuffer();

	if ((stretch == 1) ||
	        (vscreen->GetWidth() > screen_bmp->GetWidth()) ||
	        (vscreen->GetHeight() > screen_bmp->GetHeight())) {
		// If they want to stretch, or if it's bigger than the screen, then stretch
		float widthRatio = (float)vscreen->GetWidth() / (float)screen_bmp->GetWidth();
		float heightRatio = (float)vscreen->GetHeight() / (float)screen_bmp->GetHeight();

		if (widthRatio > heightRatio) {
			newWidth = vscreen->GetWidth() / widthRatio;
			newHeight = vscreen->GetHeight() / widthRatio;
		} else {
			newWidth = vscreen->GetWidth() / heightRatio;
			newHeight = vscreen->GetHeight() / heightRatio;
		}
	}

	//Now set the multimedia stream to RUN
	hr = g_pMMStream->SetState(STREAMSTATE_RUN);
	g_bAppactive = TRUE;

	if (FAILED(hr)) {
		sprintf(lastError, "Unable to play stream: 0x%08X", hr);
		ExitCode();
		CoUninitialize();
		delete vscreen;
		return -1;
	}
	// in case we're not full screen, clear the background
	screen_bmp->Clear();

	currentlyPlaying = true;

	gfxDriver->ClearDrawLists();

	while ((g_bAppactive) && (!want_exit)) {

		while (currentlyPaused) {
			platform->YieldCPU();
		}

		RenderToSurface(vscreen);
		//Sleep(0);
		int key, mbut, mwheelz;
		if (run_service_key_controls(key)) {
			if ((canskip == 1) && (key == 27))
				break;
			if (canskip >= 2)
				break;
		}
		if (run_service_mb_controls(mbut, mwheelz) && mbut >= 0 && (canskip == 3))
			break;
	}

	dxmedia_abort_video();

	return 0;
}

#if 0

int WINAPI WinMain(
    HINSTANCE hInstance,  // handle to current instance
    HINSTANCE hPrevInstance,  // handle to previous instance
    LPSTR lpCmdLine,      // pointer to command line
    int nCmdShow) {

	install_allegro(SYSTEM_AUTODETECT, &errno, atexit);

	install_keyboard();

	set_color_depth(16);
	set_gfx_mode(GFX_DIRECTX_WIN, 640, 480, 0, 0);

	set_display_switch_mode(SWITCH_BACKGROUND);

	dxmedia_play_video("f:\\download\\Seinfeld S05E04 - The Sniffing Accountant.mpg", 1, 1);
	dxmedia_play_video("f:\\download\\Family Guy S02E16 - There's Something About Paulie.mpg", 2, 1);

	return 0;
}
#endif

#endif // AGS_PLATFORM_OS_WINDOWS
