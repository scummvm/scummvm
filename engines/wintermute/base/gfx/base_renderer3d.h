/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "math/matrix4.h"
#include "math/ray.h"

#if defined(USE_OPENGL_SHADERS)

#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/texture.h"

#endif

namespace Wintermute {

class AdBlock;
class AdGeneric;
class AdWalkplane;
class BaseSurfaceOpenGL3D;
class Light3D;
class Mesh3DS;
class MeshX;
class ShadowVolume;

class BaseRenderer3D : public BaseRenderer {
public:
	BaseRenderer3D(BaseGame *inGame = nullptr);
	~BaseRenderer3D() override;

	bool setAmbientLightColor(uint32 color);
	bool setDefaultAmbientLightColor();
	virtual void setAmbientLight() = 0;

	uint32 _ambientLightColor;
	bool _overrideAmbientLightColor;

	virtual int maximumLightsCount() = 0;
	virtual void enableLight(int index) = 0;
	virtual void disableLight(int index) = 0;
	virtual void setLightParameters(int index, const Math::Vector3d &position, const Math::Vector3d &direction, const Math::Vector4d &diffuse, bool spotlight) = 0;

	virtual void setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode) = 0;

	virtual void enableCulling() = 0;
	virtual void disableCulling() = 0;

	virtual bool enableShadows() = 0;
	virtual bool disableShadows() = 0;
	virtual void displayShadow(BaseObject *object, const Math::Vector3d &light, bool lightPosRelative) = 0;
	virtual bool stencilSupported() = 0;

	Rect32 getViewPort() override;

	Graphics::PixelFormat getPixelFormat() const override;
	void fade(uint16 alpha) override;

	void initLoop() override;

	virtual bool setProjection2D() = 0;
	virtual void setWorldTransform(const Math::Matrix4 &transform) = 0;

	void project(const Math::Matrix4 &worldMatrix, const Math::Vector3d &point, int32 &x, int32 &y);
	Math::Ray rayIntoScene(int x, int y);

	Math::Matrix4 lastProjectionMatrix() {
		return _projectionMatrix3d;
	}

	virtual Mesh3DS *createMesh3DS() = 0;
	virtual MeshX *createMeshX() = 0;
	virtual ShadowVolume *createShadowVolume() = 0;

	bool drawSprite(BaseSurfaceOpenGL3D &tex, const Rect32 &rect, float zoomX, float zoomY, const Vector2 &pos,
					uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY);
	virtual bool drawSpriteEx(BaseSurfaceOpenGL3D &tex, const Rect32 &rect, const Vector2 &pos, const Vector2 &rot, const Vector2 &scale,
	                          float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) = 0;

	virtual void renderSceneGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks,
	                                 const BaseArray<AdGeneric *> &generics, const BaseArray<Light3D *> &lights, Camera3D *camera) = 0;
	virtual void renderShadowGeometry(const BaseArray<AdWalkplane *> &planes, const BaseArray<AdBlock *> &blocks, const BaseArray<AdGeneric *> &generics, Camera3D *camera) = 0;

	Math::Matrix3 build2dTransformation(const Vector2 &center, float angle);

protected:
	Math::Matrix4 _lastViewMatrix;
	Math::Matrix4 _projectionMatrix3d;
	Rect32 _viewport3dRect;
};

} // namespace Wintermute

#endif
