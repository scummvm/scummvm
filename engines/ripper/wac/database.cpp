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
#include "common/ptr.h"
#include "common/stream.h"
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
#include "ripper/wac/still_image.h"

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
static const uint16 kNoAction = WacManager::kNoAction;
static const uint16 kExitAction = WacManager::kExitAction;
static const uint16 kTextViewerAction = WacManager::kTextViewerAction;
static const uint16 kHelpAction = WacManager::kHelpAction;

class WacDatabaseMediaCallback : public MediaSequenceCallback {
public:
	using Outcome = WacDatabaseSession::DatabaseInputOutcome;
	WacDatabaseMediaCallback(WacDatabaseSession *database, byte activeEntryIndex,
			Common::SeekableReadStream *voiceoverStream = nullptr,
			const Common::String &voiceoverName = Common::String()) :
			_database(database), _activeEntryIndex(activeEntryIndex),
			_input(WacDatabaseSession::DatabaseInputMode::kPlayback), _outcome(Outcome::kContinue),
			_voiceoverStream(voiceoverStream), _voiceoverName(voiceoverName),
			_voiceoverRequested(!voiceoverName.empty()), _voiceoverStarted(false) {
	}

	~WacDatabaseMediaCallback() override {
		if (_voiceoverStarted)
			_database->engine()->getMedia()->stopSoundEffect(_voiceoverHandle);
	}

	uint16 service(uint) override {
		_outcome = _database->serviceDatabaseMediaInput(_activeEntryIndex, _input).outcome;
		switch (_outcome) {
		case Outcome::kContinue:
			break;
		case Outcome::kCloseEntry:
			return 0x1b;
		case Outcome::kExitWac:
			return kExitAction;
		case Outcome::kSelectEntry:
			return 0xfffe;
		case Outcome::kRedrawText:
			return 0xfffd;
		case Outcome::kRefreshPalette:
			return kContinueRefreshPalette;
		}
		if (!_voiceoverRequested)
			return kNoAction;
		if (!_voiceoverStarted) {
			_voiceoverStarted = true;
			if (!_database->engine()->getMedia()->playVoiceClipStream(
					_voiceoverStream.release(), _voiceoverName,
					_voiceoverHandle)) {
				warning("Ripper: could not start WAC database voiceover '%s'",
					_voiceoverName.c_str());
				return kVoiceoverComplete;
			}
			debugC(2, kDebugWac,
				"Ripper: started WAC database voiceover entry=%u media='%s' after first video frame",
				_activeEntryIndex, _voiceoverName.c_str());
			return kNoAction;
		}
		if (!_database->engine()->getMedia()->isSoundEffectActive(
				_voiceoverHandle)) {
			debugC(2, kDebugWac,
				"Ripper: completed WAC database voiceover entry=%u media='%s'; closing media viewer",
				_activeEntryIndex, _voiceoverName.c_str());
			return kVoiceoverComplete;
		}
		return kNoAction;
	}
	Outcome outcome() const { return _outcome; }

private:
	static const uint16 kVoiceoverComplete = 0xfffc;
	WacDatabaseSession *_database;
	byte _activeEntryIndex;
	WacDatabaseSession::DatabaseInputState _input;
	Outcome _outcome;
	Common::ScopedPtr<Common::SeekableReadStream> _voiceoverStream;
	Common::String _voiceoverName;
	Audio::SoundHandle _voiceoverHandle;
	bool _voiceoverRequested;
	bool _voiceoverStarted;
};

WacDatabaseSession::WacDatabaseSession(WacManager *wac) : _wac(wac),
		_databaseCornerLastMillis(0), _databaseCornerAlternate(false) {
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

void WacDatabaseSession::wrapText(const Common::String &text, uint maxWidth,
		Common::Array<Common::String> &lines) const {
	BitmapFontRenderer::wrapText(_wac->_font, text, maxWidth, lines);
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
		_databaseEntries.push_back(Common::move(entry));
		debugC(2, kDebugWac,
			"Ripper: WAC database visibleRow=%u entry=%u flag=0x%x textId=0x%x label='%s'",
			_databaseEntries.size() - 1, catalog->originalIndex,
			catalog->milestoneFlag, catalog->textResourceId,
			_databaseEntries.back().label.c_str());
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

	screen->fillRect(bounds, kWacDatabaseBackground);
	const Common::Rect screenBounds(0, 0, screen->w, screen->h);
	// WACMNU0..8 are the row-major top, middle, and bottom frame tiles.
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
					(byte *)screen->getPixels(), screen->pitch, tile, x, y,
					screenBounds);
			}
		}
	}
	if (_wac->_databaseSkin.size() >= kWacDatabaseSkinFrameCount)
		IndexedBitmapRenderer::drawBitmap((byte *)screen->getPixels(),
			screen->pitch,
			_wac->_databaseSkin[_databaseCornerAlternate ? 15 : 0],
			bounds.left, bounds.top, screenBounds);
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
			uint entryIndex;
			if (!_databaseChooser.resolveVisibleRow(row, entryIndex))
				break;
			const int top = rowTop + row * kWacDatabaseRowHeight;
			const bool selected = entryIndex == _databaseChooser.selectedIndex();
			_wac->drawText((byte *)screen->getPixels(), screen->pitch,
				client.left + kWacDatabaseTextInset,
				top + (kWacDatabaseRowHeight - _wac->_font.lineHeight) / 2,
				_databaseEntries[entryIndex].label,
				selected ? kWacDatabaseSelectedText : kWacDatabaseNormalText);
		}
	}
	g_system->unlockScreen();
	_wac->_engine->getCursor()->refresh();
	presentScreen();
}

void WacDatabaseSession::clearDatabaseMediaViewport() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	screen->fillRect(Common::Rect(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacMediaWidth, kWacMediaTop + kWacMediaHeight),
		kWacDatabaseBackground);
	g_system->unlockScreen();
}

WacDatabaseSession::DatabaseInputOutcome WacDatabaseSession::routeDatabaseAction(uint16 action) {
	if (action == 0x1b)
		return DatabaseInputOutcome::kCloseEntry;
	if (action == kExitAction || action == kDosF10Command)
		return DatabaseInputOutcome::kExitWac;
	const uint16 result = _wac->dispatchSubsceneAction(action, kWacDatabaseHelpResource, true);
	if (result == kExitAction)
		return DatabaseInputOutcome::kExitWac;
	return action == kHelpAction || action == kTextViewerAction ?
		DatabaseInputOutcome::kRefreshPalette : DatabaseInputOutcome::kContinue;
}

bool WacDatabaseSession::navigateDatabase(uint16 command, bool allowTab) {
	const uint selected = _databaseChooser.selectedIndex();
	// Media input exposes the selected row only after the queued keys drain.
	if (command == 0x4800 || (allowTab && command == 0x0f00))
		return selected > 0 && _databaseChooser.select(selected - 1, false);
	if (command == 0x5000 || (allowTab && command == 0x09))
		return _databaseChooser.select(selected + 1, false);
	return false;
}

bool WacDatabaseSession::resolveDatabaseRow(const Common::Point &point, uint &index) const {
	const Common::Rect bounds(kWacDatabaseLeft, kWacDatabaseTop,
		kWacDatabaseRight, kWacDatabaseBottom);
	if (!bounds.contains(point))
		return false;
	const int relativeY = point.y - bounds.top - kWacDatabaseHeadingInset - kWacDatabaseRowInset;
	return relativeY >= 0 && _databaseChooser.resolveVisibleRow(relativeY / kWacDatabaseRowHeight, index);
}

bool WacDatabaseSession::scrollDatabaseTextKey(TextScrollState &scroll, uint16 command, bool &redraw) {
	uint nextFirstVisible = scroll.firstVisible;
	switch (command) {
	case 0x4700:
		nextFirstVisible = 0;
		break;
	case 0x4800:
		nextFirstVisible = scroll.firstVisible > 0 ? scroll.firstVisible - 1 : 0;
		break;
	case 0x4900:
		nextFirstVisible = scroll.firstVisible > scroll.visibleRows ?
			scroll.firstVisible - scroll.visibleRows : 0;
		break;
	case 0x4f00:
		nextFirstVisible = scroll.maximumFirstVisible;
		break;
	case 0x5000:
		nextFirstVisible = MIN(scroll.firstVisible + 1, scroll.maximumFirstVisible);
		break;
	case 0x5100:
		nextFirstVisible = MIN(scroll.firstVisible + scroll.visibleRows, scroll.maximumFirstVisible);
		break;
	default:
		return false;
	}
	if (nextFirstVisible != scroll.firstVisible) {
		scroll.firstVisible = nextFirstVisible;
		redraw = true;
		debugC(2, kDebugWac,
			"Ripper: scrolled WAC database text panel firstLine=%u limit=%u command=0x%x",
			scroll.firstVisible, scroll.maximumFirstVisible, command);
	}
	return true;
}

bool WacDatabaseSession::scrollDatabaseTextMouse(TextScrollState &scroll, const MouseState &mouse) {
	bool redrawText = false;
	const Common::Rect mediaBounds(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacMediaWidth, kWacMediaTop + kWacMediaHeight);
	if (mediaBounds.contains(mouse.position) && mouse.wheel != 0) {
		int nextFirstVisible = (int)scroll.firstVisible - mouse.wheel;
		if (nextFirstVisible < 0)
			nextFirstVisible = 0;
		else if ((uint)nextFirstVisible > scroll.maximumFirstVisible)
			nextFirstVisible = scroll.maximumFirstVisible;
		if ((uint)nextFirstVisible != scroll.firstVisible) {
			scroll.firstVisible = nextFirstVisible;
			redrawText = true;
			debugC(3, kDebugWac,
				"Ripper: mouse-wheel scrolled WAC database text panel delta=%d firstLine=%u limit=%u",
				mouse.wheel, scroll.firstVisible, scroll.maximumFirstVisible);
		}
	}
	const Common::Rect textBounds(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacTextPanelWidth,
		kWacMediaTop + kWacTextPanelHeight);
	ModalDialogManager *modal = _wac->_engine->getModalDialog();
	const ModalDialogManager::TextPanelScrollControl scrollControl =
		modal->findTextPanelScrollControl(textBounds, mouse.position,
			scroll.firstVisible, scroll.maximumFirstVisible,
			ModalDialogManager::kWacPresentation);
	if (scrollControl != scroll.control) {
		scroll.control = scrollControl;
		redrawText = true;
		debugC(3, kDebugWac,
			"Ripper: WAC database text scroll hover control=%d point=%d,%d",
			scroll.control, mouse.position.x, mouse.position.y);
	}

	const Common::Rect thumbBounds = modal->textPanelScrollControlBounds(
		textBounds, ModalDialogManager::kTextPanelScrollThumb,
		scroll.firstVisible, scroll.maximumFirstVisible,
		ModalDialogManager::kWacPresentation);
	const Common::Rect trackBounds = modal->textPanelScrollControlBounds(
		textBounds, ModalDialogManager::kTextPanelScrollTrack,
		scroll.firstVisible, scroll.maximumFirstVisible,
		ModalDialogManager::kWacPresentation);
	if ((mouse.pressed & kMouseButtonLeft) != 0 &&
			(scrollControl == ModalDialogManager::kTextPanelScrollThumb ||
			scrollControl == ModalDialogManager::kTextPanelScrollTrack)) {
		scroll.dragging = true;
		scroll.dragOffset =
			scrollControl == ModalDialogManager::kTextPanelScrollThumb ?
				mouse.position.y - thumbBounds.top : thumbBounds.height() / 2;
		debugC(2, kDebugWac,
			"Ripper: began WAC database text scrollbar drag firstLine=%u point=%d,%d",
			scroll.firstVisible, mouse.position.x, mouse.position.y);
	}
	if (scroll.dragging && !trackBounds.isEmpty() &&
			!thumbBounds.isEmpty() && ((mouse.buttons & kMouseButtonLeft) != 0 ||
			(mouse.released & kMouseButtonLeft) != 0)) {
		const int travel = trackBounds.height() - thumbBounds.height();
		if (travel > 0) {
			const int thumbTop = CLIP<int>(
				mouse.position.y - scroll.dragOffset,
				trackBounds.top, trackBounds.bottom - thumbBounds.height());
			const uint nextFirstVisible =
				((thumbTop - trackBounds.top) * scroll.maximumFirstVisible +
					travel / 2) / travel;
			if (nextFirstVisible != scroll.firstVisible) {
				scroll.firstVisible = nextFirstVisible;
				redrawText = true;
				debugC(3, kDebugWac,
					"Ripper: dragged WAC database text scrollbar firstLine=%u limit=%u point=%d,%d",
					scroll.firstVisible, scroll.maximumFirstVisible,
					mouse.position.x, mouse.position.y);
			}
		}
	}
	const bool endedDrag = scroll.dragging &&
		(mouse.released & kMouseButtonLeft) != 0;
	if (endedDrag) {
		scroll.dragging = false;
		debugC(2, kDebugWac,
			"Ripper: ended WAC database text scrollbar drag firstLine=%u",
			scroll.firstVisible);
	} else if ((mouse.released & kMouseButtonLeft) != 0) {
		uint nextFirstVisible = scroll.firstVisible;
		if (scrollControl == ModalDialogManager::kTextPanelScrollUp &&
				nextFirstVisible > 0)
			--nextFirstVisible;
		else if (scrollControl == ModalDialogManager::kTextPanelScrollDown)
			nextFirstVisible = MIN(nextFirstVisible + 1,
				scroll.maximumFirstVisible);
		if (nextFirstVisible != scroll.firstVisible) {
			scroll.firstVisible = nextFirstVisible;
			redrawText = true;
			debugC(2, kDebugWac,
				"Ripper: clicked WAC database text scroll control=%d firstLine=%u limit=%u",
				scrollControl, scroll.firstVisible, scroll.maximumFirstVisible);
		}
	}
	return redrawText;
}

WacDatabaseSession::DatabaseInputResult WacDatabaseSession::serviceDatabaseMediaInput(
		byte activeEntryIndex, DatabaseInputState &input) {
	if (_wac->_engine->getInput()->pollEvents()) {
		_wac->_engine->quitGame();
		return DatabaseInputResult(DatabaseInputOutcome::kExitWac);
	}
	bool refreshPalette = false;
	bool redrawDatabase = false;
	bool redrawText = false;
	while (_wac->_engine->getInput()->hasPendingKey()) {
		const uint16 command = _wac->_engine->getInput()->consumeKey();
		if (command == 0x1b || command == kExitAction || command == kDosF10Command ||
				command == kHelpAction || command == kTextViewerAction) {
			const DatabaseInputOutcome outcome = routeDatabaseAction(command);
			if (outcome != DatabaseInputOutcome::kRefreshPalette)
				return DatabaseInputResult(outcome);
			refreshPalette = true;
			continue;
		}
		if (_databaseEntries.empty())
			continue;
		if (input.mode == DatabaseInputMode::kTextPanel && scrollDatabaseTextKey(input.text, command, redrawText))
			continue;
		if (navigateDatabase(command, true))
			redrawDatabase = true;
		else if (command == 0x0d &&
				_databaseEntries[_databaseChooser.selectedIndex()].originalIndex() != activeEntryIndex)
			return DatabaseInputResult(DatabaseInputOutcome::kSelectEntry);
	}
	_databaseChooser.select(_databaseChooser.selectedIndex());
	if (redrawDatabase)
		drawDatabase();

	DatabaseInputResult result;
	result.mouse = _wac->_engine->getInput()->publishMouseState();
	const MouseState &mouse = result.mouse;
	if (input.mode == DatabaseInputMode::kTextPanel)
		redrawText = scrollDatabaseTextMouse(input.text, mouse) || redrawText;
	_wac->serviceIdleWindowAnimations();
	serviceDatabaseCornerAnimation(input.mode == DatabaseInputMode::kTextPanel);
	const Common::Rect mediaBounds(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacMediaWidth, kWacMediaTop + kWacMediaHeight);
	const Common::Rect databaseBounds(kWacDatabaseLeft, kWacDatabaseTop,
		kWacDatabaseRight, kWacDatabaseBottom);
	const uint16 controlAction = _wac->serviceFrontEndControls(mouse,
		(databaseBounds.contains(mouse.position) ||
			(input.mode == DatabaseInputMode::kTextPanel && mediaBounds.contains(mouse.position))) ?
			kWacControlCursor : kWacDefaultCursor, input.mode != DatabaseInputMode::kViewer);
	if (controlAction != kNoAction) {
		result.outcome = routeDatabaseAction(controlAction);
		if (result.closesEntry())
			return result;
		if (result.outcome == DatabaseInputOutcome::kRefreshPalette)
			refreshPalette = true;
	}
	uint entryIndex;
	if (resolveDatabaseRow(mouse.position, entryIndex)) {
		if (_databaseChooser.select(entryIndex, false)) {
			drawDatabase();
			debugC(2, kDebugWac,
				"Ripper: WAC database media hover visibleRow=%u entry=%u activeEntry=%u point=%d,%d",
				entryIndex, _databaseEntries[entryIndex].originalIndex(),
				activeEntryIndex, mouse.position.x, mouse.position.y);
		}
		if ((mouse.released & kMouseButtonLeft) != 0 &&
				_databaseEntries[entryIndex].originalIndex() != activeEntryIndex) {
			result.outcome = DatabaseInputOutcome::kSelectEntry;
			return result;
		}
	}
	result.outcome = refreshPalette ? DatabaseInputOutcome::kRefreshPalette :
		(redrawText ? DatabaseInputOutcome::kRedrawText : DatabaseInputOutcome::kContinue);
	return result;
}

bool WacDatabaseSession::drawDatabaseTextPanel(uint bodyResourceId,
		const Common::Rect &bounds, TextScrollState &scroll) {
	if (!_wac->_engine->getModalDialog()->drawRetainedTextPanel(bodyResourceId,
			bounds, scroll.firstVisible, scroll.maximumFirstVisible, scroll.visibleRows,
			ModalDialogManager::kWacPresentation,
			scroll.control))
		return false;
	if (_wac->_databaseSkin.size() >= kWacDatabaseSkinFrameCount)
		_wac->drawBitmap(_wac->_databaseSkin[_databaseCornerAlternate ? 15 : 0],
			bounds.left, bounds.top);
	_wac->_engine->getCursor()->refresh();
	presentScreen();
	return true;
}

uint16 WacDatabaseSession::runDatabaseTextPanel(DatabaseEntry &entry, uint bodyResourceId) {
	const Common::Rect bounds(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacTextPanelWidth, kWacMediaTop + kWacTextPanelHeight);
	DatabaseInputState input(DatabaseInputMode::kTextPanel);
	TextScrollState &scroll = input.text;
	if (!drawDatabaseTextPanel(bodyResourceId, bounds, scroll))
		return kNoAction;

	_wac->_engine->getInput()->discardMouseTransitions();
	debugC(1, kDebugWac,
		"Ripper: entered WAC database text panel entry=%u label='%s' resource=0x%x bounds=%d,%d,%d,%d scrollLimit=%u",
		entry.originalIndex(), entry.label.c_str(), bodyResourceId,
		bounds.left, bounds.top, bounds.width(), bounds.height(), scroll.maximumFirstVisible);
	DatabaseInputOutcome result = DatabaseInputOutcome::kContinue;
	while (!_wac->_engine->shouldQuit()) {
		const DatabaseInputResult event = serviceDatabaseMediaInput(entry.originalIndex(), input);
		if (event.outcome == DatabaseInputOutcome::kRedrawText ||
				event.outcome == DatabaseInputOutcome::kRefreshPalette) {
			if (!drawDatabaseTextPanel(bodyResourceId, bounds, scroll))
				break;
		} else if (event.closesEntry()) {
			result = event.outcome;
			break;
		}
		presentScreen();
		g_system->delayMillis(10);
	}

	clearDatabaseMediaViewport();
	drawDatabase();
	debugC(1, kDebugWac,
		"Ripper: left WAC database text panel entry=%u resource=0x%x outcome=%d firstLine=%u",
		entry.originalIndex(), bodyResourceId, (int)result, scroll.firstVisible);
	if (entry.originalIndex() == 5) {
		_wac->_engine->getMilestones()->set(kMilestoneReadJournalEntry7,
			true, "wac-database-text");
		debugC(2, kDebugWac,
			"Ripper: completed WAC database text entry=%u resource=0x%x milestone=0x%x",
			entry.originalIndex(), bodyResourceId,
			kMilestoneReadJournalEntry7);
	}
	if (result == DatabaseInputOutcome::kSelectEntry &&
			_databaseChooser.selectedIndex() < _databaseEntries.size()) {
		debugC(2, kDebugWac,
			"Ripper: WAC database text panel switching entry=%u to entry=%u label='%s'",
			entry.originalIndex(), _databaseEntries[_databaseChooser.selectedIndex()].originalIndex(),
			_databaseEntries[_databaseChooser.selectedIndex()].label.c_str());
		return dispatchDatabaseEntry(_databaseEntries[_databaseChooser.selectedIndex()]);
	}
	return result == DatabaseInputOutcome::kExitWac ? kExitAction : kNoAction;
}

uint16 WacDatabaseSession::dispatchDatabaseEntry(DatabaseEntry &entry) {
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
			"Ripper: WAC database entry=1 label='%s' broken mug result=%d",
			entry.label.c_str(), result);
		if (result == BrokenMugPuzzle::kSolved) {
			// Replace this session's label while keeping the Broken Mug dispatch ID.
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
			"Ripper: WAC database entry=2 label='%s' completion media success=%d",
			entry.label.c_str(), played);
		_wac->drawFrontEnd();
		drawDatabase();
		return kNoAction;
	}
	if (entry.catalog->handler == kWacDatabaseHandlerJournal)
		return runJournal(entry.originalIndex(), entry.label);
	if (entry.catalog->handler == kWacDatabaseHandlerVoiceLock)
		return runVoiceLock(entry.originalIndex(), entry.label);
	if (entry.catalog->handler == kWacDatabaseHandlerStillImage) {
		const Common::String path = Common::String::format(
			"wacinv%u.pcx", entry.originalIndex());
		WacStillImageViewer viewer(this);
		return viewer.run(entry.originalIndex(), entry.label, path);
	}
	if (entry.catalog->handler == kWacDatabaseHandlerConditionalStillImage) {
		const bool alternate = _wac->_engine->getMilestones()->isSet(
			entry.catalog->presentationFlag);
		const Common::String path = alternate ? entry.catalog->imagePath :
			entry.catalog->mediaPath;
		debugC(2, kDebugWac,
			"Ripper: WAC database conditional still entry=%u flag=%u value=%d media='%s'",
			entry.originalIndex(), entry.catalog->presentationFlag, alternate,
			path.c_str());
		WacStillImageViewer viewer(this);
		return viewer.run(entry.originalIndex(), entry.label, path);
	}
	if (entry.catalog->handler == kWacDatabaseHandlerOptionalPresentation) {
		WacStillImageViewer viewer(this);
		return viewer.runWithOptionalPresentation(entry.originalIndex(),
			entry.label, entry.catalog->imagePath, entry.catalog->mediaPath,
			entry.catalog->presentationFlag, entry.catalog->completionFlag);
	}
	if (entry.catalog->handler == kWacDatabaseHandlerLoopingMedia) {
		const Common::String path = Common::String::format(
			"wacinv%u.smk", entry.originalIndex());
		WacDatabaseMediaCallback callback(this, entry.originalIndex());
		uint16 command = kNoAction;
		const Common::Rect viewport(kWacMediaLeft, kWacMediaTop,
			kWacMediaLeft + kWacMediaWidth, kWacMediaTop + kWacMediaHeight);
		_wac->_engine->getInput()->discardMouseTransitions();
		const bool played = _wac->_engine->getMedia()->playWacInterfaceSequence(
			path, viewport, 1, &callback, &command);
		clearDatabaseMediaViewport();
		drawDatabase();
		debugC(1, kDebugWac,
			"Ripper: WAC database entry=%u label='%s' media='%s' viewport=50,50,350,282 placement=centered loopStartFrame=1 voiceover=none played=%d command=0x%x",
			entry.originalIndex(), entry.label.c_str(), path.c_str(), played,
			command);
		if (callback.outcome() == DatabaseInputOutcome::kExitWac)
			return kExitAction;
		if (callback.outcome() == DatabaseInputOutcome::kSelectEntry &&
				_databaseChooser.selectedIndex() < _databaseEntries.size()) {
			debugC(2, kDebugWac,
				"Ripper: WAC database media switching entry=%u to entry=%u label='%s'",
				entry.originalIndex(),
				_databaseEntries[_databaseChooser.selectedIndex()].originalIndex(),
				_databaseEntries[_databaseChooser.selectedIndex()].label.c_str());
			return dispatchDatabaseEntry(_databaseEntries[_databaseChooser.selectedIndex()]);
		}
		return kNoAction;
	}
	if (entry.catalog->handler == kWacDatabaseHandlerConfiguredMediaWithVoiceover) {
		// RIP_GAME assets are loose files under PATHS/scene.
		ResourceManager *resources = _wac->_engine->getResources();
		Common::SeekableReadStream *mediaStream = resources ?
			resources->createReadStreamForPath(entry.catalog->imagePath) : nullptr;
		Common::SeekableReadStream *voiceoverStream = resources ?
			resources->createReadStreamForPath(entry.catalog->mediaPath) : nullptr;
		WacDatabaseMediaCallback callback(this, entry.originalIndex(),
			voiceoverStream, entry.catalog->mediaPath);
		uint16 command = kNoAction;
		const Common::Rect viewport(kWacMediaLeft, kWacMediaTop,
			kWacMediaLeft + kWacMediaWidth, kWacMediaTop + kWacMediaHeight);
		_wac->_engine->getInput()->discardMouseTransitions();
		const bool played = _wac->_engine->getMedia()->
			playWacInterfaceSequenceStream(mediaStream,
				entry.catalog->imagePath, viewport, 1, &callback, &command);
		_wac->_engine->getMilestones()->set(entry.catalog->completionFlag,
			true, "wac-database-media");
		clearDatabaseMediaViewport();
		drawDatabase();
		debugC(1, kDebugWac,
			"Ripper: WAC database entry=%u label='%s' media='%s' voiceover='%s' source=configured-scene-path played=%d command=0x%x milestone=0x%x",
			entry.originalIndex(), entry.label.c_str(), entry.catalog->imagePath,
			entry.catalog->mediaPath, played, command,
			entry.catalog->completionFlag);
		if (callback.outcome() == DatabaseInputOutcome::kExitWac)
			return kExitAction;
		if (callback.outcome() == DatabaseInputOutcome::kSelectEntry &&
				_databaseChooser.selectedIndex() < _databaseEntries.size())
			return dispatchDatabaseEntry(_databaseEntries[_databaseChooser.selectedIndex()]);
		return kNoAction;
	}
	if (entry.catalog->handler == kWacDatabaseHandlerText) {
		return runDatabaseTextPanel(entry, entry.catalog->contentResourceId);
	}
	if (entry.catalog->handler == kWacDatabaseHandlerRetailNoOp) {
		debugC(2, kDebugWac,
			"Ripper: WAC database entry=%u label='%s' completed retail no-op",
			entry.originalIndex(), entry.label.c_str());
		return kNoAction;
	}
	warning("Ripper: WAC database entry=%u label='%s' has invalid handler=%d",
		entry.originalIndex(), entry.label.c_str(), entry.catalog->handler);
	return kNoAction;
}

uint16 WacDatabaseSession::run() {
	const Common::Rect bounds(kWacDatabaseLeft, kWacDatabaseTop,
		kWacDatabaseRight, kWacDatabaseBottom);
	buildDatabaseEntries();
	_databaseChooser.reset(_databaseEntries.size(), kWacDatabaseVisibleRows);
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
			} else if (command == 0x4800 || command == 0x5000) {
				navigateDatabase(command, false);
			} else if (!_databaseEntries.empty() && command == 0x0d) {
				returnAction = dispatchDatabaseEntry(_databaseEntries[_databaseChooser.selectedIndex()]);
				if (returnAction == kExitAction)
					active = false;
			}
			_databaseChooser.select(_databaseChooser.selectedIndex());
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
		uint entryIndex;
		if (resolveDatabaseRow(mouse.position, entryIndex)) {
			if (_databaseChooser.select(entryIndex, false)) {
				debugC(2, kDebugWac,
					"Ripper: WAC database hover visibleRow=%u entry=%u label='%s' point=%d,%d",
					entryIndex, _databaseEntries[entryIndex].originalIndex(),
					_databaseEntries[entryIndex].label.c_str(), mouse.position.x, mouse.position.y);
				drawDatabase();
			}
			if ((mouse.released & kMouseButtonLeft) != 0) {
				returnAction = dispatchDatabaseEntry(_databaseEntries[entryIndex]);
				if (returnAction == kExitAction)
					active = false;
			}
		}
		// Present idle ticks too so the software cursor follows mouse motion.
		presentScreen();
		g_system->delayMillis(10);
	}
	debugC(1, kDebugWac,
		"Ripper: left WAC database chooser returnAction=0x%x", returnAction);
	return returnAction;
}


} // End of namespace Ripper
