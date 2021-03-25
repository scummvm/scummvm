/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "engines/nancy/state/map.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"

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
			_label(NancySceneState.getFrame(), this),
			_button(NancySceneState.getFrame(), this) {}

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

	_viewport.init();
	_label.init();
	_button.init();

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
	_viewport.setEdgesSize(0, 0, 0, 0);

	// Load the audio
	chunk->seek(0x18 + _mapID * 0x20, SEEK_SET);
	SoundDescription sound;
	sound.read(*chunk, SoundDescription::kMenu);
	g_nancy->_sound->loadSound(sound);
	g_nancy->_sound->playSound(0x14);

	_locations.clear();

	_locations.reserve(4);
	for (uint i = 0; i < 4; ++i) {
		chunk->seek(0x162 + i * 16, SEEK_SET);
		_locations.push_back(Location());
		Location &loc = _locations.back();
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

		// TODO this gets initialized using MAP and the textbox's on-screen location
		// but the code is annoyingly long so fpr now i just directly write the result
		loc.labelDest = Common::Rect(0x56, 0x166, 0x15E, 0x19B);

	}

	registerGraphics();
	g_nancy->_cursorManager->setCursorItemID(-1);

	_state = kRun;
}

void Map::run() {
	if (!g_nancy->_sound->isSoundPlaying(0x14) && !g_nancy->_sound->isSoundPlaying(0x13)) {
		g_nancy->_sound->playSound(0x13);
	}

	NancyInput input = g_nancy->_input->getInput();

	_label.setLabel(-1);

	_button.handleInput(input);

	if (_mapButtonClicked) {
		g_nancy->setState(NancyState::kScene);
		return;
	}

	for (uint i = 0; i < 4; ++i) {
		auto &loc = _locations[i];
		if (loc.isActive && _viewport.convertToScreen(loc.hotspot).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);

			_label.setLabel(i);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_pickedLocationID = i;
				g_nancy->setState(NancyState::kScene);
			}

			return;
		}
	}
}

bool Map::onStateExit() {
	Common::SeekableReadStream *chunk = g_nancy->getBootChunkStream("MAP");
	SoundDescription sound;
	chunk->seek(0x18 + _mapID * 0x20, SEEK_SET);
	sound.read(*chunk, SoundDescription::kMenu);
	g_nancy->_sound->stopSound(sound);

	g_nancy->setState(NancyState::kScene);

	if (_pickedLocationID != -1) {
		auto &loc = _locations[_pickedLocationID];
		NancySceneState.changeScene(loc.scenes[_mapID].sceneID, loc.scenes[_mapID].frameID, loc.scenes[_mapID].verticalOffset, false);
		_pickedLocationID = -1;

		g_nancy->_sound->playSound(0x18);
	}

	// The two sounds play at the same time if a location was picked
	g_nancy->_sound->playSound(0x14);

	_mapButtonClicked = false;

	destroy();
	return true;
}

void Map::registerGraphics() {
	_viewport.registerGraphics();
	_label.registerGraphics();
	_button.registerGraphics();
}

void Map::MapLabel::init() {
	setLabel(-1);

	RenderObject::init();
}

void Map::MapLabel::setLabel(int labelID) {
	if (labelID == -1) {
		setVisible(false);
	} else {
		_screenPosition = _parent->_locations[labelID].labelDest;
		_drawSurface.create(g_nancy->_graphicsManager->_object0, _parent->_locations[labelID].labelSrc);
		setVisible(true);
	}
}

void Map::MapButton::init() {
	Common::SeekableReadStream *map = g_nancy->getBootChunkStream("MAP");

	map->seek(0x7A, SEEK_SET);
	Common::Rect src;
	readRect(*map, src);
	_drawSurface.create(g_nancy->_graphicsManager->_object0, src);
	readRect(*map, _screenPosition);
	setVisible(true);

	RenderObject::init();
}

void Map::MapButton::onClick() {
	_parent->_mapButtonClicked = true;
}

} // End of namespace State
} // End of namespace Nancy
