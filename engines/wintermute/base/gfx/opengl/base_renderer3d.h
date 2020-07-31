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
#include "engines/wintermute/dctypes.h"
#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/math/vector2.h"
#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/texture.h"
#include "graphics/transform_struct.h"
#include "math/matrix4.h"
#include "math/ray.h"

namespace Wintermute {

class BaseSurfaceOpenGL3D;
class Mesh3DS;
class MeshX;
class ShadowVolume;

class BaseRenderer3D : public BaseRenderer {
public:
	BaseRenderer3D(BaseGame *inGame = nullptr);
	~BaseRenderer3D() override;

	virtual bool setAmbientLightColor(uint32 color) = 0;
	virtual bool setDefaultAmbientLightColor() = 0;
	virtual void setAmbientLight() = 0;

	uint32 _ambientLightColor;
	bool _overrideAmbientLightColor;

	virtual int maximumLightsCount() = 0;
	virtual void enableLight(int index) = 0;
	virtual void disableLight(int index) = 0;
	virtual void setLightParameters(int index, const Math::Vector3d &position, const Math::Vector3d &direction, const Math::Vector4d &diffuse, bool spotlight) = 0;

	virtual void setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode) = 0;

	virtual bool enableShadows() = 0;
	virtual bool disableShadows() = 0;
	virtual bool stencilSupported() = 0;

	virtual bool setProjection2D() = 0;
	virtual void resetModelViewTransform() = 0;
	virtual void setWorldTransform(const Math::Matrix4 &transform) = 0;

	virtual void project(const Math::Matrix4 &worldMatrix, const Math::Vector3d &point, int &x, int &y);
	virtual Math::Ray rayIntoScene(int x, int y);

	virtual Math::Matrix4 lastProjectionMatrix() {
		return _projectionMatrix3d;
	}

	virtual Mesh3DS *createMesh3DS() = 0;
	virtual MeshX *createMeshX() = 0;
	virtual ShadowVolume *createShadowVolume() = 0;

	virtual bool drawSprite(BaseSurfaceOpenGL3D &tex, const Rect32 &rect, float zoomX, float zoomY, const Vector2 &pos,
	                        uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) = 0;
	virtual bool drawSpriteEx(BaseSurfaceOpenGL3D &tex, const Rect32 &rect, const Vector2 &pos, const Vector2 &rot, const Vector2 &scale,
	                          float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) = 0;

protected:
	Math::Matrix4 _lastViewMatrix;
	Math::Matrix4 _projectionMatrix3d;
};

} // namespace Wintermute

#endif
