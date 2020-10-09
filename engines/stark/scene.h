/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_SCENE_H
#define STARK_SCENE_H

#include "common/rect.h"

#include "math/matrix4.h"
#include "math/ray.h"
#include "math/vector3d.h"

namespace Stark {

namespace Gfx {
class Driver;
class RenderEntry;
}

/** 
 * Manager for the current game scene
 */
class Scene {
public:
	Scene(Gfx::Driver *gfx);
	~Scene();

	void initCamera(const Math::Vector3d &position, const Math::Vector3d &lookAt,
			float fov, Common::Rect viewSize, float nearClipPlane, float farClipPlane);

	/** Configure rendering so that only the specified rect can be seen */
	void scrollCamera(const Common::Rect &viewport);

	/** Get the projection matrix (Screen -> Camera) */
	Math::Matrix4 getProjectionMatrix() const { return _projectionMatrix; }

	/** Get the projection matrix (Camera -> World) */
	Math::Matrix4 getViewMatrix() const { return _viewMatrix; }

	/**
	 * Compute a ray in world coordinates going from the camera and through the mouse
	 *
	 * @param mouse Mouse position
	 * @param origin Computed point of origin for the ray
	 * @param direction Computed direction for the ray
	 */
	Math::Ray makeRayFromMouse(const Common::Point &mouse) const;

	/**
	 * Convert a 3D position in world coordinates into a point in 2D screen space
	 *
	 * The computed 2D point is in original game view coordinates.
	 */
	Common::Point convertPosition3DToGameScreenOriginal(const Math::Vector3d &obj) const;

	/** Get and set scene fade level */
	void setFadeLevel(float fadeLevel);
	float getFadeLevel() const;

	/** Access the sway angle the actors are currently rotated by */
	void setSwayAngle(const Math::Angle &angle);
	Math::Angle getSwayAngle() const;

	/** Get the axis for the the sway actor rotation, in world coordinates */
	Math::Vector3d getSwayDirection() const;

	/** Access the float up / down current Z offset for the actors */
	void setFloatOffset(float floatOffset);
	float getFloatOffset() const;

	/** Access the maximum length of the horizontal light direction for casting shadows */
	void setupShadows(bool enabled, float length);
	bool shouldRenderShadows() const { return _shouldRenderShadows; }
	float getMaxShadowLength() const { return _maxShadowLength; }

private:
	void computeClippingRect(float *xmin, float *xmax, float *ymin, float *ymax);

	Gfx::Driver *_gfx;

	Math::Vector3d _cameraPosition;
	Math::Vector3d _cameraLookDirection;
	float _fov;
	Common::Rect _viewSize;
	Common::Rect _viewport;
	float _nearClipPlane;
	float _farClipPlane;

	Math::Matrix4 _projectionMatrix;
	Math::Matrix4 _viewMatrix;

	float _fadeLevel;
	Math::Angle _swayAngle;
	float _floatOffset;

	bool _shouldRenderShadows;
	float _maxShadowLength;
};

} // End of namespace Stark

#endif // STARK_SCENE_H
