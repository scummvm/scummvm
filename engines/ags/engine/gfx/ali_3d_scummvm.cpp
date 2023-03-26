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

#include "ags/engine/gfx/gfxfilter_scummvm_renderer.h"
#include "ags/engine/gfx/ali_3d_scummvm.h"
#include "ags/lib/std/algorithm.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/gfx/gfxfilter_scummvm_renderer.h"
#include "ags/engine/gfx/gfx_util.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/platform/base/sys_main.h"
#include "ags/engine/ac/timer.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {
namespace AGS {
namespace Engine {
namespace ALSW {

using namespace Shared;

static RGB faded_out_palette[256];


// ----------------------------------------------------------------------------
// ScummVMRendererGraphicsDriver
// ----------------------------------------------------------------------------

ScummVMRendererGraphicsDriver::ScummVMRendererGraphicsDriver() {
	_tint_red = 0;
	_tint_green = 0;
	_tint_blue = 0;
	virtualScreen = nullptr;
	_stageVirtualScreen = nullptr;

	// Initialize default sprite batch, it will be used when no other batch was activated
	ScummVMRendererGraphicsDriver::InitSpriteBatch(0, _spriteBatchDesc[0]);
}

ScummVMRendererGraphicsDriver::~ScummVMRendererGraphicsDriver() {
	delete _screen;
	ScummVMRendererGraphicsDriver::UnInit();
}

bool ScummVMRendererGraphicsDriver::IsModeSupported(const DisplayMode &mode) {
	if (mode.Width <= 0 || mode.Height <= 0 || mode.ColorDepth <= 0) {
		warning("Invalid resolution parameters: %d x %d x %d",
			mode.Width, mode.Height, mode.ColorDepth);
		return false;
	}

	Graphics::PixelFormat format;
	return ::AGS::g_vm->getPixelFormat(mode.ColorDepth, format);
}

int ScummVMRendererGraphicsDriver::GetDisplayDepthForNativeDepth(int native_color_depth) const {
	// TODO: check for device caps to know which depth is supported?
	if (native_color_depth > 8)
		return 32;
	return native_color_depth;
}

IGfxModeList *ScummVMRendererGraphicsDriver::GetSupportedModeList(int color_depth) {
	std::vector<DisplayMode> modes;
	sys_get_desktop_modes(modes, color_depth);
	if ((modes.size() == 0) && color_depth == 32) {
		// Pretend that 24-bit are 32-bit
		sys_get_desktop_modes(modes, 24);
		for (auto &m : modes)
			m.ColorDepth = 32;
	}
	return new ScummVMRendererGfxModeList(modes);
}

PGfxFilter ScummVMRendererGraphicsDriver::GetGraphicsFilter() const {
	return _filter;
}

void ScummVMRendererGraphicsDriver::SetGraphicsFilter(PSDLRenderFilter filter) {
	_filter = filter;
	OnSetFilter();

	// TODO: support separate nearest and linear filters, initialize hint by calls to filter object
	// e.g like D3D and OGL filters act
//	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	// SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
}

void ScummVMRendererGraphicsDriver::SetTintMethod(TintMethod /*method*/) {
	// TODO: support new D3D-style tint method
}

bool ScummVMRendererGraphicsDriver::SetDisplayMode(const DisplayMode &mode) {
	ReleaseDisplayMode();

	set_color_depth(mode.ColorDepth);

	if (_initGfxCallback != nullptr)
		_initGfxCallback(nullptr);

	if (!IsModeSupported(mode))
		return false;

	const int driver = GFX_SCUMMVM;
	if (set_gfx_mode(driver, mode.Width, mode.Height, mode.ColorDepth) != 0)
		return false;

	if (g_system->hasFeature(OSystem::kFeatureVSync)) {
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureVSync, mode.Vsync);
		g_system->endGFXTransaction();
	}

	OnInit();
	OnModeSet(mode);
	return true;
}

void ScummVMRendererGraphicsDriver::UpdateDeviceScreen(const Size &screen_sz) {
	_mode.Width = screen_sz.Width;
	_mode.Height = screen_sz.Height;
#if AGS_PLATFORM_OS_ANDROID
	SDL_RenderSetLogicalSize(_renderer, _mode.Width, _mode.Height);
#endif
}

void ScummVMRendererGraphicsDriver::CreateVirtualScreen() {
	if (!IsNativeSizeValid())
		return;
	DestroyVirtualScreen();
	// Initialize virtual screen; size is equal to native resolution
	const int vscreen_w = _srcRect.GetWidth();
	const int vscreen_h = _srcRect.GetHeight();
	_origVirtualScreen.reset(new Bitmap(vscreen_w, vscreen_h, _srcColorDepth));
	virtualScreen = _origVirtualScreen.get();
	_stageVirtualScreen = virtualScreen;


	_lastTexPixels = nullptr;
	_lastTexPitch = -1;
}

void ScummVMRendererGraphicsDriver::DestroyVirtualScreen() {
	delete[] _fakeTexBitmap; // don't use destroy_bitmap(), because it's a fake structure
	_fakeTexBitmap = nullptr;

	_origVirtualScreen.reset();
	virtualScreen = nullptr;
	_stageVirtualScreen = nullptr;
}

void ScummVMRendererGraphicsDriver::ReleaseDisplayMode() {
	OnModeReleased();
	ClearDrawLists();
}

bool ScummVMRendererGraphicsDriver::SetNativeResolution(const GraphicResolution &native_res) {
	OnSetNativeRes(native_res);
	CreateVirtualScreen();
	return !_srcRect.IsEmpty();
}

bool ScummVMRendererGraphicsDriver::SetRenderFrame(const Rect &dst_rect) {
	OnSetRenderFrame(dst_rect);
	return !_dstRect.IsEmpty();
}

void ScummVMRendererGraphicsDriver::ClearRectangle(int /*x1*/, int /*y1*/, int /*x2*/, int /*y2*/, RGB * /*colorToUse*/) {
	// TODO: but maybe is not necessary, as we use SDL_Renderer with accelerated gfx here?
	// See SDL_RenderDrawRect
}

void ScummVMRendererGraphicsDriver::UnInit() {
	OnUnInit();
	ReleaseDisplayMode();
	DestroyVirtualScreen();

	sys_window_destroy();
}

bool ScummVMRendererGraphicsDriver::SupportsGammaControl() {
	return _hasGamma;
}

void ScummVMRendererGraphicsDriver::SetGamma(int newGamma) {
	if (!_hasGamma) {
		return;
	}
#ifdef TODO
	uint16 gamma_red[256];
	uint16 gamma_green[256];
	uint16 gamma_blue[256];

	for (int i = 0; i < 256; i++) {
		gamma_red[i] = MIN(((int)_defaultGammaRed[i] * newGamma) / 100, 0xffff);
		gamma_green[i] = MIN(((int)_defaultGammaGreen[i] * newGamma) / 100, 0xffff);
		gamma_blue[i] = MIN(((int)_defaultGammaBlue[i] * newGamma) / 100, 0xffff);
	}

	SDL_SetWindowGammaRamp(sys_get_window(), gamma_red, gamma_green, gamma_blue);
#endif
}

bool ScummVMRendererGraphicsDriver::DoesSupportVsyncToggle() {
	return g_system->hasFeature(OSystem::kFeatureVSync);
}

bool ScummVMRendererGraphicsDriver::SetVsync(bool enabled) {
	if (_mode.Vsync == enabled) {
		return _mode.Vsync;
	}

	if (g_system->hasFeature(OSystem::kFeatureVSync)) {
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureVSync, enabled);
		g_system->endGFXTransaction();

		_mode.Vsync = g_system->getFeatureState(OSystem::kFeatureVSync);
	}
	return _mode.Vsync;
}

int ScummVMRendererGraphicsDriver::GetCompatibleBitmapFormat(int color_depth) {
	return color_depth;
}

IDriverDependantBitmap *ScummVMRendererGraphicsDriver::CreateDDB(int width, int height, int color_depth, bool opaque) {
	return new ALSoftwareBitmap(width, height, color_depth, opaque);
}

IDriverDependantBitmap *ScummVMRendererGraphicsDriver::CreateDDBFromBitmap(Bitmap *bitmap, bool hasAlpha, bool opaque) {
	return new ALSoftwareBitmap(bitmap, opaque, hasAlpha);
}

IDriverDependantBitmap *ScummVMRendererGraphicsDriver::CreateRenderTargetDDB(int width, int height, int color_depth, bool opaque) {
	return new ALSoftwareBitmap(width, height, color_depth, opaque);
}

void ScummVMRendererGraphicsDriver::UpdateDDBFromBitmap(IDriverDependantBitmap *bitmapToUpdate, Bitmap *bitmap, bool hasAlpha) {
	ALSoftwareBitmap *alSwBmp = (ALSoftwareBitmap *)bitmapToUpdate;
	alSwBmp->_bmp = bitmap;
	alSwBmp->_hasAlpha = hasAlpha;
}

void ScummVMRendererGraphicsDriver::DestroyDDB(IDriverDependantBitmap *bitmap) {
	delete (ALSoftwareBitmap *)bitmap;
}

void ScummVMRendererGraphicsDriver::InitSpriteBatch(size_t index, const SpriteBatchDesc &desc) {
	if (_spriteBatches.size() <= index)
		_spriteBatches.resize(index + 1);
	ALSpriteBatch &batch = _spriteBatches[index];
	batch.ID = index;
	// TODO: correct offsets to have pre-scale (source) and post-scale (dest) offsets!
	const int src_w = desc.Viewport.GetWidth() / desc.Transform.ScaleX;
	const int src_h = desc.Viewport.GetHeight() / desc.Transform.ScaleY;
	// Surface was prepared externally (common for room cameras)
	if (desc.Surface != nullptr) {
		batch.Surface = desc.Surface;
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
		// We need this subbitmap for plugins, which use _stageVirtualScreen and are unaware of possible multiple viewports;
		// TODO: there could be ways to optimize this further, but best is to update plugin rendering hooks (and upgrade plugins)
		if (!batch.Surface || !batch.IsVirtualScreen ||
			(batch.Surface->GetWidth() != src_w) || (batch.Surface->GetHeight() != src_h) ||
			(!batch.Surface->IsSameBitmap(virtualScreen)) ||
			(batch.Surface->GetSubOffset() != desc.Viewport.GetLT())) {
			Rect rc = RectWH(desc.Viewport.Left, desc.Viewport.Top, desc.Viewport.GetWidth(), desc.Viewport.GetHeight());
			batch.Surface.reset(BitmapHelper::CreateSubBitmap(virtualScreen, rc));
		}
		batch.Opaque = true;
		batch.IsVirtualScreen = true;
	}
	// No surface prepared and has transformation other than offset
	else if (!batch.Surface || batch.IsVirtualScreen || batch.Surface->GetWidth() != src_w || batch.Surface->GetHeight() != src_h) {
		batch.Surface.reset(new Bitmap(src_w, src_h, _srcColorDepth));
		batch.Opaque = false;
		batch.IsVirtualScreen = false;
	}
}

void ScummVMRendererGraphicsDriver::ResetAllBatches() {
	// NOTE: we don't release batches themselves here, only sprite lists.
	// This is because we cache batch surfaces, for perfomance reasons.
	_spriteList.clear();
}

void ScummVMRendererGraphicsDriver::DrawSprite(int x, int y, IDriverDependantBitmap *bitmap) {
	_spriteList.push_back(ALDrawListEntry((ALSoftwareBitmap *)bitmap, _actSpriteBatch, x, y));
}

void ScummVMRendererGraphicsDriver::SetScreenFade(int /*red*/, int /*green*/, int /*blue*/) {
	// TODO: was not necessary atm
	// TODO: checkme later
}

void ScummVMRendererGraphicsDriver::SetScreenTint(int red, int green, int blue) {
	_tint_red = red;
	_tint_green = green;
	_tint_blue = blue;
	if (((_tint_red > 0) || (_tint_green > 0) || (_tint_blue > 0)) && (_srcColorDepth > 8)) {
		_spriteList.push_back(
			ALDrawListEntry(reinterpret_cast<ALSoftwareBitmap *>(DRAWENTRY_TINT), _actSpriteBatch, 0, 0));
	}
}

void ScummVMRendererGraphicsDriver::SetStageScreen(const Size & /*sz*/, int /*x*/, int /*y*/) {
	// unsupported, as using _stageVirtualScreen instead
}

void ScummVMRendererGraphicsDriver::RenderToBackBuffer() {
	// Close unended batches, and issue a warning
	assert(_actSpriteBatch == 0);
	while (_actSpriteBatch > 0)
		EndSpriteBatch();

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
	for (size_t cur_spr = 0; cur_spr < _spriteList.size();) {
		const auto &batch_desc = _spriteBatchDesc[_spriteList[cur_spr].node];
		const ALSpriteBatch &batch = _spriteBatches[_spriteList[cur_spr].node];
		const Rect &viewport = batch_desc.Viewport;
		const SpriteTransform &transform = batch_desc.Transform;

		_rendSpriteBatch = batch.ID;
		virtualScreen->SetClip(viewport);
		Bitmap *surface = batch.Surface.get();
		const int view_offx = viewport.Left;
		const int view_offy = viewport.Top;
		if (surface) {
			if (!batch.Opaque)
				surface->ClearTransparent();
			_stageVirtualScreen = surface;
			cur_spr = RenderSpriteBatch(batch, cur_spr, surface, transform.X, transform.Y);
			if (!batch.IsVirtualScreen)
				virtualScreen->StretchBlt(surface, RectWH(view_offx, view_offy, viewport.GetWidth(), viewport.GetHeight()),
					batch.Opaque ? kBitmap_Copy : kBitmap_Transparency);
		} else {
			cur_spr = RenderSpriteBatch(batch, cur_spr, virtualScreen, view_offx + transform.X, view_offy + transform.Y);
		}
	}
	_stageVirtualScreen = virtualScreen;
	_rendSpriteBatch = UINT32_MAX;
	ClearDrawLists();
}

size_t ScummVMRendererGraphicsDriver::RenderSpriteBatch(const ALSpriteBatch &batch, size_t from, Bitmap *surface, int surf_offx, int surf_offy) {
	for (; (from < _spriteList.size()) && (_spriteList[from].node == batch.ID); ++from) {
		const auto &sprite = _spriteList[from];
		if (sprite.ddb == nullptr) {
			if (_spriteEvtCallback)
				_spriteEvtCallback(sprite.x, sprite.y);
			else
				error("Unhandled attempt to draw null sprite");
			// Stage surface could have been replaced by plugin
			surface = _stageVirtualScreen;
			continue;
		} else if (sprite.ddb == reinterpret_cast<ALSoftwareBitmap *>(DRAWENTRY_TINT)) {
			// draw screen tint fx
			set_trans_blender(_tint_red, _tint_green, _tint_blue, 0);
			surface->LitBlendBlt(surface, 0, 0, 128);
			continue;
		}

		ALSoftwareBitmap *bitmap = sprite.ddb;
		int drawAtX = sprite.x + surf_offx;
		int drawAtY = sprite.y + surf_offy;

		if (bitmap->_alpha == 0) {
		} // fully transparent, do nothing
		else if ((bitmap->_opaque) && (bitmap->_bmp == surface) && (bitmap->_alpha == 255)) {
		} else if (bitmap->_opaque) {
			surface->Blit(bitmap->_bmp, 0, 0, drawAtX, drawAtY, bitmap->_bmp->GetWidth(), bitmap->_bmp->GetHeight());
			// TODO: we need to also support non-masked translucent blend, but...
			// Allegro 4 **does not have such function ready** :( (only masked blends, where it skips magenta pixels);
			// I am leaving this problem for the future, as coincidentally software mode does not need this atm.
		} else if (bitmap->_hasAlpha) {
			if (bitmap->_alpha == 255) // no global transparency, simple alpha blend
				set_alpha_blender();
			else
				set_blender_mode(kArgbToRgbBlender, 0, 0, 0, bitmap->_alpha);

			surface->TransBlendBlt(bitmap->_bmp, drawAtX, drawAtY);
		} else {
			// here _transparency is used as alpha (between 1 and 254), but 0 means opaque!
			GfxUtil::DrawSpriteWithTransparency(surface, bitmap->_bmp, drawAtX, drawAtY,
				bitmap->_alpha);
		}
	}
	return from;
}

void ScummVMRendererGraphicsDriver::copySurface(const Graphics::Surface &src, bool mode) {
	assert(src.w == _screen->w && src.h == _screen->h && src.pitch == _screen->pitch);
	const uint32 *srcP = (const uint32 *)src.getPixels();
	uint32 *destP = (uint32 *)_screen->getPixels();
	uint32 pixel;
	int x1 = 9999, y1 = 9999, x2 = -1, y2 = -1;

	for (int y = 0; y < src.h; ++y) {
		for (int x = 0; x < src.w; ++x, ++srcP, ++destP) {
			if (!mode) {
				pixel = (*srcP & 0xff00ff00) |
					((*srcP & 0xff) << 16) |
					((*srcP >> 16) & 0xff);
			} else {
				pixel = ((*srcP & 0xffffff) << 8) |
					((*srcP >> 24) & 0xff);
			}

			if (*destP != pixel) {
				*destP = pixel;
				x1 = MIN(x1, x);
				y1 = MIN(y1, y);
				x2 = MAX(x2, x);
				y2 = MAX(y2, y);
			}
		}
	}

	if (x2 != -1)
		_screen->addDirtyRect(Common::Rect(x1, y1, x2 + 1, y2 + 1));
}

void ScummVMRendererGraphicsDriver::Present(int xoff, int yoff, Shared::GraphicFlip flip) {
	Graphics::Surface *srcTransformed = nullptr;
	if (xoff != 0 || yoff != 0 || flip != Shared::kFlip_None) {
		srcTransformed = new Graphics::Surface();
		srcTransformed->copyFrom(virtualScreen->GetAllegroBitmap()->getSurface());
		switch(flip) {
		case kFlip_Horizontal:
			srcTransformed->flipHorizontal(Common::Rect(srcTransformed->w, srcTransformed->h));
			break;
		case kFlip_Vertical:
			srcTransformed->flipVertical(Common::Rect(srcTransformed->w, srcTransformed->h));
			break;
		case kFlip_Both:
			srcTransformed->flipHorizontal(Common::Rect(srcTransformed->w, srcTransformed->h));
			srcTransformed->flipVertical(Common::Rect(srcTransformed->w, srcTransformed->h));
			break;
		default:
			break;
		}
		srcTransformed->move(xoff, yoff, srcTransformed->h);
	}

	const Graphics::Surface &src = srcTransformed ?
		*srcTransformed :
		virtualScreen->GetAllegroBitmap()->getSurface();

	enum {
		kRenderInitial, kRenderDirect, kRenderToABGR, kRenderToRGBA,
		kRenderOther
	} renderMode;

	// Check for rendering to use. The virtual screen can change, so I'm
	// playing it safe and checking the render mode for each frame
	const Graphics::PixelFormat screenFormat = g_system->getScreenFormat();

	if (src.format == screenFormat) {
		// The virtual surface can be directly blitted to the screen
		renderMode = kRenderDirect;
	} else if (src.format != Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24)) {
		// Not a 32-bit surface, so will have to use an intermediate
		// surface to correct the virtual screen to the correct format
		renderMode = kRenderOther;
	} else if (screenFormat == Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)) {
		renderMode = kRenderToRGBA;
	} else if (screenFormat == Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24)) {
		renderMode = kRenderToABGR;
	} else {
		renderMode = kRenderOther;
	}

	if (renderMode != kRenderDirect && !_screen)
		_screen = new Graphics::Screen();

	switch (renderMode) {
	case kRenderToABGR:
		// ARGB to ABGR
		copySurface(src, false);
		break;

	case kRenderToRGBA:
		// ARGB to RGBA
		copySurface(src, true);
		break;

	case kRenderOther: {
		// Blit the surface to the temporary screen, ignoring the alphas.
		// This takes care of converting to the screen format
		Graphics::Surface srcCopy = src;
		srcCopy.format.aLoss = 8;

		_screen->blitFrom(srcCopy);
		break;
	}

	case kRenderDirect:
		// Blit the virtual surface directly to the screen
		g_system->copyRectToScreen(src.getPixels(), src.pitch,
			0, 0, src.w, src.h);
		g_system->updateScreen();
		if (srcTransformed) {
			srcTransformed->free();
			delete srcTransformed;
		}
		return;

	default:
		break;
	}

	if (srcTransformed) {
		srcTransformed->free();
		delete srcTransformed;
	}

	if (_screen)
		_screen->update();
}

void ScummVMRendererGraphicsDriver::Render(int xoff, int yoff, GraphicFlip flip) {
	RenderToBackBuffer();
	Present(xoff, yoff, flip);
}

void ScummVMRendererGraphicsDriver::Render() {
	Render(0, 0, kFlip_None);
}

Bitmap *ScummVMRendererGraphicsDriver::GetMemoryBackBuffer() {
	return virtualScreen;
}

void ScummVMRendererGraphicsDriver::SetMemoryBackBuffer(Bitmap *backBuffer) {
	// We need to also test internal AL BITMAP pointer, because we may receive it raw from plugin,
	// in which case the Bitmap object may be a different wrapper over our own virtual screen.
	if (backBuffer && (backBuffer->GetAllegroBitmap() != _origVirtualScreen->GetAllegroBitmap())) {
		virtualScreen = backBuffer;
	} else {
		virtualScreen = _origVirtualScreen.get();
	}
	_stageVirtualScreen = virtualScreen;

	// Reset old virtual screen's subbitmaps;
	// NOTE: this MUST NOT be called in the midst of the RenderSpriteBatches!
	assert(_rendSpriteBatch == UINT32_MAX);
	if (_rendSpriteBatch != UINT32_MAX)
		return;
	for (auto &batch : _spriteBatches) {
		if (batch.IsVirtualScreen)
			batch.Surface.reset();
	}
}

Bitmap *ScummVMRendererGraphicsDriver::GetStageBackBuffer(bool /*mark_dirty*/) {
	return _stageVirtualScreen;
}

void ScummVMRendererGraphicsDriver::SetStageBackBuffer(Bitmap *backBuffer) {
	Bitmap *cur_stage = (_rendSpriteBatch == UINT32_MAX) ? virtualScreen : _spriteBatches[_rendSpriteBatch].Surface.get();
	// We need to also test internal AL BITMAP pointer, because we may receive it raw from plugin,
	// in which case the Bitmap object may be a different wrapper over our own virtual screen.
	if (backBuffer && (backBuffer->GetAllegroBitmap() != cur_stage->GetAllegroBitmap()))
		_stageVirtualScreen = backBuffer;
	else
		_stageVirtualScreen = cur_stage;
}

bool ScummVMRendererGraphicsDriver::GetCopyOfScreenIntoBitmap(Bitmap *destination, bool at_native_res, GraphicResolution *want_fmt) {
	(void)at_native_res; // software driver always renders at native resolution at the moment
	// software filter is taught to copy to any size
	if (destination->GetColorDepth() != _srcColorDepth) {
		if (want_fmt)
			*want_fmt = GraphicResolution(destination->GetWidth(), destination->GetHeight(), _srcColorDepth);
		return false;
	}

	if (destination->GetSize() == virtualScreen->GetSize()) {
		destination->Blit(virtualScreen, 0, 0, 0, 0, virtualScreen->GetWidth(), virtualScreen->GetHeight());
	} else {
		destination->StretchBlt(virtualScreen,
		                        RectWH(0, 0, virtualScreen->GetWidth(), virtualScreen->GetHeight()),
		                        RectWH(0, 0, destination->GetWidth(), destination->GetHeight()));
	}
	return true;
}

/**
    fade.c - High Color Fading Routines

    Last Revision: 21 June, 2002

    Author: Matthew Leverton
**/
void ScummVMRendererGraphicsDriver::highcolor_fade_in(Bitmap *vs, void(*draw_callback)(),
		int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) {
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

		if (draw_callback)
			draw_callback();
		RenderToBackBuffer();
		Present();

		sys_evt_process_pending();
		if (_pollingCallback)
			_pollingCallback();

		WaitForNextFrame();
	}
	delete bmp_buff;

	SetMemoryBackBuffer(vs);
	if (draw_callback)
		draw_callback();
	RenderToBackBuffer();
	Present();
}

void ScummVMRendererGraphicsDriver::highcolor_fade_out(Bitmap *vs, void(*draw_callback)(),
		int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) {
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

		if (draw_callback)
			draw_callback();
		RenderToBackBuffer();
		Present();

		sys_evt_process_pending();
		if (_pollingCallback)
			_pollingCallback();

		WaitForNextFrame();
	}
	delete bmp_buff;

	SetMemoryBackBuffer(vs);
	vs->Clear(clearColor);
	if (draw_callback)
		draw_callback();
	RenderToBackBuffer();
	Present();
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

void ScummVMRendererGraphicsDriver::__fade_from_range(PALETTE source, PALETTE dest, int speed, int from, int to) {
	PALETTE temp;
	int c;

	for (c = 0; c < PAL_SIZE; c++)
		temp[c] = source[c];

	for (c = 0; c < 64; c += speed) {
		fade_interpolate(source, dest, temp, c, from, to);
		set_palette_range(temp, from, to, TRUE);

		RenderToBackBuffer();
		Present();

		g_system->delayMillis(5);
		sys_evt_process_pending();
		if (_pollingCallback)
			_pollingCallback();
	}

	set_palette_range(dest, from, to, TRUE);
}

void ScummVMRendererGraphicsDriver::__fade_out_range(int speed, int from, int to, int targetColourRed, int targetColourGreen, int targetColourBlue) {
	PALETTE temp;

	initialize_fade_256(targetColourRed, targetColourGreen, targetColourBlue);
	get_palette(temp);
	__fade_from_range(temp, faded_out_palette, speed, from, to);
}

void ScummVMRendererGraphicsDriver::FadeOut(int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) {
	if (_srcColorDepth > 8) {
		highcolor_fade_out(virtualScreen, _drawPostScreenCallback, speed * 4, targetColourRed, targetColourGreen, targetColourBlue);
	} else {
		__fade_out_range(speed, 0, 255, targetColourRed, targetColourGreen, targetColourBlue);
	}
}

void ScummVMRendererGraphicsDriver::FadeIn(int speed, PALETTE p, int targetColourRed, int targetColourGreen, int targetColourBlue) {
	if (_drawScreenCallback) {
		_drawScreenCallback();
		RenderToBackBuffer();
	}
	if (_srcColorDepth > 8) {
		highcolor_fade_in(virtualScreen, _drawPostScreenCallback, speed * 4, targetColourRed, targetColourGreen, targetColourBlue);
	} else {
		initialize_fade_256(targetColourRed, targetColourGreen, targetColourBlue);
		__fade_from_range(faded_out_palette, p, speed, 0, 255);
	}
}

void ScummVMRendererGraphicsDriver::BoxOutEffect(bool blackingOut, int speed, int delay) {
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

			if (_drawPostScreenCallback)
				_drawPostScreenCallback();
			RenderToBackBuffer();
			Present();

			sys_evt_process_pending();
			if (_pollingCallback)
				_pollingCallback();

			_G(platform)->Delay(delay);
		}
		delete bmp_buff;
		SetMemoryBackBuffer(bmp_orig);
	} else {
		error("BoxOut fade-in not implemented in sw gfx driver");
	}
}
// end fading routines


ScummVMRendererGraphicsFactory *ScummVMRendererGraphicsFactory::_factory = nullptr;

ScummVMRendererGraphicsFactory::~ScummVMRendererGraphicsFactory() {
	_factory = nullptr;
}

size_t ScummVMRendererGraphicsFactory::GetFilterCount() const {
	return 1;
}

const GfxFilterInfo *ScummVMRendererGraphicsFactory::GetFilterInfo(size_t index) const {
	switch (index) {
	case 0:
		return _G(scummvmGfxFilter);
	default:
		return nullptr;
	}
}

String ScummVMRendererGraphicsFactory::GetDefaultFilterID() const {
	return _GP(scummvmGfxFilter).Id;
}

/* static */ ScummVMRendererGraphicsFactory *ScummVMRendererGraphicsFactory::GetFactory() {
	if (!_factory)
		_factory = new ScummVMRendererGraphicsFactory();
	return _factory;
}

ScummVMRendererGraphicsDriver *ScummVMRendererGraphicsFactory::EnsureDriverCreated() {
	if (!_driver)
		_driver = new ScummVMRendererGraphicsDriver();
	return _driver;
}

ScummVMRendererGfxFilter *ScummVMRendererGraphicsFactory::CreateFilter(const String &id) {
	if (_GP(scummvmGfxFilter).Id.CompareNoCase(id) == 0)
		return new ScummVMRendererGfxFilter();
	return nullptr;
}

} // namespace ALSW
} // namespace Engine
} // namespace AGS
} // namespace AGS3
