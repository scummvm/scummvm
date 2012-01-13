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

	// Keep pitch within allowed values
	pitch = CLIP(pitch, -60.0f, 80.0f);

	_vm->_state->lookAt(pitch, heading);
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

} // end of namespace Myst3
