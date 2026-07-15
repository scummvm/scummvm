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

#include "ripper/wac.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/ripper.h"
#include "ripper/script.h"

namespace Ripper {

static const int kWacWidth = 640;
static const int kWacHeight = 400;
static const int kWacControlY = 349;
static const int kWacControlX[] = { 172, 252, 326, 390 };
static const uint16 kWacControlActions[] = { 0x1900, 0x2000, 0x3100, 0x3b00 };
static const uint kWacDefaultCursor = 14;
static const uint kWacControlCursor = 16;
static const uint16 kDosF10Command = 0x4400;
static const uint kWacDatabaseEntryCount = 30;
static const uint kWacDatabaseFlagBase = 0x46;
static const uint kWacDatabaseTextBase = 0xdc;
static const uint kWacDatabaseControlId = 0x73a;
static const int kWacDatabaseLeft = 50;
static const int kWacDatabaseTop = 210;
static const int kWacDatabaseRight = 332;
static const int kWacDatabaseBottom = 400;
static const int kWacDatabaseRowHeight = 13;
static const int kWacDatabaseTitleRows = 1;
static const uint kWacDatabaseVisibleRows = 13;
static const byte kWacNormalBackground = 0;
static const byte kWacNormalText = 251;
static const byte kWacSelectedBackground = 248;
static const byte kWacSelectedText = 4;

WacManager::WacManager(RipperEngine *engine) : _engine(engine), _hoveredControl(-1),
		_pressedControl(-1), _databaseSelection(0), _databaseFirstVisible(0),
		_initialized(false) {
}

bool WacManager::initialize(ResourceManager &resources) {
	if (!resources.loadInterfacePcx("wac.pcx", _background) ||
		_background.width != kWacWidth || _background.height != kWacHeight ||
		_background.palette.size() < 256 * 3 ||
		!resources.loadInterfaceBitmapFont("small.fnt", _font) ||
		!resources.loadGameText(_gameText))
		return false;

	_controls.resize(ARRAYSIZE(kWacControlActions));
	for (uint i = 0; i < _controls.size(); ++i) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
			Common::String::format("wac%u.bbm", i), sequence) || sequence.frames.empty())
			return false;
		Control &control = _controls[i];
		control.bitmap = sequence.frames[0];
		control.bounds = Common::Rect(kWacControlX[i], kWacControlY,
			kWacControlX[i] + control.bitmap.width, kWacControlY + control.bitmap.height);
		control.action = kWacControlActions[i];
		debugC(2, kDebugWac,
			"Ripper: WAC front-end control=%u action=0x%x rect=%d,%d,%d,%d",
			i, control.action, control.bounds.left, control.bounds.top,
			control.bounds.width(), control.bounds.height());
	}

	_initialized = true;
	debugC(1, kDebugWac,
		"Ripper: initialized WAC front end background=wac.pcx controls=%u",
		_controls.size());
	return true;
}

bool WacManager::captureDisplay() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || screen->w != kWacWidth ||
		screen->h != kWacHeight) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	_savedPixels.resize(kWacWidth * kWacHeight);
	for (int y = 0; y < kWacHeight; ++y)
		memcpy(_savedPixels.data() + y * kWacWidth, screen->getBasePtr(0, y), kWacWidth);
	g_system->unlockScreen();
	_savedPalette.resize(256 * 3);
	g_system->getPaletteManager()->grabPalette(_savedPalette.data(), 0, 256);
	return true;
}

void WacManager::restoreDisplay() {
	if (_savedPixels.size() != kWacWidth * kWacHeight || _savedPalette.size() != 256 * 3)
		return;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || screen->w != kWacWidth ||
		screen->h != kWacHeight) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (int y = 0; y < kWacHeight; ++y)
		memcpy(screen->getBasePtr(0, y), _savedPixels.data() + y * kWacWidth, kWacWidth);
	g_system->unlockScreen();
	g_system->getPaletteManager()->setPalette(_savedPalette.data(), 0, 256);
	g_system->updateScreen();
	_savedPixels.clear();
	_savedPalette.clear();
}

void WacManager::drawBitmap(const BitmapAssetFrame &bitmap, int x, int y) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (uint row = 0; row < bitmap.height; ++row) {
		byte *destination = (byte *)screen->getBasePtr(x, y + row);
		const byte *source = bitmap.pixels.data() + row * bitmap.width;
		for (uint column = 0; column < bitmap.width; ++column) {
			if (source[column] != bitmap.transparentColor)
				destination[column] = source[column];
		}
	}
	g_system->unlockScreen();
}

void WacManager::drawFrontEnd() const {
	g_system->copyRectToScreen(_background.pixels.data(), _background.width,
		0, 0, _background.width, _background.height);
	g_system->getPaletteManager()->setPalette(_background.palette.data(), 0, 256);
	for (uint i = 0; i < _controls.size(); ++i)
		drawBitmap(_controls[i].bitmap, _controls[i].bounds.left, _controls[i].bounds.top);
	g_system->updateScreen();
}

int WacManager::findControl(const Common::Point &point) const {
	for (uint i = 0; i < _controls.size(); ++i) {
		if (_controls[i].bounds.contains(point))
			return i;
	}
	return -1;
}

bool WacManager::dispatchAction(uint16 action) {
	switch (action) {
	case kExitAction:
		debugC(1, kDebugWac, "Ripper: WAC front end selected exit action=0x1900");
		return false;
	case kDatabaseAction:
		runDatabase();
		drawFrontEnd();
		_engine->getInput()->discardMouseTransitions();
		break;
	case kTextViewerAction:
		debugC(1, kDebugWac,
			"Ripper: WAC text viewer action=0x3100 selected; handler is stubbed");
		break;
	case kHelpAction:
		debugC(1, kDebugWac,
			"Ripper: WAC help action=0x3b00 selected; handler is stubbed");
		break;
	default:
		warning("Ripper: unsupported WAC front-end action 0x%x", action);
		break;
	}
	return true;
}

const Common::String &WacManager::resourceString(uint resourceId) const {
	static const Common::String empty;
	if (resourceId == 0 || resourceId > _gameText.size())
		return empty;
	return _gameText[resourceId - 1];
}

uint WacManager::measureText(const Common::String &text) const {
	uint width = 0;
	for (uint i = 0; i < text.size(); ++i) {
		const byte character = (byte)text[i];
		if (character == ' ') {
			width += _font.spaceWidth;
			continue;
		}
		if (character < _font.firstCharacter ||
			character >= _font.firstCharacter + _font.glyphs.size())
			continue;
		const BitmapFontGlyph &glyph = _font.glyphs[character - _font.firstCharacter];
		width += glyph.xOffset + glyph.width + _font.characterSpacing;
	}
	return width;
}

void WacManager::drawText(byte *screen, uint pitch, int x, int y,
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
					screen[(y + glyph.yOffset + glyphY) * pitch + drawX +
						glyph.xOffset + glyphX] = color;
			}
		}
		drawX += glyph.xOffset + glyph.width + _font.characterSpacing;
	}
}

void WacManager::buildDatabaseEntries() {
	_databaseEntries.clear();
	for (uint index = 0; index < kWacDatabaseEntryCount; ++index) {
		const uint flag = kWacDatabaseFlagBase + index;
		if (!_engine->getScripts()->isMilestoneFlagSet(flag))
			continue;
		DatabaseEntry entry;
		entry.originalIndex = index;
		entry.label = resourceString(kWacDatabaseTextBase + index);
		_databaseEntries.push_back(entry);
		debugC(2, kDebugWac,
			"Ripper: WAC database visibleRow=%u entry=%u flag=0x%x textId=0x%x label='%s'",
			_databaseEntries.size() - 1, index, flag, kWacDatabaseTextBase + index,
			entry.label.c_str());
	}
	debugC(1, kDebugWac, "Ripper: built WAC database visibleEntries=%u scannedFlags=%u",
		_databaseEntries.size(), kWacDatabaseEntryCount);
}

void WacManager::drawDatabase() const {
	const Common::Rect bounds(kWacDatabaseLeft, kWacDatabaseTop,
		kWacDatabaseRight, kWacDatabaseBottom);
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	for (int y = bounds.top; y < bounds.bottom; ++y)
		memset(screen->getBasePtr(bounds.left, y), kWacNormalBackground,
			bounds.width());
	const Common::String &title = resourceString(0x4e);
	const int titleX = bounds.left + (bounds.width() - measureText(title)) / 2;
	drawText((byte *)screen->getPixels(), screen->pitch, titleX,
		bounds.top + (kWacDatabaseRowHeight - _font.lineHeight) / 2,
		title, kWacNormalText);

	if (_databaseEntries.empty()) {
		drawText((byte *)screen->getPixels(), screen->pitch, bounds.left + 4,
			bounds.top + kWacDatabaseRowHeight + 2,
			resourceString(0x46), kWacNormalText);
	} else {
		for (uint row = 0; row < kWacDatabaseVisibleRows; ++row) {
			const uint entryIndex = _databaseFirstVisible + row;
			if (entryIndex >= _databaseEntries.size())
				break;
			const int top = bounds.top +
				(kWacDatabaseTitleRows + row) * kWacDatabaseRowHeight;
			const bool selected = entryIndex == _databaseSelection;
			if (selected) {
				for (int y = top; y < top + kWacDatabaseRowHeight; ++y)
					memset(screen->getBasePtr(bounds.left, y),
						kWacSelectedBackground, bounds.width());
			}
			drawText((byte *)screen->getPixels(), screen->pitch,
				bounds.left + 4,
				top + (kWacDatabaseRowHeight - _font.lineHeight) / 2,
				_databaseEntries[entryIndex].label,
				selected ? kWacSelectedText : kWacNormalText);
		}
	}
	g_system->unlockScreen();
	g_system->updateScreen();
}

void WacManager::dispatchDatabaseEntry(const DatabaseEntry &entry) {
	if (entry.originalIndex == 1) {
		debugC(1, kDebugWac,
			"Ripper: WAC database entry=1 label='%s' routes to RunWacMugSelectionScene stub",
			entry.label.c_str());
		return;
	}
	if (entry.originalIndex == 2) {
		debugC(1, kDebugWac,
			"Ripper: WAC database entry=2 label='%s' routes to PlayMugSelectionCompletionMedia stub",
			entry.label.c_str());
		return;
	}
	debugC(1, kDebugWac,
		"Ripper: WAC database entry=%u label='%s' handler is stubbed",
		entry.originalIndex, entry.label.c_str());
}

void WacManager::runDatabase() {
	const Common::Rect bounds(kWacDatabaseLeft, kWacDatabaseTop,
		kWacDatabaseRight, kWacDatabaseBottom);
	buildDatabaseEntries();
	_databaseSelection = 0;
	_databaseFirstVisible = 0;
	_engine->getInput()->discardMouseTransitions();
	drawDatabase();
	debugC(1, kDebugWac,
		"Ripper: entered WAC database chooser control=0x%x bounds=%d,%d,%d,%d",
		kWacDatabaseControlId, bounds.left, bounds.top, bounds.width(), bounds.height());

	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == 0x1b || command == kExitAction || command == kDosF10Command) {
				active = false;
			} else if (!_databaseEntries.empty() && command == 0x4800) {
				if (_databaseSelection > 0)
					--_databaseSelection;
			} else if (!_databaseEntries.empty() && command == 0x5000) {
				if (_databaseSelection + 1 < _databaseEntries.size())
					++_databaseSelection;
			} else if (!_databaseEntries.empty() && command == 0x0d) {
				dispatchDatabaseEntry(_databaseEntries[_databaseSelection]);
			}
			if (_databaseSelection < _databaseFirstVisible)
				_databaseFirstVisible = _databaseSelection;
			else if (_databaseSelection >= _databaseFirstVisible + kWacDatabaseVisibleRows)
				_databaseFirstVisible = _databaseSelection - kWacDatabaseVisibleRows + 1;
			drawDatabase();
		}

		const MouseState mouse = _engine->getInput()->publishMouseState();
		_engine->getCursor()->update(bounds.contains(mouse.position) ?
			kWacControlCursor : kWacDefaultCursor);
		if (!_databaseEntries.empty() && bounds.contains(mouse.position)) {
			const int relativeY = mouse.position.y - bounds.top -
				kWacDatabaseTitleRows * kWacDatabaseRowHeight;
			if (relativeY >= 0) {
				const uint row = relativeY / kWacDatabaseRowHeight;
				const uint entryIndex = _databaseFirstVisible + row;
				if (row < kWacDatabaseVisibleRows && entryIndex < _databaseEntries.size() &&
					entryIndex != _databaseSelection) {
					_databaseSelection = entryIndex;
					debugC(2, kDebugWac,
						"Ripper: WAC database hover visibleRow=%u entry=%u label='%s' point=%d,%d",
						entryIndex, _databaseEntries[entryIndex].originalIndex,
						_databaseEntries[entryIndex].label.c_str(), mouse.position.x, mouse.position.y);
					drawDatabase();
				}
				if ((mouse.released & kMouseButtonLeft) != 0 && row < kWacDatabaseVisibleRows &&
					entryIndex < _databaseEntries.size())
					dispatchDatabaseEntry(_databaseEntries[entryIndex]);
			}
		}
		g_system->delayMillis(10);
	}
	debugC(1, kDebugWac, "Ripper: left WAC database chooser");
}

void WacManager::run() {
	if (!_initialized || !captureDisplay()) {
		warning("Ripper: WAC front end could not capture the active presentation");
		return;
	}

	// RunWacFrontEndLoop at 0x21865 owns a modal chooser registry and restores
	// the borrowed scene presentation after its 0x1900 or Escape exit action.
	debugC(1, kDebugWac, "Ripper: entered WAC front-end loop");
	_engine->getInput()->discardMouseTransitions();
	_hoveredControl = -1;
	_pressedControl = -1;
	drawFrontEnd();
	_engine->getCursor()->update(kWacDefaultCursor);

	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == 0x1b || command == kExitAction || command == kDosF10Command) {
				debugC(1, kDebugWac,
					"Ripper: WAC front end keyboard exit command=0x%x", command);
				active = false;
			}
		}

		const MouseState mouse = _engine->getInput()->publishMouseState();
		const int hoveredControl = findControl(mouse.position);
		if (hoveredControl != _hoveredControl) {
			_hoveredControl = hoveredControl;
			debugC(2, kDebugWac,
				"Ripper: WAC front-end hover control=%d action=0x%x point=%d,%d",
				_hoveredControl, _hoveredControl < 0 ? 0 : _controls[_hoveredControl].action,
				mouse.position.x, mouse.position.y);
		}
		_engine->getCursor()->update(
			_hoveredControl < 0 ? kWacDefaultCursor : kWacControlCursor);

		if ((mouse.pressed & kMouseButtonLeft) != 0)
			_pressedControl = _hoveredControl;
		if ((mouse.released & kMouseButtonLeft) != 0 && _pressedControl >= 0) {
			const int pressedControl = _pressedControl;
			_pressedControl = -1;
			if (pressedControl == _hoveredControl) {
				debugC(1, kDebugWac,
					"Ripper: WAC front-end selected control=%d action=0x%x",
					pressedControl, _controls[pressedControl].action);
				active = dispatchAction(_controls[pressedControl].action);
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	_engine->getCursor()->setVisible(false);
	restoreDisplay();
	debugC(1, kDebugWac, "Ripper: left WAC front-end loop");
}

} // End of namespace Ripper
