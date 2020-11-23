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
// Direct3D graphics factory
//
//=============================================================================

#ifndef AGS_ENGINE_PLATFORM_WINDOWS_ALI3DD3D_H
#define AGS_ENGINE_PLATFORM_WINDOWS_ALI3DD3D_H

#include "ags/shared/core/platform.h"

#if ! AGS_PLATFORM_OS_WINDOWS
#error This file should only be included on the Windows build
#endif

#include "ags/std/memory.h"
#include "ags/lib/allegro.h"
//include <winalleg.h>
//include <d3d9.h>
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/gfx/ddb.h"
#include "ags/shared/gfx/gfxdriverfactorybase.h"
#include "ags/shared/gfx/gfxdriverbase.h"
#include "ags/shared/util/library.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

namespace AGS {
namespace Engine {
namespace D3D {

using AGS::Shared::Bitmap;
using AGS::Shared::String;
class D3DGfxFilter;

struct D3DTextureTile : public TextureTile {
	IDirect3DTexture9 *texture;
};

class D3DBitmap : public VideoMemDDB {
public:
	// Transparency is a bit counter-intuitive
	// 0=not transparent, 255=invisible, 1..254 barely visible .. mostly visible
	void SetTransparency(int transparency) override {
		_transparency = transparency;
	}
	void SetFlippedLeftRight(bool isFlipped) override {
		_flipped = isFlipped;
	}
	void SetStretch(int width, int height, bool useResampler = true) override {
		_stretchToWidth = width;
		_stretchToHeight = height;
		_useResampler = useResampler;
	}
	void SetLightLevel(int lightLevel) override {
		_lightLevel = lightLevel;
	}
	void SetTint(int red, int green, int blue, int tintSaturation) override {
		_red = red;
		_green = green;
		_blue = blue;
		_tintSaturation = tintSaturation;
	}

	bool _flipped;
	int _stretchToWidth, _stretchToHeight;
	bool _useResampler;
	int _red, _green, _blue;
	int _tintSaturation;
	int _lightLevel;
	bool _hasAlpha;
	int _transparency;
	IDirect3DVertexBuffer9 *_vertex;
	D3DTextureTile *_tiles;
	int _numTiles;

	D3DBitmap(int width, int height, int colDepth, bool opaque) {
		_width = width;
		_height = height;
		_colDepth = colDepth;
		_flipped = false;
		_hasAlpha = false;
		_stretchToWidth = 0;
		_stretchToHeight = 0;
		_useResampler = false;
		_red = _green = _blue = 0;
		_tintSaturation = 0;
		_lightLevel = 0;
		_transparency = 0;
		_opaque = opaque;
		_vertex = NULL;
		_tiles = NULL;
		_numTiles = 0;
	}

	int GetWidthToRender() {
		return (_stretchToWidth > 0) ? _stretchToWidth : _width;
	}
	int GetHeightToRender() {
		return (_stretchToHeight > 0) ? _stretchToHeight : _height;
	}

	void Dispose();

	~D3DBitmap() override {
		Dispose();
	}
};

class D3DGfxModeList : public IGfxModeList {
public:
	D3DGfxModeList(IDirect3D9 *direct3d, D3DFORMAT d3dformat)
		: _direct3d(direct3d)
		, _pixelFormat(d3dformat) {
		_modeCount = _direct3d ? _direct3d->GetAdapterModeCount(D3DADAPTER_DEFAULT, _pixelFormat) : 0;
	}

	~D3DGfxModeList() override {
		if (_direct3d)
			_direct3d->Release();
	}

	int GetModeCount() const override {
		return _modeCount;
	}

	bool GetMode(int index, DisplayMode &mode) const override;

private:
	IDirect3D9 *_direct3d;
	D3DFORMAT   _pixelFormat;
	int         _modeCount;
};

struct CUSTOMVERTEX {
	D3DVECTOR   position; // The position.
	D3DVECTOR   normal;
	FLOAT       tu, tv;   // The texture coordinates.
};

typedef SpriteDrawListEntry<D3DBitmap> D3DDrawListEntry;
// D3D renderer's sprite batch
struct D3DSpriteBatch {
	// List of sprites to render
	std::vector<D3DDrawListEntry> List;
	// Clipping viewport
	Rect Viewport;
	// Transformation matrix, built from the batch description
	D3DMATRIX Matrix;
};
typedef std::vector<D3DSpriteBatch>    D3DSpriteBatches;


class D3DGraphicsDriver : public VideoMemoryGraphicsDriver {
public:
	const char *GetDriverName() override {
		return "Direct3D 9";
	}
	const char *GetDriverID() override {
		return "D3D9";
	}
	void SetTintMethod(TintMethod method) override;
	bool SetDisplayMode(const DisplayMode &mode, volatile int *loopTimer) override;
	bool SetNativeSize(const Size &src_size) override;
	bool SetRenderFrame(const Rect &dst_rect) override;
	int  GetDisplayDepthForNativeDepth(int native_color_depth) const override;
	IGfxModeList *GetSupportedModeList(int color_depth) override;
	bool IsModeSupported(const DisplayMode &mode) override;
	PGfxFilter GetGraphicsFilter() const override;
	// Clears the screen rectangle. The coordinates are expected in the **native game resolution**.
	void ClearRectangle(int x1, int y1, int x2, int y2, RGB *colorToUse) override;
	int  GetCompatibleBitmapFormat(int color_depth) override;
	IDriverDependantBitmap *CreateDDBFromBitmap(Bitmap *bitmap, bool hasAlpha, bool opaque) override;
	void UpdateDDBFromBitmap(IDriverDependantBitmap *bitmapToUpdate, Bitmap *bitmap, bool hasAlpha) override;
	void DestroyDDB(IDriverDependantBitmap *bitmap) override;
	void DrawSprite(int x, int y, IDriverDependantBitmap *bitmap) override;
	void SetScreenFade(int red, int green, int blue) override;
	void SetScreenTint(int red, int green, int blue) override;
	void RenderToBackBuffer() override;
	void Render() override;
	void Render(int xoff, int yoff, GlobalFlipType flip) override;
	bool GetCopyOfScreenIntoBitmap(Bitmap *destination, bool at_native_res, GraphicResolution *want_fmt) override;
	void EnableVsyncBeforeRender(bool enabled) override {
	}
	void Vsync() override;
	void RenderSpritesAtScreenResolution(bool enabled, int supersampling) override {
		_renderSprAtScreenRes = enabled;
	};
	void FadeOut(int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) override;
	void FadeIn(int speed, PALETTE p, int targetColourRed, int targetColourGreen, int targetColourBlue) override;
	void BoxOutEffect(bool blackingOut, int speed, int delay) override;
#ifndef AGS_NO_VIDEO_PLAYER
	bool PlayVideo(const char *filename, bool useSound, VideoSkipType skipType, bool stretchToFullScreen) override;
#endif
	bool SupportsGammaControl() override;
	void SetGamma(int newGamma) override;
	void UseSmoothScaling(bool enabled) override {
		_smoothScaling = enabled;
	}
	bool RequiresFullRedrawEachFrame() override {
		return true;
	}
	bool HasAcceleratedTransform() override {
		return true;
	}

	typedef std::shared_ptr<D3DGfxFilter> PD3DFilter;

	// Clears screen rect, coordinates are expected in display resolution
	void ClearScreenRect(const Rect &r, RGB *colorToUse);
	void UnInit();
	void SetGraphicsFilter(PD3DFilter filter);

	// Internal; TODO: find a way to hide these
	int _initDLLCallback(const DisplayMode &mode);
	int _resetDeviceIfNecessary();

	D3DGraphicsDriver(IDirect3D9 *d3d);
	~D3DGraphicsDriver() override;

private:
	PD3DFilter _filter;

	IDirect3D9 *direct3d;
	D3DPRESENT_PARAMETERS d3dpp;
	IDirect3DDevice9 *direct3ddevice;
	D3DGAMMARAMP defaultgammaramp;
	D3DGAMMARAMP currentgammaramp;
	D3DCAPS9 direct3ddevicecaps;
	IDirect3DVertexBuffer9 *vertexbuffer;
	IDirect3DSurface9 *pNativeSurface;
	IDirect3DTexture9 *pNativeTexture;
	RECT viewport_rect;
	UINT availableVideoMemory;
	CUSTOMVERTEX defaultVertices[4];
	String previousError;
	IDirect3DPixelShader9 *pixelShader;
	bool _smoothScaling;
	bool _legacyPixelShader;
	float _pixelRenderXOffset;
	float _pixelRenderYOffset;
	bool _renderSprAtScreenRes;

	D3DSpriteBatches _spriteBatches;
	// TODO: these draw list backups are needed only for the fade-in/out effects
	// find out if it's possible to reimplement these effects in main drawing routine.
	SpriteBatchDescs _backupBatchDescs;
	D3DSpriteBatches _backupBatches;

	D3DVIEWPORT9 _d3dViewport;

	// Called after new mode was successfully initialized
	void OnModeSet(const DisplayMode &mode) override;
	void InitSpriteBatch(size_t index, const SpriteBatchDesc &desc) override;
	void ResetAllBatches() override;
	// Called when the direct3d device is created for the first time
	int  FirstTimeInit();
	void initD3DDLL(const DisplayMode &mode);
	void InitializeD3DState();
	void SetupViewport();
	HRESULT ResetD3DDevice();
	// Unset parameters and release resources related to the display mode
	void ReleaseDisplayMode();
	void set_up_default_vertices();
	void AdjustSizeToNearestSupportedByCard(int *width, int *height);
	void UpdateTextureRegion(D3DTextureTile *tile, Bitmap *bitmap, D3DBitmap *target, bool hasAlpha);
	void CreateVirtualScreen();
	void do_fade(bool fadingOut, int speed, int targetColourRed, int targetColourGreen, int targetColourBlue);
	bool IsTextureFormatOk(D3DFORMAT TextureFormat, D3DFORMAT AdapterFormat);
	// Backup all draw lists in the temp storage
	void BackupDrawLists();
	// Restore draw lists from the temp storage
	void RestoreDrawLists();
	// Deletes draw list backups
	void ClearDrawBackups();
	void _renderAndPresent(bool clearDrawListAfterwards);
	void _render(bool clearDrawListAfterwards);
	void _reDrawLastFrame();
	void RenderSpriteBatches();
	void RenderSpriteBatch(const D3DSpriteBatch &batch);
	void _renderSprite(const D3DDrawListEntry *entry, const D3DMATRIX &matGlobal);
	void _renderFromTexture();
};


class D3DGraphicsFactory : public GfxDriverFactoryBase<D3DGraphicsDriver, D3DGfxFilter> {
public:
	~D3DGraphicsFactory() override;

	size_t               GetFilterCount() const override;
	const GfxFilterInfo *GetFilterInfo(size_t index) const override;
	String               GetDefaultFilterID() const override;

	static D3DGraphicsFactory *GetFactory();
	static D3DGraphicsDriver *GetD3DDriver();

private:
	D3DGraphicsFactory();

	D3DGraphicsDriver *EnsureDriverCreated() override;
	D3DGfxFilter *CreateFilter(const String &id) override;

	bool Init();

	static D3DGraphicsFactory *_factory;
	//
	// IMPORTANT NOTE: since the Direct3d9 device is created with
	// D3DCREATE_MULTITHREADED behavior flag, once it is created the d3d9.dll may
	// only be unloaded after window is destroyed, as noted in the MSDN's article
	// on "D3DCREATE"
	// (http://msdn.microsoft.com/en-us/library/windows/desktop/bb172527.aspx).
	// Otherwise window becomes either destroyed prematurely or broken (details
	// are unclear), which causes errors during Allegro deinitialization.
	//
	// Curiously, this problem was only confirmed under WinXP so far.
	//
	// For the purpose of avoiding this problem, we have a static library wrapper
	// that unloads library only at the very program exit (except cases of device
	// creation failure).
	//
	// TODO: find out if there is better solution.
	//
	static Library      _library;
	IDirect3D9 *_direct3d;
};

} // namespace D3D
} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
