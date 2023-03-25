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

template<>
Nancy::State::Map *Singleton<Nancy::State::Map>::makeInstance() {
	if (Nancy::g_nancy->getGameType() == Nancy::kGameTypeVampire) {
		return new Nancy::State::TVDMap();
	} else {
		return new Nancy::State::Nancy1Map();
	}
}

}

namespace Nancy {
namespace State {

Map::Map() : _state(kInit),
			_mapID(0),
			_pickedLocationID(-1),
			_label(7),
			_closedLabel(7),
			_background(0) {}

void Map::process() {
	switch (_state) {
	case kInit:
		init();
		// fall through
	case kLoad:
		load();
		// fall through
	case kRun:
		run();
		break;
	case kExit:
		g_nancy->setState(NancyState::kScene);
		break;
	}
}

void Map::onStateExit() {
	g_nancy->_graphicsManager->clearObjects();
	_viewport.unloadVideo();
	_state = kLoad;
}

void Map::load() {
	// Get a screenshot of the Scene state and set it as the background
	// to allow the labels to clear when not hovered
	const Graphics::ManagedSurface *screen = g_nancy->_graphicsManager->getScreen();
	_background._drawSurface.create(screen->w, screen->h, screen->format);
	_background._drawSurface.blitFrom(*screen);
	_background.moveTo(_background._drawSurface.getBounds());
	_background.setVisible(true);
}

void Map::registerGraphics() {
	_background.registerGraphics();
	_viewport.registerGraphics();
	_label.registerGraphics();
	_closedLabel.registerGraphics();
}

void Map::setLabel(int labelID) {
	if (labelID == -1) {
		_label.setVisible(false);
		_closedLabel.setVisible(false);
	} else {
		_label.moveTo(_locations[labelID].labelDest);
		_label._drawSurface.create(g_nancy->_graphicsManager->_object0, _locations[labelID].labelSrc);
		_label.setVisible(true);
		_label.setTransparent(true);

		if (!_locations[labelID].isActive) {
			_closedLabel.setVisible(true);
		}
	}
}

void Map::MapViewport::init() {
	Common::SeekableReadStream *viewChunk = g_nancy->getBootChunkStream("VIEW");

	if (viewChunk) {
		viewChunk->seek(0);
		Common::Rect dest;
		readRect(*viewChunk, dest);
		moveTo(dest);

		_drawSurface.create(dest.width(), dest.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	}

	RenderObject::init();
}

void Map::MapViewport::updateGraphics() {
	if (_decoder.getFrameCount() > 1) {
		if (_decoder.endOfVideo()) {
			_decoder.rewind();
		}

		if (_decoder.needsUpdate()) {
			GraphicsManager::copyToManaged(*_decoder.decodeNextFrame(), _drawSurface, g_nancy->getGameType() == kGameTypeVampire);
			_needsRedraw = true;
		}
	}
}

void Map::MapViewport::loadVideo(const Common::String &filename, const Common::String &palette) {
	if (_decoder.isVideoLoaded()) {
		_decoder.close();
	}

	if (!_decoder.loadFile(filename + ".avf")) {
		error("Couldn't load video file %s", filename.c_str());
	}

	if (palette.size()) {
		setPalette(palette);
	}

	GraphicsManager::copyToManaged(*_decoder.decodeNextFrame(), _drawSurface, palette.size());
	_needsRedraw = true;
}

TVDMap::TVDMap() : _ornaments(7), _globe(8, this) {}

void TVDMap::init() {
	_viewport.init();
	_label.init();
	_ornaments.init();
	_globe.init();

	Common::SeekableReadStream *chunk = g_nancy->getBootChunkStream("MAP");
	Common::Rect textboxScreenPosition = NancySceneState.getTextbox().getScreenPosition();

	if (chunk) {
		chunk->seek(0);

		_mapNames.resize(4);
		for (uint i = 0; i < 4; ++i) {
			readFilename(*chunk, _mapNames[i]);
		}

		_mapPalettes.resize(4);
		for (uint i = 0; i < 4; ++i) {
			readFilename(*chunk, _mapPalettes[i]);
		}

		chunk->skip(4);

		_mapSounds.resize(4);
		for (uint i = 0; i < 4; ++i) {
			_mapSounds[i].read(*chunk, SoundDescription::kMenu);
		}

		chunk->seek(0x1E6);
		Common::Rect closedLabelSrc;
		readRect(*chunk, closedLabelSrc);

		_closedLabel._drawSurface.create(g_nancy->_graphicsManager->_object0, closedLabelSrc);

		Common::Rect newScreenRect;

		newScreenRect.left = textboxScreenPosition.left + ((textboxScreenPosition.width() - closedLabelSrc.width()) / 2);
		newScreenRect.right = newScreenRect.left + closedLabelSrc.width();
		newScreenRect.bottom = textboxScreenPosition.bottom - 10;
		newScreenRect.top = newScreenRect.bottom - closedLabelSrc.height();

		_closedLabel.moveTo(newScreenRect);
		_closedLabel.setTransparent(true);

		char buf[30];
		_locations.resize(7);
		for (uint i = 0; i < 7; ++i) {
			Location &loc = _locations[i];

			chunk->seek(0x226 + i * 30);
			chunk->read(buf, 30);
			buf[29] = '\0';
			loc.description = buf;

			chunk->seek(0x2F8 + i * 16);
			readRect(*chunk, loc.hotspot);

			loc.scenes.resize(2);
			for (uint j = 0; j < 2; ++j) {
				SceneChangeDescription &sc = loc.scenes[j];
				chunk->seek(0x38A + (8 * i) + (56 * j));
				sc.sceneID = chunk->readUint16LE();
				sc.frameID = chunk->readUint16LE();
				sc.verticalOffset = chunk->readUint16LE();
				sc.paletteID = chunk->readUint16LE();
			}

			chunk->seek(0x186 + i * 16);
			readRect(*chunk, loc.labelSrc);

			Common::Rect closedScreenRect = _closedLabel.getScreenPosition();

			loc.labelDest.left = textboxScreenPosition.left + ((textboxScreenPosition.width() - loc.labelSrc.width()) / 2);
			loc.labelDest.right = loc.labelDest.left + loc.labelSrc.width();
			loc.labelDest.bottom = closedScreenRect.bottom - ((closedScreenRect.bottom - loc.labelSrc.height() - textboxScreenPosition.top) / 2) - 10;
			loc.labelDest.top = loc.labelDest.bottom - loc.labelSrc.height();

			loc.isActive = true;
		}

		chunk->seek(0x386);
		_cursorPosition.x = chunk->readUint16LE();
		_cursorPosition.y = chunk->readUint16LE();
	}

	_state = kLoad;
}

void TVDMap::load() {
	Map::load();

	// Determine which version of the map will be shown
	if (NancySceneState.getEventFlag(82, kEvOccurred)) {
		_mapID = 3;										// Storm
		//
	} else {
		// Determine map based on the in-game time
		byte timeOfDay = NancySceneState.getPlayerTOD();
		if (timeOfDay == kPlayerDay) {
			_mapID = 0; 								// Day
		} else if (timeOfDay == kPlayerNight) {
			_mapID = 1;									// Night
		} else {
			_mapID = 2;									// Dusk/dawn
		}
	}

	_viewport.loadVideo(_mapNames[_mapID], _mapPalettes[_mapID]);

	g_nancy->_cursorManager->setCursorItemID(-1);

	_viewport.setVisible(false);
	_globe.setOpen(true);
	_globe.setVisible(true);

	if (!g_nancy->_sound->isSoundPlaying(getSound())) {
		g_nancy->_sound->loadSound(getSound());
	}

	g_nancy->_sound->playSound("GLOB");

	registerGraphics();
	_state = kRun;
}

void TVDMap::onStateExit() {
	if (_pickedLocationID != -1) {
		auto &loc = _locations[_pickedLocationID];
		NancySceneState.changeScene(loc.scenes[NancySceneState.getPlayerTOD() == kPlayerDay ? 0 : 1]);

		g_nancy->_sound->playSound("BUOK");
	} else {
		g_nancy->_sound->stopSound(getSound());
	}

	Map::onStateExit();
}

void TVDMap::run() {
	if (!g_nancy->_sound->isSoundPlaying("GLOB") && !g_nancy->_sound->isSoundPlaying(getSound())) {
		g_nancy->_sound->playSound(getSound());
	}

	setLabel(-1);
	g_nancy->_cursorManager->setCursorType(CursorManager::kNormal);

	if (!_globe.isPlaying()) {
		NancyInput input = g_nancy->_input->getInput();

		_globe.handleInput(input);

		for (uint i = 0; i < _locations.size(); ++i) {
			auto &loc = _locations[i];
			if (_viewport.convertToScreen(loc.hotspot).contains(input.mousePos)) {
				setLabel(i);

				if (loc.isActive){
					g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

					if (input.input & NancyInput::kLeftMouseButtonUp) {
						_pickedLocationID = i;
						_globe.setOpen(false);
						g_nancy->_sound->playSound("GLOB");
					}
				}

				return;
			}
		}
	}
}

void TVDMap::registerGraphics() {
	Map::registerGraphics();
	_ornaments.registerGraphics();
	_globe.registerGraphics();
}

void TVDMap::MapGlobe::init() {
	Common::SeekableReadStream *chunk = g_nancy->getBootChunkStream("MAP");

	if (chunk) {
		chunk->seek(0xf4);

		_frameTime = chunk->readUint16LE();

		_srcRects.resize(8);
		for (uint i = 0; i < 8; ++i) {
			readRect(*chunk, _srcRects[i]);
		}

		Common::Rect screenDest;
		readRect(*chunk, screenDest);
		moveTo(screenDest);

		Common::Rect gargoyleSrc, gargoyleDest;
		chunk->skip(0x80);
		readRect(*chunk, gargoyleSrc);
		readRect(*chunk, gargoyleDest);

		_gargoyleEyes._drawSurface.create(g_nancy->_graphicsManager->_object0, gargoyleSrc);
		_gargoyleEyes.moveTo(gargoyleDest);
		_gargoyleEyes.setTransparent(true);
		_gargoyleEyes.setVisible(false);

		_alwaysHighlightCursor = false;
		_hotspot = _screenPosition;
	}

	AnimatedButton::init();
}

void TVDMap::MapGlobe::registerGraphics() {
	AnimatedButton::registerGraphics();
	_gargoyleEyes.registerGraphics();
}

void TVDMap::MapGlobe::onClick() {
	_gargoyleEyes.setVisible(false);
	g_nancy->_sound->playSound("GLOB");
}

void TVDMap::MapGlobe::onTrigger() {
	if (_isOpen) {
		_gargoyleEyes.setVisible(true);
		_owner->_viewport.setVisible(true);
		_owner->_viewport.playVideo();
		g_system->warpMouse(_owner->_cursorPosition.x, _owner->_cursorPosition.y);
		g_nancy->setMouseEnabled(true);
	} else {
		_owner->_state = kExit;
		_nextFrameTime = 0;
	}
}

Nancy1Map::Nancy1Map() : _button(nullptr)/*, _mapButtonClicked(false)*/ {}

Nancy1Map::~Nancy1Map() {
	delete _button;
}

void Nancy1Map::init() {
	_viewport.init();
	_label.init();

	Common::SeekableReadStream *chunk = g_nancy->getBootChunkStream("MAP");
	Common::Rect textboxScreenPosition = NancySceneState.getTextbox().getScreenPosition();

	if (chunk) {
		chunk->seek(0);

		_mapNames.resize(2);
		for (uint i = 0; i < 2; ++i) {
			readFilename(*chunk, _mapNames[i]);
		}

		chunk->skip(4);

		_mapSounds.resize(2);
		for (uint i = 0; i < 2; ++i) {
			_mapSounds[i].read(*chunk, SoundDescription::kMenu);
		}

		Common::Rect buttonSrc, buttonDest;
		chunk->seek(0x7A, SEEK_SET);
		readRect(*chunk, buttonSrc);
		readRect(*chunk, buttonDest);

		chunk->seek(0xDA, SEEK_SET);
		Common::Rect closedLabelSrc;
		readRect(*chunk, closedLabelSrc);

		_closedLabel._drawSurface.create(g_nancy->_graphicsManager->_object0, closedLabelSrc);

		Common::Rect newScreenRect;

		newScreenRect.left = textboxScreenPosition.left + ((textboxScreenPosition.width() - closedLabelSrc.width()) / 2);
		newScreenRect.right = newScreenRect.left + closedLabelSrc.width() - 1;
		newScreenRect.bottom = textboxScreenPosition.bottom - 11;
		newScreenRect.top = newScreenRect.bottom - closedLabelSrc.height() + 1;

		_closedLabel.moveTo(newScreenRect);

		_button = new UI::Button(9, g_nancy->_graphicsManager->_object0, buttonSrc, buttonDest);
		_button->init();
		_button->setVisible(true);

		char buf[30];
		_locations.resize(4);
		for (uint i = 0; i < 4; ++i) {
			Location &loc = _locations[i];

			chunk->seek(0xEA + i * 16);
			chunk->read(buf, 30);
			buf[29] = '\0';
			loc.description = buf;

			chunk->seek(0x162 + i * 16);
			readRect(*chunk, loc.hotspot);

			loc.scenes.resize(2);
			for (uint j = 0; j < 2; ++j) {
				SceneChangeDescription &sc = loc.scenes[j];
				chunk->seek(0x1BE + (6 * i) + (24 * j));
				sc.sceneID = chunk->readUint16LE();
				sc.frameID = chunk->readUint16LE();
				sc.verticalOffset = chunk->readUint16LE();
			}

			chunk->seek(0x9A + i * 16);
			readRect(*chunk, loc.labelSrc);

			Common::Rect closedScreenRect = _closedLabel.getScreenPosition();

			loc.labelDest.left = textboxScreenPosition.left + ((textboxScreenPosition.width() - loc.labelSrc.width()) / 2);
			loc.labelDest.right = loc.labelDest.left + loc.labelSrc.width() - 1;
			loc.labelDest.bottom = closedScreenRect.bottom - ((closedScreenRect.bottom - loc.labelSrc.height() - textboxScreenPosition.top) / 2) - 11;
			loc.labelDest.top = loc.labelDest.bottom - loc.labelSrc.height() + 1;

			loc.isActive = true;
		}
	}

	_state = kLoad;
}

void Nancy1Map::load() {
	Map::load();

	// Determine which version of the map will be shown
	if (NancySceneState.getEventFlag(40, kEvOccurred) &&	// Has set up sting
		NancySceneState.getEventFlag(95, kEvOccurred)) {	// Connie chickens
		_mapID = 1;		// Night

		_locations[1].isActive = _locations[3].isActive = false;
	} else {
		_mapID = 0;		// Day
	}

	_viewport.loadVideo(_mapNames[_mapID]);

	setLabel(-1);
	g_nancy->_cursorManager->setCursorItemID(-1);

	if (!g_nancy->_sound->isSoundPlaying(getSound())) {
		g_nancy->_sound->loadSound(getSound());
	}

	registerGraphics();
	_state = kRun;
}

void Nancy1Map::run() {
	if (!g_nancy->_sound->isSoundPlaying("GLOB") && !g_nancy->_sound->isSoundPlaying(getSound())) {
		g_nancy->_sound->playSound(getSound());
	}

	NancyInput input = g_nancy->_input->getInput();

	setLabel(-1);

	_button->handleInput(input);

	if (_button->_isClicked) {
		_button->_isClicked = false;
		_state = kExit;
		return;
	}

	for (uint i = 0; i < _locations.size(); ++i) {
		auto &loc = _locations[i];
		if (_viewport.convertToScreen(loc.hotspot).contains(input.mousePos)) {
			setLabel(i);

			if (loc.isActive){
				g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					_pickedLocationID = i;
					_state = kExit;
				}
			}

			return;
		}
	}
}

void Nancy1Map::registerGraphics() {
	Map::registerGraphics();
	_button->registerGraphics();
}

void Nancy1Map::onStateExit() {
	if (_pickedLocationID != -1) {
		auto &loc = _locations[_pickedLocationID];
		NancySceneState.changeScene(loc.scenes[_mapID]);

		g_nancy->_sound->playSound("BUOK");
	}

	g_nancy->_sound->stopSound(getSound());
	g_nancy->_sound->playSound("GLOB");

	Map::onStateExit();
}

} // End of namespace State
} // End of namespace Nancy
