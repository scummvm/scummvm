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

#include "ripper/wac/database.h"

#include "ripper/wac/wac.h"

#include "common/debug.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/display.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/wac/broken_mug.h"
#include "ripper/wac/journal.h"
#include "ripper/wac/still_image.h"
#include "ripper/wac/voice_lock.h"

namespace Ripper {

static const uint kWacDefaultCursor = 14;
static const uint kWacControlCursor = 16;
static const uint32 kDosTickMillis = 55;
static const uint32 kWacDatabaseCornerInterval = 5 * kDosTickMillis;
static const uint16 kDosF10Command = 0x4400;
static const uint kWacDatabaseControlId = 0x73a;
static const int kWacDatabaseLeft = 400;
static const int kWacDatabaseTop = 50;
static const int kWacDatabaseRight = 590;
static const int kWacDatabaseBottom = 332;
static const int kWacDatabaseHeadingInset = 20;
static const int kWacDatabaseTitleTopInset = 1;
static const int kWacDatabaseBottomInset = 6;
static const int kWacDatabaseLeftInset = 5;
static const int kWacDatabaseRightInset = 20;
static const int kWacDatabaseRowInset = 1;
static const int kWacDatabaseTextInset = 2;
static const int kWacDatabaseRowHeight = 14;
static const uint kWacDatabaseVisibleRows = 18;
static const uint kWacDatabaseFrameTileCount = 9;
static const uint kWacDatabaseSkinFrameCount = 16;
static const byte kWacDatabaseBackground = 4;
static const byte kWacDatabaseTitleText = 248;
static const byte kWacDatabaseNormalText = 251;
static const byte kWacDatabaseSelectedText = 254;
static const int kWacMediaLeft = 50;
static const int kWacMediaTop = 50;
static const int kWacMediaWidth = 350;
static const int kWacMediaHeight = 282;
static const int kWacTextPanelWidth = 330;
static const int kWacTextPanelHeight = 222;
static const uint kWacDatabaseHelpResource = 406;
static const uint16 kWacDatabaseSelectionChanged = 0xfffe;
static const uint16 kWacDatabaseTextScrolled = 0xfffd;
static const uint16 kNoAction = WacManager::kNoAction;
static const uint16 kExitAction = WacManager::kExitAction;
static const uint16 kTextViewerAction = WacManager::kTextViewerAction;
static const uint16 kHelpAction = WacManager::kHelpAction;

class WacDatabaseMediaCallback : public MediaSequenceCallback {
public:
	WacDatabaseMediaCallback(WacDatabaseSession *database, byte activeEntryIndex) :
			_database(database), _activeEntryIndex(activeEntryIndex) {
	}

	uint16 service(uint) override {
		return _database->serviceDatabaseMediaInput(_activeEntryIndex);
	}

private:
	WacDatabaseSession *_database;
	byte _activeEntryIndex;
};

WacDatabaseSession::WacDatabaseSession(WacManager *wac) : _wac(wac),
		_databaseSelection(0), _databaseFirstVisible(0),
		_databaseTextScrollControl(0), _databaseTextScrollDragOffset(0),
		_databaseCornerLastMillis(0), _databaseCornerAlternate(false),
		_databaseTextScrollDragging(false) {
}

RipperEngine *WacDatabaseSession::engine() const {
	return _wac->_engine;
}

const Common::String &WacDatabaseSession::resourceString(uint resourceId) const {
	return _wac->resourceString(resourceId);
}

uint WacDatabaseSession::measureText(const Common::String &text) const {
	return _wac->measureText(text);
}

void WacDatabaseSession::drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const {
	_wac->drawText(screen, pitch, x, y, text, color);
}

void WacDatabaseSession::drawBitmap(const BitmapAssetFrame &bitmap,
		int x, int y) const {
	_wac->drawBitmap(bitmap, x, y);
}

void WacDatabaseSession::drawAnimatedCorner(int x, int y) const {
	if (_wac->_databaseSkin.size() >= kWacDatabaseSkinFrameCount)
		_wac->drawBitmap(_wac->_databaseSkin[
			_databaseCornerAlternate ? 15 : 0], x, y);
}

void WacDatabaseSession::serviceIdleEffects() {
	_wac->serviceIdleWindowAnimations();
}

bool WacDatabaseSession::persistentControlHovered() const {
	return _wac->_hoveredControl >= 0;
}

const Common::Array<BitmapAssetFrame> &
WacDatabaseSession::databaseScrollArrows() const {
	return _wac->_databaseScrollArrows;
}

void WacDatabaseSession::serviceDatabaseCornerAnimation(bool textPanelActive) {
	const uint32 now = g_system->getMillis(true);
	if (now - _databaseCornerLastMillis < kWacDatabaseCornerInterval ||
			_wac->_databaseSkin.size() < kWacDatabaseSkinFrameCount)
		return;

	_wac->drawBitmap(_wac->_databaseSkin[_databaseCornerAlternate ? 15 : 0],
		kWacDatabaseLeft, kWacDatabaseTop);
	if (textPanelActive)
		_wac->drawBitmap(_wac->_databaseSkin[_databaseCornerAlternate ? 15 : 0],
			kWacMediaLeft, kWacMediaTop);
	debugC(11, kDebugWac,
		"Ripper: WAC chooser corners frame=%u database=%d,%d textPanel=%d",
		_databaseCornerAlternate ? 15 : 0, kWacDatabaseLeft, kWacDatabaseTop,
		textPanelActive);
	_databaseCornerAlternate = !_databaseCornerAlternate;
	_databaseCornerLastMillis = now;
	_wac->_engine->getCursor()->refresh();
}

void WacDatabaseSession::buildDatabaseEntries() {
	_databaseEntries.clear();
	const uint catalogEntryCount = getWacDatabaseCatalogEntryCount();
	for (uint index = 0; index < catalogEntryCount; ++index) {
		const WacDatabaseCatalogEntry *catalog =
			getWacDatabaseCatalogEntry(index);
		if (!catalog ||
				!_wac->_engine->getMilestones()->isSet(catalog->milestoneFlag))
			continue;
		DatabaseEntry entry;
		entry.catalog = catalog;
		entry.label = _wac->resourceString(catalog->textResourceId);
		_databaseEntries.push_back(entry);
		debugC(2, kDebugWac,
			"Ripper: WAC database visibleRow=%u entry=%u flag=0x%x textId=0x%x label='%s'",
			_databaseEntries.size() - 1, catalog->originalIndex,
			catalog->milestoneFlag, catalog->textResourceId,
			entry.label.c_str());
	}
	debugC(1, kDebugWac, "Ripper: built WAC database visibleEntries=%u scannedFlags=%u",
		_databaseEntries.size(), catalogEntryCount);
}

void WacDatabaseSession::drawDatabase() const {
	const Common::Rect bounds(kWacDatabaseLeft, kWacDatabaseTop,
		kWacDatabaseRight, kWacDatabaseBottom);
	const Common::Rect client(bounds.left + kWacDatabaseLeftInset,
		bounds.top + kWacDatabaseHeadingInset,
		bounds.right - kWacDatabaseRightInset,
		bounds.bottom - kWacDatabaseBottomInset);
	const int rowTop = client.top + kWacDatabaseRowInset;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	for (int y = bounds.top; y < bounds.bottom; ++y)
		memset(screen->getBasePtr(bounds.left, y), kWacDatabaseBackground,
			bounds.width());
	// TileChooserControlFrame at 0x54fbe walks the original presentation's
	// transposed coordinate pair. In screen coordinates WACMNU0..8 are the
	// row-major top, middle, and bottom frame tiles.
	if (_wac->_databaseSkin.size() >= kWacDatabaseFrameTileCount) {
		const int tileWidth = _wac->_databaseSkin[0].width;
		const int tileHeight = _wac->_databaseSkin[0].height;
		const int columns = (bounds.width() + tileWidth - 1) / tileWidth;
		const int rows = (bounds.height() + tileHeight - 1) / tileHeight;
		for (int column = 0; column < columns; ++column) {
			for (int row = 0; row < rows; ++row) {
				const uint columnBand = column == 0 ? 0 :
					(column == columns - 1 ? 2 : 1);
				const uint rowBand = row == 0 ? 0 : (row == rows - 1 ? 2 : 1);
				const BitmapAssetFrame &tile = _wac->_databaseSkin[rowBand * 3 + columnBand];
				const int x = column == columns - 1 ? bounds.right - tile.width :
					bounds.left + column * tileWidth;
				const int y = row == rows - 1 ? bounds.bottom - tile.height :
					bounds.top + row * tileHeight;
				IndexedBitmapRenderer::drawBitmap(
					(byte *)screen->getPixels(), screen->pitch, tile, x, y);
			}
		}
	}
	if (_wac->_databaseSkin.size() >= kWacDatabaseSkinFrameCount)
		IndexedBitmapRenderer::drawBitmap((byte *)screen->getPixels(),
			screen->pitch,
			_wac->_databaseSkin[_databaseCornerAlternate ? 15 : 0],
			bounds.left, bounds.top);
	const Common::String &title = _wac->resourceString(0x4e);
	const int titleX = client.left + (client.width() - _wac->measureText(title)) / 2;
	_wac->drawText((byte *)screen->getPixels(), screen->pitch, titleX,
		bounds.top + kWacDatabaseTitleTopInset,
		title, kWacDatabaseTitleText);

	if (_databaseEntries.empty()) {
		_wac->drawText((byte *)screen->getPixels(), screen->pitch,
			client.left + kWacDatabaseTextInset,
			rowTop + (kWacDatabaseRowHeight - _wac->_font.lineHeight) / 2,
			_wac->resourceString(0x46), kWacDatabaseNormalText);
	} else {
		for (uint row = 0; row < kWacDatabaseVisibleRows; ++row) {
			const uint entryIndex = _databaseFirstVisible + row;
			if (entryIndex >= _databaseEntries.size())
				break;
			const int top = rowTop + row * kWacDatabaseRowHeight;
			const bool selected = entryIndex == _databaseSelection;
			_wac->drawText((byte *)screen->getPixels(), screen->pitch,
				client.left + kWacDatabaseTextInset,
				top + (kWacDatabaseRowHeight - _wac->_font.lineHeight) / 2,
				_databaseEntries[entryIndex].label,
				selected ? kWacDatabaseSelectedText : kWacDatabaseNormalText);
		}
	}
	g_system->unlockScreen();
	_wac->_engine->getCursor()->refresh();
	g_system->updateScreen();
}

void WacDatabaseSession::clearDatabaseMediaViewport() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (int y = kWacMediaTop; y < kWacMediaTop + kWacMediaHeight; ++y)
		memset(screen->getBasePtr(kWacMediaLeft, y), kWacDatabaseBackground,
			kWacMediaWidth);
	g_system->unlockScreen();
}

uint16 WacDatabaseSession::serviceDatabaseMediaInput(byte activeEntryIndex,
		uint *textFirstVisible, uint textMaximumFirstVisible, uint textPageRows,
		MouseState *publishedMouse, bool deferCursorUpdate) {
	if (_wac->_engine->getInput()->pollEvents()) {
		_wac->_engine->quitGame();
		return kExitAction;
	}

	bool refreshPalette = false;
	bool redrawDatabase = false;
	bool redrawText = false;
	while (_wac->_engine->getInput()->hasPendingKey()) {
		const uint16 command = _wac->_engine->getInput()->consumeKey();
		if (command == 0x1b)
			return command;
		if (command == kExitAction || command == kDosF10Command)
			return kExitAction;
		if (command == kHelpAction || command == kTextViewerAction) {
			const uint16 result = _wac->dispatchSubsceneAction(command,
				kWacDatabaseHelpResource, true);
			if (result != kNoAction)
				return result;
			refreshPalette = true;
			continue;
		}
		if (_databaseEntries.empty())
			continue;
		if (textFirstVisible) {
			uint nextFirstVisible = *textFirstVisible;
			switch (command) {
			case 0x4700:
				nextFirstVisible = 0;
				break;
			case 0x4800:
				nextFirstVisible = *textFirstVisible > 0 ? *textFirstVisible - 1 : 0;
				break;
			case 0x4900:
				nextFirstVisible = *textFirstVisible > textPageRows ?
					*textFirstVisible - textPageRows : 0;
				break;
			case 0x4f00:
				nextFirstVisible = textMaximumFirstVisible;
				break;
			case 0x5000:
				nextFirstVisible = MIN(*textFirstVisible + 1, textMaximumFirstVisible);
				break;
			case 0x5100:
				nextFirstVisible = MIN(*textFirstVisible + textPageRows,
					textMaximumFirstVisible);
				break;
			default:
				break;
			}
			if (nextFirstVisible != *textFirstVisible) {
				*textFirstVisible = nextFirstVisible;
				redrawText = true;
				debugC(2, kDebugWac,
					"Ripper: scrolled WAC database text panel firstLine=%u limit=%u command=0x%x",
					*textFirstVisible, textMaximumFirstVisible, command);
			}
			if (command == 0x4700 || command == 0x4800 || command == 0x4900 ||
					command == 0x4f00 || command == 0x5000 || command == 0x5100)
				continue;
		}
		if ((command == 0x4800 || command == 0x0f00) && _databaseSelection > 0) {
			--_databaseSelection;
			redrawDatabase = true;
		} else if ((command == 0x5000 || command == 0x09) &&
				_databaseSelection + 1 < _databaseEntries.size()) {
			++_databaseSelection;
			redrawDatabase = true;
		} else if (command == 0x0d &&
				_databaseEntries[_databaseSelection].originalIndex() != activeEntryIndex) {
			return kWacDatabaseSelectionChanged;
		}
	}

	if (_databaseSelection < _databaseFirstVisible)
		_databaseFirstVisible = _databaseSelection;
	else if (_databaseSelection >= _databaseFirstVisible + kWacDatabaseVisibleRows)
		_databaseFirstVisible = _databaseSelection - kWacDatabaseVisibleRows + 1;
	if (redrawDatabase)
		drawDatabase();

	const MouseState mouse = _wac->_engine->getInput()->publishMouseState();
	if (publishedMouse)
		*publishedMouse = mouse;
	const Common::Rect mediaBounds(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacMediaWidth, kWacMediaTop + kWacMediaHeight);
	if (textFirstVisible && mediaBounds.contains(mouse.position) && mouse.wheel != 0) {
		int nextFirstVisible = (int)*textFirstVisible - mouse.wheel;
		if (nextFirstVisible < 0)
			nextFirstVisible = 0;
		else if ((uint)nextFirstVisible > textMaximumFirstVisible)
			nextFirstVisible = textMaximumFirstVisible;
		if ((uint)nextFirstVisible != *textFirstVisible) {
			*textFirstVisible = nextFirstVisible;
			redrawText = true;
			debugC(3, kDebugWac,
				"Ripper: mouse-wheel scrolled WAC database text panel delta=%d firstLine=%u limit=%u",
				mouse.wheel, *textFirstVisible, textMaximumFirstVisible);
		}
	}
	if (textFirstVisible) {
		const Common::Rect textBounds(kWacMediaLeft, kWacMediaTop,
			kWacMediaLeft + kWacTextPanelWidth,
			kWacMediaTop + kWacTextPanelHeight);
		ModalDialogManager *modal = _wac->_engine->getModalDialog();
		const ModalDialogManager::TextPanelScrollControl scrollControl =
			modal->findTextPanelScrollControl(textBounds, mouse.position,
				*textFirstVisible, textMaximumFirstVisible,
				ModalDialogManager::kWacPresentation);
		if ((int)scrollControl != _databaseTextScrollControl) {
			_databaseTextScrollControl = scrollControl;
			redrawText = true;
			debugC(3, kDebugWac,
				"Ripper: WAC database text scroll hover control=%d point=%d,%d",
				_databaseTextScrollControl, mouse.position.x, mouse.position.y);
		}

		const Common::Rect thumbBounds = modal->textPanelScrollControlBounds(
			textBounds, ModalDialogManager::kTextPanelScrollThumb,
			*textFirstVisible, textMaximumFirstVisible,
			ModalDialogManager::kWacPresentation);
		const Common::Rect trackBounds = modal->textPanelScrollControlBounds(
			textBounds, ModalDialogManager::kTextPanelScrollTrack,
			*textFirstVisible, textMaximumFirstVisible,
			ModalDialogManager::kWacPresentation);
		if ((mouse.pressed & kMouseButtonLeft) != 0 &&
				(scrollControl == ModalDialogManager::kTextPanelScrollThumb ||
				scrollControl == ModalDialogManager::kTextPanelScrollTrack)) {
			_databaseTextScrollDragging = true;
			_databaseTextScrollDragOffset =
				scrollControl == ModalDialogManager::kTextPanelScrollThumb ?
					mouse.position.y - thumbBounds.top : thumbBounds.height() / 2;
			debugC(2, kDebugWac,
				"Ripper: began WAC database text scrollbar drag firstLine=%u point=%d,%d",
				*textFirstVisible, mouse.position.x, mouse.position.y);
		}
		if (_databaseTextScrollDragging && !trackBounds.isEmpty() &&
				!thumbBounds.isEmpty() && ((mouse.buttons & kMouseButtonLeft) != 0 ||
				(mouse.released & kMouseButtonLeft) != 0)) {
			const int travel = trackBounds.height() - thumbBounds.height();
			if (travel > 0) {
				const int thumbTop = CLIP<int>(
					mouse.position.y - _databaseTextScrollDragOffset,
					trackBounds.top, trackBounds.bottom - thumbBounds.height());
				const uint nextFirstVisible =
					((thumbTop - trackBounds.top) * textMaximumFirstVisible +
						travel / 2) / travel;
				if (nextFirstVisible != *textFirstVisible) {
					*textFirstVisible = nextFirstVisible;
					redrawText = true;
					debugC(3, kDebugWac,
						"Ripper: dragged WAC database text scrollbar firstLine=%u limit=%u point=%d,%d",
						*textFirstVisible, textMaximumFirstVisible,
						mouse.position.x, mouse.position.y);
				}
			}
		}
		const bool endedDrag = _databaseTextScrollDragging &&
			(mouse.released & kMouseButtonLeft) != 0;
		if (endedDrag) {
			_databaseTextScrollDragging = false;
			debugC(2, kDebugWac,
				"Ripper: ended WAC database text scrollbar drag firstLine=%u",
				*textFirstVisible);
		} else if ((mouse.released & kMouseButtonLeft) != 0) {
			uint nextFirstVisible = *textFirstVisible;
			if (scrollControl == ModalDialogManager::kTextPanelScrollUp &&
					nextFirstVisible > 0)
				--nextFirstVisible;
			else if (scrollControl == ModalDialogManager::kTextPanelScrollDown)
				nextFirstVisible = MIN(nextFirstVisible + 1,
					textMaximumFirstVisible);
			if (nextFirstVisible != *textFirstVisible) {
				*textFirstVisible = nextFirstVisible;
				redrawText = true;
				debugC(2, kDebugWac,
					"Ripper: clicked WAC database text scroll control=%d firstLine=%u limit=%u",
					scrollControl, *textFirstVisible, textMaximumFirstVisible);
			}
		}
	}
	_wac->serviceIdleWindowAnimations();
	serviceDatabaseCornerAnimation(textFirstVisible != nullptr);
	const uint16 controlAction = _wac->serviceFrontEndControls(mouse,
		(Common::Rect(kWacDatabaseLeft, kWacDatabaseTop,
			kWacDatabaseRight, kWacDatabaseBottom).contains(mouse.position) ||
			(textFirstVisible && mediaBounds.contains(mouse.position))) ?
			kWacControlCursor : kWacDefaultCursor, !deferCursorUpdate);
	if (controlAction != kNoAction) {
		const uint16 result = _wac->dispatchSubsceneAction(controlAction,
			kWacDatabaseHelpResource, true);
		if (result != kNoAction)
			return result;
		if (controlAction == kHelpAction || controlAction == kTextViewerAction)
			refreshPalette = true;
	}

	if (!_databaseEntries.empty()) {
		const Common::Rect bounds(kWacDatabaseLeft, kWacDatabaseTop,
			kWacDatabaseRight, kWacDatabaseBottom);
		if (bounds.contains(mouse.position)) {
			const int relativeY = mouse.position.y - bounds.top -
				kWacDatabaseHeadingInset - kWacDatabaseRowInset;
			if (relativeY >= 0) {
				const uint row = relativeY / kWacDatabaseRowHeight;
				const uint entryIndex = _databaseFirstVisible + row;
				if (row < kWacDatabaseVisibleRows && entryIndex < _databaseEntries.size()) {
					if (entryIndex != _databaseSelection) {
						_databaseSelection = entryIndex;
						drawDatabase();
						debugC(2, kDebugWac,
							"Ripper: WAC database media hover visibleRow=%u entry=%u activeEntry=%u point=%d,%d",
							entryIndex, _databaseEntries[entryIndex].originalIndex(),
							activeEntryIndex, mouse.position.x, mouse.position.y);
					}
					if ((mouse.released & kMouseButtonLeft) != 0 &&
							_databaseEntries[entryIndex].originalIndex() != activeEntryIndex)
						return kWacDatabaseSelectionChanged;
				}
			}
		}
	}

	if (refreshPalette)
		return MediaSequenceCallback::kContinueRefreshPalette;
	return redrawText ? kWacDatabaseTextScrolled : kNoAction;
}

bool WacDatabaseSession::drawDatabaseTextPanel(uint bodyResourceId,
		const Common::Rect &bounds, uint firstVisible,
		uint &maximumFirstVisible, uint &visibleRows) {
	if (!_wac->_engine->getModalDialog()->drawRetainedTextPanel(bodyResourceId,
			bounds, firstVisible, maximumFirstVisible, visibleRows,
			ModalDialogManager::kWacPresentation,
			static_cast<ModalDialogManager::TextPanelScrollControl>(
				_databaseTextScrollControl)))
		return false;
	if (_wac->_databaseSkin.size() >= kWacDatabaseSkinFrameCount)
		_wac->drawBitmap(_wac->_databaseSkin[_databaseCornerAlternate ? 15 : 0],
			bounds.left, bounds.top);
	_wac->_engine->getCursor()->refresh();
	g_system->updateScreen();
	return true;
}

uint16 WacDatabaseSession::runDatabaseTextPanel(DatabaseEntry &entry, uint bodyResourceId) {
	// RunCenteredTextPanelUntilExitAction at 0x2330c creates the untitled
	// 330-by-222 panel from the same tertiary WACMNU chooser template used by
	// contextual WAC help, then leaves the database chooser plus persistent WAC
	// controls active around its input loop.
	const Common::Rect bounds(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacTextPanelWidth, kWacMediaTop + kWacTextPanelHeight);
	uint firstVisible = 0;
	uint maximumFirstVisible = 0;
	uint visibleRows = 0;
	_databaseTextScrollControl = ModalDialogManager::kTextPanelScrollNone;
	_databaseTextScrollDragging = false;
	_databaseTextScrollDragOffset = 0;
	if (!drawDatabaseTextPanel(bodyResourceId, bounds, firstVisible,
			maximumFirstVisible, visibleRows))
		return kNoAction;

	_wac->_engine->getInput()->discardMouseTransitions();
	debugC(1, kDebugWac,
		"Ripper: entered WAC database text panel entry=%u label='%s' resource=0x%x bounds=%d,%d,%d,%d scrollLimit=%u",
		entry.originalIndex(), entry.label.c_str(), bodyResourceId,
		bounds.left, bounds.top, bounds.width(), bounds.height(), maximumFirstVisible);
	uint16 result = kNoAction;
	while (!_wac->_engine->shouldQuit()) {
		const uint16 command = serviceDatabaseMediaInput(entry.originalIndex(),
			&firstVisible, maximumFirstVisible, visibleRows);
		if (command == kWacDatabaseTextScrolled ||
				command == MediaSequenceCallback::kContinueRefreshPalette) {
			if (!drawDatabaseTextPanel(bodyResourceId, bounds, firstVisible,
					maximumFirstVisible, visibleRows))
				break;
		} else if (command == kWacDatabaseSelectionChanged ||
				command == kExitAction || command == 0x1b) {
			result = command;
			break;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	clearDatabaseMediaViewport();
	_databaseTextScrollControl = ModalDialogManager::kTextPanelScrollNone;
	_databaseTextScrollDragging = false;
	drawDatabase();
	debugC(1, kDebugWac,
		"Ripper: left WAC database text panel entry=%u resource=0x%x result=0x%x firstLine=%u",
		entry.originalIndex(), bodyResourceId, result, firstVisible);
	if (result == kWacDatabaseSelectionChanged &&
			_databaseSelection < _databaseEntries.size()) {
		debugC(2, kDebugWac,
			"Ripper: WAC database text panel switching entry=%u to entry=%u label='%s'",
			entry.originalIndex(), _databaseEntries[_databaseSelection].originalIndex(),
			_databaseEntries[_databaseSelection].label.c_str());
		return dispatchDatabaseEntry(_databaseEntries[_databaseSelection]);
	}
	return result == kExitAction ? kExitAction : kNoAction;
}

uint16 WacDatabaseSession::dispatchDatabaseEntry(DatabaseEntry &entry) {
	// RunWacInventorySelectionLoop at 0x2252a clears the left media viewport
	// before dispatching every selected database row.
	clearDatabaseMediaViewport();
	if (entry.catalog->handler == kWacDatabaseHandlerBrokenMug) {
		debugC(2, kDebugWac,
			"Ripper: WAC database retaining chooser while entering Broken Mug viewport=50,50,282,350");
		BrokenMugPuzzle::Result result;
		if (_wac->_engine->getMilestones()->isSet(kMilestoneCompletedMug)) {
			result = BrokenMugPuzzle::playCompletionMedia(_wac->_engine) ?
				BrokenMugPuzzle::kSolved : BrokenMugPuzzle::kLoadFailed;
		} else {
			BrokenMugPuzzle puzzle(_wac->_engine);
			result = puzzle.run();
		}
		debugC(1, kDebugWac,
			"Ripper: WAC database entry=1 label='%s' RunWacMugSelectionScene result=%d",
			entry.label.c_str(), result);
		if (result == BrokenMugPuzzle::kSolved) {
			// RunWacMugSelectionScene at 0x236b9 replaces the active row's text
			// pointer with resource 0xde after setting flags 0x47 and 0x48. The
			// visible row keeps original dispatch entry 1 for this chooser session.
			const WacDatabaseCatalogEntry *completedCatalog =
				getWacDatabaseCatalogEntry(2);
			const Common::String &completedLabel =
				_wac->resourceString(completedCatalog->textResourceId);
			debugC(1, kDebugWac,
				"Ripper: WAC database replaced completed cup row entry=1 oldLabel='%s' textId=0x%x newLabel='%s'",
				entry.label.c_str(), completedCatalog->textResourceId,
				completedLabel.c_str());
			entry.label = completedLabel;
		}
		_wac->drawFrontEnd();
		drawDatabase();
		return result == BrokenMugPuzzle::kExitWac ? kExitAction : kNoAction;
	}
	if (entry.catalog->handler == kWacDatabaseHandlerBrokenMugCompletion) {
		debugC(2, kDebugWac,
			"Ripper: WAC database retaining chooser while presenting completed Broken Mug");
		const bool played = BrokenMugPuzzle::playCompletionMedia(_wac->_engine);
		debugC(1, kDebugWac,
			"Ripper: WAC database entry=2 label='%s' PlayMugSelectionCompletionMedia success=%d",
			entry.label.c_str(), played);
		_wac->drawFrontEnd();
		drawDatabase();
		return kNoAction;
	}
	if (entry.catalog->handler == kWacDatabaseHandlerJournal) {
		WacJournalPuzzle puzzle(this);
		return puzzle.run(entry.originalIndex(), entry.label);
	}
	if (entry.catalog->handler == kWacDatabaseHandlerVoiceLock) {
		WacVoiceLockPuzzle puzzle(this);
		return puzzle.run(entry.originalIndex(), entry.label);
	}
	if (entry.catalog->handler == kWacDatabaseHandlerStillImage) {
		const Common::String path = Common::String::format(
			"wacinv%u.pcx", entry.originalIndex());
		WacStillImageViewer viewer(this);
		return viewer.run(entry.originalIndex(), entry.label, path);
	}
	if (entry.catalog->handler == kWacDatabaseHandlerLoopingMedia) {
		// RunWacInventorySelectionLoop at 0x2252a cases 0x0d and 0x0e resolve
		// WACINV13.SMK and WACINV14.SMK from INTERFAC.PL and enter
		// RunStaticMediaScreenWithOptionalVoiceover at 0x2339d without audio.
		// Both 320x200 sequences are centered in the 350x282 WAC viewport and
		// loop from frame one while the persistent WAC controls stay active.
		const Common::String path = Common::String::format(
			"wacinv%u.smk", entry.originalIndex());
		WacDatabaseMediaCallback callback(this, entry.originalIndex());
		uint16 command = kNoAction;
		_wac->_engine->getInput()->discardMouseTransitions();
		const bool played = _wac->_engine->getMedia()->playWacInterfaceSequence(
			path, 65, 91, 1, &callback, &command);
		clearDatabaseMediaViewport();
		drawDatabase();
		debugC(1, kDebugWac,
			"Ripper: WAC database entry=%u label='%s' media='%s' position=65,91 loopStartFrame=1 voiceover=none played=%d command=0x%x",
			entry.originalIndex(), entry.label.c_str(), path.c_str(), played,
			command);
		if (command == kExitAction)
			return kExitAction;
		if (command == kWacDatabaseSelectionChanged &&
				_databaseSelection < _databaseEntries.size()) {
			debugC(2, kDebugWac,
				"Ripper: WAC database media switching entry=%u to entry=%u label='%s'",
				entry.originalIndex(),
				_databaseEntries[_databaseSelection].originalIndex(),
				_databaseEntries[_databaseSelection].label.c_str());
			return dispatchDatabaseEntry(_databaseEntries[_databaseSelection]);
		}
		return kNoAction;
	}
	if (entry.catalog->handler == kWacDatabaseHandlerText) {
		return runDatabaseTextPanel(entry, entry.catalog->contentResourceId);
	}
	debugC(1, kDebugWac,
		"Ripper: WAC database entry=%u label='%s' handler is stubbed",
		entry.originalIndex(), entry.label.c_str());
	return kNoAction;
}

uint16 WacDatabaseSession::run() {
	const Common::Rect bounds(kWacDatabaseLeft, kWacDatabaseTop,
		kWacDatabaseRight, kWacDatabaseBottom);
	buildDatabaseEntries();
	_databaseSelection = 0;
	_databaseFirstVisible = 0;
	_databaseCornerAlternate = true;
	_databaseCornerLastMillis = g_system->getMillis(true);
	_wac->_engine->getInput()->discardMouseTransitions();
	drawDatabase();
	debugC(1, kDebugWac,
		"Ripper: entered WAC database chooser control=0x%x bounds=%d,%d,%d,%d client=%d,%d,%d,%d rows=%u",
		kWacDatabaseControlId, bounds.left, bounds.top, bounds.width(), bounds.height(),
		bounds.left + kWacDatabaseLeftInset, bounds.top + kWacDatabaseHeadingInset,
		bounds.width() - kWacDatabaseLeftInset - kWacDatabaseRightInset,
		bounds.height() - kWacDatabaseHeadingInset - kWacDatabaseBottomInset,
		kWacDatabaseVisibleRows);

	uint16 returnAction = kNoAction;
	bool active = true;
	while (active && !_wac->_engine->shouldQuit()) {
		if (_wac->_engine->getInput()->pollEvents()) {
			_wac->_engine->quitGame();
			break;
		}
		while (_wac->_engine->getInput()->hasPendingKey()) {
			const uint16 command = _wac->_engine->getInput()->consumeKey();
			if (command == 0x1b) {
				active = false;
			} else if (command == kExitAction || command == kDosF10Command) {
				returnAction = kExitAction;
				active = false;
			} else if (command == kHelpAction || command == kTextViewerAction) {
				returnAction = _wac->dispatchSubsceneAction(command,
					kWacDatabaseHelpResource, true);
				if (returnAction == kExitAction)
					active = false;
			} else if (!_databaseEntries.empty() && command == 0x4800) {
				if (_databaseSelection > 0)
					--_databaseSelection;
			} else if (!_databaseEntries.empty() && command == 0x5000) {
				if (_databaseSelection + 1 < _databaseEntries.size())
					++_databaseSelection;
			} else if (!_databaseEntries.empty() && command == 0x0d) {
				returnAction = dispatchDatabaseEntry(_databaseEntries[_databaseSelection]);
				if (returnAction == kExitAction)
					active = false;
			}
			if (_databaseSelection < _databaseFirstVisible)
				_databaseFirstVisible = _databaseSelection;
			else if (_databaseSelection >= _databaseFirstVisible + kWacDatabaseVisibleRows)
				_databaseFirstVisible = _databaseSelection - kWacDatabaseVisibleRows + 1;
			drawDatabase();
		}
		if (!active)
			break;

		const MouseState mouse = _wac->_engine->getInput()->publishMouseState();
		_wac->serviceIdleWindowAnimations();
		serviceDatabaseCornerAnimation();
		const uint fallbackCursor = bounds.contains(mouse.position) ?
			kWacControlCursor : kWacDefaultCursor;
		const uint16 controlAction = _wac->serviceFrontEndControls(mouse, fallbackCursor);
		if (controlAction != kNoAction) {
			returnAction = _wac->dispatchSubsceneAction(controlAction,
				kWacDatabaseHelpResource, true);
			if (returnAction == kExitAction)
				active = false;
		}
		if (!active)
			break;
		if (!_databaseEntries.empty() && bounds.contains(mouse.position)) {
			const int relativeY = mouse.position.y - bounds.top -
				kWacDatabaseHeadingInset - kWacDatabaseRowInset;
			if (relativeY >= 0) {
				const uint row = relativeY / kWacDatabaseRowHeight;
				const uint entryIndex = _databaseFirstVisible + row;
				if (row < kWacDatabaseVisibleRows && entryIndex < _databaseEntries.size() &&
					entryIndex != _databaseSelection) {
					_databaseSelection = entryIndex;
					debugC(2, kDebugWac,
						"Ripper: WAC database hover visibleRow=%u entry=%u label='%s' point=%d,%d",
						entryIndex, _databaseEntries[entryIndex].originalIndex(),
						_databaseEntries[entryIndex].label.c_str(), mouse.position.x, mouse.position.y);
					drawDatabase();
				}
				if ((mouse.released & kMouseButtonLeft) != 0 && row < kWacDatabaseVisibleRows &&
					entryIndex < _databaseEntries.size()) {
					returnAction = dispatchDatabaseEntry(_databaseEntries[entryIndex]);
					if (returnAction == kExitAction)
						active = false;
				}
			}
		}
		// RunWacSceneInputLoopUntilExitAction at 0x221e3 services the software
		// selection presentation every tick while the inventory chooser is active.
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	debugC(1, kDebugWac,
		"Ripper: left WAC database chooser returnAction=0x%x", returnAction);
	return returnAction;
}


} // End of namespace Ripper
