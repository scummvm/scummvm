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

#include "engines/stark/resources/location.h"

#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/layer.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"

namespace Stark {
namespace Resources {

Location::~Location() {
}

Location::Location(Object *parent, byte subType, uint16 index, const Common::String &name) :
				Object(parent, subType, index, name),
				_canScroll(false) {
	_type = TYPE;
}

void Location::onAllLoaded() {
	Object::onAllLoaded();

	_layers = listChildren<Layer>();
}

bool Location::has3DLayer() {
	return findChildWithSubtype<Layer>(Layer::kLayer3D) != nullptr;
}

RenderEntryArray Location::listRenderEntries() {
	RenderEntryArray renderEntries;

	for (uint i = 0; i < _layers.size(); i++) {
		Layer *layer = _layers[i];
		renderEntries.push_back(layer->listRenderEntries());
	}

	return renderEntries;
}

void Location::initScroll(const Common::Point &maxScroll) {
	_maxScroll = maxScroll;
	_canScroll = _maxScroll.x != 0 || _maxScroll.y != 0;
}

Common::Point Location::getScrollPosition() const {
	return _scroll;
}

void Location::setScrollPosition(const Common::Point &position) {
	Scene *scene = StarkServices::instance().scene;

	_scroll.x = CLIP<int16>(position.x, 0, _maxScroll.x);
	_scroll.y = CLIP<int16>(position.y, 0, _maxScroll.y);


	// Setup the layers scroll position
	for (uint i = 0; i < _layers.size(); i++) {
		_layers[i]->setScrollPosition(_scroll);
	}

	// Reconfigure the camera
	Common::Rect viewport(640, 365);
	viewport.translate(_scroll.x, _scroll.y);
	scene->scrollCamera(viewport);
}

void Location::printData() {
}

} // End of namespace Resources
} // End of namespace Stark
