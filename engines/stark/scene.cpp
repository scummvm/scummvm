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

#include "engines/stark/scene.h"

#include "engines/stark/archive.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/renderentry.h"
#include "engines/stark/visual/actor.h"
#include "engines/stark/xmg.h"

namespace Stark {

Scene::Scene(GfxDriver *gfx) :
		_gfx(gfx),
		_fov(45.0),
		_nearClipPlane(100.0),
		_farClipPlane(64000.0) {
	// Open the scene archive
	XARCArchive xarc;
	if (!xarc.open("45/00/00.xarc"))
		warning("couldn't open archive");

	VisualActor *actor = VisualActor::load(&xarc, "oldapril.cir");
	actor->setAnim(&xarc, "oldapril_idle.ani");
	actor->setTexture(&xarc, "oldapril.tm");

	RenderEntry *oldApril = new RenderEntry(nullptr, "Old April");
	oldApril->setVisual(actor);

	_elements.push_back(oldApril);
}

Scene::~Scene() {
}

void Scene::initCamera(const Math::Vector3d &position, const Math::Vector3d &lookAt,
		float fov, Common::Rect viewport, float nearClipPlane, float farClipPlane) {
	_cameraPosition = position;
	_cameraLookAt = lookAt;
	_fov = fov;
	_viewport = viewport;
	_nearClipPlane = nearClipPlane;
	_farClipPlane = farClipPlane;
}

void Scene::render(RenderEntryArray renderEntries, uint32 delta) {
	RenderEntryArray allElements;
	allElements.push_back(renderEntries);
	allElements.push_back(_elements);

	// setup cam
	_gfx->setupPerspective(_fov, _nearClipPlane, _farClipPlane);
	_gfx->setupCamera(_cameraPosition, _cameraPosition + _cameraLookAt);

	// Draw bg

	// Draw other things

	// Render all the scene elements
	RenderEntryArray::iterator element = allElements.begin();
	while (element != allElements.end()) {
		// Draw the current element
		(*element)->update(delta);
		(*element)->render(_gfx);

		// Go for the next one
		element++;
	}

	//_gfx->set3DMode();

	// setup lights

	// draw actors

	// draw overlay
}

} // End of namespace Stark
