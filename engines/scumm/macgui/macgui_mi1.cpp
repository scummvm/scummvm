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

#include "common/system.h"
#include "common/config-manager.h"
#include "common/macresman.h"

#include "engines/engine.h"

#include "graphics/maccursor.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/surface.h"

#include "scumm/scumm.h"
#include "scumm/detection.h"
#include "scumm/macgui/macgui_impl.h"
#include "scumm/macgui/macgui_mi1.h"
#include "scumm/music.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"

namespace Scumm {

// ===========================================================================
// The Mac Monkey Island 1 GUI.
// ===========================================================================

MacMI1Gui::MacMI1Gui(ScummEngine *vm, const Common::Path &resourceFile) : MacGuiImpl(vm, resourceFile) {
	readStrings();
}

MacMI1Gui::~MacMI1Gui() {
}

void MacMI1Gui::readStrings() {
	Common::MacResManager resource;
	resource.open(_resourceFile);
	uint32 strsLen = resource.getResLength(MKTAG('S', 'T', 'R', 'S'), 0);

	if (strsLen <= 0)
		return;

	Common::SeekableReadStream *strsStream = resource.getResource(MKTAG('S', 'T', 'R', 'S'), 0);
	uint8 *strsBlock = (uint8 *)malloc(strsLen);
	strsStream->read(strsBlock, strsLen);

	uint8 *strsData = strsBlock;

	// Most of these are debug strings. We parse the entire STRS block anyway,
	// for any future need.

	// Debug strings
	for (int i = 0; i < 93; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	// "\x14", "About Monkey Island...<B;(-"
	for (int i = 0; i < 2; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// "Are you sure you want to quit?", "Are you sure you want to quit?",
	// "Are you sure you want to restart this game from the beginning?",
	// "Are you sure you want to quit?"
	for (int i = 0; i < 4; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	// "macPixHead is NULL", "Monkey Island"
	for (int i = 0; i < 2; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// "CopyBits error:"
	_strsStrings.emplace_back(readCString(strsData));

	// "Open Game File...", "Save Game File as..." "Game file"
	for (int i = 0; i < 3; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// "This disk is full.  The game was not saved."
	_strsStrings.emplace_back(readCString(strsData));

	// "An error occured while saving.  The game was not saved.  Please try saving the game to another disk."
	_strsStrings.emplace_back(readCString(strsData));

	// "At this size, you will not be able to fit the whole game on your screen."
	_strsStrings.emplace_back(readCString(strsData));

	// Debug strings
	for (int i = 0; i < 3; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	for (int i = 0; i < 2; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// All the "About" dialog strings
	for (int i = 0; i < 32; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// Remaining debug strings
	for (int i = 0; i < 75; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	free(strsBlock);
	delete strsStream;
}

const Graphics::Font *MacMI1Gui::getFontByScummId(int32 id) {
	switch (id) {
	case 0:
		return getFont(kLoomFontLarge);

	default:
		error("MacMI1Gui::getFontByScummId: Invalid font id %d", id);
	}
}

bool MacMI1Gui::getFontParams(FontId fontId, int &id, int &size, int &slant) const {
	switch (fontId) {
	case kAboutFontRegular:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;
	case kAboutFontBold:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontBold;
		return true;
	case kAboutFontHeaderSimple1:
		id = Graphics::kMacFontGeneva;
		size = 12;
		slant = Graphics::kMacFontBold | Graphics::kMacFontItalic | Graphics::kMacFontOutline;
		return true;
	case kAboutFontHeaderSimple2:
		id = Graphics::kMacFontChicago;
		size = 12;
		slant = Graphics::kMacFontBold | Graphics::kMacFontItalic | Graphics::kMacFontOutline;
		return true;
	default:
		return MacGuiImpl::getFontParams(fontId, id, size, slant);
	}
}

void MacMI1Gui::setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) {
	if (_vm->_game.id == GID_MONKEY) {
		_windowManager->replaceCursor(Graphics::MacGUIConstants::kMacCursorArrow);
	} else if (_vm->_game.version == 5) {
		Common::MacResManager resource;
		Graphics::MacCursor macCursor;

		resource.open(_resourceFile);

		Common::SeekableReadStream *curs = resource.getResource(MKTAG('C', 'U', 'R', 'S'), 128);

		if (macCursor.readFromStream(*curs)) {
			width = macCursor.getWidth();
			height = macCursor.getHeight();
			hotspotX = macCursor.getHotspotX();
			hotspotY = macCursor.getHotspotY();
			animate = 0;

			_windowManager->replaceCursor(Graphics::MacGUIConstants::kMacCursorCustom, &macCursor);
		}

		delete curs;
		resource.close();
	}
}

bool MacMI1Gui::handleMenu(int id, Common::String &name) {
	if (MacGuiImpl::handleMenu(id, name))
		return true;

	switch (id) {
	case 204:   // Fix color map
		break; // Do a no-op

	case 205:   // Options
		runOptionsDialog();
		break;

	case 206:   // Quit
		if (runQuitDialog())
			_vm->quitGame();
		break;

	default:
		warning("Unknown menu command: %d", id);
		break;
	}

	return false;
}

void MacMI1Gui::runAboutDialog() {
	switch (_vm->_game.id) {
	case GID_MONKEY:
		runAboutDialogMI1();
		break;
	case GID_MONKEY2:
		runAboutDialogMI2();
		break;
	case GID_INDY4:
		runAboutDialogIndy4();
		break;
	default:
		break;
	}
}

void MacMI1Gui::runAboutDialogMI1() {
	int width = 416;
	int height = 166;
	int x = (640 - width) / 2;
	int y = (400 - height) / 2;

	Common::Rect bounds(x, y, x + width, y + height);
	MacDialogWindow *window = createWindow(bounds);
//	Graphics::Surface *lucasArts = loadPict(5000);
//	Graphics::Surface *monkeys = loadPict(5001);

	Graphics::Surface *s = window->innerSurface();

	const TextLine page3[] = {
		{ 0, 68, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[114].c_str() }, // "PRESENTS"
		TEXT_END_MARKER
	};

	const TextLine page12[] = {
		{ 0, 0, kStyleHeaderSimple1, Graphics::kTextAlignLeft, _strsStrings[115].c_str() }, // "The Secret"
		{ 87, 13, kStyleBold, Graphics::kTextAlignLeft, _strsStrings[117].c_str() }, // "of"
		{ 40, 26, kStyleHeaderSimple1, Graphics::kTextAlignLeft, _strsStrings[116].c_str() }, // "Monkey Island(TM)"
		{ 178, 120, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[118].c_str() }, // "TM & (C) 1990 LucasArts Entertainment Company."
		{ 312, 133, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[119].c_str() }, // "All rights reserved."
		TEXT_END_MARKER
	};

	const TextLine page13[] = {
		{ 0, 47, kStyleRegular, Graphics::kTextAlignCenter, _strsStrings[120].c_str() }, // "Macintosh version by"
		{ 50, 62, kStyleHeaderSimple2, Graphics::kTextAlignLeft, _strsStrings[121].c_str() }, // "Eric Johnston"
		TEXT_END_MARKER
	};

	const TextLine page14[] = {
		{ 85, 32, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[122].c_str() }, // "Created by"
		{ 60, 47, kStyleHeaderSimple2, Graphics::kTextAlignLeft, _strsStrings[124].c_str() }, // "Ron Gilbert"
		{ 39, 70, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[123].c_str() }, // "Macintosh Version Produced by"
		{ 47, 85, kStyleHeaderSimple2, Graphics::kTextAlignLeft, _strsStrings[125].c_str() }, // "Brenna Holden"
		TEXT_END_MARKER
	};

	const TextLine page15[] = {
		{ 59, 27, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[126].c_str() }, // "SCUMM Story System"
		{ 85, 37, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[127].c_str() }, // "created by"
		{ 35, 57, kStyleHeaderSimple2, Graphics::kTextAlignLeft, _strsStrings[129].c_str() }, // "Ron Gilbert"
		{ 102, 72, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[128].c_str() }, // "and"
		{ 59, 87, kStyleHeaderSimple2, Graphics::kTextAlignLeft, _strsStrings[130].c_str() }, // "Aric Wilmunder"
		TEXT_END_MARKER
	};

	const TextLine page16[] = {
		{ 29, 37, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[131].c_str() }, // "Stumped? Hint books are available"
		{ 15, 55, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[134].c_str() }, // "In the U.S. call"
		{ 89, 55, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[132].c_str() }, // "1 (800) STAR-WARS"
		{ 89, 65, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[136].c_str() }, // "that's 1 (800)782-7927"
		{ 19, 85, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[135].c_str() }, // "In Canada call"
		{ 89, 85, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[133].c_str() }, // "1 (800) 828-7927"
		TEXT_END_MARKER
	};

	const TextLine page17[] = {
		{ 27, 32, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[137].c_str() }, // "Need a hint NOW?  Having problems?"
		{ 6, 47, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[140].c_str() }, // "For technical support call"
		{ 130, 47, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[138].c_str() }, // "1 (415) 721-3333"
		{ 62, 57, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[141].c_str() }, // "For hints call"
		{ 130, 57, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[139].c_str() }, // "1 (900) 740-JEDI"
		{ 5, 72, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[142].c_str() }, // "The charge for the hint line is 75\xA2 per minute."
		{ 10, 82, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[143].c_str() }, // "(You must have your parents' permission to"
		{ 25, 92, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[144].c_str() }, // "call this number if you are under 18.)"
		TEXT_END_MARKER
	};

	struct AboutPage {
		const TextLine *text;
		int drawArea;
		uint32 delayMs;
	};

	AboutPage aboutPages[] = {
		{ nullptr, 0,  3600 },
		{ nullptr, 0,   100 },
		{ nullptr, 0,   100 },
		{ page3,   0,  2100 },
		{ nullptr, 0,   300 },
		{ nullptr, 0,   400 },
		{ nullptr, 0,   400 },
		{ nullptr, 0,   300 },
		{ nullptr, 0,   400 },
		{ nullptr, 0,   100 },
		{ nullptr, 0,   100 },
		{ nullptr, 0,   600 },
		{ page12,  1,  7000 },
		{ page13,  2,  4300 },
		{ page14,  2,  4200 },
		{ page15,  2,  4200 },
		{ page16,  2, 14000 },
		{ page17,  2,     0 }
	};

	Common::Rect drawAreas[] = {
		Common::Rect(2, 2, s->w - 2, s->h - 2),
		Common::Rect(0, 7, s->w, s->h - 2),
		Common::Rect(176, 10, 394, 144)
	};

	// We probably can't trust our ellipse drawing function to get pixel
	// perfect shadows, so at least for now we define our own. Note that
	// all of the shadows have an even height. The bottom half is just a
	// mirror of the top half.

	Common::Pair<int, int> shadow1[] = {
		Common::Pair<int, int>(0, 10)
	};

	Common::Pair<int, int> shadow2[] = {
		Common::Pair<int, int>(4, 21),
		Common::Pair<int, int>(0, 30)
	};

	Common::Pair<int, int> shadow3[] = {
		Common::Pair<int, int>(13, 34),
		Common::Pair<int, int>(4, 52),
		Common::Pair<int, int>(0, 60),
	};

	Common::Pair<int, int> shadow4[] = {
		Common::Pair<int, int>(25, 50),
		Common::Pair<int, int>(10, 80),
		Common::Pair<int, int>(3, 94),
		Common::Pair<int, int>(0, 100)
	};

	Common::Pair<int, int> shadow5[] = {
		Common::Pair<int, int>(41, 67),
		Common::Pair<int, int>(21, 108),
		Common::Pair<int, int>(10, 130),
		Common::Pair<int, int>(3, 144),
		Common::Pair<int, int>(0, 150)
	};

	bool allowMegaSkip = false;
	bool megaSkip = false;
	int page = 0;

	window->show();

	while (!_vm->shouldQuit() && page < ARRAYSIZE(aboutPages)) {
		Common::Rect &drawArea = drawAreas[aboutPages[page].drawArea];

		switch (page) {
		case 0:
			s->fillRect(drawArea, kBlack);
			break;
		case 1:
			window->fillPattern(drawArea, 0xD7D7, false, true);
			break;
		case 2:
			s->fillRect(drawArea, kWhite);
			break;
		case 4:
			allowMegaSkip = true;
			s->fillRect(drawArea, kWhite);
			drawShadow(s, 77, 141, 2 * ARRAYSIZE(shadow1), shadow1);
			break;
		case 5:
			drawShadow(s, 67, 140, 2 * ARRAYSIZE(shadow2), shadow2);
			break;
		case 6:
			drawShadow(s, 52, 139, 2 * ARRAYSIZE(shadow3), shadow3);
			break;
		case 7:
			drawShadow(s, 32, 138, 2 * ARRAYSIZE(shadow4), shadow4);
			break;
		case 8:
			drawShadow(s, 7, 137, 2 * ARRAYSIZE(shadow5), shadow5);
			break;
		case 9:
			// Monkeys
			break;
		case 10:
			// Monkeys
			break;
		case 11:
			// Monkeys (shadow still barely visible)
			allowMegaSkip = megaSkip = false;
			break;
		case 13:
			s->fillRect(Common::Rect(178, 127, s->w, s->h), kWhite);
			window->markRectAsDirty(Common::Rect(178, 127, s->w, s->h));
			break;
		default:
			break;
		}

		if (aboutPages[page].text) {
			if (aboutPages[page].drawArea == 2) {
				window->drawTextBox(drawArea, aboutPages[page].text);
			} else {
				s->fillRect(drawArea, kWhite);
				window->drawTexts(drawArea, aboutPages[page].text);
			}
		}

		if (aboutPages[page].drawArea != 2)
			window->markRectAsDirty(drawArea);

		window->update();

		uint32 ms = megaSkip ? 100 : aboutPages[page].delayMs;

		if (delay(ms) == kDelayInterrupted && allowMegaSkip)
			megaSkip = true;

		page++;
	}

//	delete lucasArts;
//	delete monkeys;
	delete window;
}

void MacMI1Gui::drawShadow(Graphics::Surface *s, int x, int y, int h, Common::Pair<int, int> *drawData) {
	int y1 = y;
	int y2 = y + h - 1;

	for (int i = 0; i < h / 2; i++) {
		int x1 = x + drawData[i].first;
		for (int j = 0; j < drawData[i].second; j++) {
			if ((x1 + y1) & 1) {
				s->setPixel(x1, y1, kWhite);
				s->setPixel(x1, y2, kBlack);
			} else {
				s->setPixel(x1, y1, kBlack);
				s->setPixel(x1, y2, kWhite);
			}
			x1++;
		}
		y1++;
		y2--;
	}
}

void MacMI1Gui::runAboutDialogMI2() {
}

void MacMI1Gui::runAboutDialogIndy4() {
}

// A standard file picker dialog doesn't really make sense in ScummVM, so we
// make something that just looks similar to one.

bool MacMI1Gui::runOpenDialog(int &saveSlotToHandle) {
	Common::Rect bounds(88, 28, 448, 210);

	MacDialogWindow *window = createWindow(bounds);

	window->addButton(Common::Rect(254, 137, 334, 157), "Open", true);
	window->addButton(Common::Rect(254, 106, 334, 126), "Cancel", true);
	window->addButton(Common::Rect(254, 62, 334, 82), "Desktop", false);
	window->addButton(Common::Rect(254, 34, 334, 54), "Eject", false);

	window->drawDottedHLine(253, 93, 334);

	bool availSlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, availSlots, slotIds, ARRAYSIZE(availSlots));

	drawFakePathList(window, Common::Rect(14, 8, 232, 26), _folderIcon, "Monkey Island", Graphics::kTextAlignLeft);
	drawFakeDriveLabel(window, Common::Rect(242, 10, 340, 28), _hardDriveIcon, "ScummVM", Graphics::kTextAlignLeft);

	window->addListBox(Common::Rect(14, 31, 232, 161), savegameNames, true);

	window->setDefaultWidget(0);

	// When quitting, the default action is to not open a saved game
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0 || clicked == 6) {
			saveSlotToHandle =
				window->getWidgetValue(6) < ARRAYSIZE(slotIds) ?
				slotIds[window->getWidgetValue(6)] : -1;
			ret = true;
			break;
		}

		if (clicked == 1)
			break;
	}

	delete window;
	return ret;
}

bool MacMI1Gui::runSaveDialog(int &saveSlotToHandle, Common::String &name) {
	Common::Rect bounds(110, 27, 470, 231);

	MacDialogWindow *window = createWindow(bounds);

	window->addButton(Common::Rect(254, 163, 334, 183), "Save", true);
	window->addButton(Common::Rect(254, 132, 334, 152), "Cancel", true);
	window->addButton(Common::Rect(254, 90, 334, 110), "New", false);
	window->addButton(Common::Rect(254, 62, 334, 82), "Desktop", false);
	window->addButton(Common::Rect(254, 34, 334, 54), "Eject", false);

	bool busySlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, busySlots, slotIds, ARRAYSIZE(busySlots));

	Common::String saveGameFileAsResStr = _strsStrings[103].c_str();
	Common::String gameFileResStr = _strsStrings[104].c_str();

	int firstAvailableSlot = -1;
	for (int i = 1; i < ARRAYSIZE(busySlots); i++) { // Skip the autosave slot
		if (!busySlots[i]) {
			firstAvailableSlot = i;
			break;
		}
	}

	drawFakePathList(window, Common::Rect(14, 8, 232, 26), _folderIcon, "Monkey Island", Graphics::kTextAlignLeft);
	drawFakeDriveLabel(window, Common::Rect(242, 10, 340, 28), _hardDriveIcon, "ScummVM", Graphics::kTextAlignLeft);

	window->addListBox(Common::Rect(14, 31, 232, 129), savegameNames, true, true);

	MacGuiImpl::MacEditText *editText = window->addEditText(Common::Rect(16, 159, 229, 175), gameFileResStr, true);

	Graphics::Surface *s = window->innerSurface();
	const Graphics::Font *font = getFont(kSystemFont);

	s->frameRect(Common::Rect(14, 156, 232, 178), kBlack);

	window->drawDottedHLine(253, 121, 334);

	font->drawString(s, saveGameFileAsResStr, 14, 138, 218, kBlack, Graphics::kTextAlignLeft, 4);

	window->setDefaultWidget(0);
	editText->selectAll();

	// When quitting, the default action is to not open a saved game
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0) {
			ret = true;
			name = editText->getText();
			saveSlotToHandle = firstAvailableSlot;
			break;
		}

		if (clicked == 1)
			break;

		if (clicked == -2) {
			// Cycle through deferred actions
			for (uint i = 0; i < deferredActionsIds.size(); i++) {
				// Edit text widget
				if (deferredActionsIds[i] == 4) {
					MacGuiImpl::MacWidget *wid = window->getWidget(deferredActionsIds[i]);

					// Disable "Save" button when text is empty
					window->getWidget(0)->setEnabled(!wid->getText().empty());
				}
			}
		}
	}

	delete window;
	return ret;
}

bool MacMI1Gui::runOptionsDialog() {
	// Widgets:
	//
	// 0 - Okay button
	// 1 - Cancel button
	// 2 - Sound checkbox
	// 3 - Music checkbox
	// 4 - Picture (text speed background)
	// 5 - Picture (text speed handle)
	// 6 - Scrolling checkbox
	// 7 - Full Animation checkbox
	// 8 - Picture (music quality background)
	// 9 - Picture (music quality handle)
	// 10 - "Machine Speed:  ^0" text
	// 11 - Text speed slider (manually created)
	// 12 - Music quality slider (manually created)

	int sound = 1;
	int music = 1;
	if (_vm->VAR(167) == 2) {
		sound = music = 0;
	} else if (_vm->VAR(167) == 1) {
		music = 0;
	}

	int scrolling = _vm->_snapScroll == 0;
	int fullAnimation = _vm->VAR(_vm->VAR_MACHINE_SPEED) == 1 ? 0 : 1;
	int textSpeed = _vm->_defaultTextSpeed;
	int musicQuality = ConfMan.hasKey("mac_snd_quality") ? ConfMan.getInt("mac_snd_quality") : 0;
	int musicQualityOption = (musicQuality == 0) ? 1 : (musicQuality - 1) % 3;
	musicQuality = (musicQuality == 0) ? (_vm->VAR(_vm->VAR_SOUNDCARD) == 10 ? 0 : 2) : (musicQuality - 1) / 3;

	MacDialogWindow *window = createDialog(1000);

	window->setWidgetValue(2, sound);
	window->setWidgetValue(3, music);
	window->setWidgetValue(6, scrolling);
	window->setWidgetValue(7, fullAnimation);

	if (!sound)
		window->setWidgetEnabled(3, false);

	window->addPictureSlider(4, 5, true, 5, 105, 0, 9);
	window->setWidgetValue(11, textSpeed);

	window->addPictureSlider(8, 9, true, 5, 69, 0, 2, 6, 4);
	window->setWidgetValue(12, musicQualityOption);

	// Machine rating
	window->addSubstitution(Common::String::format("%d", _vm->VAR(53)));

	// When quitting, the default action is not to not apply options
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0) {
			ret = true;
			break;
		}

		if (clicked == 1)
			break;

		if (clicked == 2)
			window->setWidgetEnabled(3, window->getWidgetValue(2) != 0);
	}

	if (ret) {
		// Update settings

		// TEXT SPEED
		_vm->_defaultTextSpeed = CLIP<int>(window->getWidgetValue(11), 0, 9);
		ConfMan.setInt("original_gui_text_speed", _vm->_defaultTextSpeed);
		_vm->setTalkSpeed(_vm->_defaultTextSpeed);

		// SOUND&MUSIC ACTIVATION
		// 0 - Sound&Music on
		// 1 - Sound on, music off
		// 2 - Sound&Music off
		int musicVariableValue = 0;

		if (window->getWidgetValue(2) == 0) {
			musicVariableValue = 2;
		} else if (window->getWidgetValue(2) == 1 && window->getWidgetValue(3) == 0) {
			musicVariableValue = 1;
		}

		_vm->VAR(167) = musicVariableValue;

		if (musicVariableValue != 0) {
			if (_vm->VAR(169) != 0) {
				_vm->_sound->stopSound(_vm->VAR(169));
				_vm->VAR(169) = 0;
			}
		}

		// SCROLLING ACTIVATION
		_vm->_snapScroll = window->getWidgetValue(6) == 0;

		if (_vm->VAR_CAMERA_FAST_X != 0xFF)
			_vm->VAR(_vm->VAR_CAMERA_FAST_X) = _vm->_snapScroll;

		// FULL ANIMATION ACTIVATION
		_vm->VAR(_vm->VAR_MACHINE_SPEED) = window->getWidgetValue(7) == 1 ? 0 : 1;

		// MUSIC QUALITY SELECTOR
		musicQuality = musicQuality * 3 + 1 + window->getWidgetValue(12);
		_vm->_musicEngine->setQuality(musicQuality);
		ConfMan.setInt("mac_snd_quality", musicQuality);

		_vm->syncSoundSettings();
		ConfMan.flushToDisk();
	}

	delete window;
	return ret;
}

void MacMI1Gui::resetAfterLoad() {
	reset();
	_windowManager->replaceCursor(Graphics::MacGUIConstants::kMacCursorArrow);
}

bool MacMI1Gui::handleEvent(Common::Event event) {
	if (MacGuiImpl::handleEvent(event))
		return true;

	if (_vm->isPaused())
		return false;

	if (_vm->_userPut <= 0)
		return false;

	return false;
}

} // End of namespace Scumm
