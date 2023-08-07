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
			_background(0) {
	_mapData = g_nancy->_mapData;
	assert(_mapData);
}

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

void Map::onStateEnter(const NancyState::NancyState prevState) {
	// Unpause sound and video when coming back from the GMM
	if (prevState == NancyState::kPause) {
		g_nancy->_sound->pauseSound(getSound(), false);
		if (_viewport._decoder.getFrameCount() > 1) {
			_viewport._decoder.pauseVideo(false);
		}
	}
}

bool Map::onStateExit(const NancyState::NancyState nextState) {
	// Only pause when going to the GMM
	if (nextState == NancyState::kPause) {
		g_nancy->_sound->pauseSound(getSound(), true);
		if (_viewport._decoder.getFrameCount() > 1) {
			_viewport._decoder.pauseVideo(true);
		}
	} else {
		g_nancy->_graphicsManager->clearObjects();
		_viewport.unloadVideo();
		_state = kLoad;
	}

	return false;
}

const SoundDescription &Map::getSound() {
	return _mapData->sounds[_mapID];
}

void Map::load() {
	// Get a screenshot of the Scene state and set it as the background
	// to allow the labels to clear when not hovered
	const Graphics::ManagedSurface *screen = g_nancy->_graphicsManager->getScreen();
	_background._drawSurface.create(screen->w, screen->h, screen->format);
	_background._drawSurface.blitFrom(*screen);
	_background.moveTo(_background._drawSurface.getBounds());
	_background.setVisible(true);

	// The clock may become invisible after the map is opened, resulting in the left half appearing still open
	// This is a slightly hacky solution but it works
	if (g_nancy->getGameType() == kGameTypeVampire) {
		Common::Rect r(52, 100);
		_background._drawSurface.blitFrom(NancySceneState.getFrame()._drawSurface, r, r);
	}
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
		_label.moveTo(_locationLabelDests[labelID]);
		_label._drawSurface.create(g_nancy->_graphicsManager->_object0, _mapData->locations[labelID].labelSrc);
		_label.setVisible(true);
		_label.setTransparent(true);

		if (!_activeLocations[labelID]) {
			_closedLabel.setVisible(true);
		}
	}
}

void Map::MapViewport::init() {
	moveTo(g_nancy->_viewportData->screenPosition);
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphicsManager->getInputPixelFormat());

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

	Common::Rect textboxScreenPosition = g_nancy->_bootSummary->textboxScreenPosition;
	_closedLabel._drawSurface.create(g_nancy->_graphicsManager->_object0, _mapData->closedLabelSrc);

	Common::Rect closedScreenRect;
	closedScreenRect.left = textboxScreenPosition.left + ((textboxScreenPosition.width() - _mapData->closedLabelSrc.width()) / 2);
	closedScreenRect.right = closedScreenRect.left + _mapData->closedLabelSrc.width();
	closedScreenRect.bottom = textboxScreenPosition.bottom - 10;
	closedScreenRect.top = closedScreenRect.bottom - _mapData->closedLabelSrc.height();

	_closedLabel.moveTo(closedScreenRect);
	_closedLabel.setTransparent(true);

	_activeLocations.resize(7, true);
	_locationLabelDests.resize(7);

	for (uint i = 0; i < 7; ++i) {
		_locationLabelDests[i].left = textboxScreenPosition.left + ((textboxScreenPosition.width() - _mapData->locations[i].labelSrc.width()) / 2);
		_locationLabelDests[i].right = _locationLabelDests[i].left + _mapData->locations[i].labelSrc.width();
		_locationLabelDests[i].bottom = closedScreenRect.bottom - ((closedScreenRect.bottom - _mapData->locations[i].labelSrc.height() - textboxScreenPosition.top) / 2) - 10;
		_locationLabelDests[i].top = _locationLabelDests[i].bottom - _mapData->locations[i].labelSrc.height();
	}

	_state = kLoad;
}

void TVDMap::load() {
	Map::load();

	// Determine which version of the map will be shown
	if (NancySceneState.getEventFlag(82, g_nancy->_true)) {
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

	_viewport.loadVideo(_mapData->mapNames[_mapID], _mapData->mapPaletteNames[_mapID]);

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

bool TVDMap::onStateExit(const NancyState::NancyState nextState) {
	if (nextState != NancyState::kPause) {
		if (_pickedLocationID != -1) {
			NancySceneState.changeScene(_mapData->locations[_pickedLocationID].scenes[NancySceneState.getPlayerTOD() == kPlayerDay ? 0 : 1]);

			g_nancy->_sound->playSound("BUOK");
		} else {
			g_nancy->_sound->stopSound(getSound());
		}
	}

	return Map::onStateExit(nextState);
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

		for (uint i = 0; i < 7; ++i) {
			if (_viewport.convertToScreen(_mapData->locations[i].hotspot).contains(input.mousePos)) {
				setLabel(i);

				if (_activeLocations[i]){
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
	moveTo(_owner->_mapData->globeDest);

	_frameTime = _owner->_mapData->globeFrameTime;
	_srcRects = _owner->_mapData->globeSrcs;

	_gargoyleEyes._drawSurface.create(g_nancy->_graphicsManager->_object0, _owner->_mapData->globeGargoyleSrc);
	_gargoyleEyes.moveTo(_owner->_mapData->globeGargoyleDest);
	_gargoyleEyes.setTransparent(true);
	_gargoyleEyes.setVisible(false);

	_alwaysHighlightCursor = false;
	_hotspot = _screenPosition;

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
		g_system->warpMouse(_owner->_mapData->cursorPosition.x, _owner->_mapData->cursorPosition.y);
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

	Common::Rect textboxScreenPosition = NancySceneState.getTextbox().getScreenPosition();
	_closedLabel._drawSurface.create(g_nancy->_graphicsManager->_object0, _mapData->closedLabelSrc);

	Common::Rect closedScreenRect;
	closedScreenRect.left = textboxScreenPosition.left + ((textboxScreenPosition.width() - _mapData->closedLabelSrc.width()) / 2);
	closedScreenRect.right = closedScreenRect.left + _mapData->closedLabelSrc.width() - 1;
	closedScreenRect.bottom = textboxScreenPosition.bottom - 11;
	closedScreenRect.top = closedScreenRect.bottom - _mapData->closedLabelSrc.height() + 1;

	_closedLabel.moveTo(closedScreenRect);

	_activeLocations.resize(4, true);
	_locationLabelDests.resize(4);

	for (uint i = 0; i < 4; ++i) {
		_locationLabelDests[i].left = textboxScreenPosition.left + ((textboxScreenPosition.width() - _mapData->locations[i].labelSrc.width()) / 2);
		_locationLabelDests[i].right = _locationLabelDests[i].left + _mapData->locations[i].labelSrc.width() - 1;
		_locationLabelDests[i].bottom = closedScreenRect.bottom - ((closedScreenRect.bottom - _mapData->locations[i].labelSrc.height() - textboxScreenPosition.top) / 2) - 11;
		_locationLabelDests[i].top = _locationLabelDests[i].bottom - _mapData->locations[i].labelSrc.height() + 1;
	}

	_button = new UI::Button(9, g_nancy->_graphicsManager->_object0, _mapData->buttonSrc, _mapData->buttonDest);
	_button->init();
	_button->setVisible(true);

	_state = kLoad;
}

void Nancy1Map::load() {
	Map::load();

	// Determine which version of the map will be shown
	if (NancySceneState.getEventFlag(40, g_nancy->_true) &&	// Has set up sting
		NancySceneState.getEventFlag(95, g_nancy->_true)) {	// Connie chickens
		_mapID = 1;		// Night

		_activeLocations[1] = _activeLocations[3] = false;
	} else {
		_mapID = 0;		// Day
	}

	_viewport.loadVideo(_mapData->mapNames[_mapID]);

	setLabel(-1);
	g_nancy->_cursorManager->setCursorItemID(-1);
	g_system->warpMouse(_mapData->cursorPosition.x, _mapData->cursorPosition.y);

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

	for (uint i = 0; i < 4; ++i) {
		if (_viewport.convertToScreen(_mapData->locations[i].hotspot).contains(input.mousePos)) {
			setLabel(i);

			if (_activeLocations[i]){
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

bool Nancy1Map::onStateExit(const NancyState::NancyState nextState) {
	if (nextState != NancyState::kPause) {
		if (_pickedLocationID != -1) {
			NancySceneState.changeScene(_mapData->locations[_pickedLocationID].scenes[_mapID]);

			g_nancy->_sound->playSound("BUOK");
		}

		g_nancy->_sound->stopSound(getSound());
		g_nancy->_sound->playSound("GLOB");
	}

	return Map::onStateExit(nextState);
}

} // End of namespace State
} // End of namespace Nancy
