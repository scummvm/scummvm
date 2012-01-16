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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/myst3/scene.h"
#include "engines/myst3/node.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/state.h"

#include "graphics/colormasks.h"

namespace Myst3 {

Scene::Scene(Myst3Engine *vm) :
	_vm(vm)
{
}

void Scene::updateCamera(Common::Point &mouse) {
	float pitch = _vm->_state->getLookAtPitch();
	float heading = _vm->_state->getLookAtHeading();

	pitch -= mouse.y / 3.0f;
	heading += mouse.x / 3.0f;

	// Keep heading in 0..360 range
	if (heading > 360.0f)
		heading -= 360.0f;
	else if (heading < 0.0f)
		heading += 360.0f;

	// Keep heading within allowed values
	if (_vm->_state->isCameraLimited()) {
		float minHeading = _vm->_state->getMinHeading();
		float maxHeading = _vm->_state->getMaxHeading();

		if (minHeading < maxHeading) {
			heading = CLIP(heading, minHeading, maxHeading);
		} else {
			if (heading < minHeading && heading > maxHeading) {
				uint distToMin = abs(heading - minHeading);
				uint distToMax = abs(heading - maxHeading);
				if (distToMin > distToMax)
					heading = maxHeading;
				else
					heading = minHeading;
			}
		}
	}

	// Keep pitch within allowed values
	float minPitch = _vm->_state->getCameraMinPitch();
	float maxPitch = _vm->_state->getCameraMaxPitch();

	if (_vm->_state->isCameraLimited()) {
		minPitch = _vm->_state->getMinPitch();
		maxPitch = _vm->_state->getMaxPitch();
	}

	pitch = CLIP(pitch, minPitch, maxPitch);

	_vm->_state->lookAt(pitch, heading);
	_vm->_state->setCameraPitch(pitch);
	_vm->_state->setCameraHeading(heading);
}

void Scene::drawBlackBorders() {
	Common::Rect top = Common::Rect(Renderer::kOriginalWidth, kTopBorderHeight);

	Common::Rect bottom = Common::Rect(Renderer::kOriginalWidth, kBottomBorderHeight);
	bottom.translate(0, kTopBorderHeight + kFrameHeight);

	uint32 black = Graphics::ARGBToColor< Graphics::ColorMasks<8888> >(255, 0, 0, 0);
	_vm->_gfx->drawRect2D(top, black);
	_vm->_gfx->drawRect2D(bottom, black);
}

void Scene::drawSunspotFlare(const SunSpot &s) {
	Common::Rect frame = Common::Rect(Renderer::kOriginalWidth, kFrameHeight);
	frame.translate(0, kTopBorderHeight);

	uint8 a = s.intensity * s.radius;
	uint8 r, g, b;
	Graphics::colorToRGB< Graphics::ColorMasks<888> >(s.color, r, g, b);
	uint32 color = Graphics::ARGBToColor< Graphics::ColorMasks<8888> >(a, r, g, b);

	_vm->_gfx->drawRect2D(frame, color);
}


static Math::Vector3d directionToVector(float pitch, float heading) {
	Math::Vector3d v;

	float radHeading = Math::degreeToRadian(heading);
	float radPitch = Math::degreeToRadian(pitch);

	v.setValue(0, cos(radPitch) * cos(radHeading));
	v.setValue(1, sin(radPitch));
	v.setValue(2, cos(radPitch) * sin(radHeading));

	return v;
}

float Scene::distanceToZone(float spotHeading, float spotPitch, float spotRadius, float heading, float pitch) {
	Math::Vector3d vLookAt = directionToVector(pitch, heading);
	Math::Vector3d vSun = directionToVector(spotPitch, spotHeading);
	float dotProduct = Math::Vector3d::dotProduct(vLookAt, -vSun);

	float distance = (0.05 * spotRadius - (dotProduct + 1.0) * 90) / (0.05 * spotRadius);
	return CLIP<float>(distance, 0.0, 1.0);
}

} // end of namespace Myst3
