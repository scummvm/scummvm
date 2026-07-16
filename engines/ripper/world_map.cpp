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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/world_map.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/milestones.h"
#include "ripper/ripper.h"

namespace Ripper {

static const uint kWorldMapWidth = 640;
static const uint kWorldMapHeight = 400;
static const uint kTravelLocationCount = 15;
static const uint kSceneEntryCount = 25;
static const uint kTravelLocationNameResource = 100;
static const uint kTravelTargetResource = 150;
static const uint kWorldMapCursor = 16;
static const int kLocationListLeft = 45;
static const int kLocationListTop = 80;
static const int kLocationListRight = 235;
static const int kLocationListBottom = 270;
static const byte kListBackgroundColor = 0;
static const byte kListTextColor = 251;
static const byte kSelectedBackgroundColor = 248;
static const byte kSelectedTextColor = 4;

// RunSceneSelectionMenu at 0x20808 copies this table from
// g_sceneSelectionEntryGroupIds at 0x20702 before testing flags 20..44.
static const byte kSceneEntryGroups[kSceneEntryCount] = {
	0, 1, 2, 2, 3, 4, 4, 4, 4, 4, 5, 5, 6,
	7, 8, 9, 9, 9, 9, 10, 11, 12, 12, 13, 14
};

WorldMap::WorldMap(RipperEngine *engine) : _engine(engine), _selectedVisible(0),
		_firstVisible(0), _pressedVisible(-1), _initialized(false) {
}

bool WorldMap::initialize(ResourceManager &resources) {
	if (!resources.loadInterfacePcx("worldmap.pcx", _background) ||
		_background.width != kWorldMapWidth || _background.height != kWorldMapHeight ||
		_background.palette.size() < 256 * 3 ||
		!resources.loadInterfaceBitmapFont("small.fnt", _font) ||
		!resources.loadGameText(_gameText) || _gameText.size() < 164)
		return false;

	_locations.clear();
	_locations.resize(kTravelLocationCount);
	for (uint i = 0; i < _locations.size(); ++i) {
		_locations[i].name = resourceString(kTravelLocationNameResource + i);
		_locations[i].targetPattern = resourceString(kTravelTargetResource + i);
	}
	for (uint entryIndex = 0; entryIndex < kSceneEntryCount; ++entryIndex)
		_locations[kSceneEntryGroups[entryIndex]].entryIndices.push_back(entryIndex);

	_initialized = true;
	debugC(1, kDebugScene,
		"Ripper: initialized world map locations=%u unlockFlags=%u-%u",
		_locations.size(), kMilestoneFirstTravelLocation, kMilestoneLastTravelLocation);
	return true;
}

bool WorldMap::isLocationAvailable(uint locationIndex) const {
	return locationIndex < _locations.size() && _locations[locationIndex].available;
}

const Common::String &WorldMap::locationName(uint locationIndex) const {
	static const Common::String empty;
	return locationIndex < _locations.size() ? _locations[locationIndex].name : empty;
}

const Common::String &WorldMap::resourceString(uint resourceId) const {
	static const Common::String empty;
	if (resourceId == 0 || resourceId > _gameText.size())
		return empty;
	return _gameText[resourceId - 1];
}

bool WorldMap::captureDisplay() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
		screen->w < (int)kWorldMapWidth || screen->h < (int)kWorldMapHeight) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	_savedPixels.resize(kWorldMapWidth * kWorldMapHeight);
	for (uint y = 0; y < kWorldMapHeight; ++y)
		memcpy(_savedPixels.data() + y * kWorldMapWidth, screen->getBasePtr(0, y), kWorldMapWidth);
	g_system->unlockScreen();
	_savedPalette.resize(256 * 3);
	g_system->getPaletteManager()->grabPalette(_savedPalette.data(), 0, 256);
	return true;
}

void WorldMap::restoreDisplay() {
	if (!_savedPixels.empty())
		g_system->copyRectToScreen(_savedPixels.data(), kWorldMapWidth,
			0, 0, kWorldMapWidth, kWorldMapHeight);
	if (_savedPalette.size() >= 256 * 3)
		g_system->getPaletteManager()->setPalette(_savedPalette.data(), 0, 256);
	g_system->updateScreen();
	_savedPixels.clear();
	_savedPalette.clear();
}

void WorldMap::refreshLocations() {
	_visibleLocations.clear();
	for (uint locationIndex = 0; locationIndex < _locations.size(); ++locationIndex) {
		TravelLocation &location = _locations[locationIndex];
		const bool wasAvailable = location.available;
		location.available = false;
		for (uint entry = 0; entry < location.entryIndices.size(); ++entry) {
			if (_engine->getMilestones()->isSet(
				kMilestoneFirstTravelLocation + location.entryIndices[entry])) {
				location.available = true;
				break;
			}
		}
		if (location.available) {
			_visibleLocations.push_back(locationIndex);
			if (!wasAvailable)
				debugC(2, kDebugScene,
					"Ripper: world map location unlocked index=%u name='%s' target='%s'",
					locationIndex, location.name.c_str(), location.targetPattern.c_str());
		}
	}
	if (_selectedVisible >= _visibleLocations.size())
		_selectedVisible = _visibleLocations.empty() ? 0 : _visibleLocations.size() - 1;
	updateFirstVisible();
}

void WorldMap::drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const {
	int drawX = x;
	for (uint i = 0; i < text.size(); ++i) {
		const byte character = (byte)text[i];
		if (character == ' ') {
			drawX += _font.spaceWidth;
			continue;
		}
		if (character < _font.firstCharacter ||
			character >= _font.firstCharacter + _font.glyphs.size())
			continue;
		const BitmapFontGlyph &glyph = _font.glyphs[character - _font.firstCharacter];
		for (uint glyphY = 0; glyphY < glyph.height; ++glyphY) {
			for (uint glyphX = 0; glyphX < glyph.width; ++glyphX) {
				const byte pixel = _font.pixels[glyph.pixelOffset + glyphY * glyph.width + glyphX];
				if (pixel != _font.transparentColor)
					screen[(y + glyph.yOffset + glyphY) * pitch +
						drawX + glyph.xOffset + glyphX] = color;
			}
		}
		drawX += glyph.xOffset + glyph.width + _font.characterSpacing;
	}
}

void WorldMap::draw() const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	for (uint y = 0; y < kWorldMapHeight; ++y)
		memcpy(screen->getBasePtr(0, y), _background.pixels.data() + y * kWorldMapWidth,
			kWorldMapWidth);

	const uint rowHeight = _font.lineHeight + 4;
	const uint visibleRows = (kLocationListBottom - kLocationListTop) / rowHeight;
	byte *pixels = (byte *)screen->getPixels();
	for (uint row = 0; row < visibleRows && _firstVisible + row < _visibleLocations.size(); ++row) {
		const uint visibleIndex = _firstVisible + row;
		const int top = kLocationListTop + row * rowHeight;
		const bool selected = visibleIndex == _selectedVisible;
		for (uint y = 0; y < rowHeight; ++y)
			memset(screen->getBasePtr(kLocationListLeft, top + y),
				selected ? kSelectedBackgroundColor : kListBackgroundColor,
				kLocationListRight - kLocationListLeft);
		drawText(pixels, screen->pitch, kLocationListLeft + 3, top + 2,
			_locations[_visibleLocations[visibleIndex]].name,
			selected ? kSelectedTextColor : kListTextColor);
	}
	g_system->unlockScreen();
	g_system->updateScreen();
}

int WorldMap::findVisibleLocation(const Common::Point &point) const {
	if (point.x < kLocationListLeft || point.x >= kLocationListRight ||
		point.y < kLocationListTop || point.y >= kLocationListBottom)
		return -1;
	const uint rowHeight = _font.lineHeight + 4;
	const uint visibleIndex = _firstVisible +
		(point.y - kLocationListTop) / rowHeight;
	return visibleIndex < _visibleLocations.size() ? (int)visibleIndex : -1;
}

void WorldMap::updateFirstVisible() {
	const uint rowHeight = _font.lineHeight + 4;
	const uint visibleRows = (kLocationListBottom - kLocationListTop) / rowHeight;
	if (_selectedVisible < _firstVisible)
		_firstVisible = _selectedVisible;
	else if (_selectedVisible >= _firstVisible + visibleRows)
		_firstVisible = _selectedVisible - visibleRows + 1;
}

uint WorldMap::resolveChapter() const {
	// ResolveHighestSetSelectionFlag at 0x20394 checks named flags 4 through 1
	// and supplies the highest set value to the destination script pattern.
	for (uint flag = kMilestoneCompletedAct3; flag != 0; --flag) {
		if (_engine->getMilestones()->isSet(flag))
			return flag;
	}
	return 0;
}

Common::String WorldMap::resolveTargetScript(uint locationIndex) const {
	if (locationIndex >= _locations.size())
		return Common::String();
	return Common::String::format(_locations[locationIndex].targetPattern.c_str(),
		(int)resolveChapter());
}

bool WorldMap::run(Common::String &targetScript) {
	targetScript.clear();
	if (!_initialized || !captureDisplay())
		return false;

	refreshLocations();
	if (_visibleLocations.empty()) {
		warning("Ripper: world map has no unlocked travel locations");
		restoreDisplay();
		return true;
	}

	_selectedVisible = 0;
	_firstVisible = 0;
	_pressedVisible = -1;
	g_system->getPaletteManager()->setPalette(_background.palette.data(), 0, 256);
	draw();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setVisible(true);
	_engine->getCursor()->update(kWorldMapCursor);
	debugC(1, kDebugScene, "Ripper: entered world map availableLocations=%u",
		_visibleLocations.size());

	bool cancelled = false;
	bool selected = false;
	while (!_engine->shouldQuit() && !cancelled && !selected) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 key = _engine->getInput()->consumeKey();
			if (key == 27) {
				cancelled = true;
				break;
			}
			if (key == 13) {
				selected = true;
				break;
			}
			if (key == 0x4800 && _selectedVisible != 0) {
				--_selectedVisible;
				updateFirstVisible();
				draw();
			} else if (key == 0x5000 && _selectedVisible + 1 < _visibleLocations.size()) {
				++_selectedVisible;
				updateFirstVisible();
				draw();
			}
		}
		if (cancelled || selected)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		const int hoveredVisible = findVisibleLocation(mouse.position);
		_engine->getCursor()->update(hoveredVisible >= 0 ? kWorldMapCursor : 14);
		if ((mouse.pressed & kMouseButtonLeft) != 0)
			_pressedVisible = hoveredVisible;
		if ((mouse.released & kMouseButtonLeft) != 0) {
			if (_pressedVisible >= 0 && _pressedVisible == hoveredVisible) {
				_selectedVisible = hoveredVisible;
				selected = true;
			}
			_pressedVisible = -1;
		}
		if (!selected && hoveredVisible >= 0 && (uint)hoveredVisible != _selectedVisible) {
			_selectedVisible = hoveredVisible;
			updateFirstVisible();
			draw();
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	_engine->getCursor()->setVisible(false);
	if (selected) {
		const uint locationIndex = _visibleLocations[_selectedVisible];
		targetScript = resolveTargetScript(locationIndex);
		debugC(1, kDebugScene,
			"Ripper: world map selected location=%u name='%s' target='%s'",
			locationIndex, _locations[locationIndex].name.c_str(), targetScript.c_str());
		g_system->fillScreen(0);
		g_system->updateScreen();
		_savedPixels.clear();
		_savedPalette.clear();
	} else {
		debugC(1, kDebugScene, "Ripper: world map cancelled");
		restoreDisplay();
	}
	return true;
}

} // End of namespace Ripper
