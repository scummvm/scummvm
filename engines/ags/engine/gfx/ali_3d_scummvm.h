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
// Software graphics factory, draws raw bitmaps onto a virtual screen,
// converts to SDL_Texture and finally presents with SDL_Renderer.
//
// TODO: replace nearest-neighbour software filter with SDL's own accelerated
// scaling, maybe add more filter types if SDL renderer supports them.
// Only keep Hqx filter as a software option (might need to change how the
// filter code works).
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_ALI_3D_SCUMMVM_H
#define AGS_ENGINE_GFX_ALI_3D_SCUMMVM_H

#include "common/std/memory.h"
#include "common/std/vector.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/ddb.h"
#include "ags/engine/gfx/gfx_driver_factory_base.h"
#include "ags/engine/gfx/gfx_driver_base.h"

namespace AGS3 {
namespace AGS {
namespace Engine {
namespace ALSW {

class ScummVMRendererGraphicsDriver;
class ScummVMRendererGfxFilter;
using AGS::Shared::Bitmap;

enum RendererFlip {
	FLIP_NONE = 0x00000000,       /**< Do not flip */
	FLIP_HORIZONTAL = 0x00000001, /**< flip horizontally */
	FLIP_VERTICAL = 0x00000002    /**< flip vertically */
};

class ALSoftwareBitmap : public BaseDDB {
public:
	uint32_t GetRefID() const override { return UINT32_MAX /* not supported */; }

	int  GetAlpha() const override {
		return _alpha;
	}
	void SetAlpha(int alpha) override {
		_alpha = alpha;
	}
	void SetFlippedLeftRight(bool isFlipped) override {
		_flipped = isFlipped;
	}
	void SetStretch(int width, int height, bool /*useResampler*/) override {
		_stretchToWidth = width;
		_stretchToHeight = height;
	}
	void SetLightLevel(int /*lightLevel*/) override {}
	void SetTint(int /*red*/, int /*green*/, int /*blue*/, int /*tintSaturation*/) override {}

	Bitmap *_bmp = nullptr;
	bool _flipped = false;
	int _stretchToWidth = 0, _stretchToHeight = 0;
	int _alpha = 255;

	ALSoftwareBitmap(int width, int height, int color_depth, bool opaque) {
		_width = width;
		_height = height;
		_colDepth = color_depth;
		_opaque = opaque;
		_stretchToWidth = _width;
		_stretchToHeight = _height;
	}

	ALSoftwareBitmap(Bitmap *bmp, bool has_alpha, bool opaque) {
		_bmp = bmp;
		_width = bmp->GetWidth();
		_height = bmp->GetHeight();
		_colDepth = bmp->GetColorDepth();
		_opaque = opaque;
		_hasAlpha = has_alpha;
		_stretchToWidth = _width;
		_stretchToHeight = _height;
	}

	int GetWidthToRender() {
		return _stretchToWidth;
	}
	int GetHeightToRender() {
		return _stretchToHeight;
	}

	~ALSoftwareBitmap() override = default;
};


class ScummVMRendererGfxModeList : public IGfxModeList {
public:
	ScummVMRendererGfxModeList(const std::vector<DisplayMode> &modes)
		: _modes(modes) {
	}

	int GetModeCount() const override {
		return _modes.size();
	}

	bool GetMode(int index, DisplayMode &mode) const override {
		if (index >= 0 && (size_t)index < _modes.size()) {
			mode = _modes[index];
			return true;
		}
		return false;
	}

private:
	std::vector<DisplayMode> _modes;
};


typedef SpriteDrawListEntry<ALSoftwareBitmap> ALDrawListEntry;
// Software renderer's sprite batch
struct ALSpriteBatch {
	uint32_t ID = 0u;
	// Clipping viewport, also used as a destination for blitting optional Surface;
	// in *relative* coordinates to parent surface.
	Rect Viewport;
	// Optional model transformation, to be applied to each sprite
	SpriteTransform Transform;
	// Intermediate surface which will be drawn upon and transformed if necessary
	std::shared_ptr<Bitmap> Surface;
	// Whether surface is a parent surface's region (e.g. virtual screen)
	bool IsParentRegion = false;
	// Tells whether the surface is treated as opaque or transparent
	bool Opaque = false;
};
typedef std::vector<ALSpriteBatch> ALSpriteBatches;


class ScummVMRendererGraphicsDriver : public GraphicsDriverBase {
public:
	ScummVMRendererGraphicsDriver();
	~ScummVMRendererGraphicsDriver() override;

	const char *GetDriverID() override {
		return "Software";
	}

	bool RequiresFullRedrawEachFrame() override { return false; }
	bool HasAcceleratedTransform() override { return false; }
	bool UsesMemoryBackBuffer() override { return true; }
	bool ShouldReleaseRenderTargets() override { return false; }

	const char *GetDriverName() override {
		return "ScummVM 2D renderer";
	}

	void SetTintMethod(TintMethod /*method*/) override;
	bool SetDisplayMode(const DisplayMode &mode) override;
	void UpdateDeviceScreen(const Size &screen_sz) override;
	bool SetNativeResolution(const GraphicResolution &native_res) override;
	bool SetRenderFrame(const Rect &dst_rect) override;
	bool IsModeSupported(const DisplayMode &mode) override;
	int  GetDisplayDepthForNativeDepth(int native_color_depth) const override;
	IGfxModeList *GetSupportedModeList(int color_depth) override;
	PGfxFilter GetGraphicsFilter() const override;
	void UnInit();
	// Clears the screen rectangle. The coordinates are expected in the **native game resolution**.
	void ClearRectangle(int x1, int y1, int x2, int y2, RGB *colorToUse) override;
	int  GetCompatibleBitmapFormat(int color_depth) override;
	size_t GetAvailableTextureMemory() override {
		// not using textures for sprites anyway
		return 0;
	}
	IDriverDependantBitmap *CreateDDB(int width, int height, int color_depth, bool opaque) override;
	IDriverDependantBitmap *CreateDDBFromBitmap(Bitmap *bitmap, bool has_alpha, bool opaque) override;
	IDriverDependantBitmap *CreateRenderTargetDDB(int width, int height, int color_depth, bool opaque) override;
	void UpdateDDBFromBitmap(IDriverDependantBitmap *ddb, Bitmap *bitmap, bool has_alpha) override;
	void DestroyDDB(IDriverDependantBitmap *ddb) override;

	IDriverDependantBitmap *GetSharedDDB(uint32_t /*sprite_id*/,
		Bitmap *bitmap, bool has_alpha, bool opaque) override {
		// Software renderer does not require a texture cache, because it uses bitmaps directly
		return CreateDDBFromBitmap(bitmap, has_alpha, opaque);
	}

	void UpdateSharedDDB(uint32_t /*sprite_id*/, Bitmap */*bitmap*/, bool /*has_alpha*/, bool /*opaque*/) override {
		/* do nothing */
	}
	void ClearSharedDDB(uint32_t /*sprite_id*/) override {
		/* do nothing */
	}

	void DrawSprite(int x, int y, IDriverDependantBitmap *ddb) override;
	void SetScreenFade(int red, int green, int blue) override;
	void SetScreenTint(int red, int green, int blue) override;
	void SetStageScreen(const Size &sz, int x = 0, int y = 0) override;

	void RenderToBackBuffer() override;
	void Render() override;
	void Render(int xoff, int yoff, Shared::GraphicFlip flip) override;
	bool GetCopyOfScreenIntoBitmap(Bitmap *destination, const Rect *src_rect, bool at_native_res, GraphicResolution *want_fmt,
								   uint32_t batch_skip_filter = 0u) override;
	void FadeOut(int speed, int targetColourRed, int targetColourGreen, int targetColourBlue,
				 uint32_t batch_skip_filter = 0u) override;
	void FadeIn(int speed, PALETTE pal, int targetColourRed, int targetColourGreen, int targetColourBlue,
				uint32_t batch_skip_filter = 0u) override;
	void BoxOutEffect(bool blackingOut, int speed, int delay, uint32_t batch_skip_filter = 0u) override;
	bool SupportsGammaControl() override;
	void SetGamma(int newGamma) override;
	void UseSmoothScaling(bool /*enabled*/) override {}
	bool DoesSupportVsyncToggle() override;
	void RenderSpritesAtScreenResolution(bool /*enabled*/) override {}
	Bitmap *GetMemoryBackBuffer() override;
	void SetMemoryBackBuffer(Bitmap *backBuffer) override;
	Bitmap *GetStageBackBuffer(bool mark_dirty) override;
	void SetStageBackBuffer(Bitmap *backBuffer) override;
	bool GetStageMatrixes(RenderMatrixes & /*rm*/) override {
		return false; /* not supported */
	}

	typedef std::shared_ptr<ScummVMRendererGfxFilter> PSDLRenderFilter;

	void SetGraphicsFilter(PSDLRenderFilter filter);

protected:
	bool SetVsyncImpl(bool vsync, bool &vsync_res) override;
	size_t GetLastDrawEntryIndex() override {
		return _spriteList.size();
	}

private:
	Graphics::Screen *_screen = nullptr;
	PSDLRenderFilter _filter;

	bool _hasGamma = false;
#ifdef TODO
	uint16 _defaultGammaRed[256] {};
	uint16 _defaultGammaGreen[256] {};
	uint16 _defaultGammaBlue[256] {};
	int _gamma = 100;
#endif

	/*  SDL_Renderer *_renderer = nullptr;
	    SDL_Texture *_screenTex = nullptr; */
	// BITMAP struct for wrapping screen texture locked pixels, so that we may use blit()
	BITMAP *_fakeTexBitmap = nullptr;
	unsigned char *_lastTexPixels = nullptr;
	int _lastTexPitch = -1;

	// Original virtual screen created and managed by the renderer.
	std::unique_ptr<Bitmap> _origVirtualScreen;
	// Current virtual screen bitmap; may be either pointing to _origVirtualScreen,
	// or provided by external user (for example - plugin).
	// Its pixels are copied to the video texture to be presented by SDL_Renderer.
	Bitmap *virtualScreen;
	// Stage screen meant for particular rendering stages, may be referencing
	// actual virtual screen or separate bitmap of different size that is
	// blitted to virtual screen at the stage finalization.
	Bitmap *_stageVirtualScreen;
	int _tint_red, _tint_green, _tint_blue;

	// Sprite batches (parent scene nodes)
	ALSpriteBatches _spriteBatches;
	// List of sprites to render
	std::vector<ALDrawListEntry> _spriteList;

	void InitSpriteBatch(size_t index, const SpriteBatchDesc &desc) override;
	void ResetAllBatches() override;

	// Use gfx filter to create a new virtual screen
	void CreateVirtualScreen();
	void DestroyVirtualScreen();
	// Unset parameters and release resources related to the display mode
	void ReleaseDisplayMode();
	// Renders single sprite batch on the precreated surface
	size_t RenderSpriteBatch(const ALSpriteBatch &batch, size_t from, Shared::Bitmap *surface, int surf_offx, int surf_offy);

	void highcolor_fade_in(Bitmap *vs, void(*draw_callback)(), int speed, int targetColourRed, int targetColourGreen, int targetColourBlue);
	void highcolor_fade_out(Bitmap *vs, void(*draw_callback)(), int speed, int targetColourRed, int targetColourGreen, int targetColourBlue);
	void __fade_from_range(PALETTE source, PALETTE dest, int speed, int from, int to);
	void __fade_out_range(int speed, int from, int to, int targetColourRed, int targetColourGreen, int targetColourBlue);
	// Copy raw screen bitmap pixels to the screen
	void copySurface(const Graphics::Surface &src, bool mode);
	// Render bitmap on screen
	void Present(int xoff = 0, int yoff = 0, Shared::GraphicFlip flip = Shared::kFlip_None);
};


class ScummVMRendererGraphicsFactory : public GfxDriverFactoryBase<ScummVMRendererGraphicsDriver, ScummVMRendererGfxFilter> {
public:
	~ScummVMRendererGraphicsFactory() override;

	size_t               GetFilterCount() const override;
	const GfxFilterInfo *GetFilterInfo(size_t index) const override;
	String               GetDefaultFilterID() const override;

	static ScummVMRendererGraphicsFactory *GetFactory();

private:
	ScummVMRendererGraphicsDriver *EnsureDriverCreated() override;
	ScummVMRendererGfxFilter *CreateFilter(const String &id) override;

	static ScummVMRendererGraphicsFactory *_factory;
};

} // namespace ALSW
} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
