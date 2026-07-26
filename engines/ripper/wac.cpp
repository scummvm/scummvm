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
#include "common/file.h"
#include "common/random.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/display.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/puzzles/broken_mug.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"

namespace Ripper {

static const int kWacWidth = 640;
static const int kWacHeight = 400;
static const int kWacControlY = 349;
static const int kWacControlX[] = { 172, 252, 326, 390 };
static const uint16 kWacControlActions[] = { 0x1900, 0x2000, 0x3100, 0x3b00 };
static const uint kWacDefaultCursor = 14;
static const uint kWacControlCursor = 16;
static const int kWacIdleWindowX[] = { 64, 460 };
static const int kWacIdleWindowY[] = { 21, 19 };
static const uint32 kDosTickMillis = 55;
static const uint32 kWacIdleWindowInterval = 3 * kDosTickMillis;
static const uint32 kWacDatabaseCornerInterval = 5 * kDosTickMillis;
static const uint16 kDosF10Command = 0x4400;
static const uint kWacDatabaseEntryCount = 30;
static const uint kWacDatabaseTextBase = 0xdc;
static const uint kWacDatabaseControlId = 0x73a;
static const int kWacDatabaseLeft = 400;
static const int kWacDatabaseTop = 50;
static const int kWacDatabaseRight = 590;
static const int kWacDatabaseBottom = 332;
static const int kWacDatabaseHeadingInset = 20;
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
static const int kWacMediaScrollX = 355;
static const int kWacMediaScrollUpY = 60;
static const int kWacMediaScrollDownY = 90;
static const int kWacMediaScrollStep = 10;
static const int kWacTextPanelWidth = 330;
static const int kWacTextPanelHeight = 222;
static const uint kWacMediaPaletteFirst = 10;
static const uint kWacMediaPaletteCount = 140;
static const uint kWacFrontEndHelpResource = 404;
static const uint kWacDatabaseHelpResource = 406;
static const uint kWacNotebookTitleResource = 0x49;
static const uint kWacNotebookMaximumBytes = 0x27c0;
static const uint kWacCircuitManualResource = 0xb6;
static const uint kWacJournalProgressResource = 0xae;
static const uint kWacJournalTextResource = 0xaf;
static const uint kWacJournalPromptResource = 0xb5;
static const uint kWacJournalPasswordBaseResource = 0xb6;
static const uint kWacJournalHelpResource = 407;
static const uint kWacJournalUnlockedFlag = 0xd8;
static const uint kWacJournalRevealFlagBase = 0xfa;
static const uint kWacJournalCompletionFlagBase = 0x104;
static const uint kWacJournalSectionCount = 8;
static const uint kWacJournalPasswordMaximumLength = 20;
static const uint32 kWacJournalRevealInterval = 36 * kDosTickMillis;
static const uint32 kWacJournalShuffleInterval = kDosTickMillis;
static const int kWacJournalProgressLeft = kWacMediaLeft + 50;
static const int kWacJournalProgressTop = kWacMediaTop + 1;
static const int kWacJournalTextEntryWidth = 200;
static const int kWacJournalTextEntryHeight = 42;
static const int kWacJournalTextEntryTop =
	kWacMediaTop + kWacTextPanelHeight + 5;
static const uint16 kWacDatabaseSelectionChanged = 0xfffe;
static const uint16 kWacDatabaseTextScrolled = 0xfffd;

static bool wacJournalAnswersMatch(const Common::String &entered,
		const Common::String &expected) {
	uint enteredIndex = 0;
	uint expectedIndex = 0;
	while (true) {
		while (enteredIndex < entered.size() &&
				!Common::isAlnum((byte)entered[enteredIndex]))
			++enteredIndex;
		while (expectedIndex < expected.size() &&
				!Common::isAlnum((byte)expected[expectedIndex]))
			++expectedIndex;
		if (enteredIndex == entered.size() || expectedIndex == expected.size())
			break;
		byte enteredCharacter = (byte)entered[enteredIndex++];
		byte expectedCharacter = (byte)expected[expectedIndex++];
		if (enteredCharacter >= 'A' && enteredCharacter <= 'Z')
			enteredCharacter += 'a' - 'A';
		if (expectedCharacter >= 'A' && expectedCharacter <= 'Z')
			expectedCharacter += 'a' - 'A';
		if (enteredCharacter != expectedCharacter)
			return false;
	}
	while (enteredIndex < entered.size() &&
			!Common::isAlnum((byte)entered[enteredIndex]))
		++enteredIndex;
	while (expectedIndex < expected.size() &&
			!Common::isAlnum((byte)expected[expectedIndex]))
		++expectedIndex;
	return enteredIndex == entered.size() && expectedIndex == expected.size();
}

static void shuffleJournalLine(Common::String &line,
		Common::RandomSource &randomSource) {
	if (line.size() < 2)
		return;
	// RunWacJournalRevealScene performs twenty random byte swaps per wrapped
	// line on entry and again while each unrevealed row remains visible.
	for (uint iteration = 0; iteration < 20; ++iteration) {
		const uint first = randomSource.getRandomNumber(line.size() - 1);
		const uint second = randomSource.getRandomNumber(line.size() - 1);
		const char temporary = line[first];
		line.setChar(line[second], first);
		line.setChar(temporary, second);
	}
}

class WacDatabaseMediaCallback : public MediaSequenceCallback {
public:
	WacDatabaseMediaCallback(WacManager *wac, byte activeEntryIndex) :
			_wac(wac), _activeEntryIndex(activeEntryIndex) {
	}

	uint16 service(uint) override {
		return _wac->serviceDatabaseMediaInput(_activeEntryIndex);
	}

private:
	WacManager *_wac;
	byte _activeEntryIndex;
};

static void blitBitmap(Graphics::Surface *screen, const BitmapAssetFrame &bitmap,
		int x, int y) {
	for (uint row = 0; row < bitmap.height; ++row) {
		byte *destination = (byte *)screen->getBasePtr(x, y + row);
		const byte *source = bitmap.pixels.data() + row * bitmap.width;
		for (uint column = 0; column < bitmap.width; ++column) {
			if (source[column] != bitmap.transparentColor)
				destination[column] = source[column];
		}
	}
}

WacManager::WacManager(RipperEngine *engine) : _engine(engine), _hoveredControl(-1),
		_pressedControl(-1), _databaseSelection(0), _databaseFirstVisible(0),
		_databaseStillImageScroll(0), _databaseScrollControl(0),
		_databaseTextScrollControl(0), _databaseTextScrollDragOffset(0),
		_idleWindowLastMillis(0), _databaseCornerLastMillis(0),
		_databaseCornerAlternate(false), _databaseTextScrollDragging(false),
		_initialized(false) {
	_idleWindowFrame[0] = 0;
	_idleWindowFrame[1] = 0;
}

bool WacManager::initialize(ResourceManager &resources) {
	if (!resources.loadInterfacePcx("wac.pcx", _background) ||
		_background.width != kWacWidth || _background.height != kWacHeight ||
		_background.palette.size() < 256 * 3 ||
		!resources.loadInterfaceBitmapFont("small.fnt", _font) ||
		!resources.loadGameText(_gameText))
		return false;

	_controls.clear();
	_controlBitmaps.clear();
	for (uint i = 0; i < ARRAYSIZE(kWacControlActions); ++i) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
			Common::String::format("wac%u.bbm", i), sequence) || sequence.frames.empty())
			return false;
		_controlBitmaps.push_back(sequence.frames[0]);
		const BitmapAssetFrame &bitmap = _controlBitmaps.back();
		const Common::Rect bounds(kWacControlX[i], kWacControlY,
			kWacControlX[i] + bitmap.width, kWacControlY + bitmap.height);
		_controls.add(bounds, kWacControlActions[i]);
		debugC(2, kDebugWac,
			"Ripper: WAC front-end control=%u action=0x%x rect=%d,%d,%d,%d",
			i, _controls[i].action, bounds.left, bounds.top,
			bounds.width(), bounds.height());
	}

	for (uint i = 0; i < ARRAYSIZE(_idleWindowAnimations); ++i) {
		if (!resources.loadInterfaceBitmapSet(
			Common::String::format("wacwn%u", i + 1), _idleWindowAnimations[i]))
			return false;
		debugC(2, kDebugWac,
			"Ripper: WAC idle window slot=%u position=%d,%d frames=%u intervalTicks=3",
			i, kWacIdleWindowX[i], kWacIdleWindowY[i], _idleWindowAnimations[i].size());
	}

	_databaseSkin.clear();
	for (uint i = 0; i < kWacDatabaseSkinFrameCount; ++i) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
			Common::String::format("wacmnu%u", i), sequence) || sequence.frames.empty())
			return false;
		_databaseSkin.push_back(sequence.frames[0]);
	}

	_databaseScrollArrows.resize(4);
	for (uint i = 0; i < _databaseScrollArrows.size(); ++i) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
				Common::String::format("mnarrow%u.bbm", i), sequence) ||
				sequence.frames.empty())
			return false;
		_databaseScrollArrows[i] = sequence.frames[0];
	}

	_initialized = true;
	debugC(1, kDebugWac,
		"Ripper: initialized WAC front end background=wac.pcx controls=%u databaseSkin=wacmnu frames=%u layout=screen-row-major",
		_controls.size(), _databaseSkin.size());
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
	blitBitmap(screen, bitmap, x, y);
	g_system->unlockScreen();
}

void WacManager::drawFrontEnd() const {
	g_system->copyRectToScreen(_background.pixels.data(), _background.width,
		0, 0, _background.width, _background.height);
	g_system->getPaletteManager()->setPalette(_background.palette.data(), 0, 256);
	for (uint i = 0; i < _controls.size(); ++i)
		drawBitmap(_controlBitmaps[i], _controls[i].bounds.left, _controls[i].bounds.top);
	g_system->updateScreen();
}

void WacManager::serviceIdleWindowAnimations() {
	const uint32 now = g_system->getMillis(true);
	if (now - _idleWindowLastMillis < kWacIdleWindowInterval)
		return;

	for (uint slot = 0; slot < ARRAYSIZE(_idleWindowAnimations); ++slot) {
		if (_idleWindowAnimations[slot].empty())
			continue;
		drawBitmap(_idleWindowAnimations[slot][_idleWindowFrame[slot]],
			kWacIdleWindowX[slot], kWacIdleWindowY[slot]);
		debugC(11, kDebugWac,
			"Ripper: WAC idle window slot=%u frame=%u position=%d,%d",
			slot, _idleWindowFrame[slot], kWacIdleWindowX[slot], kWacIdleWindowY[slot]);
		_idleWindowFrame[slot] = (_idleWindowFrame[slot] + 1) %
			_idleWindowAnimations[slot].size();
	}
	_engine->getCursor()->refresh();
	_idleWindowLastMillis = now;
}

void WacManager::serviceIdleEffects() {
	serviceIdleWindowAnimations();
}

void WacManager::serviceDatabaseCornerAnimation(bool textPanelActive) {
	const uint32 now = g_system->getMillis(true);
	if (now - _databaseCornerLastMillis < kWacDatabaseCornerInterval ||
		_databaseSkin.size() < kWacDatabaseSkinFrameCount)
		return;

	drawBitmap(_databaseSkin[_databaseCornerAlternate ? 15 : 0],
		kWacDatabaseLeft, kWacDatabaseTop);
	if (textPanelActive)
		drawBitmap(_databaseSkin[_databaseCornerAlternate ? 15 : 0],
			kWacMediaLeft, kWacMediaTop);
	debugC(11, kDebugWac,
		"Ripper: WAC chooser corners frame=%u database=%d,%d textPanel=%d",
		_databaseCornerAlternate ? 15 : 0, kWacDatabaseLeft, kWacDatabaseTop,
		textPanelActive);
	_databaseCornerAlternate = !_databaseCornerAlternate;
	_databaseCornerLastMillis = now;
	_engine->getCursor()->refresh();
}

int WacManager::findControl(const Common::Point &point) const {
	return _controls.findFirst(point);
}

uint16 WacManager::serviceFrontEndControls(const MouseState &mouse,
		uint fallbackCursor) {
	// RunWacFrontEndLoop at 0x21865 leaves these four UiControlState records in
	// the shared list. ServiceWacSceneInputAction at 0x21eef therefore services
	// them from every WAC subscene, not only from the front page.
	const int hoveredControl = findControl(mouse.position);
	if (hoveredControl != _hoveredControl) {
		_hoveredControl = hoveredControl;
		debugC(2, kDebugWac,
			"Ripper: WAC persistent-control hover control=%d action=0x%x point=%d,%d",
			_hoveredControl, _hoveredControl < 0 ? 0 : _controls[_hoveredControl].action,
			mouse.position.x, mouse.position.y);
	}
	_engine->getCursor()->update(
		_hoveredControl < 0 ? fallbackCursor : kWacControlCursor);

	if ((mouse.pressed & kMouseButtonLeft) != 0)
		_pressedControl = _hoveredControl;
	if ((mouse.released & kMouseButtonLeft) == 0 || _pressedControl < 0)
		return kNoAction;

	const int pressedControl = _pressedControl;
	_pressedControl = -1;
	if (pressedControl != _hoveredControl)
		return kNoAction;

	debugC(1, kDebugWac,
		"Ripper: WAC persistent-control selected control=%d action=0x%x",
		pressedControl, _controls[pressedControl].action);
	return _controls[pressedControl].action;
}

bool WacManager::runNotebook() {
	Common::String body;
	Common::File file;
	if (file.open("ripper.txt")) {
		const uint32 byteCount = MIN<uint32>(file.size(), kWacNotebookMaximumBytes);
		Common::Array<char> buffer;
		buffer.resize(byteCount);
		const uint32 bytesRead = byteCount == 0 ? 0 : file.read(buffer.data(), byteCount);
		if (bytesRead != 0)
			body = Common::String(buffer.data(), buffer.data() + bytesRead);
	}

	const Common::String &title = resourceString(kWacNotebookTitleResource);
	debugC(1, kDebugWac,
		"Ripper: opening WAC notebook action=0x3100 source='ripper.txt' bytes=%u",
		body.size());
	_engine->getCursor()->setVisible(true);
	return _engine->getModalDialog()->runText(title, body, "ripper.txt");
}

uint16 WacManager::dispatchSubsceneAction(uint16 action, uint helpResourceId,
		bool databaseActive) {
	switch (action) {
	case kNoAction:
		return kNoAction;
	case kExitAction:
		debugC(1, kDebugWac,
			"Ripper: WAC persistent power action=0x1900 requested front-end exit");
		return kExitAction;
	case kDatabaseAction:
		if (!databaseActive)
			return kDatabaseAction;
		debugC(2, kDebugWac,
			"Ripper: ignored nested WAC database action=0x2000 while chooser is active");
		return kNoAction;
	case kTextViewerAction:
		if (!runNotebook())
			warning("Ripper: WAC notebook action failed");
		_engine->getInput()->discardMouseTransitions();
		_pressedControl = -1;
		return kNoAction;
	case kHelpAction:
		debugC(1, kDebugWac,
			"Ripper: opening contextual WAC help resource=%u", helpResourceId);
		_engine->getCursor()->setVisible(true);
		if (!_engine->getModalDialog()->run(helpResourceId, false,
				ModalDialogManager::kWacPresentation))
			warning("Ripper: WAC contextual help resource=%u failed", helpResourceId);
		_engine->getInput()->discardMouseTransitions();
		_pressedControl = -1;
		return kNoAction;
	default:
		warning("Ripper: unsupported WAC front-end action 0x%x", action);
		return kNoAction;
	}
}

bool WacManager::dispatchAction(uint16 action) {
	const uint16 result = dispatchSubsceneAction(action, kWacFrontEndHelpResource, false);
	if (result == kExitAction)
		return false;
	if (result == kDatabaseAction) {
		const uint16 databaseResult = runDatabase();
		drawFrontEnd();
		_engine->getInput()->discardMouseTransitions();
		return databaseResult != kExitAction;
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
	return BitmapFontRenderer::measureText(_font, text);
}

void WacManager::drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const {
	BitmapFontRenderer::drawText(screen, pitch, _font, x, y, text, color);
}

void WacManager::buildDatabaseEntries() {
	_databaseEntries.clear();
	for (uint index = 0; index < kWacDatabaseEntryCount; ++index) {
		const uint flag = kMilestoneFirstWacDatabaseEntry + index;
		if (!_engine->getMilestones()->isSet(flag))
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
	if (_databaseSkin.size() >= kWacDatabaseFrameTileCount) {
		const int tileWidth = _databaseSkin[0].width;
		const int tileHeight = _databaseSkin[0].height;
		const int columns = (bounds.width() + tileWidth - 1) / tileWidth;
		const int rows = (bounds.height() + tileHeight - 1) / tileHeight;
		for (int column = 0; column < columns; ++column) {
			for (int row = 0; row < rows; ++row) {
				const uint columnBand = column == 0 ? 0 :
					(column == columns - 1 ? 2 : 1);
				const uint rowBand = row == 0 ? 0 : (row == rows - 1 ? 2 : 1);
				const BitmapAssetFrame &tile = _databaseSkin[rowBand * 3 + columnBand];
				const int x = column == columns - 1 ? bounds.right - tile.width :
					bounds.left + column * tileWidth;
				const int y = row == rows - 1 ? bounds.bottom - tile.height :
					bounds.top + row * tileHeight;
				blitBitmap(screen, tile, x, y);
			}
		}
	}
	if (_databaseSkin.size() >= kWacDatabaseSkinFrameCount)
		blitBitmap(screen, _databaseSkin[_databaseCornerAlternate ? 15 : 0],
			bounds.left, bounds.top);
	const Common::String &title = resourceString(0x4e);
	const int titleX = client.left + (client.width() - measureText(title)) / 2;
	drawText((byte *)screen->getPixels(), screen->pitch, titleX,
		bounds.top + (kWacDatabaseHeadingInset - _font.lineHeight) / 2,
		title, kWacDatabaseTitleText);

	if (_databaseEntries.empty()) {
		drawText((byte *)screen->getPixels(), screen->pitch,
			client.left + kWacDatabaseTextInset,
			rowTop + (kWacDatabaseRowHeight - _font.lineHeight) / 2,
			resourceString(0x46), kWacDatabaseNormalText);
	} else {
		for (uint row = 0; row < kWacDatabaseVisibleRows; ++row) {
			const uint entryIndex = _databaseFirstVisible + row;
			if (entryIndex >= _databaseEntries.size())
				break;
			const int top = rowTop + row * kWacDatabaseRowHeight;
			const bool selected = entryIndex == _databaseSelection;
			drawText((byte *)screen->getPixels(), screen->pitch,
				client.left + kWacDatabaseTextInset,
				top + (kWacDatabaseRowHeight - _font.lineHeight) / 2,
				_databaseEntries[entryIndex].label,
				selected ? kWacDatabaseSelectedText : kWacDatabaseNormalText);
		}
	}
	g_system->unlockScreen();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

void WacManager::clearDatabaseMediaViewport() {
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
	_databaseStillImage = BitmapAssetFrame();
	_databaseStillImageScroll = 0;
	_databaseScrollControl = 0;
}

bool WacManager::loadDatabaseStillImage(const Common::String &path) {
	BitmapAssetFrame image;
	if (!_engine->getResources()->loadInterfacePcx(path, image))
		return false;
	if (image.palette.size() < (kWacMediaPaletteFirst + kWacMediaPaletteCount) * 3)
		return false;

	_databaseStillImage = Common::move(image);
	_databaseStillImageScroll = 0;
	_databaseScrollControl = 0;
	g_system->getPaletteManager()->setPalette(
		_databaseStillImage.palette.data() + kWacMediaPaletteFirst * 3,
		kWacMediaPaletteFirst, kWacMediaPaletteCount);
	drawDatabaseStillImage();
	debugC(1, kDebugWac,
		"Ripper: loaded WAC database still image='%s' size=%ux%u viewport=%d,%d,%d,%d scrollLimit=%u",
		path.c_str(), _databaseStillImage.width, _databaseStillImage.height,
		kWacMediaLeft, kWacMediaTop, kWacMediaWidth, kWacMediaHeight,
		_databaseStillImage.height > kWacMediaHeight ?
			_databaseStillImage.height - kWacMediaHeight : 0);
	return true;
}

void WacManager::drawDatabaseStillImage() const {
	if (_databaseStillImage.pixels.empty())
		return;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	for (int y = kWacMediaTop; y < kWacMediaTop + kWacMediaHeight; ++y)
		memset(screen->getBasePtr(kWacMediaLeft, y), kWacDatabaseBackground,
			kWacMediaWidth);
	const uint copyWidth = MIN<uint>(_databaseStillImage.width, kWacMediaWidth);
	const uint copyHeight = MIN<uint>(
		_databaseStillImage.height - _databaseStillImageScroll, kWacMediaHeight);
	const int x = _databaseStillImage.height > kWacMediaHeight ? kWacMediaLeft :
		kWacMediaLeft + (kWacMediaWidth - copyWidth) / 2;
	const int y = _databaseStillImage.height > kWacMediaHeight ? kWacMediaTop :
		kWacMediaTop + (kWacMediaHeight - copyHeight) / 2;
	for (uint row = 0; row < copyHeight; ++row) {
		memcpy(screen->getBasePtr(x, y + row),
			_databaseStillImage.pixels.data() +
				(_databaseStillImageScroll + row) * _databaseStillImage.width,
			copyWidth);
	}
	g_system->unlockScreen();
	drawDatabaseScrollControls();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

void WacManager::drawDatabaseScrollControls() const {
	if (_databaseStillImage.height <= kWacMediaHeight ||
		_databaseScrollArrows.size() < 4)
		return;
	const uint maximumScroll = _databaseStillImage.height - kWacMediaHeight;
	drawBitmap(_databaseScrollArrows[
		_databaseScrollControl == 1 && _databaseStillImageScroll > 0 ? 1 : 0],
		kWacMediaScrollX, kWacMediaScrollUpY);
	drawBitmap(_databaseScrollArrows[
		_databaseScrollControl == 2 && _databaseStillImageScroll < maximumScroll ? 3 : 2],
		kWacMediaScrollX, kWacMediaScrollDownY);
}

int WacManager::findDatabaseScrollControl(const Common::Point &point) const {
	if (_databaseStillImage.height <= kWacMediaHeight ||
		_databaseScrollArrows.size() < 4)
		return 0;
	const Common::Rect up(kWacMediaScrollX, kWacMediaScrollUpY,
		kWacMediaScrollX + _databaseScrollArrows[0].width,
		kWacMediaScrollUpY + _databaseScrollArrows[0].height);
	const Common::Rect down(kWacMediaScrollX, kWacMediaScrollDownY,
		kWacMediaScrollX + _databaseScrollArrows[2].width,
		kWacMediaScrollDownY + _databaseScrollArrows[2].height);
	if (up.contains(point))
		return 1;
	if (down.contains(point))
		return 2;
	return 0;
}

void WacManager::scrollDatabaseStillImage(int delta) {
	if (_databaseStillImage.height <= kWacMediaHeight)
		return;
	const uint maximumScroll = _databaseStillImage.height - kWacMediaHeight;
	int nextScroll = (int)_databaseStillImageScroll + delta;
	if (nextScroll < 0)
		nextScroll = 0;
	else if ((uint)nextScroll > maximumScroll)
		nextScroll = maximumScroll;
	if ((uint)nextScroll == _databaseStillImageScroll)
		return;
	_databaseStillImageScroll = nextScroll;
	debugC(2, kDebugWac,
		"Ripper: scrolled WAC database still image offset=%u limit=%u delta=%d",
		_databaseStillImageScroll, maximumScroll, delta);
	drawDatabaseStillImage();
}

uint16 WacManager::serviceDatabaseMediaInput(byte activeEntryIndex,
		uint *textFirstVisible, uint textMaximumFirstVisible, uint textPageRows) {
	if (_engine->getInput()->pollEvents()) {
		_engine->quitGame();
		return kExitAction;
	}

	bool refreshPalette = false;
	bool redrawDatabase = false;
	bool redrawText = false;
	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == 0x1b)
			return command;
		if (command == kExitAction || command == kDosF10Command)
			return kExitAction;
		if (command == kHelpAction || command == kTextViewerAction) {
			const uint16 result = dispatchSubsceneAction(command,
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
				_databaseEntries[_databaseSelection].originalIndex != activeEntryIndex) {
			return kWacDatabaseSelectionChanged;
		}
	}

	if (_databaseSelection < _databaseFirstVisible)
		_databaseFirstVisible = _databaseSelection;
	else if (_databaseSelection >= _databaseFirstVisible + kWacDatabaseVisibleRows)
		_databaseFirstVisible = _databaseSelection - kWacDatabaseVisibleRows + 1;
	if (redrawDatabase)
		drawDatabase();

	const MouseState mouse = _engine->getInput()->publishMouseState();
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
		ModalDialogManager *modal = _engine->getModalDialog();
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
	serviceIdleWindowAnimations();
	serviceDatabaseCornerAnimation(textFirstVisible != nullptr);
	const uint16 controlAction = serviceFrontEndControls(mouse,
		(Common::Rect(kWacDatabaseLeft, kWacDatabaseTop,
			kWacDatabaseRight, kWacDatabaseBottom).contains(mouse.position) ||
			(textFirstVisible && mediaBounds.contains(mouse.position))) ?
			kWacControlCursor : kWacDefaultCursor);
	if (controlAction != kNoAction) {
		const uint16 result = dispatchSubsceneAction(controlAction,
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
							entryIndex, _databaseEntries[entryIndex].originalIndex,
							activeEntryIndex, mouse.position.x, mouse.position.y);
					}
					if ((mouse.released & kMouseButtonLeft) != 0 &&
							_databaseEntries[entryIndex].originalIndex != activeEntryIndex)
						return kWacDatabaseSelectionChanged;
				}
			}
		}
	}

	if (refreshPalette)
		return MediaSequenceCallback::kContinueRefreshPalette;
	return redrawText ? kWacDatabaseTextScrolled : kNoAction;
}

bool WacManager::drawDatabaseTextPanel(uint bodyResourceId,
		const Common::Rect &bounds, uint firstVisible,
		uint &maximumFirstVisible, uint &visibleRows) {
	if (!_engine->getModalDialog()->drawRetainedTextPanel(bodyResourceId,
			bounds, firstVisible, maximumFirstVisible, visibleRows,
			ModalDialogManager::kWacPresentation,
			static_cast<ModalDialogManager::TextPanelScrollControl>(
				_databaseTextScrollControl)))
		return false;
	if (_databaseSkin.size() >= kWacDatabaseSkinFrameCount)
		drawBitmap(_databaseSkin[_databaseCornerAlternate ? 15 : 0],
			bounds.left, bounds.top);
	_engine->getCursor()->refresh();
	g_system->updateScreen();
	return true;
}

void WacManager::wrapJournalText(const Common::String &text, uint maximumWidth,
		Common::Array<Common::String> &lines) const {
	lines.clear();
	Common::String line;
	Common::String word;
	for (uint index = 0; index <= text.size(); ++index) {
		const char character = index < text.size() ? text[index] : '\n';
		if (character == '\r')
			continue;
		if (character != ' ' && character != '\n') {
			word += character;
			continue;
		}

		if (!word.empty()) {
			const Common::String candidate = line.empty() ? word : line + " " + word;
			if (!line.empty() && measureText(candidate) > maximumWidth) {
				lines.push_back(line);
				line = word;
			} else {
				line = candidate;
			}
			word.clear();
		}
		if (character == '\n') {
			lines.push_back(line);
			line.clear();
		}
	}
	if (lines.empty())
		lines.push_back(Common::String());
}

bool WacManager::drawJournalTextPanel(
		const Common::Array<Common::String> &lines, uint progress,
		uint firstVisible, uint &maximumFirstVisible, uint &visibleRows) {
	const Common::Rect bounds(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacTextPanelWidth, kWacMediaTop + kWacTextPanelHeight);
	if (!_engine->getModalDialog()->drawRetainedTextPanelLines(lines, bounds,
			firstVisible, maximumFirstVisible, visibleRows,
			ModalDialogManager::kWacPresentation,
			static_cast<ModalDialogManager::TextPanelScrollControl>(
				_databaseTextScrollControl)))
		return false;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	for (int y = bounds.bottom; y < kWacMediaTop + kWacMediaHeight; ++y)
		memset(screen->getBasePtr(kWacMediaLeft, y), kWacDatabaseBackground,
			kWacMediaWidth);
	const Common::String progressText = Common::String::format("%s %u%%",
		resourceString(kWacJournalProgressResource).c_str(), progress);
	drawText((byte *)screen->getPixels(), screen->pitch,
		kWacJournalProgressLeft, kWacJournalProgressTop, progressText,
		kWacDatabaseNormalText);
	g_system->unlockScreen();
	if (_databaseSkin.size() >= kWacDatabaseSkinFrameCount)
		drawBitmap(_databaseSkin[_databaseCornerAlternate ? 15 : 0],
			bounds.left, bounds.top);
	_engine->getCursor()->refresh();
	g_system->updateScreen();
	return true;
}

bool WacManager::drawJournalTextPanelLine(
		const Common::Array<Common::String> &lines,
		uint firstVisible, uint visibleRows, uint lineIndex) {
	const Common::Rect bounds(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacTextPanelWidth, kWacMediaTop + kWacTextPanelHeight);
	return _engine->getModalDialog()->drawRetainedTextPanelLine(lines, bounds,
		firstVisible, visibleRows, lineIndex,
		ModalDialogManager::kWacPresentation);
}

uint16 WacManager::runJournalRevealScene(DatabaseEntry &entry) {
	// RunWacJournalRevealScene at 0x24261 owns control 0x7b2. It wraps resource
	// 0xaf once, scrambles those row buffers in place, and restores one original
	// row after the current section's 36-tick delay while flags 0xfa+n permit it.
	const Common::String &journalText = resourceString(kWacJournalTextResource);
	const Common::String &prompt = resourceString(kWacJournalPromptResource);
	const uint puzzleLevel = CLIP<uint>(_engine->getSettings()->getPuzzleLevel(), 1, 3);
	const uint passwordResource = kWacJournalPasswordBaseResource + puzzleLevel;
	const Common::String &expectedPassword = resourceString(passwordResource);
	if (journalText.empty() || prompt.empty() || expectedPassword.empty()) {
		warning("Ripper: WAC journal resources are incomplete text=0x%x prompt=0x%x password=0x%x",
			kWacJournalTextResource, kWacJournalPromptResource, passwordResource);
		return kNoAction;
	}

	Common::Array<Common::String> sourceLines;
	// The tertiary WAC text panel reserves five pixels on the left, twenty on
	// the right, and two more on either side of the actual text rows.
	wrapJournalText(journalText,
		kWacTextPanelWidth - kWacDatabaseLeftInset -
			kWacDatabaseRightInset - kWacDatabaseTextInset * 2,
		sourceLines);
	Common::Array<Common::String> displayLines = sourceLines;
	Milestones *milestones = _engine->getMilestones();
	milestones->set(kWacJournalRevealFlagBase, true, "wac-journal-entry");

	uint revealFlagIndex = 0;
	uint revealedLines = 0;
	bool unlocked = milestones->isSet(kWacJournalUnlockedFlag);
	if (unlocked) {
		while (revealedLines < sourceLines.size() &&
				revealFlagIndex < kWacJournalSectionCount &&
				milestones->isSet(kWacJournalRevealFlagBase + revealFlagIndex)) {
			milestones->set(kWacJournalCompletionFlagBase + revealFlagIndex,
				true, "wac-journal-resume");
			++revealedLines;
			if (revealedLines < sourceLines.size() &&
					sourceLines[revealedLines].empty())
				++revealFlagIndex;
		}
	}
	bool revealComplete = revealedLines >= sourceLines.size();

	Common::RandomSource randomSource("ripper-wac-journal");
	if (!revealComplete) {
		for (uint lineIndex = revealedLines; lineIndex < displayLines.size(); ++lineIndex)
			shuffleJournalLine(displayLines[lineIndex], randomSource);
	}

	uint firstVisible = 0;
	uint maximumFirstVisible = 0;
	uint visibleRows = 0;
	_databaseTextScrollControl = ModalDialogManager::kTextPanelScrollNone;
	_databaseTextScrollDragging = false;
	_databaseTextScrollDragOffset = 0;
	uint progress = sourceLines.empty() ? 100 :
		revealedLines * 100 / sourceLines.size();
	if (!drawJournalTextPanel(displayLines, progress, firstVisible,
			maximumFirstVisible, visibleRows))
		return kNoAction;

	_engine->getInput()->discardMouseTransitions();
	debugC(1, kDebugWac,
		"Ripper: entered WAC journal scene entry=%u label='%s' function=RunWacJournalRevealScene@0x24261 lines=%u unlocked=%d revealed=%u section=%u passwordResource=0x%x",
		entry.originalIndex, entry.label.c_str(), sourceLines.size(), unlocked,
		revealedLines, revealFlagIndex, passwordResource);
	debugC(2, kDebugWac,
		"Ripper: WAC journal chooser geometry panel=%d,%d,%d,%d progress=%d,%d password=%d,%d,%d,%d source=RunWacJournalRevealScene@0x24538",
		kWacMediaLeft, kWacMediaTop, kWacTextPanelWidth, kWacTextPanelHeight,
		kWacJournalProgressLeft, kWacJournalProgressTop,
		kWacMediaLeft, kWacJournalTextEntryTop,
		kWacJournalTextEntryWidth, kWacJournalTextEntryHeight);

	bool cancelled = false;
	while (!unlocked && !_engine->shouldQuit()) {
		// RunWacJournalRevealScene at 0x24538 places the password control five
		// pixels below the journal panel. Its -1 size request produces one
		// 14-pixel row plus the WAC heading, bottom, and frame insets.
		const Common::Rect entryBounds(kWacMediaLeft, kWacJournalTextEntryTop,
			kWacMediaLeft + kWacJournalTextEntryWidth,
			kWacJournalTextEntryTop + kWacJournalTextEntryHeight);
		if (!_engine->getModalDialog()->beginTextEntry(prompt,
				kWacJournalPasswordMaximumLength, kWacJournalHelpResource,
				"wac-journal-password", ModalDialogManager::kWacPresentation,
				entryBounds)) {
			cancelled = true;
			break;
		}

		Common::String enteredPassword;
		ModalDialogManager::TextEntryResult textResult =
			ModalDialogManager::kTextEntryPending;
		while (textResult == ModalDialogManager::kTextEntryPending &&
				!_engine->shouldQuit()) {
			textResult = _engine->getModalDialog()->serviceTextEntry(enteredPassword);
			serviceIdleWindowAnimations();
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
		if (textResult == ModalDialogManager::kTextEntryCancelled ||
				textResult == ModalDialogManager::kTextEntryFailed) {
			cancelled = true;
			break;
		}
		if (!wacJournalAnswersMatch(enteredPassword, expectedPassword)) {
			debugC(2, kDebugWac,
				"Ripper: WAC journal rejected decryption key resource=0x%x enteredLength=%u",
				passwordResource, enteredPassword.size());
			continue;
		}

		milestones->set(kWacJournalUnlockedFlag, true, "wac-journal-password");
		unlocked = true;
		revealedLines = 0;
		revealFlagIndex = 0;
		revealComplete = false;
		progress = 0;
		debugC(1, kDebugWac,
			"Ripper: WAC journal accepted decryption key resource=0x%x flag=0x%x",
			passwordResource, kWacJournalUnlockedFlag);
	}

	Audio::SoundHandle journalAudio;
	bool audioStarted = false;
	if (unlocked && !revealComplete && !cancelled) {
		audioStarted = _engine->getMedia()->playVoiceClip("wacjrnl.wav", journalAudio);
		if (!drawJournalTextPanel(displayLines, progress, firstVisible,
				maximumFirstVisible, visibleRows))
			cancelled = true;
	}

	uint16 result = cancelled ? 0x1b : kNoAction;
	uint32 revealStartMillis = g_system->getMillis(true);
	uint32 lastShuffleMillis = revealStartMillis;
	uint visibleShuffleLine = MAX<uint>(revealedLines, firstVisible);
	while (!cancelled && !_engine->shouldQuit()) {
		const uint16 command = serviceDatabaseMediaInput(entry.originalIndex,
			&firstVisible, maximumFirstVisible, visibleRows);
		bool redraw = false;
		int redrawLine = -1;
		if (command == kWacDatabaseTextScrolled ||
				command == MediaSequenceCallback::kContinueRefreshPalette) {
			visibleShuffleLine = MAX<uint>(revealedLines, firstVisible);
			redraw = true;
		} else if (command == kWacDatabaseSelectionChanged ||
				command == kExitAction || command == 0x1b) {
			result = command;
			break;
		}

		const uint32 now = g_system->getMillis(true);
		if (unlocked && !revealComplete &&
				now - revealStartMillis >= kWacJournalRevealInterval) {
			displayLines[revealedLines] = sourceLines[revealedLines];
			const bool sectionEnabled =
				revealFlagIndex < kWacJournalSectionCount &&
				milestones->isSet(kWacJournalRevealFlagBase + revealFlagIndex);
			if (sectionEnabled) {
				milestones->set(kWacJournalCompletionFlagBase + revealFlagIndex,
					true, "wac-journal-reveal");
				++revealedLines;
				if (revealedLines < sourceLines.size() &&
						sourceLines[revealedLines].empty())
					++revealFlagIndex;
			}
			revealComplete = revealedLines >= sourceLines.size();
			progress = sourceLines.empty() ? 100 :
				revealedLines * 100 / sourceLines.size();
			revealStartMillis = now;
			visibleShuffleLine = MAX<uint>(revealedLines, firstVisible);
			redraw = true;
			debugC(2, kDebugWac,
				"Ripper: WAC journal reveal line=%u/%u section=%u enabled=%d progress=%u complete=%d delayTicks=36",
				revealedLines, sourceLines.size(), revealFlagIndex,
				sectionEnabled, progress, revealComplete);
		}

		if (unlocked && !revealComplete &&
				now - lastShuffleMillis >= kWacJournalShuffleInterval) {
			const uint firstShuffleLine = MAX<uint>(revealedLines, firstVisible);
			const uint shuffleLimit = MIN<uint>(displayLines.size(),
				firstVisible + visibleRows);
			if (visibleShuffleLine < firstShuffleLine ||
					visibleShuffleLine >= shuffleLimit)
				visibleShuffleLine = firstShuffleLine;
			if (visibleShuffleLine < shuffleLimit) {
				const uint shuffledLine = visibleShuffleLine;
				shuffleJournalLine(displayLines[shuffledLine], randomSource);
				++visibleShuffleLine;
				redrawLine = shuffledLine;
			}
			lastShuffleMillis = now;
		}

		if (redraw && !drawJournalTextPanel(displayLines, progress, firstVisible,
				maximumFirstVisible, visibleRows))
			break;
		if (!redraw && redrawLine >= 0 &&
				!drawJournalTextPanelLine(displayLines, firstVisible,
					visibleRows, redrawLine))
			break;
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	if (audioStarted)
		_engine->getMedia()->stopSoundEffect(journalAudio);
	clearDatabaseMediaViewport();
	_databaseTextScrollControl = ModalDialogManager::kTextPanelScrollNone;
	_databaseTextScrollDragging = false;
	drawDatabase();
	debugC(1, kDebugWac,
		"Ripper: left WAC journal scene entry=%u result=0x%x unlocked=%d revealed=%u/%u section=%u audioStarted=%d",
		entry.originalIndex, result, unlocked, revealedLines, sourceLines.size(),
		revealFlagIndex, audioStarted);
	if (result == kWacDatabaseSelectionChanged &&
			_databaseSelection < _databaseEntries.size())
		return dispatchDatabaseEntry(_databaseEntries[_databaseSelection]);
	return result == kExitAction ? kExitAction : kNoAction;
}

uint16 WacManager::runDatabaseTextPanel(DatabaseEntry &entry, uint bodyResourceId) {
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

	_engine->getInput()->discardMouseTransitions();
	debugC(1, kDebugWac,
		"Ripper: entered WAC database text panel entry=%u label='%s' resource=0x%x bounds=%d,%d,%d,%d scrollLimit=%u",
		entry.originalIndex, entry.label.c_str(), bodyResourceId,
		bounds.left, bounds.top, bounds.width(), bounds.height(), maximumFirstVisible);
	uint16 result = kNoAction;
	while (!_engine->shouldQuit()) {
		const uint16 command = serviceDatabaseMediaInput(entry.originalIndex,
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
		entry.originalIndex, bodyResourceId, result, firstVisible);
	if (result == kWacDatabaseSelectionChanged &&
			_databaseSelection < _databaseEntries.size()) {
		debugC(2, kDebugWac,
			"Ripper: WAC database text panel switching entry=%u to entry=%u label='%s'",
			entry.originalIndex, _databaseEntries[_databaseSelection].originalIndex,
			_databaseEntries[_databaseSelection].label.c_str());
		return dispatchDatabaseEntry(_databaseEntries[_databaseSelection]);
	}
	return result == kExitAction ? kExitAction : kNoAction;
}

uint16 WacManager::dispatchDatabaseEntry(DatabaseEntry &entry) {
	// RunWacInventorySelectionLoop at 0x2252a clears the left media viewport
	// before dispatching every selected database row.
	clearDatabaseMediaViewport();
	if (entry.originalIndex == 1) {
		debugC(2, kDebugWac,
			"Ripper: WAC database retaining chooser while entering Broken Mug viewport=50,50,282,350");
		BrokenMugPuzzle::Result result;
		if (_engine->getMilestones()->isSet(kMilestoneCompletedMug)) {
			result = BrokenMugPuzzle::playCompletionMedia(_engine) ?
				BrokenMugPuzzle::kSolved : BrokenMugPuzzle::kLoadFailed;
		} else {
			BrokenMugPuzzle puzzle(_engine);
			result = puzzle.run();
		}
		debugC(1, kDebugWac,
			"Ripper: WAC database entry=1 label='%s' RunWacMugSelectionScene result=%d",
			entry.label.c_str(), result);
		if (result == BrokenMugPuzzle::kSolved) {
			// RunWacMugSelectionScene at 0x236b9 replaces the active row's text
			// pointer with resource 0xde after setting flags 0x47 and 0x48. The
			// visible row keeps original dispatch entry 1 for this chooser session.
			const Common::String &completedLabel =
				resourceString(kWacDatabaseTextBase + 2);
			debugC(1, kDebugWac,
				"Ripper: WAC database replaced completed cup row entry=1 oldLabel='%s' textId=0x%x newLabel='%s'",
				entry.label.c_str(), kWacDatabaseTextBase + 2, completedLabel.c_str());
			entry.label = completedLabel;
		}
		drawFrontEnd();
		drawDatabase();
		return result == BrokenMugPuzzle::kExitWac ? kExitAction : kNoAction;
	}
	if (entry.originalIndex == 2) {
		debugC(2, kDebugWac,
			"Ripper: WAC database retaining chooser while presenting completed Broken Mug");
		const bool played = BrokenMugPuzzle::playCompletionMedia(_engine);
		debugC(1, kDebugWac,
			"Ripper: WAC database entry=2 label='%s' PlayMugSelectionCompletionMedia success=%d",
			entry.label.c_str(), played);
		drawFrontEnd();
		drawDatabase();
		return kNoAction;
	}
	if (entry.originalIndex == 3)
		return runJournalRevealScene(entry);
	if (entry.originalIndex == 0 ||
			entry.originalIndex == 10 || entry.originalIndex == 11) {
		const Common::String path = Common::String::format(
			"wacinv%u.pcx", entry.originalIndex);
		const bool loaded = loadDatabaseStillImage(path);
		debugC(1, kDebugWac,
			"Ripper: WAC database entry=%u label='%s' stillImage='%s' loaded=%d",
			entry.originalIndex, entry.label.c_str(), path.c_str(), loaded);
		return kNoAction;
	}
	if (entry.originalIndex == 13) {
		// RunWacInventorySelectionLoop at 0x2252a case 0x0d resolves
		// WACINV13.SMK from INTERFAC.PL and enters
		// RunStaticMediaScreenWithOptionalVoiceover at 0x2339d without audio.
		// The 320x200 sequence is centered in the 350x282 WAC viewport and
		// loops from frame one while the persistent WAC controls stay active.
		WacDatabaseMediaCallback callback(this, entry.originalIndex);
		uint16 command = kNoAction;
		_engine->getInput()->discardMouseTransitions();
		const bool played = _engine->getMedia()->playWacInterfaceSequence(
			"wacinv13.smk", 65, 91, 1, &callback, &command);
		clearDatabaseMediaViewport();
		drawDatabase();
		debugC(1, kDebugWac,
			"Ripper: WAC database entry=13 label='%s' media='wacinv13.smk' position=65,91 loopStartFrame=1 played=%d command=0x%x",
			entry.label.c_str(), played, command);
		if (command == kExitAction)
			return kExitAction;
		if (command == kWacDatabaseSelectionChanged &&
				_databaseSelection < _databaseEntries.size()) {
			debugC(2, kDebugWac,
				"Ripper: WAC database media switching entry=13 to entry=%u label='%s'",
				_databaseEntries[_databaseSelection].originalIndex,
				_databaseEntries[_databaseSelection].label.c_str());
			return dispatchDatabaseEntry(_databaseEntries[_databaseSelection]);
		}
		return kNoAction;
	}
	if (entry.originalIndex == 15) {
		return runDatabaseTextPanel(entry, kWacCircuitManualResource);
	}
	debugC(1, kDebugWac,
		"Ripper: WAC database entry=%u label='%s' handler is stubbed",
		entry.originalIndex, entry.label.c_str());
	return kNoAction;
}

uint16 WacManager::runDatabase() {
	const Common::Rect bounds(kWacDatabaseLeft, kWacDatabaseTop,
		kWacDatabaseRight, kWacDatabaseBottom);
	buildDatabaseEntries();
	_databaseSelection = 0;
	_databaseFirstVisible = 0;
	_databaseStillImage = BitmapAssetFrame();
	_databaseStillImageScroll = 0;
	_databaseScrollControl = 0;
	_databaseCornerAlternate = true;
	_databaseCornerLastMillis = g_system->getMillis(true);
	_engine->getInput()->discardMouseTransitions();
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
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == 0x1b) {
				active = false;
			} else if (command == kExitAction || command == kDosF10Command) {
				returnAction = kExitAction;
				active = false;
			} else if (command == kHelpAction || command == kTextViewerAction) {
				returnAction = dispatchSubsceneAction(command,
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

		const MouseState mouse = _engine->getInput()->publishMouseState();
		serviceIdleWindowAnimations();
		serviceDatabaseCornerAnimation();
		const int scrollControl = findDatabaseScrollControl(mouse.position);
		if (scrollControl != _databaseScrollControl) {
			_databaseScrollControl = scrollControl;
			drawDatabaseScrollControls();
			debugC(2, kDebugWac,
				"Ripper: WAC database still-image scroll hover control=%d point=%d,%d",
				_databaseScrollControl, mouse.position.x, mouse.position.y);
		}
		const uint fallbackCursor = (bounds.contains(mouse.position) || scrollControl != 0) ?
			kWacControlCursor : kWacDefaultCursor;
		const uint16 controlAction = serviceFrontEndControls(mouse, fallbackCursor);
		if (controlAction != kNoAction) {
			returnAction = dispatchSubsceneAction(controlAction,
				kWacDatabaseHelpResource, true);
			if (returnAction == kExitAction)
				active = false;
		}
		if (!active)
			break;
		if ((mouse.released & kMouseButtonLeft) != 0) {
			if (scrollControl == 1)
				scrollDatabaseStillImage(-kWacMediaScrollStep);
			else if (scrollControl == 2)
				scrollDatabaseStillImage(kWacMediaScrollStep);
		}
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
						entryIndex, _databaseEntries[entryIndex].originalIndex,
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
	_idleWindowFrame[0] = 0;
	_idleWindowFrame[1] = 0;
	_idleWindowLastMillis = g_system->getMillis(true);
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
			} else if (command == kHelpAction || command == kTextViewerAction ||
					command == kDatabaseAction) {
				active = dispatchAction(command);
			}
		}

		const MouseState mouse = _engine->getInput()->publishMouseState();
		serviceIdleWindowAnimations();
		const uint16 controlAction = serviceFrontEndControls(mouse, kWacDefaultCursor);
		if (controlAction != kNoAction)
			active = dispatchAction(controlAction);
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	_engine->getCursor()->setVisible(false);
	restoreDisplay();
	debugC(1, kDebugWac, "Ripper: left WAC front-end loop");
}

} // End of namespace Ripper
