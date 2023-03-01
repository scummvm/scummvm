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

//=============================================================================
//
// Implementation base for graphics driver
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_GFX_DRIVER_BASE_H
#define AGS_ENGINE_GFX_GFX_DRIVER_BASE_H

#include "ags/lib/std/memory.h"
#include "engines/ags/lib/std/map.h"
#include "ags/lib/std/vector.h"
#include "ags/engine/gfx/ddb.h"
#include "ags/shared/gfx/gfx_def.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/shared/util/scaling.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

using Shared::Bitmap;
using Shared::PlaneScaling;

// Sprite batch, defines viewport and an optional model transformation for the list of sprites
struct SpriteBatchDesc {
	uint32_t                 Parent = 0;
	// View rectangle for positioning and clipping, in resolution coordinates
	// (this may be screen or game frame resolution, depending on circumstances)
	Rect                     Viewport;
	// Optional model transformation, to be applied to each sprite
	SpriteTransform          Transform;
	// Optional flip, applied to the whole batch as the last transform
	Shared::GraphicFlip      Flip = Shared::kFlip_None;
	// Optional bitmap to draw sprites upon. Used exclusively by the software rendering mode.
	PBitmap                  Surface;

	SpriteBatchDesc() = default;
	SpriteBatchDesc(uint32_t parent, const Rect viewport, const SpriteTransform & transform,
		Shared::GraphicFlip flip = Shared::kFlip_None, PBitmap surface = nullptr)
		: Parent(parent)
		, Viewport(viewport)
		, Transform(transform)
		, Flip(flip)
		, Surface(surface) {
	}
};

typedef std::vector<SpriteBatchDesc> SpriteBatchDescs;

// The single sprite entry in the render list
template<class T_DDB>
struct SpriteDrawListEntry {
	T_DDB *ddb = nullptr; // TODO: use shared pointer?
	uint32_t node = 0; // sprite batch / scene node index
	int x = 0, y = 0; // sprite position, in local batch / node coordinates
	bool skip = false;

	SpriteDrawListEntry() = default;
	SpriteDrawListEntry(T_DDB * ddb_, uint32_t node_, int x_, int y_)
		: ddb(ddb_)
		, node(node_)
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
	                             Shared::GraphicFlip flip = Shared::kFlip_None, PBitmap surface = nullptr) override;
	void        EndSpriteBatch() override;
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
	void        SetCallbackForNullSprite(GFXDRV_CLIENTCALLBACKXY callback) override {
		_nullSpriteCallback = callback;
	}

protected:
	// Special internal values, applied to DrawListEntry
	static const intptr_t DRAWENTRY_STAGECALLBACK = 0x0;
	static const intptr_t DRAWENTRY_FADE = 0x1;
	static const intptr_t DRAWENTRY_TINT = 0x2;

	// Called after graphics driver was initialized for use for the first time
	virtual void OnInit();
	// Called just before graphics mode is going to be uninitialized and its
	// resources released
	virtual void OnUnInit();
	// Called after new mode was successfully initialized
	virtual void OnModeSet(const DisplayMode &mode);
	// Called when the new native size is set
	virtual void OnSetNativeRes(const GraphicResolution &native_res);
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
	int                 _srcColorDepth; // rendering source color depth (in bits per pixel)
	Rect                _dstRect;       // rendering destination rect
	Rect                _filterRect;    // filter scaling destination rect (before final scaling)
	PlaneScaling        _scaling;       // native -> render dest coordinate transformation

	// Callbacks
	GFXDRV_CLIENTCALLBACK _pollingCallback;
	GFXDRV_CLIENTCALLBACK _drawScreenCallback;
	GFXDRV_CLIENTCALLBACK _drawPostScreenCallback;
	GFXDRV_CLIENTCALLBACKXY _nullSpriteCallback;
	GFXDRV_CLIENTCALLBACKINITGFX _initGfxCallback;

	// Sprite batch parameters
	SpriteBatchDescs _spriteBatchDesc; // sprite batches list
	size_t _actSpriteBatch; // active batch index
};




// Parent class for the video memory DDBs
class BaseDDB : public IDriverDependantBitmap {
public:
	int GetWidth() const override {
		return _width;
	}
	int GetHeight() const override {
		return _height;
	}
	int GetColorDepth() const override {
		return _colDepth;
	}

	int _width = 0, _height = 0;
	int _colDepth = 0;
	bool _hasAlpha = false; // has meaningful alpha channel
	bool _opaque = false;   // no mask color

protected:
	BaseDDB() {}
	virtual ~BaseDDB() {}
};

// A base parent for the otherwise opaque texture data object;
// TextureData refers to the pixel data itself, with no additional
// properties. It may be shared between multiple sprites if necessary.
struct TextureData {
	uint32_t ID = UINT32_MAX;
	virtual ~TextureData() = default;
protected:
	TextureData() = default;
};

// Generic TextureTile base
struct TextureTile {
	int x = 0, y = 0;
	int width = 0, height = 0;
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
	Bitmap *GetStageBackBuffer(bool mark_dirty) override;
	bool GetStageMatrixes(RenderMatrixes &rm) override;
	IDriverDependantBitmap *CreateDDB(int width, int height, int color_depth, bool opaque) override = 0;
	IDriverDependantBitmap *CreateDDBFromBitmap(Bitmap *bitmap, bool hasAlpha, bool opaque = false) override;
	// Get shared texture from cache, or create from bitmap and assign ID
	IDriverDependantBitmap *GetSharedDDB(uint32_t sprite_id, Bitmap *bitmap, bool hasAlpha, bool opaque) override;
	// Removes the shared texture reference, will force the texture to recreate next time
	 void ClearSharedDDB(uint32_t sprite_id) override;
	 // Updates shared texture data, but only if it is present in the cache
	 void UpdateSharedDDB(uint32_t sprite_id, Bitmap *bitmap, bool hasAlpha, bool opaque) override;
	void DestroyDDB(IDriverDependantBitmap* ddb) override;

protected:
	// Create texture data with the given parameters
	virtual TextureData *CreateTextureData(int width, int height, bool opaque) = 0;
	// Update texture data from the given bitmap
	virtual void UpdateTextureData(TextureData *txdata, Bitmap *bmp, bool opaque, bool hasAlpha) = 0;
	// Create DDB using preexisting texture data
	virtual IDriverDependantBitmap *CreateDDB(std::shared_ptr<TextureData> txdata,
		  int width, int height, int color_depth, bool opaque) = 0;
	// Retrieve shared texture data object from the given DDB
	virtual std::shared_ptr<TextureData> GetTextureData(IDriverDependantBitmap *ddb) = 0;
	virtual void DestroyDDBImpl(IDriverDependantBitmap* ddb) = 0;

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
	void BitmapToVideoMem(const Bitmap *bitmap, const bool has_alpha, const TextureTile *tile,
		char *dst_ptr, const int dst_pitch, const bool usingLinearFiltering);
	// Same but optimized for opaque source bitmaps which ignore transparent "mask color"
	void BitmapToVideoMemOpaque(const Bitmap *bitmap, const bool has_alpha, const TextureTile *tile,
		char *dst_ptr, const int dst_pitch);

	// Stage virtual screen is used to let plugins draw custom graphics
	// in between render stages (between room and GUI, after GUI, and so on)
	PBitmap _stageVirtualScreen;
	IDriverDependantBitmap *_stageVirtualScreenDDB;
	// Stage matrixes are used to let plugins with hardware acceleration know model matrix;
	// these matrixes are filled compatible with each given renderer
	RenderMatrixes _stageMatrixes;

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

	// Texture short-term cache:
	// - caches textures while they are in the immediate use;
	// - this lets to share same texture data among multiple sprites on screen.
	// TextureCacheItem stores weak references to the existing texture tiles,
	// identified by an arbitrary uint32 number.
	struct TextureCacheItem {
		GraphicResolution Res;
		std::weak_ptr<TextureData> Data;
		TextureCacheItem() = default;
		TextureCacheItem(std::shared_ptr<TextureData> data, const GraphicResolution &res)
			: Data(data), Res(res) {}
	};
	std::unordered_map<uint32_t, TextureCacheItem> _txRefs;
};

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
