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
// OpenGL graphics factory
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_ALI3DOGL_H
#define AGS_ENGINE_GFX_ALI3DOGL_H

#include <memory>
#include "gfx/bitmap.h"
#include "gfx/ddb.h"
#include "gfx/gfxdriverfactorybase.h"
#include "gfx/gfxdriverbase.h"
#include "util/string.h"
#include "util/version.h"

#include "ogl_headers.h"

namespace AGS {
namespace Engine {

namespace OGL {

using Common::Bitmap;
using Common::String;
using Common::Version;

typedef struct _OGLVECTOR {
	float x;
	float y;
} OGLVECTOR2D;


struct OGLCUSTOMVERTEX {
	OGLVECTOR2D position;
	float tu;
	float tv;
};

struct OGLTextureTile : public TextureTile {
	unsigned int texture;
};

class OGLBitmap : public VideoMemDDB {
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
	void SetLightLevel(int lightLevel) override  {
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
	OGLCUSTOMVERTEX *_vertex;
	OGLTextureTile *_tiles;
	int _numTiles;

	OGLBitmap(int width, int height, int colDepth, bool opaque) {
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
		_vertex = nullptr;
		_tiles = nullptr;
		_numTiles = 0;
	}

	int GetWidthToRender() const {
		return (_stretchToWidth > 0) ? _stretchToWidth : _width;
	}
	int GetHeightToRender() const {
		return (_stretchToHeight > 0) ? _stretchToHeight : _height;
	}

	void Dispose();

	~OGLBitmap() override {
		Dispose();
	}
};

typedef SpriteDrawListEntry<OGLBitmap> OGLDrawListEntry;
typedef struct GLMATRIX {
	GLfloat m[16];
} GLMATRIX;
struct OGLSpriteBatch {
	// List of sprites to render
	std::vector<OGLDrawListEntry> List;
	// Clipping viewport
	Rect Viewport;
	// Transformation matrix, built from the batch description
	GLMATRIX Matrix;
};
typedef std::vector<OGLSpriteBatch>    OGLSpriteBatches;


class OGLDisplayModeList : public IGfxModeList {
public:
	OGLDisplayModeList(const std::vector<DisplayMode> &modes)
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


class OGLGfxFilter;

class OGLGraphicsDriver : public VideoMemoryGraphicsDriver {
public:
	const char *GetDriverName() override {
		return "OpenGL";
	}
	const char *GetDriverID() override {
		return "OGL";
	}
	void SetTintMethod(TintMethod method) override;
	bool SetDisplayMode(const DisplayMode &mode, volatile int *loopTimer) override;
	bool SetNativeSize(const Size &src_size) override;
	bool SetRenderFrame(const Rect &dst_rect) override;
	int GetDisplayDepthForNativeDepth(int native_color_depth) const override;
	IGfxModeList *GetSupportedModeList(int color_depth) override;
	bool IsModeSupported(const DisplayMode &mode) override;
	PGfxFilter GetGraphicsFilter() const override;
	void UnInit();
	// Clears the screen rectangle. The coordinates are expected in the **native game resolution**.
	void ClearRectangle(int x1, int y1, int x2, int y2, RGB *colorToUse) override;
	int  GetCompatibleBitmapFormat(int color_depth) override;
	IDriverDependantBitmap *CreateDDBFromBitmap(Bitmap *bitmap, bool hasAlpha, bool opaque) override;
	void UpdateDDBFromBitmap(IDriverDependantBitmap *bitmapToUpdate, Bitmap *bitmap, bool hasAlpha) override;
	void DestroyDDB(IDriverDependantBitmap *bitmap) override;
	void DrawSprite(int x, int y, IDriverDependantBitmap *bitmap) override;
	void RenderToBackBuffer() override;
	void Render() override;
	void Render(int xoff, int yoff, GlobalFlipType flip) override;
	bool GetCopyOfScreenIntoBitmap(Bitmap *destination, bool at_native_res, GraphicResolution *want_fmt) override;
	void EnableVsyncBeforeRender(bool enabled) override { }
	void Vsync() override;
	void RenderSpritesAtScreenResolution(bool enabled, int supersampling) override;
	void FadeOut(int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) override;
	void FadeIn(int speed, PALETTE p, int targetColourRed, int targetColourGreen, int targetColourBlue) override;
	void BoxOutEffect(bool blackingOut, int speed, int delay) override;
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
	void SetScreenFade(int red, int green, int blue) override;
	void SetScreenTint(int red, int green, int blue) override;

	typedef std::shared_ptr<OGLGfxFilter> POGLFilter;

	void SetGraphicsFilter(POGLFilter filter);

	OGLGraphicsDriver();
	~OGLGraphicsDriver() override;

private:
	POGLFilter _filter;

#if AGS_PLATFORM_OS_WINDOWS
	HDC _hDC;
	HGLRC _hRC;
	HWND _hWnd;
	HINSTANCE _hInstance;
	GLuint _oldPixelFormat;
	PIXELFORMATDESCRIPTOR _oldPixelFormatDesc;
#endif
#if AGS_PLATFORM_OS_LINUX
	bool _have_window;
	GLXContext _glxContext;
#endif
	bool _firstTimeInit;
	// Position of backbuffer texture in world space
	GLfloat _backbuffer_vertices[8];
	// Relative position of source image on the backbuffer texture,
	// in local coordinates
	GLfloat _backbuffer_texture_coordinates[8];
	OGLCUSTOMVERTEX defaultVertices[4];
	String previousError;
	bool _smoothScaling;
	bool _legacyPixelShader;
	// Shader program and its variable references;
	// the variables are rather specific for AGS use (sprite tinting).
	struct ShaderProgram {
		GLuint Program;
		GLuint SamplerVar;      // texture ID
		GLuint ColorVar;        // primary operation variable
		GLuint AuxVar;          // auxiliary variable

		ShaderProgram();
	};
	ShaderProgram _tintShader;
	ShaderProgram _lightShader;

	int device_screen_physical_width;
	int device_screen_physical_height;

	// Viewport and scissor rect, in OpenGL screen coordinates (0,0 is at left-bottom)
	Rect _viewportRect;

	// These two flags define whether driver can, and should (respectively)
	// render sprites to texture, and then texture to screen, as opposed to
	// rendering to screen directly. This is known as supersampling mode
	bool _can_render_to_texture;
	bool _do_render_to_texture;
	// Backbuffer texture multiplier, used to determine a size of texture
	// relative to the native game size.
	int _super_sampling;
	unsigned int _backbuffer;
	unsigned int _fbo;
	// Size of the backbuffer drawing area, equals to native size
	// multiplied by _super_sampling
	Size _backRenderSize;
	// Actual size of the backbuffer texture, created by OpenGL
	Size _backTextureSize;

	OGLSpriteBatches _spriteBatches;
	// TODO: these draw list backups are needed only for the fade-in/out effects
	// find out if it's possible to reimplement these effects in main drawing routine.
	SpriteBatchDescs _backupBatchDescs;
	OGLSpriteBatches _backupBatches;

	void InitSpriteBatch(size_t index, const SpriteBatchDesc &desc) override;
	void ResetAllBatches() override;

	// Sets up GL objects not related to particular display mode
	void FirstTimeInit();
	// Initializes Gl rendering context
	bool InitGlScreen(const DisplayMode &mode);
	bool CreateGlContext(const DisplayMode &mode);
	void DeleteGlContext();
	// Sets up general rendering parameters
	void InitGlParams(const DisplayMode &mode);
	void SetupDefaultVertices();
	// Test if rendering to texture is supported
	void TestRenderToTexture();
	// Test if supersampling should be allowed with the current setup
	void TestSupersampling();
	// Create shader programs for sprite tinting and changing light level
	void CreateShaders();
	void CreateTintShader();
	void CreateLightShader();
	void CreateShaderProgram(ShaderProgram &prg, const char *name, const char *fragment_shader_src,
	                         const char *sampler_var, const char *color_var, const char *aux_var);
	void DeleteShaderProgram(ShaderProgram &prg);
	void OutputShaderError(GLuint obj_id, const String &obj_name, bool is_shader);
	// Configure backbuffer texture, that is used in render-to-texture mode
	void SetupBackbufferTexture();
	void DeleteBackbufferTexture();
#if AGS_PLATFORM_OS_WINDOWS || AGS_PLATFORM_OS_LINUX
	void CreateDesktopScreen(int width, int height, int depth);
#elif AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS
	void UpdateDeviceScreen();
#endif
	// Unset parameters and release resources related to the display mode
	void ReleaseDisplayMode();
	void AdjustSizeToNearestSupportedByCard(int *width, int *height);
	void UpdateTextureRegion(OGLTextureTile *tile, Bitmap *bitmap, OGLBitmap *target, bool hasAlpha);
	void CreateVirtualScreen();
	void do_fade(bool fadingOut, int speed, int targetColourRed, int targetColourGreen, int targetColourBlue);
	void _renderSprite(const OGLDrawListEntry *entry, const GLMATRIX &matGlobal);
	void SetupViewport();
	// Converts rectangle in top->down coordinates into OpenGL's native bottom->up coordinates
	Rect ConvertTopDownRect(const Rect &top_down_rect, int surface_height);

	// Backup all draw lists in the temp storage
	void BackupDrawLists();
	// Restore draw lists from the temp storage
	void RestoreDrawLists();
	// Deletes draw list backups
	void ClearDrawBackups();
	void _render(bool clearDrawListAfterwards);
	void RenderSpriteBatches();
	void RenderSpriteBatch(const OGLSpriteBatch &batch);
	void _reDrawLastFrame();
};


class OGLGraphicsFactory : public GfxDriverFactoryBase<OGLGraphicsDriver, OGLGfxFilter> {
public:
	~OGLGraphicsFactory() override;

	size_t               GetFilterCount() const override;
	const GfxFilterInfo *GetFilterInfo(size_t index) const override;
	String               GetDefaultFilterID() const override;

	static OGLGraphicsFactory   *GetFactory();

private:
	OGLGraphicsDriver   *EnsureDriverCreated() override;
	OGLGfxFilter        *CreateFilter(const String &id) override;

	static OGLGraphicsFactory *_factory;
};

} // namespace OGL
} // namespace Engine
} // namespace AGS

#endif
