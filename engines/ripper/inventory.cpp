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

#include "ripper/inventory.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kFirstUnlockFlag = 50;
static const uint kLastUnlockFlag = 69;
static const uint kConsumedFlagOffset = 50;
static const uint kLabelResourceBase = 200;
static const uint kSceneLabelResourceBase = 270;
static const uint kInventoryHelpResource = 0x1bb;
static const uint kInvalidInventoryResource = 0x4d;
static const uint kDefaultCursor = 14;
static const uint kChoiceCursor = 16;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kEnterCommand = 0x0d;
static const uint16 kUpCommand = 0x4800;
static const uint16 kDownCommand = 0x5000;
static const uint kInventoryBitmapCount = 9;
static const uint kMenuSkinFrameCount = 16;
static const uint kFrameTileCount = 9;
static const uint kMaximumVisibleEntries = 5;
static const int kMenuWidth = 320;
static const int kHeadingHeight = 30;
static const int kRowHeight = 64;
static const int kBottomPadding = 10;
static const int kButtonHeight = 20;
static const int kLeftPadding = 8;
static const int kRightPadding = 28;
static const int kItemImageInset = 12;
static const int kItemTextInset = 78;
static const byte kContentColor = 0;
static const byte kNormalTextColor = 251;
static const byte kSelectedTextColor = 254;
static const byte kButtonColor = 253;
static const byte kButtonTextColor = 4;
static const byte kButtonBorderColor = 4;
static const int kNoControl = -1;
static const int kUseControl = -2;
static const int kDoneControl = -3;

} // End of anonymous namespace

Inventory::Inventory(RipperEngine *engine) : _engine(engine),
		_selectedEntry(0), _firstVisibleEntry(0), _visibleEntries(0),
		_active(false), _initialized(false) {
}

bool Inventory::initialize(ResourceManager &resources) {
	if (!resources.loadGameText(_gameText) ||
			_gameText.size() < kSceneLabelResourceBase +
				(kLastUnlockFlag - kFirstUnlockFlag) ||
			!resources.loadInterfaceBitmapFont("small.fnt", _font) ||
			!loadFrameSet(resources, "mnu", kMenuSkinFrameCount, _skin) ||
			!loadFrameSet(resources, "invtry", kInventoryBitmapCount,
				_itemBitmaps)) {
		warning("Ripper: inventory resources are incomplete");
		return false;
	}
	_initialized = true;
	debugC(1, kDebugScene,
		"Ripper: initialized inventory unlockFlags=%u..%u consumedFlags=%u..%u labels=%u..%u useLabels=%u..%u mnuFrames=%u itemBitmaps=%u font=small.fnt",
		kFirstUnlockFlag, kLastUnlockFlag,
		kFirstUnlockFlag + kConsumedFlagOffset,
		kLastUnlockFlag + kConsumedFlagOffset,
		kLabelResourceBase, kLabelResourceBase + kLastUnlockFlag - kFirstUnlockFlag,
		kSceneLabelResourceBase,
		kSceneLabelResourceBase + kLastUnlockFlag - kFirstUnlockFlag,
		_skin.size(), _itemBitmaps.size());
	return true;
}

bool Inventory::loadFrameSet(ResourceManager &resources, const char *prefix,
		uint count, Common::Array<BitmapAssetFrame> &frames) {
	frames.clear();
	for (uint frameIndex = 0; frameIndex < count; ++frameIndex) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
				Common::String::format("%s%u.bbm", prefix, frameIndex), sequence) ||
				sequence.frames.empty())
			return false;
		frames.push_back(Common::move(sequence.frames.front()));
	}
	return true;
}

bool Inventory::isAvailable(uint unlockFlag) const {
	return unlockFlag >= kFirstUnlockFlag && unlockFlag <= kLastUnlockFlag &&
		_engine->getMilestones()->isSet(unlockFlag) &&
		!_engine->getMilestones()->isSet(unlockFlag + kConsumedFlagOffset);
}

bool Inventory::grant(uint unlockFlag, const char *source) {
	if (unlockFlag < kFirstUnlockFlag || unlockFlag > kLastUnlockFlag) {
		warning("Ripper: inventory unlock flag %u is outside %u..%u",
			unlockFlag, kFirstUnlockFlag, kLastUnlockFlag);
		return false;
	}
	// DispatchSceneEntryAction at 0x36892 action 3 sets the unlock bit and
	// clears its paired consumed bit before opening RunUnlockGatedSelectionMenu.
	if (!_engine->getMilestones()->set(unlockFlag, true, source) ||
			!_engine->getMilestones()->set(unlockFlag + kConsumedFlagOffset,
				false, source))
		return false;
	debugC(1, kDebugScene,
		"Ripper: granted inventory unlockFlag=%u item=%u consumedFlag=%u source=%s",
		unlockFlag, unlockFlag - kFirstUnlockFlag,
		unlockFlag + kConsumedFlagOffset, source ? source : "unknown");
	return true;
}

bool Inventory::buildEntries(int initialUnlockFlag) {
	_entries.clear();
	for (uint unlockFlag = kFirstUnlockFlag; unlockFlag <= kLastUnlockFlag; ++unlockFlag) {
		if (!isAvailable(unlockFlag))
			continue;
		const uint choiceId = unlockFlag - kFirstUnlockFlag;
		Entry entry;
		entry.unlockFlag = unlockFlag;
		entry.bitmapIndex = choiceId;
		entry.label = _gameText[kLabelResourceBase + choiceId - 1];
		_entries.push_back(entry);
	}
	if (_entries.empty()) {
		debugC(2, kDebugScene, "Ripper: inventory has no available items");
		return false;
	}
	_selectedEntry = 0;
	if (initialUnlockFlag >= 0) {
		for (uint entryIndex = 0; entryIndex < _entries.size(); ++entryIndex) {
			if (_entries[entryIndex].unlockFlag == (uint)initialUnlockFlag) {
				_selectedEntry = entryIndex;
				break;
			}
		}
	}
	_visibleEntries = MIN<uint>(_entries.size(), kMaximumVisibleEntries);
	_firstVisibleEntry = _selectedEntry >= _visibleEntries ?
		_selectedEntry - _visibleEntries + 1 : 0;
	updateLayout();
	debugC(1, kDebugScene,
		"Ripper: built inventory chooser available=%u initialUnlockFlag=%d selected=%u visible=%u bounds=%d,%d,%d,%d",
		_entries.size(), initialUnlockFlag, _selectedEntry, _visibleEntries,
		_menuBounds.left, _menuBounds.top, _menuBounds.width(), _menuBounds.height());
	return true;
}

bool Inventory::captureDisplay() {
	if (!_savedDisplay.capture())
		return false;
	_active = true;
	return true;
}

void Inventory::restoreDisplay() {
	if (!_active)
		return;
	_savedDisplay.restore(true, false);
	_savedDisplay.clear();
	_active = false;
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

void Inventory::applyPalette() {
	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	_engine->getToolbar()->applySharedPalettePatch(palette, 256);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void Inventory::updateLayout() {
	const int menuHeight = kHeadingHeight + _visibleEntries * kRowHeight +
		kBottomPadding;
	const int totalHeight = menuHeight + kButtonHeight;
	const int left = (640 - kMenuWidth) / 2;
	const int top = (400 - totalHeight) / 2;
	_menuBounds = Common::Rect(left, top, left + kMenuWidth, top + menuHeight);
	const int middle = left + kMenuWidth / 2;
	_useBounds = Common::Rect(left, _menuBounds.bottom, middle,
		_menuBounds.bottom + kButtonHeight);
	_doneBounds = Common::Rect(middle, _menuBounds.bottom,
		left + kMenuWidth, _menuBounds.bottom + kButtonHeight);
}

uint Inventory::measureText(const Common::String &text) const {
	return BitmapFontRenderer::measureText(_font, text);
}

void Inventory::drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const {
	BitmapFontRenderer::drawText(screen, pitch, _font, x, y, text, color);
}

void Inventory::drawBitmap(byte *screen, uint pitch,
		const BitmapAssetFrame &bitmap, int x, int y) const {
	for (uint row = 0; row < bitmap.height; ++row) {
		for (uint column = 0; column < bitmap.width; ++column) {
			const byte pixel = bitmap.pixels[row * bitmap.width + column];
			if (pixel != bitmap.transparentColor)
				screen[(y + row) * pitch + x + column] = pixel;
		}
	}
}

void Inventory::drawFrame(byte *screen, uint pitch,
		const Common::Rect &bounds) const {
	if (_skin.size() < kFrameTileCount)
		return;
	const int tileWidth = _skin[0].width;
	const int tileHeight = _skin[0].height;
	const int columns = (bounds.width() + tileWidth - 1) / tileWidth;
	const int rows = (bounds.height() + tileHeight - 1) / tileHeight;
	int y = bounds.top;
	for (int row = 0; row < rows; ++row) {
		int x = bounds.left;
		const BitmapAssetFrame *lastTile = nullptr;
		for (int column = 0; column < columns; ++column) {
			const uint columnBand = column == 0 ? 0 :
				(column == columns - 1 ? 2 : 1);
			const uint rowBand = row == 0 ? 0 : (row == rows - 1 ? 2 : 1);
			const BitmapAssetFrame &tile = _skin[rowBand * 3 + columnBand];
			drawBitmap(screen, pitch, tile, x, y);
			lastTile = &tile;
			if (column == columns - 2)
				x = bounds.right - tile.width;
			else
				x += tile.width;
		}
		if (lastTile) {
			if (row == rows - 2)
				y = bounds.bottom - lastTile->height;
			else
				y += lastTile->height;
		}
	}
}

void Inventory::drawButton(byte *screen, uint pitch,
		const Common::Rect &bounds, const Common::String &label,
		bool pressed) const {
	for (int y = bounds.top; y < bounds.bottom; ++y)
		memset(screen + y * pitch + bounds.left, kButtonColor, bounds.width());
	for (int x = bounds.left; x < bounds.right; ++x) {
		screen[bounds.top * pitch + x] = pressed ? kButtonBorderColor : 255;
		screen[(bounds.bottom - 1) * pitch + x] = kButtonBorderColor;
	}
	for (int y = bounds.top; y < bounds.bottom; ++y) {
		screen[y * pitch + bounds.left] = pressed ? kButtonBorderColor : 255;
		screen[y * pitch + bounds.right - 1] = kButtonBorderColor;
	}
	drawText(screen, pitch,
		bounds.left + (bounds.width() - measureText(label)) / 2 + (pressed ? 1 : 0),
		bounds.top + (bounds.height() - _font.lineHeight) / 2 + (pressed ? 1 : 0),
		label, kButtonTextColor);
}

Common::Rect Inventory::rowBounds(uint visibleRow) const {
	return Common::Rect(_menuBounds.left + kLeftPadding,
		_menuBounds.top + kHeadingHeight + visibleRow * kRowHeight,
		_menuBounds.right - kRightPadding,
		_menuBounds.top + kHeadingHeight + (visibleRow + 1) * kRowHeight);
}

void Inventory::draw(bool usePressed, bool donePressed) const {
	if (!_active || !_initialized || _entries.empty())
		return;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	byte *pixels = (byte *)screen->getPixels();
	drawFrame(pixels, screen->pitch, _menuBounds);
	// DrawPromptChooserTemplateLabelCallback at 0x16d0f confines its title
	// pass to the label strip over the tiled MNU frame. Preserve those frame
	// pixels here instead of replacing the full heading interior.
	for (int y = _menuBounds.top + kHeadingHeight;
			y < _menuBounds.bottom - kBottomPadding; ++y)
		memset(screen->getBasePtr(_menuBounds.left + kLeftPadding, y),
			kContentColor, _menuBounds.width() - kLeftPadding - kRightPadding);
	const Common::String &title = _gameText[2];
	drawText(pixels, screen->pitch,
		_menuBounds.left + (_menuBounds.width() - measureText(title)) / 2,
		_menuBounds.top + (kHeadingHeight - _font.lineHeight) / 2,
		title, kNormalTextColor);
	for (uint visibleRow = 0; visibleRow < _visibleEntries; ++visibleRow) {
		const uint entryIndex = _firstVisibleEntry + visibleRow;
		if (entryIndex >= _entries.size())
			break;
		const Entry &entry = _entries[entryIndex];
		const Common::Rect bounds = rowBounds(visibleRow);
		if (entry.bitmapIndex < _itemBitmaps.size()) {
			const BitmapAssetFrame &bitmap = _itemBitmaps[entry.bitmapIndex];
			drawBitmap(pixels, screen->pitch, bitmap,
				bounds.left + kItemImageInset,
				bounds.top + (bounds.height() - bitmap.height) / 2);
		}
		drawText(pixels, screen->pitch, bounds.left + kItemTextInset,
			bounds.top + (bounds.height() - _font.lineHeight) / 2,
			entry.label, entryIndex == _selectedEntry ?
				kSelectedTextColor : kNormalTextColor);
	}
	drawButton(pixels, screen->pitch, _useBounds, _gameText[70], usePressed);
	drawButton(pixels, screen->pitch, _doneBounds, _gameText[71], donePressed);
	g_system->unlockScreen();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

int Inventory::findControl(const Common::Point &point) const {
	if (_useBounds.contains(point))
		return kUseControl;
	if (_doneBounds.contains(point))
		return kDoneControl;
	for (uint visibleRow = 0; visibleRow < _visibleEntries; ++visibleRow) {
		if (rowBounds(visibleRow).contains(point) &&
				_firstVisibleEntry + visibleRow < _entries.size())
			return visibleRow;
	}
	return kNoControl;
}

bool Inventory::moveSelection(int delta) {
	const uint previous = _selectedEntry;
	if (delta < 0 && _selectedEntry > 0)
		--_selectedEntry;
	else if (delta > 0 && _selectedEntry + 1 < _entries.size())
		++_selectedEntry;
	if (_selectedEntry < _firstVisibleEntry)
		_firstVisibleEntry = _selectedEntry;
	else if (_selectedEntry >= _firstVisibleEntry + _visibleEntries)
		_firstVisibleEntry = _selectedEntry - _visibleEntries + 1;
	if (previous == _selectedEntry)
		return false;
	debugC(2, kDebugScene,
		"Ripper: inventory selection index=%u unlockFlag=%u item=%u",
		_selectedEntry, _entries[_selectedEntry].unlockFlag,
		_entries[_selectedEntry].bitmapIndex);
	return true;
}

void Inventory::closePresentation(const char *reason) {
	restoreDisplay();
	_engine->getCursor()->update(0);
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->discardMouseTransitions();
	debugC(2, kDebugScene, "Ripper: closed inventory presentation reason=%s",
		reason);
}

Inventory::ChoiceResult Inventory::executeChoice(uint unlockFlag,
		const Common::String &sceneLabel) {
	if (unlockFlag < kFirstUnlockFlag || unlockFlag > kLastUnlockFlag ||
			!isAvailable(unlockFlag))
		return kChoiceRejected;
	const uint choiceId = unlockFlag - kFirstUnlockFlag;
	const Common::String &requiredSceneLabel =
		_gameText[kSceneLabelResourceBase + choiceId - 1];
	// ExecuteUnlockSelectionChoice at 0x364be resolves the active frame's
	// labelOffset (+5 in the compiled 0x22-byte frame record) and compares it
	// case-insensitively with startup resource 270 + choice ID.
	if (!sceneLabel.equalsIgnoreCase(requiredSceneLabel)) {
		debugC(2, kDebugScene,
			"Ripper: rejected inventory item=%u unlockFlag=%u sceneLabel='%s' requiredLabel='%s'",
			choiceId, unlockFlag, sceneLabel.c_str(), requiredSceneLabel.c_str());
		_engine->getModalDialog()->run(kInvalidInventoryResource, true);
		return kChoiceRejected;
	}
	if ((choiceId == 5 &&
			(!_engine->getMilestones()->isSet(58) ||
			 _engine->getMilestones()->isSet(108))) ||
			(choiceId == 8 && _engine->getMilestones()->isSet(105))) {
		debugC(2, kDebugScene,
			"Ripper: rejected restricted inventory item=%u unlockFlag=%u",
			choiceId, unlockFlag);
		_engine->getModalDialog()->run(kInvalidInventoryResource, true);
		return kChoiceRejected;
	}

	// ExecuteUnlockSelectionChoice at 0x364be leaves the chooser active for an
	// invalid-use dialog, but accepted branches release it before presenting
	// item-specific media.
	restoreDisplay();
	if (choiceId == 1 || choiceId == 5 || choiceId == 8) {
		warning("Ripper: inventory item %u requires an unimplemented dedicated presentation", choiceId);
		return kChoiceFailed;
	}

	if (!_engine->getMilestones()->set(unlockFlag + kConsumedFlagOffset,
			true, "inventory-use"))
		return kChoiceFailed;
	if (choiceId == 0) {
		if (!_engine->getMilestones()->set(26, true, "inventory-scan-card") ||
				!_engine->getMedia()->play("verify.avi", true))
			return kChoiceFailed;
	} else if (choiceId == 6) {
		if (!_engine->getMedia()->play("q_cs_3.avi", true))
			return kChoiceFailed;
	} else if (choiceId == 7) {
		if (!_engine->getMedia()->play("p_shadow.avi", true) ||
				!_engine->getMilestones()->set(unlockFlag + kConsumedFlagOffset,
					false, "inventory-reusable-item"))
			return kChoiceFailed;
	}
	debugC(1, kDebugScene,
		"Ripper: used inventory item=%u unlockFlag=%u consumedFlag=%u sceneLabel='%s'",
		choiceId, unlockFlag, unlockFlag + kConsumedFlagOffset, sceneLabel.c_str());
	return kChoiceUsed;
}

Inventory::Result Inventory::run(const Common::String &sceneLabel,
		int initialUnlockFlag, uint *usedUnlockFlag) {
	if (usedUnlockFlag)
		*usedUnlockFlag = 0;
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->update(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	if (!buildEntries(initialUnlockFlag)) {
		closePresentation("inventory-empty");
		return kCancelled;
	}
	if (!captureDisplay()) {
		warning("Ripper: could not capture display for inventory chooser");
		return kLoadFailed;
	}
	applyPalette();
	draw();
	debugC(1, kDebugScene,
		"Ripper: entered inventory available=%u selected=%u unlockFlag=%u",
		_entries.size(), _selectedEntry, _entries[_selectedEntry].unlockFlag);

	int pressedControl = kNoControl;
	int hoveredControl = kNoControl;
	while (!_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		bool useSelection = false;
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == kEscapeCommand) {
				closePresentation("inventory-escape");
				debugC(1, kDebugScene, "Ripper: exited inventory result=cancelled");
				return kCancelled;
			}
			if (command == kHelpCommand) {
				_engine->getModalDialog()->run(kInventoryHelpResource, true);
				draw();
				continue;
			}
			if (command == kUpCommand) {
				if (moveSelection(-1))
					draw();
			} else if (command == kDownCommand) {
				if (moveSelection(1))
					draw();
			} else if (command == kEnterCommand) {
				useSelection = true;
				break;
			}
		}
		if (!useSelection) {
			const MouseState mouse = _engine->getInput()->publishMouseState();
			const int newHoveredControl = findControl(mouse.position);
			if (newHoveredControl != hoveredControl) {
				hoveredControl = newHoveredControl;
				_engine->getCursor()->update(hoveredControl != kNoControl ?
					kChoiceCursor : kDefaultCursor);
				if (hoveredControl >= 0) {
					const uint entryIndex = _firstVisibleEntry + hoveredControl;
					if (entryIndex < _entries.size() && entryIndex != _selectedEntry) {
						_selectedEntry = entryIndex;
						draw();
					}
				}
				debugC(3, kDebugScene,
					"Ripper: inventory hover control=%d point=%d,%d selected=%u",
					hoveredControl, mouse.position.x, mouse.position.y, _selectedEntry);
			}
			if ((mouse.pressed & kMouseButtonLeft) != 0) {
				pressedControl = hoveredControl;
				draw(pressedControl == kUseControl,
					pressedControl == kDoneControl);
			}
			if ((mouse.released & kMouseButtonLeft) != 0) {
				const int releasedControl = hoveredControl;
				draw();
				if (pressedControl == releasedControl) {
					if (releasedControl == kUseControl)
						useSelection = true;
					else if (releasedControl == kDoneControl) {
						closePresentation("inventory-done");
						debugC(1, kDebugScene,
							"Ripper: exited inventory result=cancelled");
						return kCancelled;
					}
				}
				pressedControl = kNoControl;
			}
		}
		if (useSelection) {
			const uint selectedUnlockFlag = _entries[_selectedEntry].unlockFlag;
			const ChoiceResult choiceResult = executeChoice(selectedUnlockFlag, sceneLabel);
			if (choiceResult == kChoiceUsed) {
				if (usedUnlockFlag)
					*usedUnlockFlag = selectedUnlockFlag;
				closePresentation("inventory-used");
				return kUsed;
			}
			if (choiceResult == kChoiceFailed) {
				closePresentation("inventory-failed");
				return kLoadFailed;
			}
			draw();
		}
		_engine->getCursor()->refresh();
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	closePresentation("inventory-quit");
	return _engine->shouldQuit() ? kCancelled : kLoadFailed;
}

Inventory::Result Inventory::grantAndRun(uint unlockFlag,
		const Common::String &sceneLabel, const char *source) {
	if (!grant(unlockFlag, source))
		return kLoadFailed;
	return run(sceneLabel, unlockFlag);
}

} // End of namespace Ripper
