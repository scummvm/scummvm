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

#include "engines/myst3/hotspot.h"
#include "engines/myst3/scene.h"
#include "engines/myst3/state.h"

#include "common/config-manager.h"
#include "common/math.h"

#include "math/ray.h"

namespace Myst3 {

static void polarRectTo3dRect(const PolarRect &polarRect,
                              Math::Vector3d &topLeft, Math::Vector3d &topRight,
                              Math::Vector3d &bottomLeft, Math::Vector3d &bottomRight) {
	static const float scale = 50.0;

	Math::Vector3d direction = Scene::directionToVector(polarRect.centerPitch, 90.0 - polarRect.centerHeading) * scale;

	Math::Vector3d u = Math::Vector3d(direction.z(), 0.0, -direction.x());
	u.normalize();

	Math::Vector3d v = Math::Vector3d::crossProduct(direction, u);
	v.normalize();

	Math::Vector3d sizeU = u * polarRect.width  / 90.0 * scale;
	Math::Vector3d sizeV = v * polarRect.height / 90.0 * scale;

	topRight = direction + sizeV + sizeU;
	bottomRight = direction - sizeV + sizeU;
	bottomLeft = direction - sizeV - sizeU;
	topLeft = direction + sizeV - sizeU;
}

bool static rayIntersectsRect(const Math::Ray &ray, const Math::Vector3d &topLeft, const Math::Vector3d &topRight,
                              const Math::Vector3d &bottomLeft, const Math::Vector3d &bottomRight) {
	// Orthogonal basis in rectangle coordinates
	Math::Vector3d topRectDir = topRight - topLeft;
	Math::Vector3d leftRectDir = bottomLeft - topLeft;
	Math::Vector3d n = Math::Vector3d::crossProduct(topRectDir, leftRectDir);

	float nDotDir = Math::Vector3d::dotProduct(n, ray.getDirection());
	if (ABS(nDotDir) < 1e-6) {
		// The ray is coplanar with the rectangle
		return false;
	}

	// Solution to the system (intersection of line with plane):
	// Line equation: V = ray.origin + t * ray.direction
	// Plane equation: dot(n, V) = 0
	float t = -Math::Vector3d::dotProduct(n, ray.getOrigin() - topLeft) / nDotDir;

	if (t < 0.0) {
		// The intersection is not in the ray direction
		return false;
	}

	// Intersection point in world coordinates
	Math::Vector3d intersection = ray.getOrigin() + ray.getDirection() * t;

	// Intersection point in 2D rect coordinates
	Math::Vector3d intersect2D = intersection - topLeft;
	float u = Math::Vector3d::dotProduct(intersect2D, topRectDir);
	float v = Math::Vector3d::dotProduct(intersect2D, leftRectDir);

	// Intersection inside the rectangle
	return (u >= 0.0 && u <= Math::Vector3d::dotProduct(topRectDir, topRectDir)
	        && v >= 0.0 && v <= Math::Vector3d::dotProduct(leftRectDir, leftRectDir));
}

HotSpot::HotSpot() :
		condition(0),
		cursor(0) {
}

int32 HotSpot::isPointInRectsCube(float pitch, float heading) {
	for (uint j = 0; j < rects.size(); j++) {
		Math::Ray ray = Math::Ray(Math::Vector3d(), Scene::directionToVector(pitch, 90.0 - heading));

		Math::Vector3d topLeft, topRight, bottomLeft, bottomRight;
		polarRectTo3dRect(rects[j], topLeft, topRight, bottomLeft, bottomRight);

		if (rayIntersectsRect(ray, topLeft, topRight, bottomLeft, bottomRight)) {
			return j;
		}
	}

	return -1;
}

int32 HotSpot::isPointInRectsFrame(GameState *state, const Common::Point &p) {
	for (uint j = 0; j < rects.size(); j++) {
		int16 x = rects[j].centerPitch;
		int16 y = rects[j].centerHeading;
		int16 w = rects[j].width;
		int16 h = rects[j].height;

		if (y < 0) {
			x = state->getVar(x);
			y = state->getVar(-y);
			h = -h;
		}

		Common::Rect rect = Common::Rect(w, h);
		rect.translate(x, y);
		if (rect.contains(p))
			return j;
	}

	return -1;
}

bool HotSpot::isEnabled(GameState *state, uint16 var) {
	if (!state->evaluate(condition))
		return false;

	if (isZip()) {
		if (!ConfMan.getBool("zip_mode") || !isZipDestinationAvailable(state)) {
			return false;
		}
	}

	if (var == 0)
		return cursor <= 13;
	else
		return cursor == var;
}

int32 HotSpot::isZipDestinationAvailable(GameState *state) {
	assert(isZip() && script.size() != 0);

	uint16 node;
	uint16 room = state->getLocationRoom();
	uint32 age = state->getLocationAge();

	// Get the zip destination from the script
	Opcode op = script[0];
	switch (op.op) {
	case 140:
	case 142:
		node = op.args[0];
		break;
	case 141:
	case 143:
		node = op.args[1];
		room = op.args[0];
		break;
	default:
		error("Expected zip action");
	}

	return state->isZipDestinationAvailable(node, room, age);
}

} // End of namespace Myst3
