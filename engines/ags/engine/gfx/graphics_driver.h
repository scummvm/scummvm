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
// Graphics driver interface
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_GRAPHICS_DRIVER_H
#define AGS_ENGINE_GFX_GRAPHICS_DRIVER_H

//#include "math/matrix.h"
#include "ags/lib/std/memory.h"
#include "ags/lib/allegro.h" // RGB, PALETTE
#include "ags/engine/gfx/gfx_defines.h"
#include "ags/engine/gfx/gfx_mode_list.h"
#include "ags/shared/util/geometry.h"

namespace AGS3 {
namespace AGS {

namespace Shared {
class Bitmap;
typedef std::shared_ptr<Shared::Bitmap> PBitmap;
} // namespace Shared

namespace Engine {

// Forward declaration
class IDriverDependantBitmap;
class IGfxFilter;
typedef std::shared_ptr<IGfxFilter> PGfxFilter;
using Shared::PBitmap;

enum TintMethod {
	TintReColourise = 0,
	TintSpecifyMaximum = 1
};

enum VideoSkipType {
	VideoSkipNone = 0,
	VideoSkipEscape = 1,
	VideoSkipAnyKey = 2,
	VideoSkipKeyOrMouse = 3
};

// Sprite transformation
// TODO: combine with stretch parameters in the IDriverDependantBitmap?
struct SpriteTransform {
	// Translate
	int X, Y;
	float ScaleX, ScaleY;
	float Rotate; // angle, in radians

	SpriteTransform()
		: X(0), Y(0), ScaleX(1.f), ScaleY(1.f), Rotate(0.f) {
	}

	SpriteTransform(int x, int y, float scalex = 1.0f, float scaley = 1.0f, float rotate = 0.0f)
		: X(x), Y(y), ScaleX(scalex), ScaleY(scaley), Rotate(rotate) {
	}
};

// Describes 3 render matrixes: world, view and projection
struct RenderMatrixes {
	/*
	glm::mat4 World;
	glm::mat4 View;
	glm::mat4 Projection;
	*/
};


typedef void (*GFXDRV_CLIENTCALLBACK)();
typedef bool (*GFXDRV_CLIENTCALLBACKXY)(int x, int y);
typedef void (*GFXDRV_CLIENTCALLBACKINITGFX)(void *data);

class IGraphicsDriver {
public:
	virtual const char *GetDriverName() = 0;
	virtual const char *GetDriverID() = 0;
	virtual void SetTintMethod(TintMethod method) = 0;
	// Initialize given display mode
	virtual bool SetDisplayMode(const DisplayMode &mode) = 0;
	// Updates previously set display mode, accomodating to the new screen size
	virtual void UpdateDeviceScreen(const Size &screen_size) = 0;
	// Gets if a graphics mode was initialized
	virtual bool IsModeSet() const = 0;
	// Set the size of the native image size
	virtual bool SetNativeResolution(const GraphicResolution &native_res) = 0;
	virtual bool IsNativeSizeValid() const = 0;
	// Set game render frame and translation
	virtual bool SetRenderFrame(const Rect &dst_rect) = 0;
	virtual bool IsRenderFrameValid() const = 0;
	// Report which color depth options are best suited for the given native color depth
	virtual int  GetDisplayDepthForNativeDepth(int native_color_depth) const = 0;
	virtual IGfxModeList *GetSupportedModeList(int color_depth) = 0;
	virtual bool IsModeSupported(const DisplayMode &mode) = 0;
	virtual DisplayMode GetDisplayMode() const = 0;
	virtual PGfxFilter GetGraphicsFilter() const = 0;
	virtual Size GetNativeSize() const = 0;
	virtual Rect GetRenderDestination() const = 0;
	virtual void SetCallbackForPolling(GFXDRV_CLIENTCALLBACK callback) = 0;
	// TODO: get rid of draw screen callback at some point when all fade functions are more or less grouped in one
	virtual void SetCallbackToDrawScreen(GFXDRV_CLIENTCALLBACK callback, GFXDRV_CLIENTCALLBACK post_callback) = 0;
	virtual void SetCallbackOnInit(GFXDRV_CLIENTCALLBACKINITGFX callback) = 0;
	// The NullSprite callback is called in the main render loop when a
	// null sprite is encountered. You can use this to hook into the rendering
	// process.
	virtual void SetCallbackForNullSprite(GFXDRV_CLIENTCALLBACKXY callback) = 0;
	// Clears the screen rectangle. The coordinates are expected in the **native game resolution**.
	virtual void ClearRectangle(int x1, int y1, int x2, int y2, RGB *colorToUse) = 0;
	// Gets closest recommended bitmap format (currently - only color depth) for the given original format.
	// Engine needs to have game bitmaps brought to the certain range of formats, easing conversion into the video bitmaps.
	virtual int  GetCompatibleBitmapFormat(int color_depth) = 0;
	virtual IDriverDependantBitmap *CreateDDBFromBitmap(Shared::Bitmap *bitmap, bool hasAlpha, bool opaque = false) = 0;
	virtual void UpdateDDBFromBitmap(IDriverDependantBitmap *bitmapToUpdate, Shared::Bitmap *bitmap, bool hasAlpha) = 0;
	virtual void DestroyDDB(IDriverDependantBitmap *bitmap) = 0;

	// Prepares next sprite batch, a list of sprites with defined viewport and optional
	// global model transformation; all subsequent calls to DrawSprite will be adding
	// sprites to this batch's list.
	virtual void BeginSpriteBatch(const Rect &viewport, const SpriteTransform &transform,
	                              const Point offset = Point(), GlobalFlipType flip = kFlip_None, PBitmap surface = nullptr) = 0;
	// Adds sprite to the active batch
	virtual void DrawSprite(int x, int y, IDriverDependantBitmap *bitmap) = 0;
	// Adds fade overlay fx to the active batch
	virtual void SetScreenFade(int red, int green, int blue) = 0;
	// Adds tint overlay fx to the active batch
	// TODO: redesign this to allow various post-fx per sprite batch?
	virtual void SetScreenTint(int red, int green, int blue) = 0;
	// Clears all sprite batches, resets batch counter
	virtual void ClearDrawLists() = 0;
	virtual void RenderToBackBuffer() = 0;
	virtual void Render() = 0;
	// Renders with additional final offset and flip
	// TODO: leftover from old code, solely for software renderer; remove when
	// software mode either discarded or scene node graph properly implemented.
	virtual void Render(int xoff, int yoff, GlobalFlipType flip) = 0;
	// Copies contents of the game screen into bitmap using simple blit or pixel copy.
	// Bitmap must be of supported size and pixel format. If it's not the method will
	// fail and optionally write wanted destination format into 'want_fmt' pointer.
	virtual bool GetCopyOfScreenIntoBitmap(Shared::Bitmap *destination, bool at_native_res, GraphicResolution *want_fmt = nullptr) = 0;
	virtual void EnableVsyncBeforeRender(bool enabled) = 0;
	virtual void Vsync() = 0;
	// Enables or disables rendering mode that draws sprite list directly into
	// the final resolution, as opposed to drawing to native-resolution buffer
	// and scaling to final frame. The effect may be that sprites that are
	// drawn with additional fractional scaling will appear more detailed than
	// the rest of the game. The effect is stronger for the low-res games being
	// rendered in the high-res mode.
	virtual void RenderSpritesAtScreenResolution(bool enabled, int supersampling = 1) = 0;
	// TODO: move fade-in/out/boxout functions out of the graphics driver!! make everything render through
	// main drawing procedure. Since currently it does not - we need to init our own sprite batch
	// internally to let it set up correct viewport settings instead of relying on a chance.
	// Runs fade-out animation in a blocking manner.
	virtual void FadeOut(int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) = 0;
	// Runs fade-in animation in a blocking manner.
	virtual void FadeIn(int speed, PALETTE p, int targetColourRed, int targetColourGreen, int targetColourBlue) = 0;
	// Runs box-out animation in a blocking manner.
	virtual void BoxOutEffect(bool blackingOut, int speed, int delay) = 0;
	virtual void UseSmoothScaling(bool enabled) = 0;
	virtual bool SupportsGammaControl() = 0;
	virtual void SetGamma(int newGamma) = 0;
	// Returns the virtual screen. Will return NULL if renderer does not support memory backbuffer.
	// In normal case you should use GetStageBackBuffer() instead.
	virtual Shared::Bitmap *GetMemoryBackBuffer() = 0;
	// Sets custom backbuffer bitmap to render to.
	// Passing NULL pointer will tell renderer to switch back to its original virtual screen.
	// Note that only software renderer supports this.
	virtual void SetMemoryBackBuffer(Shared::Bitmap *backBuffer) = 0;
	// Returns memory backbuffer for the current rendering stage (or base virtual screen if called outside of render pass).
	// All renderers should support this.
	virtual Shared::Bitmap *GetStageBackBuffer(bool mark_dirty) = 0;
	// Retrieves 3 transform matrixes for the current rendering stage: world (model), view and projection.
	// These matrixes will be filled in accordance to the renderer's compatible format;
	// returns false if renderer does not use matrixes (not a 3D renderer).
	virtual bool GetStageMatrixes(RenderMatrixes &rm) = 0;
	virtual bool RequiresFullRedrawEachFrame() = 0;
	virtual bool HasAcceleratedTransform() = 0;
	virtual bool UsesMemoryBackBuffer() = 0;
	virtual ~IGraphicsDriver() {}
};

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
