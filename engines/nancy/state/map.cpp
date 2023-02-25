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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/map.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/button.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::Map);
}

namespace Nancy {
namespace State {

Map::Map() : _state(kInit),
			_mapID(0),
			_mapButtonClicked(false),
			_pickedLocationID(-1),
			_viewport(),
			_label(7),
			_closedLabel(7),
			_button(nullptr) {}

Map::~Map() {
	delete _button;
}

void Map::process() {
	switch (_state) {
	case kInit:
		init();
		// fall through
	case kRun:
		run();
		break;
	}
}

void Map::init() {
	Common::SeekableReadStream *chunk = g_nancy->getBootChunkStream("MAP");
	Common::Rect textboxScreenPosition = NancySceneState.getTextbox().getScreenPosition();

	_viewport.init();
	_label.init();

	Common::Rect buttonSrc, buttonDest;
	chunk->seek(0x7A, SEEK_SET);
	readRect(*chunk, buttonSrc);
	readRect(*chunk, buttonDest);

	chunk->seek(0xDA, SEEK_SET);
	Common::Rect closedLabelSrc;
	readRect(*chunk, closedLabelSrc);

	_closedLabel._drawSurface.create(g_nancy->_graphicsManager->_object0, closedLabelSrc);

	_closedLabel._screenPosition.left = textboxScreenPosition.left + ((textboxScreenPosition.width() - closedLabelSrc.width()) / 2);
	_closedLabel._screenPosition.right = _closedLabel._screenPosition.left + closedLabelSrc.width() - 1;
	_closedLabel._screenPosition.bottom = textboxScreenPosition.bottom - 11;
	_closedLabel._screenPosition.top = _closedLabel._screenPosition.bottom - closedLabelSrc.height() + 1;

	setLabel(-1);

	_button = new UI::Button(9, g_nancy->_graphicsManager->_object0, buttonSrc, buttonDest);
	_button->init();
	_button->setVisible(true);

	if (NancySceneState.getEventFlag(40, kTrue) && // Has set up sting
		NancySceneState.getEventFlag(95, kTrue)) { // Connie chickens
		_mapID = 1;
	} else {
		_mapID = 0;
	}

	// Load the video
	chunk->seek(_mapID * 10, SEEK_SET);
	Common::String videoName;
	readFilename(*chunk, videoName);

	_viewport.loadVideo(videoName, 0, 0);
	_viewport.disableEdges(kLeft | kRight | kUp | kDown);

	// Load the audio
	chunk->seek(0x18 + _mapID * 0x20, SEEK_SET);
	_sound.read(*chunk, SoundDescription::kMenu);
	g_nancy->_sound->loadSound(_sound);
	g_nancy->_sound->playSound("GLOB");

	_locations.clear();

	_locations.reserve(4);
	char buf[30];
	for (uint i = 0; i < 4; ++i) {
		_locations.push_back(Location());
		Location &loc = _locations.back();

		chunk->seek(0xEA + i * 16, SEEK_SET);
		chunk->read(buf, 30);
		buf[29] = '\0';
		loc.description = buf;

		chunk->seek(0x162 + i * 16, SEEK_SET);
		readRect(*chunk, loc.hotspot);

		if (_mapID == 1 && (i % 2) != 0) {
			loc.isActive = false;
		} else {
			loc.isActive = true;
		}

		loc.scenes.reserve(2);
		for (uint j = 0; j < 2; ++j) {
			loc.scenes.push_back(Location::SceneChange());
			Location::SceneChange &sc = loc.scenes[j];
			chunk->seek(0x1BE + 6 * i + j * 24, SEEK_SET);
			sc.sceneID = chunk->readUint16LE();
			sc.frameID = chunk->readUint16LE();
			sc.verticalOffset = chunk->readUint16LE();
		}

		chunk->seek(0x9A + i * 16, SEEK_SET);
		readRect(*chunk, loc.labelSrc);

		loc.labelDest.left = textboxScreenPosition.left + ((textboxScreenPosition.width() - loc.labelSrc.width()) / 2);
		loc.labelDest.right = loc.labelDest.left + loc.labelSrc.width() - 1;
		loc.labelDest.bottom = _closedLabel._screenPosition.bottom - ((_closedLabel._screenPosition.bottom - loc.labelSrc.height() - textboxScreenPosition.top) / 2) - 11;
		loc.labelDest.top = loc.labelDest.bottom - loc.labelSrc.height() + 1;
	}

	registerGraphics();
	g_nancy->_cursorManager->setCursorItemID(-1);

	_state = kRun;
}

void Map::run() {
	if (!g_nancy->_sound->isSoundPlaying("GLOB") && !g_nancy->_sound->isSoundPlaying(_sound)) {
		g_nancy->_sound->playSound(_sound);
	}

	NancyInput input = g_nancy->_input->getInput();

	setLabel(-1);

	_button->handleInput(input);

	if (_button->_isClicked) {
		_button->_isClicked = false;
		g_nancy->setState(NancyState::kScene);
		return;
	}

	for (uint i = 0; i < 4; ++i) {
		auto &loc = _locations[i];
		if (_viewport.convertToScreen(loc.hotspot).contains(input.mousePos)) {
			setLabel(i);

			if (loc.isActive){
				g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					_pickedLocationID = i;
					g_nancy->setState(NancyState::kScene);
				}
			}

			return;
		}
	}
}

void Map::onStateExit() {
	g_nancy->_sound->stopSound(_sound);

	if (_pickedLocationID != -1) {
		auto &loc = _locations[_pickedLocationID];
		NancySceneState.changeScene(loc.scenes[_mapID].sceneID, loc.scenes[_mapID].frameID, loc.scenes[_mapID].verticalOffset, false);
		_pickedLocationID = -1;

		g_nancy->_sound->playSound("BUOK");
	}

	// The two sounds play at the same time if a location was picked
	g_nancy->_sound->playSound("GLOB");

	_mapButtonClicked = false;

	destroy();
}

void Map::registerGraphics() {
	_viewport.registerGraphics();
	_label.registerGraphics();
	_closedLabel.registerGraphics();
	_button->registerGraphics();
}

void Map::setLabel(int labelID) {
	if (labelID == -1) {
		_label.setVisible(false);
		_closedLabel.setVisible(false);
	} else {
		_label._screenPosition = _locations[labelID].labelDest;
		_label._drawSurface.create(g_nancy->_graphicsManager->_object0, _locations[labelID].labelSrc);
		_label.setVisible(true);

		if (!_locations[labelID].isActive) {
			_closedLabel.setVisible(true);
		}
	}
}

} // End of namespace State
} // End of namespace Nancy
