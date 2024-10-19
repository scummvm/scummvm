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
#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/math/vector2.h"

#include "graphics/transform_struct.h"
#include "graphics/surface.h"

#include "math/matrix4.h"
#include "math/ray.h"

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

	//virtual void DumpData(char* Filename);
	virtual bool enableShadows() = 0;
	virtual bool disableShadows() = 0;
	virtual bool usingStencilBuffer() = 0;
	virtual void displayShadow(BaseObject *object, const DXVector3 *light, bool lightPosRelative) = 0;
	//HRESULT InvalidateTexture(LPDIRECT3DTEXTURE Texture);

	virtual void setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode) = 0;
	// declared in sub class: virtual const char* GetName();
	// declared in sub class: virtual HRESULT DisplayDebugInfo();

	// declared in sub class: virtual CBImage* TakeScreenshot();
	// declared in sub class: virtual HRESULT SetViewport(int left, int top, int right, int bottom);
	// NOT declared in sub class: HRESULT InvalidateDeviceObjects();
	// NOT declared in sub class: HRESULT RestoreDeviceObjects();
	void fade(uint16 alpha) override;
	// declared in sub class: virtual HRESULT FadeToColor(DWORD Color, RECT* rect=NULL);
	// declared in sub class: virtual HRESULT DrawLine(int X1, int Y1, int X2, int Y2, DWORD Color);
	// declared in sub class: virtual HRESULT SetProjection();
	bool drawSprite(BaseSurfaceOpenGL3D &tex, const Rect32 &rect, float zoomX, float zoomY, const Vector2 &pos,
					uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY);
	virtual bool drawSpriteEx(BaseSurfaceOpenGL3D &tex, const Rect32 &rect, const Vector2 &pos, const Vector2 &rot, const Vector2 &scale,
							  float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) = 0;
	// declared in sub class: virtual HRESULT Setup3D(C3DCamera* Camera=NULL, bool Force=false);
	// NOT declared in sub class: virtual HRESULT Setup3DCustom(D3DXMATRIX* ViewMat, D3DXMATRIX* ProjMat);
	// declared in sub class: virtual HRESULT Setup2D(bool Force=false);
	// declared in sub class: virtual HRESULT SetupLines();
	// declared in sub class: HRESULT ResetDevice();
	void initLoop() override;
	// declared in sub class: virtual HRESULT Fill(BYTE r, BYTE g, BYTE b, RECT* rect=NULL);
	// declared in sub class: virtual HRESULT Flip();
	// declared in sub class: virtual HRESULT InitRenderer(CHWManager* hwManager);
	// NOT declared in sub class: virtual HRESULT SwitchFullscreen();
	// declared in sub class: virtual HRESULT WindowedBlt();

	// declared in sub class: virtual bool UsingStencilBuffer();

	// declared in sub class: virtual HRESULT StartSpriteBatch();
	// declared in sub class: virtual HRESULT EndSpriteBatch();
	// NOT declared in sub class: HRESULT CommitSpriteBatch();

	// declared in sub class: virtual HRESULT DrawShaderQuad();

	
	// ScummVM specific methods -->

	virtual void enableLight(int index) = 0;
	virtual void disableLight(int index) = 0;
	virtual void setLightParameters(int index, const DXVector3 &position, const DXVector3 &direction,
	                                const DXVector4 &diffuse, bool spotlight) = 0;

	virtual void enableCulling() = 0;
	virtual void disableCulling() = 0;

	Rect32 getViewPort() override;

	Graphics::PixelFormat getPixelFormat() const override;

	virtual bool setProjection2D() = 0;

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

	Camera3D *_camera;

	virtual Mesh3DS *createMesh3DS() = 0;
	virtual XMesh *createXMesh() = 0;
	virtual ShadowVolume *createShadowVolume() = 0;


	virtual void renderSceneGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks,
	                                 const BaseArray<AdGeneric *> &generics, const BaseArray<Light3D *> &lights, Camera3D *camera) = 0;
	virtual void renderShadowGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks, const BaseArray<AdGeneric *> &generics, Camera3D *camera) = 0;

	Math::Matrix3 build2dTransformation(const Vector2 &center, float angle);

	// ScummVM specific methods <--

protected:
	DXMatrix _worldMatrix;
	DXMatrix _viewMatrix;
	DXMatrix _projectionMatrix;
	Rect32 _viewport;
	float _fov;
	float _nearClipPlane;
	float _farClipPlane;
	TRendererState _state;
	bool _spriteBatchMode;

	// NOT declared in sub class: HRESULT CreateShaderQuad();
	virtual void setAmbientLightRenderState() = 0;
	// NOT declared in sub class: D3DMATRIX* BuildMatrix(D3DMATRIX* pOut, const D3DXVECTOR2* centre, const D3DXVECTOR2* scaling, float angle);
	// NOT declared in sub class: void TransformVertices(struct SPRITEVERTEX* vertices, const D3DXVECTOR2* pCentre, const D3DXVECTOR2* pScaling, float angle);

	// ScummVM specific methods:
	void flipVertical(Graphics::Surface *s);
};

} // namespace Wintermute

#endif
