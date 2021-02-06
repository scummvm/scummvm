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
// Implementation base for graphics driver
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_GFXDRIVERBASE_H
#define AGS_ENGINE_GFX_GFXDRIVERBASE_H

#include "ags/lib/std/vector.h"
#include "ags/engine/gfx/ddb.h"
#include "ags/engine/gfx/graphicsdriver.h"
#include "ags/engine/util/scaling.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

using Shared::Bitmap;

// Sprite batch, defines viewport and an optional model transformation for the list of sprites
struct SpriteBatchDesc {
	// View rectangle for positioning and clipping, in resolution coordinates
	// (this may be screen or game frame resolution, depending on circumstances)
	Rect                     Viewport;
	// Optional model transformation, to be applied to each sprite
	SpriteTransform          Transform;
	// Global node offset applied to the whole batch as the last transform
	Point                    Offset;
	// Global node flip applied to the whole batch as the last transform
	GlobalFlipType           Flip;
	// Optional bitmap to draw sprites upon. Used exclusively by the software rendering mode.
	PBitmap                  Surface;

	SpriteBatchDesc() : Flip(kFlip_None) {
	}
	SpriteBatchDesc(const Rect viewport, const SpriteTransform &transform, const Point offset = Point(),
		GlobalFlipType flip = kFlip_None, PBitmap surface = nullptr)
		: Viewport(viewport)
		, Transform(transform)
		, Offset(offset)
		, Flip(flip)
		, Surface(surface) {
	}
};

typedef std::vector<SpriteBatchDesc> SpriteBatchDescs;

// The single sprite entry in the render list
template<class T_DDB>
struct SpriteDrawListEntry {
	T_DDB *bitmap; // TODO: use shared pointer?
	int x, y; // sprite position, in camera coordinates
	bool skip;

	SpriteDrawListEntry()
		: bitmap(nullptr)
		, x(0)
		, y(0)
		, skip(false) {
	}

	SpriteDrawListEntry(T_DDB *ddb, int x_ = 0, int y_ = 0)
		: bitmap(ddb)
		, x(x_)
		, y(y_)
		, skip(false) {
	}
};


// GraphicsDriverBase - is the parent class for all graphics drivers in AGS,
// that incapsulates the most common functionality.
class GraphicsDriverBase : public IGraphicsDriver {
public:
	GraphicsDriverBase();

	bool        IsModeSet() const override;
	bool        IsNativeSizeValid() const override;
	bool        IsRenderFrameValid() const override;
	DisplayMode GetDisplayMode() const override;
	Size        GetNativeSize() const override;
	Rect        GetRenderDestination() const override;

	void        BeginSpriteBatch(const Rect &viewport, const SpriteTransform &transform,
		const Point offset = Point(), GlobalFlipType flip = kFlip_None, PBitmap surface = nullptr) override;
	void        ClearDrawLists() override;

	void        SetCallbackForPolling(GFXDRV_CLIENTCALLBACK callback) override {
		_pollingCallback = callback;
	}
	void        SetCallbackToDrawScreen(GFXDRV_CLIENTCALLBACK callback, GFXDRV_CLIENTCALLBACK post_callback) override {
		_drawScreenCallback = callback;
		_drawPostScreenCallback = post_callback;
	}
	void        SetCallbackOnInit(GFXDRV_CLIENTCALLBACKINITGFX callback) override {
		_initGfxCallback = callback;
	}
	void        SetCallbackOnSurfaceUpdate(GFXDRV_CLIENTCALLBACKSURFACEUPDATE callback) override {
		_initSurfaceUpdateCallback = callback;
	}
	void        SetCallbackForNullSprite(GFXDRV_CLIENTCALLBACKXY callback) override {
		_nullSpriteCallback = callback;
	}

protected:
	// Called after graphics driver was initialized for use for the first time
	virtual void OnInit(volatile int *loopTimer);
	// Called just before graphics mode is going to be uninitialized and its
	// resources released
	virtual void OnUnInit();
	// Called after new mode was successfully initialized
	virtual void OnModeSet(const DisplayMode &mode);
	// Called when the new native size is set
	virtual void OnSetNativeSize(const Size &src_size);
	// Called before display mode is going to be released
	virtual void OnModeReleased();
	// Called when new render frame is set
	virtual void OnSetRenderFrame(const Rect &dst_rect);
	// Called when the new filter is set
	virtual void OnSetFilter();
	// Initialize sprite batch and allocate necessary resources
	virtual void InitSpriteBatch(size_t index, const SpriteBatchDesc &desc) = 0;
	// Clears sprite lists
	virtual void ResetAllBatches() = 0;

	void         OnScalingChanged();

	DisplayMode         _mode;          // display mode settings
	Rect                _srcRect;       // rendering source rect
	Rect                _dstRect;       // rendering destination rect
	Rect                _filterRect;    // filter scaling destination rect (before final scaling)
	PlaneScaling        _scaling;       // native -> render dest coordinate transformation

	// Callbacks
	GFXDRV_CLIENTCALLBACK _pollingCallback;
	GFXDRV_CLIENTCALLBACK _drawScreenCallback;
	GFXDRV_CLIENTCALLBACK _drawPostScreenCallback;
	GFXDRV_CLIENTCALLBACKXY _nullSpriteCallback;
	GFXDRV_CLIENTCALLBACKINITGFX _initGfxCallback;
	GFXDRV_CLIENTCALLBACKSURFACEUPDATE _initSurfaceUpdateCallback;

	// Sprite batch parameters
	SpriteBatchDescs _spriteBatchDesc; // sprite batches list
	size_t _actSpriteBatch; // active batch index
};



// Generic TextureTile base
struct TextureTile {
	int x, y;
	int width, height;
};

// Parent class for the video memory DDBs
class VideoMemDDB : public IDriverDependantBitmap {
public:
	int GetWidth() override {
		return _width;
	}
	int GetHeight() override {
		return _height;
	}
	int GetColorDepth() override {
		return _colDepth;
	}

	int _width, _height;
	int _colDepth;
	bool _opaque; // no mask color
};

// VideoMemoryGraphicsDriver - is the parent class for the graphic drivers
// which drawing method is based on passing the sprite stack into GPU,
// rather than blitting to flat screen bitmap.
class VideoMemoryGraphicsDriver : public GraphicsDriverBase {
public:
	VideoMemoryGraphicsDriver();
	~VideoMemoryGraphicsDriver() override;

	bool UsesMemoryBackBuffer() override;
	Bitmap *GetMemoryBackBuffer() override;
	void SetMemoryBackBuffer(Bitmap *backBuffer) override;
	Bitmap *GetStageBackBuffer() override;

protected:
	// Stage screens are raw bitmap buffers meant to be sent to plugins on demand
	// at certain drawing stages. If used at least once these buffers are then
	// rendered as additional sprites in their respected order.
	PBitmap CreateStageScreen(size_t index, const Size &sz);
	PBitmap GetStageScreen(size_t index);
	void DestroyAllStageScreens();
	// Use engine callback to acquire replacement for the null sprite;
	// returns true if the sprite was provided onto the virtual screen,
	// and false if this entry should be skipped.
	bool DoNullSpriteCallback(int x, int y);

	// Prepare and get fx item from the pool
	IDriverDependantBitmap *MakeFx(int r, int g, int b);
	// Resets fx pool counter
	void ResetFxPool();
	// Disposes all items in the fx pool
	void DestroyFxPool();

	// Prepares bitmap to be applied to the texture, copies pixels to the provided buffer
	void BitmapToVideoMem(const Bitmap *bitmap, const bool has_alpha, const TextureTile *tile, const VideoMemDDB *target,
		char *dst_ptr, const int dst_pitch, const bool usingLinearFiltering);
	// Same but optimized for opaque source bitmaps which ignore transparent "mask color"
	void BitmapToVideoMemOpaque(const Bitmap *bitmap, const bool has_alpha, const TextureTile *tile, const VideoMemDDB *target,
		char *dst_ptr, const int dst_pitch);

	// Stage virtual screen is used to let plugins draw custom graphics
	// in between render stages (between room and GUI, after GUI, and so on)
	PBitmap _stageVirtualScreen;
	IDriverDependantBitmap *_stageVirtualScreenDDB;

	// Color component shifts in video bitmap format (set by implementations)
	int _vmem_a_shift_32;
	int _vmem_r_shift_32;
	int _vmem_g_shift_32;
	int _vmem_b_shift_32;

private:
	// Virtual screens for rendering stages (sprite batches)
	std::vector<PBitmap> _stageScreens;
	// Flag which indicates whether stage screen was drawn upon during engine
	// callback and has to be inserted into sprite stack.
	bool _stageScreenDirty;

	// Fx quads pool (for screen overlay effects)
	struct ScreenFx {
		Bitmap *Raw = nullptr;
		IDriverDependantBitmap *DDB = nullptr;
		int Red = -1;
		int Green = -1;
		int Blue = -1;
	};
	std::vector<ScreenFx> _fxPool;
	size_t _fxIndex; // next free pool item
};

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
