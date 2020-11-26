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
// Allegro Interface for 3D; Software mode Allegro driver
//
//=============================================================================

#include "ags/engine/gfx/ali3dsw.h"
#include "ags/shared/core/platform.h"
#include "ags/engine/gfx/ali3dexception.h"
#include "ags/engine/gfx/gfxfilter_allegro.h"
#include "ags/engine/gfx/gfxfilter_hqx.h"
#include "ags/engine/gfx/gfx_util.h"
#include "ags/engine/main/main_allegro.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/engine/ac/timer.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/opengl/opengl.h"
#include "ags/std/algorithm.h"
#include "ags/ags.h"

namespace AGS3 {

#if AGS_DDRAW_GAMMA_CONTROL
// NOTE: this struct and variables are defined internally in Allegro
typedef struct DDRAW_SURFACE {
	LPDIRECTDRAWSURFACE2 id;
	int flags;
	int lock_nesting;
	BITMAP *parent_bmp;
	struct DDRAW_SURFACE *next;
	struct DDRAW_SURFACE *prev;
} DDRAW_SURFACE;

extern "C" extern LPDIRECTDRAW2 directdraw;
extern "C" DDRAW_SURFACE * gfx_directx_primary_surface;
#endif // AGS_DDRAW_GAMMA_CONTROL

#ifndef AGS_NO_VIDEO_PLAYER
extern int dxmedia_play_video(const char *, bool, int, int);
#endif

namespace AGS {
namespace Engine {
namespace ALSW {

using namespace Shared;

bool ALSoftwareGfxModeList::GetMode(int index, DisplayMode &mode) const {
	if (_gfxModeList && index >= 0 && index < _gfxModeList->num_modes) {
		mode.Width = _gfxModeList->mode[index].width;
		mode.Height = _gfxModeList->mode[index].height;
		mode.ColorDepth = _gfxModeList->mode[index].bpp;
		return true;
	}
	return false;
}

unsigned long _trans_alpha_blender32(unsigned long x, unsigned long y, unsigned long n);
RGB faded_out_palette[256];


ALSoftwareGraphicsDriver::ALSoftwareGraphicsDriver() {
	_tint_red = 0;
	_tint_green = 0;
	_tint_blue = 0;
	_autoVsync = false;
	//_spareTintingScreen = nullptr;
	_gfxModeList = nullptr;
#if AGS_DDRAW_GAMMA_CONTROL
	dxGammaControl = nullptr;
#endif
	_allegroScreenWrapper = nullptr;
	_origVirtualScreen = nullptr;
	virtualScreen = nullptr;
	_stageVirtualScreen = nullptr;

	// Initialize default sprite batch, it will be used when no other batch was activated
	ALSoftwareGraphicsDriver::InitSpriteBatch(0, _spriteBatchDesc[0]);
}

bool ALSoftwareGraphicsDriver::IsModeSupported(const DisplayMode &mode) {
	if (mode.Width <= 0 || mode.Height <= 0 || mode.ColorDepth <= 0) {
		set_allegro_error("Invalid resolution parameters: %d x %d x %d", mode.Width, mode.Height, mode.ColorDepth);
		return false;
	}
#if AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_MACOS
	// Everything is drawn to a virtual screen, so all resolutions are supported.
	return true;
#endif

	if (mode.Windowed) {
		return true;
	}
	if (_gfxModeList == nullptr) {
		_gfxModeList = get_gfx_mode_list(GetAllegroGfxDriverID(mode.Windowed));
	}
	if (_gfxModeList != nullptr) {
		// if a list is available, check if the mode exists. This prevents the screen flicking
		// between loads of unsupported resolutions
		for (int i = 0; i < _gfxModeList->num_modes; i++) {
			if ((_gfxModeList->mode[i].width == mode.Width) &&
				(_gfxModeList->mode[i].height == mode.Height) &&
				(_gfxModeList->mode[i].bpp == mode.ColorDepth)) {
				return true;
			}
		}
		set_allegro_error("This graphics mode is not supported");
		return false;
	}
	return true;
}

int ALSoftwareGraphicsDriver::GetDisplayDepthForNativeDepth(int native_color_depth) const {
	// TODO: check for device caps to know which depth is supported?
	if (native_color_depth > 8)
		return 32;
	return native_color_depth;
}

IGfxModeList *ALSoftwareGraphicsDriver::GetSupportedModeList(int color_depth) {
	if (_gfxModeList == nullptr) {
		_gfxModeList = get_gfx_mode_list(GetAllegroGfxDriverID(false));
	}
	if (_gfxModeList == nullptr) {
		return nullptr;
	}
	return new ALSoftwareGfxModeList(_gfxModeList);
}

PGfxFilter ALSoftwareGraphicsDriver::GetGraphicsFilter() const {
	return _filter;
}

int ALSoftwareGraphicsDriver::GetAllegroGfxDriverID(bool windowed) {
#if AGS_PLATFORM_OS_WINDOWS
	if (windowed)
		return GFX_DIRECTX_WIN;
	return GFX_DIRECTX;
#elif AGS_PLATFORM_OS_LINUX && (!defined (ALLEGRO_MAGIC_DRV))
	if (windowed)
		return GFX_XWINDOWS;
	return GFX_XWINDOWS_FULLSCREEN;
#elif AGS_PLATFORM_OS_MACOS
	if (windowed) {
		return GFX_COCOAGL_WINDOW;
	}
	return GFX_COCOAGL_FULLSCREEN;
#else
	if (windowed)
		return GFX_AUTODETECT_WINDOWED;
	return GFX_AUTODETECT_FULLSCREEN;
#endif
}

void ALSoftwareGraphicsDriver::SetGraphicsFilter(PALSWFilter filter) {
	_filter = filter;
	OnSetFilter();

	// If we already have a gfx mode set, then use the new filter to update virtual screen immediately
	CreateVirtualScreen();
}

void ALSoftwareGraphicsDriver::SetTintMethod(TintMethod method) {
	// TODO: support new D3D-style tint method
}

bool ALSoftwareGraphicsDriver::SetDisplayMode(const DisplayMode &mode, volatile int *loopTimer) {
	ReleaseDisplayMode();

	const int driver = GetAllegroGfxDriverID(mode.Windowed);

	set_color_depth(mode.ColorDepth);

	if (_initGfxCallback != nullptr)
		_initGfxCallback(nullptr);

	if (!IsModeSupported(mode) || set_gfx_mode(driver, mode.Width, mode.Height, 0, 0) != 0)
		return false;

	OnInit(loopTimer);
	OnModeSet(mode);
	// set_gfx_mode is an allegro function that creates screen bitmap;
	// following code assumes the screen is already created, therefore we should
	// ensure global bitmap wraps over existing allegro screen bitmap.
	_allegroScreenWrapper = BitmapHelper::CreateRawBitmapWrapper(screen);
	_allegroScreenWrapper->Clear();

	// If we already have a gfx filter, then use it to update virtual screen immediately
	CreateVirtualScreen();

#if AGS_DDRAW_GAMMA_CONTROL
	if (!mode.Windowed) {
		memset(&ddrawCaps, 0, sizeof(ddrawCaps));
		ddrawCaps.dwSize = sizeof(ddrawCaps);
		IDirectDraw2_GetCaps(directdraw, &ddrawCaps, NULL);

		if ((ddrawCaps.dwCaps2 & DDCAPS2_PRIMARYGAMMA) == 0) {
		} else if (IDirectDrawSurface2_QueryInterface(gfx_directx_primary_surface->id, IID_IDirectDrawGammaControl, (void **)&dxGammaControl) == 0) {
			dxGammaControl->GetGammaRamp(0, &defaultGammaRamp);
		}
	}
#endif

	return true;
}

void ALSoftwareGraphicsDriver::CreateVirtualScreen() {
	if (!IsModeSet() || !IsRenderFrameValid() || !IsNativeSizeValid() || !_filter)
		return;
	DestroyVirtualScreen();
	// Adjust clipping so nothing gets drawn outside the game frame
	_allegroScreenWrapper->SetClip(_dstRect);
	// Initialize scaling filter and receive virtual screen pointer
	// (which may or not be the same as real screen)
	_origVirtualScreen = _filter->InitVirtualScreen(_allegroScreenWrapper, _srcRect.GetSize(), _dstRect);
	// Apparently we must still create a virtual screen even if its same size and color depth,
	// because drawing sprites directly on real screen bitmap causes blinking (unless I missed something here...)
	if (_origVirtualScreen == _allegroScreenWrapper) {
		_origVirtualScreen = BitmapHelper::CreateBitmap(_srcRect.GetWidth(), _srcRect.GetHeight(), _mode.ColorDepth);
	}
	virtualScreen = _origVirtualScreen;
	_stageVirtualScreen = virtualScreen;
	// Set Allegro's screen pointer to what may be the real or virtual screen
	screen = (BITMAP *)_origVirtualScreen->GetAllegroBitmap();
}

void ALSoftwareGraphicsDriver::DestroyVirtualScreen() {
	if (_filter && _origVirtualScreen) {
		screen = (BITMAP *)_filter->ShutdownAndReturnRealScreen()->GetAllegroBitmap();
	}
	_origVirtualScreen = nullptr;
	virtualScreen = nullptr;
	_stageVirtualScreen = nullptr;
}

void ALSoftwareGraphicsDriver::ReleaseDisplayMode() {
	OnModeReleased();
	ClearDrawLists();

#if AGS_DDRAW_GAMMA_CONTROL
	if (dxGammaControl != NULL) {
		dxGammaControl->Release();
		dxGammaControl = NULL;
	}
#endif

	DestroyVirtualScreen();

	// Note this does not destroy the underlying allegro screen bitmap, only wrapper.
	delete _allegroScreenWrapper;
	_allegroScreenWrapper = nullptr;
}

bool ALSoftwareGraphicsDriver::SetNativeSize(const Size &src_size) {
	OnSetNativeSize(src_size);
	// If we already have a gfx mode and gfx filter set, then use it to update virtual screen immediately
	CreateVirtualScreen();
	return !_srcRect.IsEmpty();
}

bool ALSoftwareGraphicsDriver::SetRenderFrame(const Rect &dst_rect) {
	OnSetRenderFrame(dst_rect);
	// If we already have a gfx mode and gfx filter set, then use it to update virtual screen immediately
	CreateVirtualScreen();
	return !_dstRect.IsEmpty();
}

void ALSoftwareGraphicsDriver::ClearRectangle(int x1, int y1, int x2, int y2, RGB *colorToUse) {
	if (!_filter) return;
	int color = 0;
	if (colorToUse != nullptr)
		color = makecol_depth(_mode.ColorDepth, colorToUse->r, colorToUse->g, colorToUse->b);
	// NOTE: filter will do coordinate scaling for us
	_filter->ClearRect(x1, y1, x2, y2, color);
}

ALSoftwareGraphicsDriver::~ALSoftwareGraphicsDriver() {
	ALSoftwareGraphicsDriver::UnInit();
}

void ALSoftwareGraphicsDriver::UnInit() {
	OnUnInit();
	ReleaseDisplayMode();

	if (_gfxModeList != nullptr) {
		destroy_gfx_mode_list(_gfxModeList);
		_gfxModeList = nullptr;
	}
}

bool ALSoftwareGraphicsDriver::SupportsGammaControl() {
#if AGS_DDRAW_GAMMA_CONTROL

	if (dxGammaControl != NULL) {
		return 1;
	}

#endif

	return 0;
}

void ALSoftwareGraphicsDriver::SetGamma(int newGamma) {
#if AGS_DDRAW_GAMMA_CONTROL
	for (int i = 0; i < 256; i++) {
		int newValue = ((int)defaultGammaRamp.red[i] * newGamma) / 100;
		if (newValue >= 65535)
			newValue = 65535;
		gammaRamp.red[i] = newValue;
		gammaRamp.green[i] = newValue;
		gammaRamp.blue[i] = newValue;
	}

	dxGammaControl->SetGammaRamp(0, &gammaRamp);
#endif
}

int ALSoftwareGraphicsDriver::GetCompatibleBitmapFormat(int color_depth) {
	return color_depth;
}

IDriverDependantBitmap *ALSoftwareGraphicsDriver::CreateDDBFromBitmap(Bitmap *bitmap, bool hasAlpha, bool opaque) {
	ALSoftwareBitmap *newBitmap = new ALSoftwareBitmap(bitmap, opaque, hasAlpha);
	return newBitmap;
}

void ALSoftwareGraphicsDriver::UpdateDDBFromBitmap(IDriverDependantBitmap *bitmapToUpdate, Bitmap *bitmap, bool hasAlpha) {
	ALSoftwareBitmap *alSwBmp = (ALSoftwareBitmap *)bitmapToUpdate;
	alSwBmp->_bmp = bitmap;
	alSwBmp->_hasAlpha = hasAlpha;
}

void ALSoftwareGraphicsDriver::DestroyDDB(IDriverDependantBitmap *bitmap) {
	delete bitmap;
}

void ALSoftwareGraphicsDriver::InitSpriteBatch(size_t index, const SpriteBatchDesc &desc) {
	if (_spriteBatches.size() <= index)
		_spriteBatches.resize(index + 1);
	ALSpriteBatch &batch = _spriteBatches[index];
	batch.List.clear();
	// TODO: correct offsets to have pre-scale (source) and post-scale (dest) offsets!
	const int src_w = desc.Viewport.GetWidth() / desc.Transform.ScaleX;
	const int src_h = desc.Viewport.GetHeight() / desc.Transform.ScaleY;
	// Surface was prepared externally (common for room cameras)
	if (desc.Surface != nullptr) {
		batch.Surface = std::static_pointer_cast<Bitmap>(desc.Surface);
		batch.Opaque = true;
		batch.IsVirtualScreen = false;
	}
	// In case something was not initialized
	else if (desc.Viewport.IsEmpty() || !virtualScreen) {
		batch.Surface.reset();
		batch.Opaque = false;
		batch.IsVirtualScreen = false;
	}
	// Drawing directly on a viewport without transformation (other than offset)
	else if (desc.Transform.ScaleX == 1.f && desc.Transform.ScaleY == 1.f) {
		if (!batch.Surface || !batch.IsVirtualScreen || batch.Surface->GetWidth() != src_w || batch.Surface->GetHeight() != src_h) {
			Rect rc = RectWH(desc.Viewport.Left, desc.Viewport.Top, desc.Viewport.GetWidth(), desc.Viewport.GetHeight());
			batch.Surface.reset(BitmapHelper::CreateSubBitmap(virtualScreen, rc));
		}
		batch.Opaque = true;
		batch.IsVirtualScreen = true;
	}
	// No surface prepared and has transformation other than offset
	else if (!batch.Surface || batch.IsVirtualScreen || batch.Surface->GetWidth() != src_w || batch.Surface->GetHeight() != src_h) {
		batch.Surface.reset(new Bitmap(src_w, src_h));
		batch.Opaque = false;
		batch.IsVirtualScreen = false;
	}
}

void ALSoftwareGraphicsDriver::ResetAllBatches() {
	for (ALSpriteBatches::iterator it = _spriteBatches.begin(); it != _spriteBatches.end(); ++it)
		it->List.clear();
}

void ALSoftwareGraphicsDriver::DrawSprite(int x, int y, IDriverDependantBitmap *bitmap) {
	_spriteBatches[_actSpriteBatch].List.push_back(ALDrawListEntry((ALSoftwareBitmap *)bitmap, x, y));
}

void ALSoftwareGraphicsDriver::SetScreenFade(int red, int green, int blue) {
	// TODO: was not necessary atm
}

void ALSoftwareGraphicsDriver::SetScreenTint(int red, int green, int blue) {
	_tint_red = red;
	_tint_green = green;
	_tint_blue = blue;
	if (((_tint_red > 0) || (_tint_green > 0) || (_tint_blue > 0)) && (_mode.ColorDepth > 8)) {
		_spriteBatches[_actSpriteBatch].List.push_back(ALDrawListEntry((ALSoftwareBitmap *)0x1, 0, 0));
	}
}

void ALSoftwareGraphicsDriver::RenderToBackBuffer() {
	// Render all the sprite batches with necessary transformations
	//
	// NOTE: that's not immediately clear whether it would be faster to first draw upon a camera-sized
	// surface then stretch final result to the viewport on screen, or stretch-blit each individual
	// sprite right onto screen bitmap. We'd need to do proper profiling to know that.
	// An important thing is that Allegro does not provide stretching functions for drawing sprites
	// with blending and translucency; it seems you'd have to first stretch the original sprite onto a
	// temp buffer and then TransBlendBlt / LitBlendBlt it to the final destination. Of course, doing
	// that here would slow things down significantly, so if we ever go that way sprite caching will
	// be required (similarily to how AGS caches flipped/scaled object sprites now for).
	//
	for (size_t i = 0; i <= _actSpriteBatch; ++i) {
		const Rect &viewport = _spriteBatchDesc[i].Viewport;
		const SpriteTransform &transform = _spriteBatchDesc[i].Transform;
		const ALSpriteBatch &batch = _spriteBatches[i];

		virtualScreen->SetClip(viewport);
		Bitmap *surface = batch.Surface.get();
		const int view_offx = viewport.Left;
		const int view_offy = viewport.Top;
		if (surface) {
			if (!batch.Opaque)
				surface->ClearTransparent();
			_stageVirtualScreen = surface;
			RenderSpriteBatch(batch, surface, transform.X, transform.Y);
			if (!batch.IsVirtualScreen)
				virtualScreen->StretchBlt(surface, RectWH(view_offx, view_offy, viewport.GetWidth(), viewport.GetHeight()),
					batch.Opaque ? kBitmap_Copy : kBitmap_Transparency);
		} else {
			RenderSpriteBatch(batch, virtualScreen, view_offx + transform.X, view_offy + transform.Y);
		}
		_stageVirtualScreen = virtualScreen;
	}
	ClearDrawLists();
}

void ALSoftwareGraphicsDriver::RenderSpriteBatch(const ALSpriteBatch &batch, Shared::Bitmap *surface, int surf_offx, int surf_offy) {
	const std::vector<ALDrawListEntry> &drawlist = batch.List;
	for (size_t i = 0; i < drawlist.size(); i++) {
		if (drawlist[i].bitmap == nullptr) {
			if (_nullSpriteCallback)
				_nullSpriteCallback(drawlist[i].x, drawlist[i].y);
			else
				error("Unhandled attempt to draw null sprite");

			continue;
		} else if (drawlist[i].bitmap == (ALSoftwareBitmap *)0x1) {
			// draw screen tint fx
			set_trans_blender(_tint_red, _tint_green, _tint_blue, 0);
			surface->LitBlendBlt(surface, 0, 0, 128);
			continue;
		}

		ALSoftwareBitmap *bitmap = drawlist[i].bitmap;
		int drawAtX = drawlist[i].x + surf_offx;
		int drawAtY = drawlist[i].y + surf_offy;

		if (bitmap->_transparency >= 255) {
		} // fully transparent, do nothing
		else if ((bitmap->_opaque) && (bitmap->_bmp == surface) && (bitmap->_transparency == 0)) {
		} else if (bitmap->_opaque) {
			surface->Blit(bitmap->_bmp, 0, 0, drawAtX, drawAtY, bitmap->_bmp->GetWidth(), bitmap->_bmp->GetHeight());
			// TODO: we need to also support non-masked translucent blend, but...
			// Allegro 4 **does not have such function ready** :( (only masked blends, where it skips magenta pixels);
			// I am leaving this problem for the future, as coincidentally software mode does not need this atm.
		} else if (bitmap->_hasAlpha) {
			if (bitmap->_transparency == 0) // no global transparency, simple alpha blend
				set_alpha_blender();
			else
				// here _transparency is used as alpha (between 1 and 254)
				set_blender_mode(nullptr, nullptr, _trans_alpha_blender32, 0, 0, 0, bitmap->_transparency);

			surface->TransBlendBlt(bitmap->_bmp, drawAtX, drawAtY);
		} else {
			// here _transparency is used as alpha (between 1 and 254), but 0 means opaque!
			GfxUtil::DrawSpriteWithTransparency(surface, bitmap->_bmp, drawAtX, drawAtY,
				bitmap->_transparency ? bitmap->_transparency : 255);
		}
	}
	// NOTE: following is experimental tint code (currently unused)
	/*  This alternate method gives the correct (D3D-style) result, but is just too slow!
		if ((_spareTintingScreen != NULL) &&
			((_spareTintingScreen->GetWidth() != surface->GetWidth()) || (_spareTintingScreen->GetHeight() != surface->GetHeight())))
		{
		  destroy_bitmap(_spareTintingScreen);
		  _spareTintingScreen = NULL;
		}
		if (_spareTintingScreen == NULL)
		{
		  _spareTintingScreen = BitmapHelper::CreateBitmap_(GetColorDepth(surface), surface->GetWidth(), surface->GetHeight());
		}
		tint_image(surface, _spareTintingScreen, _tint_red, _tint_green, _tint_blue, 100, 255);
		Blit(_spareTintingScreen, surface, 0, 0, 0, 0, _spareTintingScreen->GetWidth(), _spareTintingScreen->GetHeight());*/
}

void ALSoftwareGraphicsDriver::Render(int xoff, int yoff, GlobalFlipType flip) {
	RenderToBackBuffer();

	if (_autoVsync)
		this->Vsync();

	if (flip == kFlip_None)
		_filter->RenderScreen(virtualScreen, xoff, yoff);
	else
		_filter->RenderScreenFlipped(virtualScreen, xoff, yoff, flip);
}

void ALSoftwareGraphicsDriver::Render() {
	Render(0, 0, kFlip_None);
}

void ALSoftwareGraphicsDriver::Vsync() {
	vsync();
}

Bitmap *ALSoftwareGraphicsDriver::GetMemoryBackBuffer() {
	return virtualScreen;
}

void ALSoftwareGraphicsDriver::SetMemoryBackBuffer(Bitmap *backBuffer) {
	if (backBuffer) {
		virtualScreen = backBuffer;
	} else {
		virtualScreen = _origVirtualScreen;
	}
	_stageVirtualScreen = virtualScreen;

	// Reset old virtual screen's subbitmaps
	for (auto &batch : _spriteBatches) {
		if (batch.IsVirtualScreen)
			batch.Surface.reset();
	}
}

Bitmap *ALSoftwareGraphicsDriver::GetStageBackBuffer() {
	return _stageVirtualScreen;
}

bool ALSoftwareGraphicsDriver::GetCopyOfScreenIntoBitmap(Bitmap *destination, bool at_native_res, GraphicResolution *want_fmt) {
	(void)at_native_res; // software driver always renders at native resolution at the moment
	// software filter is taught to copy to any size
	if (destination->GetColorDepth() != _mode.ColorDepth) {
		if (want_fmt)
			*want_fmt = GraphicResolution(destination->GetWidth(), destination->GetHeight(), _mode.ColorDepth);
		return false;
	}
	_filter->GetCopyOfScreenIntoBitmap(destination);
	return true;
}

/**
	fade.c - High Color Fading Routines

	Last Revision: 21 June, 2002

	Author: Matthew Leverton
**/
void ALSoftwareGraphicsDriver::highcolor_fade_in(Bitmap *vs, void(*draw_callback)(), int offx, int offy, int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) {
	Bitmap *bmp_orig = vs;
	const int col_depth = bmp_orig->GetColorDepth();
	const int clearColor = makecol_depth(col_depth, targetColourRed, targetColourGreen, targetColourBlue);
	if (speed <= 0) speed = 16;

	Bitmap *bmp_buff = new Bitmap(bmp_orig->GetWidth(), bmp_orig->GetHeight(), col_depth);
	SetMemoryBackBuffer(bmp_buff);
	for (int a = 0; a < 256; a += speed) {
		bmp_buff->Fill(clearColor);
		set_trans_blender(0, 0, 0, a);
		bmp_buff->TransBlendBlt(bmp_orig, 0, 0);
		if (draw_callback) {
			draw_callback();
			RenderToBackBuffer();
		}
		this->Vsync();
		_filter->RenderScreen(bmp_buff, offx, offy);
		if (_pollingCallback)
			_pollingCallback();
		WaitForNextFrame();
	}
	delete bmp_buff;

	SetMemoryBackBuffer(vs);
	if (draw_callback) {
		draw_callback();
		RenderToBackBuffer();
	}
	_filter->RenderScreen(vs, offx, offy);
}

void ALSoftwareGraphicsDriver::highcolor_fade_out(Bitmap *vs, void(*draw_callback)(), int offx, int offy, int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) {
	Bitmap *bmp_orig = vs;
	const int col_depth = vs->GetColorDepth();
	const int clearColor = makecol_depth(col_depth, targetColourRed, targetColourGreen, targetColourBlue);
	if (speed <= 0) speed = 16;

	Bitmap *bmp_buff = new Bitmap(bmp_orig->GetWidth(), bmp_orig->GetHeight(), col_depth);
	SetMemoryBackBuffer(bmp_buff);
	for (int a = 255 - speed; a > 0; a -= speed) {
		bmp_buff->Fill(clearColor);
		set_trans_blender(0, 0, 0, a);
		bmp_buff->TransBlendBlt(bmp_orig, 0, 0);
		if (draw_callback) {
			draw_callback();
			RenderToBackBuffer();
		}
		this->Vsync();
		_filter->RenderScreen(bmp_buff, offx, offy);
		if (_pollingCallback)
			_pollingCallback();
		WaitForNextFrame();
	}
	delete bmp_buff;

	SetMemoryBackBuffer(vs);
	vs->Clear(clearColor);
	if (draw_callback) {
		draw_callback();
		RenderToBackBuffer();
	}
	_filter->RenderScreen(vs, offx, offy);
}
/** END FADE.C **/

// palette fading routiens
// from allegro, modified for mp3
void initialize_fade_256(int r, int g, int b) {
	int a;
	for (a = 0; a < 256; a++) {
		faded_out_palette[a].r = r / 4;
		faded_out_palette[a].g = g / 4;
		faded_out_palette[a].b = b / 4;
	}
}

void ALSoftwareGraphicsDriver::__fade_from_range(PALETTE source, PALETTE dest, int speed, int from, int to) {
	PALETTE temp;
	int c;

	for (c = 0; c < PAL_SIZE; c++)
		temp[c] = source[c];

	for (c = 0; c < 64; c += speed) {
		fade_interpolate(source, dest, temp, c, from, to);
		set_palette_range(temp, from, to, TRUE);
		if (_pollingCallback) _pollingCallback();
		set_palette_range(temp, from, to, TRUE);
	}

	set_palette_range(dest, from, to, TRUE);
}

void ALSoftwareGraphicsDriver::__fade_out_range(int speed, int from, int to, int targetColourRed, int targetColourGreen, int targetColourBlue) {
	PALETTE temp;

	initialize_fade_256(targetColourRed, targetColourGreen, targetColourBlue);
	get_palette(temp);
	__fade_from_range(temp, faded_out_palette, speed, from, to);
}

void ALSoftwareGraphicsDriver::FadeOut(int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) {
	if (_mode.ColorDepth > 8) {
		highcolor_fade_out(virtualScreen, _drawPostScreenCallback, 0, 0, speed * 4, targetColourRed, targetColourGreen, targetColourBlue);
	} else {
		__fade_out_range(speed, 0, 255, targetColourRed, targetColourGreen, targetColourBlue);
	}
}

void ALSoftwareGraphicsDriver::FadeIn(int speed, PALETTE p, int targetColourRed, int targetColourGreen, int targetColourBlue) {
	if (_drawScreenCallback) {
		_drawScreenCallback();
		RenderToBackBuffer();
	}
	if (_mode.ColorDepth > 8) {
		highcolor_fade_in(virtualScreen, _drawPostScreenCallback, 0, 0, speed * 4, targetColourRed, targetColourGreen, targetColourBlue);
	} else {
		initialize_fade_256(targetColourRed, targetColourGreen, targetColourBlue);
		__fade_from_range(faded_out_palette, p, speed, 0, 255);
	}
}

void ALSoftwareGraphicsDriver::BoxOutEffect(bool blackingOut, int speed, int delay) {
	if (blackingOut) {
		int yspeed = _srcRect.GetHeight() / (_srcRect.GetWidth() / speed);
		int boxwid = speed, boxhit = yspeed;
		Bitmap *bmp_orig = virtualScreen;
		Bitmap *bmp_buff = new Bitmap(bmp_orig->GetWidth(), bmp_orig->GetHeight(), bmp_orig->GetColorDepth());
		SetMemoryBackBuffer(bmp_buff);

		while (boxwid < _srcRect.GetWidth()) {
			boxwid += speed;
			boxhit += yspeed;
			int vcentre = _srcRect.GetHeight() / 2;
			bmp_orig->FillRect(Rect(_srcRect.GetWidth() / 2 - boxwid / 2, vcentre - boxhit / 2,
				_srcRect.GetWidth() / 2 + boxwid / 2, vcentre + boxhit / 2), 0);
			bmp_buff->Fill(0);
			bmp_buff->Blit(bmp_orig);
			if (_drawPostScreenCallback) {
				_drawPostScreenCallback();
				RenderToBackBuffer();
			}
			this->Vsync();
			_filter->RenderScreen(bmp_buff, 0, 0);

			if (_pollingCallback)
				_pollingCallback();

			platform->Delay(delay);
		}
		delete bmp_buff;
		SetMemoryBackBuffer(bmp_orig);
	} else {
		error("BoxOut fade-in not implemented in sw gfx driver");
	}
}
// end fading routines

#ifndef AGS_NO_VIDEO_PLAYER

bool ALSoftwareGraphicsDriver::PlayVideo(const char *filename, bool useAVISound, VideoSkipType skipType, bool stretchToFullScreen) {
#if AGS_PLATFORM_OS_WINDOWS
	int result = dxmedia_play_video(filename, useAVISound, skipType, stretchToFullScreen ? 1 : 0);
	return (result == 0);
#else
	return 0;
#endif
}

#endif

// add the alpha values together, used for compositing alpha images
unsigned long _trans_alpha_blender32(unsigned long x, unsigned long y, unsigned long n) {
	unsigned long res, g;

	n = (n * geta32(x)) / 256;

	if (n)
		n++;

	res = ((x & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
	y &= 0xFF00;
	x &= 0xFF00;
	g = (x - y) * n / 256 + y;

	res &= 0xFF00FF;
	g &= 0xFF00;

	return res | g;
}


ALSWGraphicsFactory *ALSWGraphicsFactory::_factory = nullptr;

ALSWGraphicsFactory::~ALSWGraphicsFactory() {
	_factory = nullptr;
}

size_t ALSWGraphicsFactory::GetFilterCount() const {
	return 2;
}

const GfxFilterInfo *ALSWGraphicsFactory::GetFilterInfo(size_t index) const {
	switch (index) {
	case 0:
		return &AllegroGfxFilter::FilterInfo;
	case 1:
		return &HqxGfxFilter::FilterInfo;
	default:
		return nullptr;
	}
}

String ALSWGraphicsFactory::GetDefaultFilterID() const {
	return AllegroGfxFilter::FilterInfo.Id;
}

/* static */ ALSWGraphicsFactory *ALSWGraphicsFactory::GetFactory() {
	if (!_factory)
		_factory = new ALSWGraphicsFactory();
	return _factory;
}

ALSoftwareGraphicsDriver *ALSWGraphicsFactory::EnsureDriverCreated() {
	if (!_driver)
		_driver = new ALSoftwareGraphicsDriver();
	return _driver;
}

AllegroGfxFilter *ALSWGraphicsFactory::CreateFilter(const String &id) {
	if (AllegroGfxFilter::FilterInfo.Id.CompareNoCase(id) == 0)
		return new AllegroGfxFilter();
	else if (HqxGfxFilter::FilterInfo.Id.CompareNoCase(id) == 0)
		return new HqxGfxFilter();
	return nullptr;
}

} // namespace ALSW
} // namespace Engine
} // namespace AGS
} // namespace AGS3
