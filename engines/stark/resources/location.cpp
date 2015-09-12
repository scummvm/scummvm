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
#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/item.h"
#include "engines/stark/resources/layer.h"

#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/global.h"

namespace Stark {
namespace Resources {

Location::~Location() {
}

Location::Location(Object *parent, byte subType, uint16 index, const Common::String &name) :
				Object(parent, subType, index, name),
				_canScroll(false),
				_currentLayer(nullptr) {
	_type = TYPE;
}

void Location::onAllLoaded() {
	Object::onAllLoaded();

	_layers = listChildren<Layer>();
}

void Location::onGameLoop() {
	Object::onGameLoop();

	// TODO: Add conditions
	scrollToCharacter();
}

bool Location::has3DLayer() {
	return findChildWithSubtype<Layer>(Layer::kLayer3D) != nullptr;
}

Gfx::RenderEntryArray Location::listRenderEntries() {
	Gfx::RenderEntryArray renderEntries;

	for (uint i = 0; i < _layers.size(); i++) {
		Layer *layer = _layers[i];
		if (layer->isEnabled()) {
			renderEntries.push_back(layer->listRenderEntries());
		}
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
	_scroll.x = CLIP<int16>(position.x, 0, _maxScroll.x);
	_scroll.y = CLIP<int16>(position.y, 0, _maxScroll.y);


	// Setup the layers scroll position
	for (uint i = 0; i < _layers.size(); i++) {
		_layers[i]->setScrollPosition(_scroll);
	}

	// Reconfigure the camera
	Common::Rect viewport(Gfx::Driver::kGameViewportWidth, Gfx::Driver::kGameViewportHeight);
	viewport.translate(_scroll.x, _scroll.y);
	StarkScene->scrollCamera(viewport);
}

Common::Point Location::getCharacterScrollPosition() {
	// TODO: Use April's 2D bounding box
	ModelItem *april = StarkGlobal->getCurrent()->getInteractive();
	Common::Point position2D = StarkScene->convertPosition3DToScreen(april->getPosition3D());

	Common::Point newScroll;
	if (_maxScroll.x > 0) {
		newScroll.x = _scroll.x + position2D.x - Gfx::Driver::kGameViewportWidth / 2;
		newScroll.y = _scroll.y;
	} else {
		newScroll.x = _scroll.x;
		newScroll.y = _scroll.y + position2D.y - Gfx::Driver::kGameViewportHeight / 2;
	}

	return newScroll;
}

void Location::scrollToCharacter() {
	if (!_canScroll) {
		return;
	}

	Common::Point newScroll = getCharacterScrollPosition();
	if (_maxScroll.x > 0) {
		if (newScroll.x < _scroll.x - 15 || newScroll.x > _scroll.x + 15) {
			newScroll.x = CLIP<int16>(newScroll.x, 0, _maxScroll.x);
			scrollToSmooth(newScroll);
		}
	} else {
		if (newScroll.y < _scroll.y - 15 || newScroll.y > _scroll.y + 15) {
			newScroll.y = CLIP<int16>(newScroll.y, 0, _maxScroll.y);
			scrollToSmooth(newScroll);
		}
	}
}

void Location::scrollToCharacterImmediate() {
	if (!_canScroll) {
		return;
	}

	setScrollPosition(getCharacterScrollPosition());
}

uint Location::getScrollStepMovement() {
	ModelItem *april = StarkGlobal->getCurrent()->getInteractive();
	Common::Point position2D = StarkScene->convertPosition3DToScreen(april->getPosition3D());

	// TODO: Complete

	uint scrollStep;
	if (_maxScroll.x > 0) {
		scrollStep = abs((Gfx::Driver::kGameViewportWidth / 2 - position2D.x) / 16);
	} else {
		scrollStep = abs((Gfx::Driver::kGameViewportHeight / 2 - position2D.y) / 16);
	}

	return CLIP<uint>(scrollStep, 1, 4);
}

void Location::scrollToSmooth(const Common::Point &position) {
	uint scrollStep = getScrollStepMovement(); //TODO: Select correct value according to case

	Common::Point delta;
	if (position.x < _scroll.x) {
		delta.x = -scrollStep;
		delta.x = CLIP<int16>(delta.x, position.x - _scroll.x, 0);
	} else if (position.x > _scroll.x) {
		delta.x = scrollStep;
		delta.x = CLIP<int16>(delta.x, 0, position.x - _scroll.x);
	}

	if (position.y < _scroll.y) {
		delta.y = -scrollStep;
		delta.y = CLIP<int16>(delta.y, position.y - _scroll.y, 0);
	} else if (position.y > _scroll.y) {
		delta.y = scrollStep;
		delta.y = CLIP<int16>(delta.y, 0, position.y - _scroll.y);
	}

	setScrollPosition(_scroll + delta);
}

void Location::goToLayer(Layer *layer) {
	if (_currentLayer) {
		_currentLayer->enable(false);
	}

	layer->enable(true);
	_currentLayer = layer;
}

void Location::printData() {
}

} // End of namespace Resources
} // End of namespace Stark
