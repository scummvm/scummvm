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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "harvester/flow/flow.h"
#include "harvester/flow/flow_support.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/ptr.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/art.h"
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/harvester.h"
#include "harvester/scene/room_presentation.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static const char *const kTownMapPalettePath = "1:/GRAPHIC/PAL/HARVMAP.PAL";
static const char *const kTownMapMusicPath = "SOUND/MUSIC/MENACE.CMP";
static const char *const kTownMapBitmapPaths[] = {
	"1:/GRAPHIC/TOWN/HARVMAP1.BM",
	"1:/GRAPHIC/TOWN/HARVMAP2.BM",
	"1:/GRAPHIC/TOWN/HARVMAP3.BM",
	"1:/GRAPHIC/TOWN/HARVMAP4.BM"
};
static const float kTownMapNightPaletteBrightness = 0.6f;
static const int kTownMapEdgeThreshold = 9;
static const int kTownMapCursorHitExtent = 5;
static const byte kTownMapLabelColor = 0x28;

static int clampTownMapPanelIndex(int panelIndex) {
	return CLIP<int>(panelIndex, 0, ARRAYSIZE(kTownMapBitmapPaths) - 1);
}

static int resolveTownMapEdgePanel(int currentPanel, const Common::Point &mousePos, int width, int height) {
	if (mousePos.x <= kTownMapEdgeThreshold) {
		if (currentPanel == 1)
			return 0;
		if (currentPanel == 3)
			return 2;
	} else if (mousePos.x >= width - 1 - kTownMapEdgeThreshold) {
		if (currentPanel == 0)
			return 1;
		if (currentPanel == 2)
			return 3;
	} else if (mousePos.y <= kTownMapEdgeThreshold) {
		if (currentPanel == 2)
			return 0;
		if (currentPanel == 3)
			return 1;
	} else if (mousePos.y >= height - 1 - kTownMapEdgeThreshold) {
		if (currentPanel == 0)
			return 2;
		if (currentPanel == 1)
			return 3;
	}

	return currentPanel;
}

static int resolveTownMapKeyPanel(int currentPanel, Common::KeyCode keycode) {
	switch (keycode) {
	case Common::KEYCODE_LEFT:
		if (currentPanel == 1)
			return 0;
		if (currentPanel == 3)
			return 2;
		break;
	case Common::KEYCODE_RIGHT:
		if (currentPanel == 0)
			return 1;
		if (currentPanel == 2)
			return 3;
		break;
	case Common::KEYCODE_UP:
		if (currentPanel == 2)
			return 0;
		if (currentPanel == 3)
			return 1;
		break;
	case Common::KEYCODE_DOWN:
		if (currentPanel == 0)
			return 2;
		if (currentPanel == 1)
			return 3;
		break;
	default:
		break;
	}

	return currentPanel;
}

static bool isTownMapLocationHovered(const MapLocationRecord &location, int currentPanel,
		const Common::Point &mousePos) {
	if (location.panelIndex != currentPanel)
		return false;

	const int cursorRight = mousePos.x + kTownMapCursorHitExtent;
	const int cursorBottom = mousePos.y + kTownMapCursorHitExtent;
	return mousePos.x <= location.maxX && cursorRight >= location.minX &&
		mousePos.y <= location.maxY && cursorBottom >= location.minY;
}

static const MapLocationRecord *findTownMapLocationAt(
		const Common::Array<MapLocationRecord> &locations, int currentPanel,
		const Common::Point &mousePos) {
	for (const MapLocationRecord &location : locations) {
		if (isTownMapLocationHovered(location, currentPanel, mousePos))
			return &location;
	}

	return nullptr;
}

class TownMapSelector {
public:
	TownMapSelector(HarvesterEngine &engine, Flow &flow, Common::Point &mousePos,
		const Common::String &mapEntryName, Common::String &destinationEntranceName)
		: _engine(engine), _flow(flow), _mousePos(mousePos), _mapEntryName(mapEntryName),
		  _destinationEntranceName(destinationEntranceName) {}

	bool load();
	Common::Error run();

private:
	void centerCursor();
	void restorePreviousMusic();
	void draw(const MapLocationRecord *hoveredLocation);
	bool handleEvent(const Common::Event &event);

	HarvesterEngine &_engine;
	Flow &_flow;
	Common::Point &_mousePos;
	const Common::String &_mapEntryName;
	Common::String &_destinationEntranceName;
	Script *_script = nullptr;
	Graphics::Screen *_screen = nullptr;
	const Graphics::Font *_font = nullptr;
	Common::ScopedPtr<HarvesterCftFont> _townMapFont;
	const MapEntranceRecord *_mapEntrance = nullptr;
	float _paletteBrightness = 1.0f;
	byte _palette[256 * 3];
	Common::Array<IndexedBitmap> _panels;
	Common::String _previousMusicPath;
	int _currentPanel = 0;
	bool _needsRedraw = true;
};

bool TownMapSelector::load() {
	_script = _engine.getScript();
	ResourceManager *resources = _engine.getResources();
	_screen = _engine.getScreen();
	_font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	const Text *text = _engine.getText();
	if (const CftFontResource *townMapFontResource = text ? text->findFont("TEXTFONT") : nullptr) {
		_townMapFont.reset(new HarvesterCftFont(*townMapFontResource));
		if (_townMapFont->isValid())
			_font = _townMapFont.get();
		else
			_townMapFont.reset();
	}
	if (!_script || !resources || !_screen || !_font)
		return false;

	_mapEntrance = _script->findMapEntranceRecord(_mapEntryName);
	if (!_mapEntrance) {
		warning("Harvester: unresolved town map entry '%s'", _mapEntryName.c_str());
		return false;
	}
	_paletteBrightness = _script->getFlagValue("DAY_FLAG")
		? 1.0f
		: kTownMapNightPaletteBrightness;

	if (!Art::loadPaletteResource(*resources, kTownMapPalettePath, _palette))
		return false;

	_panels.resize(ARRAYSIZE(kTownMapBitmapPaths));
	for (uint i = 0; i < ARRAYSIZE(kTownMapBitmapPaths); ++i) {
		if (!loadBitmapResource(*resources, kTownMapBitmapPaths[i], _panels[i]))
			return false;
	}

	_previousMusicPath = _engine.getMusicPath();
	(void)_engine.playMusic(kTownMapMusicPath);

	return true;
}

void TownMapSelector::centerCursor() {
	_mousePos = Common::Point(_screen->w / 2, _screen->h / 2);
	if (EntityManager *entityManager = _engine.getRuntimeEntities())
		(void)entityManager->syncCursorEntityPosition(_mousePos);
}

void TownMapSelector::restorePreviousMusic() {
	if (!_destinationEntranceName.empty())
		return;
	if (!_previousMusicPath.empty())
		(void)_engine.playMusic(_previousMusicPath);
	else
		_engine.stopMusic();
}

void TownMapSelector::draw(const MapLocationRecord *hoveredLocation) {
	setScaledPalette(*_screen, _palette, _paletteBrightness);
	_screen->fillRect(_screen->getBounds(), 0);
	RoomPresentation::blitBitmap(*_screen, _panels[(uint)_currentPanel], 0, 0);
	if (hoveredLocation) {
		RoomPresentation::drawShadowedString(*_screen, *_font, hoveredLocation->labelText,
			hoveredLocation->labelX, hoveredLocation->labelY, _screen->w, kTownMapLabelColor);
	}
	if (EntityManager *entityManager = _engine.getRuntimeEntities())
		entityManager->drawCursor(*_screen);
	_screen->makeAllDirty();
	_screen->update();
}

bool TownMapSelector::handleEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		_needsRedraw = true;
		break;
	case Common::EVENT_LBUTTONDOWN: {
		const MapLocationRecord *clickedLocation =
			findTownMapLocationAt(_script->getMapLocations(), _currentPanel, _mousePos);
		if (!clickedLocation)
			break;

		const EntranceRecord *destinationEntrance =
			_script->findEntranceRecord(clickedLocation->destinationEntranceName);
		if (!destinationEntrance) {
			warning("Harvester: unresolved town map destination '%s' from '%s'",
				clickedLocation->destinationEntranceName.c_str(), _mapEntryName.c_str());
			break;
		}

		_destinationEntranceName = destinationEntrance->entranceName;
		debugC(1, kDebugGeneral,
			"Harvester: town map selection entry='%s' panel=%d label='%s' destination='%s'",
			_mapEntryName.c_str(), _currentPanel, clickedLocation->labelText.c_str(),
			_destinationEntranceName.c_str());
		return true;
	}
	case Common::EVENT_KEYDOWN: {
		const int nextPanel = resolveTownMapKeyPanel(_currentPanel, event.kbd.keycode);
		if (nextPanel != _currentPanel) {
			_currentPanel = nextPanel;
			centerCursor();
			_needsRedraw = true;
		}
		break;
	}
	default:
		break;
	}
	return false;
}

Common::Error TownMapSelector::run() {
	if (_mousePos.x < 0 || _mousePos.y < 0 || _mousePos.x >= _screen->w || _mousePos.y >= _screen->h)
		_mousePos = Common::Point(_screen->w / 2, _screen->h / 2);
	if (EntityManager *entityManager = _engine.getRuntimeEntities())
		(void)entityManager->syncCursorEntityPosition(_mousePos);

	_currentPanel = clampTownMapPanelIndex(_mapEntrance->initialPanelIndex);
	_needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		const int edgePanel = resolveTownMapEdgePanel(_currentPanel, _mousePos, _screen->w, _screen->h);
		if (edgePanel != _currentPanel) {
			_currentPanel = edgePanel;
			centerCursor();
			_needsRedraw = true;
		}

		const MapLocationRecord *hoveredLocation =
			findTownMapLocationAt(_script->getMapLocations(), _currentPanel, _mousePos);
		if (_needsRedraw) {
			draw(hoveredLocation);
			_needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (_flow.handleSystemEvent(event, result)) {
				restorePreviousMusic();
				return result;
			}

			if (handleEvent(event))
				return Common::kNoError;
		}

		if (_flow.tickRuntimeEntities())
			_needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	restorePreviousMusic();
	return Common::kNoError;
}

} // End of anonymous namespace

Common::Error Flow::runTownMapSelector(const Common::String &mapEntryName,
		Common::String &destinationEntranceName) {
	destinationEntranceName.clear();

	TownMapSelector selector(_engine, *this, _mousePos, mapEntryName, destinationEntranceName);
	if (!selector.load())
		return Common::kReadingFailed;

	if (!ensureCursorEntity())
		return Common::kReadingFailed;
	resetCursorAnimationSequence();

	return selector.run();
}

} // End of namespace Harvester
