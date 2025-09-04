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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WINTERMUTE_BASE_RENDERER_3D_H
#define WINTERMUTE_BASE_RENDERER_3D_H

#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/dctypes.h"

#include "graphics/transform_struct.h"
#include "graphics/surface.h"

#if defined(USE_OPENGL_SHADERS)

#include "graphics/opengl/system_headers.h"

#include "engines/wintermute/base/gfx/xmath.h"

#endif

namespace Wintermute {

class AdBlock;
class AdGeneric;
class AdWalkplane;
class BaseSurfaceOpenGL3D;
class Light3D;
class Mesh3DS;
class XMesh;
class ShadowVolume;

#define DEFAULT_NEAR_PLANE 90.0f
#define DEFAULT_FAR_PLANE  10000.0f

enum PostFilter {
	kPostFilterOff,
	kPostFilterBlackAndWhite,
	kPostFilterSepia
};

class BaseRenderer3D : public BaseRenderer {
public:
	BaseRenderer3D(BaseGame *inGame = nullptr);
	~BaseRenderer3D() override;

	bool getProjectionParams(float *resWidth, float *resHeight, float *layerWidth, float *layerHeight,
	                         float *modWidth, float *modHeight, bool *customViewport);
	virtual int getMaxActiveLights() = 0;

	bool setAmbientLightColor(uint32 color);
	bool setDefaultAmbientLightColor();

	uint32 _ambientLightColor;
	bool _ambientLightOverride;

	void dumpData(const char *filename) {};
	bool setup3DCustom(DXMatrix &viewMat, DXMatrix &projMat);
	virtual bool enableShadows() = 0;
	virtual bool disableShadows() = 0;
	virtual bool stencilSupported() = 0;
	virtual bool invalidateTexture(BaseSurfaceOpenGL3D *texture) = 0;

	virtual void setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode, bool forceChange = false) = 0;

	virtual bool invalidateDeviceObjects() = 0;
	virtual bool restoreDeviceObjects() = 0;
	BaseSurfaceOpenGL3D *_lastTexture;
	void fade(uint16 alpha) override;
	bool drawSprite(BaseSurface *texture, const Common::Rect32 &rect, float zoomX, float zoomY, const DXVector2 &pos,
	                uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY);
	virtual bool drawSpriteEx(BaseSurface *texture, const Common::Rect32 &rect, const DXVector2 &pos, const DXVector2 &rot, const DXVector2 &scale,
	                float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) = 0;
	Camera3D *_camera;
	virtual bool resetDevice() = 0;
	void initLoop() override;
	bool windowedBlt() override;

	virtual bool startSpriteBatch() override = 0;
	virtual bool endSpriteBatch() override = 0;
	virtual bool commitSpriteBatch() = 0;


	// ScummVM specific methods -->

	virtual void lightEnable(int index, bool enable) = 0;
	virtual void setLightParameters(int index, const DXVector3 &position, const DXVector3 &direction,
	                                const DXVector4 &diffuse, bool spotlight) = 0;

	virtual void enableCulling() = 0;
	virtual void disableCulling() = 0;

	DXViewport getViewPort();

	void setWindowed(bool windowed) override;
	void onWindowChange() override;

	Graphics::PixelFormat getPixelFormat() const override;

	virtual bool setWorldTransform(const DXMatrix &transform) = 0;
	virtual bool setViewTransform(const DXMatrix &transform) = 0;
	virtual bool setProjectionTransform(const DXMatrix &transform) = 0;

	void getWorldTransform(DXMatrix *transform) {
		*transform = _worldMatrix;
	}

	void getViewTransform(DXMatrix *transform) {
		*transform = _viewMatrix;
	}

	void getProjectionTransform(DXMatrix *transform) {
		*transform = _projectionMatrix;
	}

	virtual Mesh3DS *createMesh3DS() = 0;
	virtual XMesh *createXMesh() = 0;
	virtual ShadowVolume *createShadowVolume() = 0;


	virtual void renderSceneGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks,
	                                 const BaseArray<AdGeneric *> &generics, const BaseArray<Light3D *> &lights, Camera3D *camera) = 0;
	virtual void renderShadowGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks, const BaseArray<AdGeneric *> &generics, Camera3D *camera) = 0;

	virtual void displaySimpleShadow(BaseObject *object) = 0;

	virtual void postfilter() = 0;
	virtual void setPostfilter(PostFilter postFilter) = 0;
	bool flip() override;
	bool indicatorFlip(int32 x, int32 y, int32 width, int32 height) override;
	bool forcedFlip() override;
	virtual bool setViewport3D(DXViewport *viewport) = 0;

	// ScummVM specific methods <--

protected:
	DXMatrix _worldMatrix;
	DXMatrix _viewMatrix;
	DXMatrix _projectionMatrix;
	DXViewport _viewport{};
	float _fov;
	float _nearClipPlane;
	float _farClipPlane;
	TRendererState _state;
	PostFilter _postFilterMode;

	virtual void setAmbientLightRenderState() = 0;
};

} // namespace Wintermute

#endif
