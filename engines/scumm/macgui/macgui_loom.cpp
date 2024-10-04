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
#include "scumm/macgui/macgui_loom.h"
#include "scumm/music.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"

namespace Scumm {

// ===========================================================================
// The Mac Loom GUI. This one is pretty simple.
// ===========================================================================

MacLoomGui::MacLoomGui(ScummEngine *vm, const Common::Path &resourceFile) : MacGuiImpl(vm, resourceFile) {
	// The practice box can be moved, but this is its default position on
	// a large screen, and it's not saved.

	_practiceBoxPos = Common::Point(215, 376 + 2 * _vm->_screenDrawOffset);
	readStrings();
}

MacLoomGui::~MacLoomGui() {
	if (_practiceBox) {
		_practiceBox->free();
		delete _practiceBox;
	}
}

void MacLoomGui::readStrings() {
	Common::MacResManager resource;
	resource.open(_resourceFile);
	uint32 strsLen = resource.getResLength(MKTAG('S', 'T', 'R', 'S'), 0);
	Common::SeekableReadStream *strsStream = resource.getResource(MKTAG('S', 'T', 'R', 'S'), 0);
	uint8 *strsBlock = (uint8 *)malloc(strsLen);
	strsStream->read(strsBlock, strsLen);

	uint8 *strsData = strsBlock;

	// Most of these are debug strings. We parse the entire STRS block anyway,
	// for any future need.

	// Debug strings
	for (int i = 0; i < 6; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	_strsStrings.emplace_back(readPascalString(strsData));
	_strsStrings.emplace_back(readPascalString(strsData));

	_strsStrings.emplace_back(readCString(strsData));
	_strsStrings.emplace_back(readCString(strsData));

	// "\x14", "About Loom...<B;(-", "MacScumm", "MacScumm"
	for (int i = 0; i < 4; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// "Are you sure you want to restart this game from the beginning?"
	_strsStrings.emplace_back(readCString(strsData));

	// "Are you sure you want to quit?"
	_strsStrings.emplace_back(readCString(strsData));

	// "Open Game File...", "Save Game File as..." "Game file"
	for (int i = 0; i < 3; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// "This disk is full.  The game was not saved."
	_strsStrings.emplace_back(readCString(strsData));

	// "An error occured while saving.  The game was not saved.  Please try saving the game to another disk."
	_strsStrings.emplace_back(readCString(strsData));

	// "Select a color"
	_strsStrings.emplace_back(readPascalString(strsData));

	// Debug strings
	for (int i = 0; i < 67; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	// "About", "PRESENTS"
	for (int i = 0; i < 2; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// "%s Interpreter version %c.%c.%c"
	_strsStrings.emplace_back(readCString(strsData));

	// All the other "About" dialog strings
	for (int i = 0; i < 30; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// "ERROR #%d"
	_strsStrings.emplace_back(readCString(strsData));

	// Other debug strings...
	for (int i = 0; i < 4; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	for (int i = 0; i < 3; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	_strsStrings.emplace_back(readPascalString(strsData));
	_strsStrings.emplace_back(readPascalString(strsData));

	// "Copyright (c) 1989 Lucasfilm Ltd. All Rights Reserved.", "rb, "wb", "wb"
	for (int i = 0; i < 4; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	// Other debug strings...
	_strsStrings.emplace_back(readPascalString(strsData));

	_strsStrings.emplace_back(readCString(strsData));

	for (int i = 0; i < 5; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	for (int i = 0; i < 7; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	free(strsBlock);
	delete strsStream;
}

const Graphics::Font *MacLoomGui::getFontByScummId(int32 id) {
	switch (id) {
	case 0:
		return getFont(kLoomFontLarge);

	default:
		error("MacLoomGui::getFontByScummId: Invalid font id %d", id);
	}
}

bool MacLoomGui::getFontParams(FontId fontId, int &id, int &size, int &slant) const {
	if (MacGuiImpl::getFontParams(fontId, id, size, slant))
		return true;

	// Loom uses only font size 13 for in-game text, but size 12 is used
	// for system messages, e.g. the original pause dialog.
	//
	// Special characters:
	//
	// 16-23 are the note names c through c'.
	// 60 is an upside-down note, i.e. the one used for c'.
	// 95 is a used for the rest of the notes.

	switch (fontId) {
	case kLoomFontSmall:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;

	case kLoomFontMedium:
		id = _gameFontId;
		size = 12;
		slant = Graphics::kMacFontRegular;
		return true;

	case kLoomFontLarge:
		id = _gameFontId;
		size = 13;
		slant = Graphics::kMacFontRegular;
		return true;

	default:
		error("MacLoomGui: getFontParams: Unknown font id %d", (int)fontId);
	}

	return false;
}

void MacLoomGui::setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) {
	Common::MacResManager resource;
	Graphics::MacCursor macCursor;

	resource.open(_resourceFile);

	Common::SeekableReadStream *curs = resource.getResource(MKTAG('C', 'U', 'R', 'S'), 1000);

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

bool MacLoomGui::handleMenu(int id, Common::String &name) {
	if (MacGuiImpl::handleMenu(id, name))
		return true;

	switch (id) {
	case 101:	// Drafts inventory
		runDraftsInventory();
		break;

	case 204:	// Options
		runOptionsDialog();
		break;

	case 205:	// Quit
		if (runQuitDialog())
			_vm->quitGame();
		break;

	default:
		warning("Unknown menu command: %d", id);
		break;
	}

	return false;
}

void MacLoomGui::runAboutDialog() {
	// The About window is not a a dialog resource. Its size appears to be
	// hard-coded (416x166), and it's drawn centered. The graphics are in
	// PICT 5000 and 5001.

	int width = 416;
	int height = 166;
	int x = (640 - width) / 2;
	int y = (400 - height) / 2;

	Common::Rect bounds(x, y, x + width, y + height);
	MacDialogWindow *window = createWindow(bounds);
	Graphics::Surface *lucasFilm = loadPict(5000);
	Graphics::Surface *loom = loadPict(5001);

	const char *subVers = (const char *)_vm->getStringAddress(5);
	Common::String version = Common::String::format(_strsStrings[91].c_str(), subVers, '5', '1', '6');

	const TextLine page1[] = {
		{ 0, 23, kStyleExtraBold, Graphics::kTextAlignCenter, _strsStrings[90].c_str() }, // "PRESENTS"
		TEXT_END_MARKER
	};

	const TextLine page2[] = {
		{ 1, 59, kStyleRegular, Graphics::kTextAlignCenter, _strsStrings[92].c_str() }, // "TM & \xA9 1990 LucasArts Entertainment Company.  All rights reserved."
		{ 0, 70, kStyleRegular, Graphics::kTextAlignCenter, version.c_str() }, // "Release Version 1.2  25-JAN-91 Interpreter version 5.1.6"
		TEXT_END_MARKER
	};

	const TextLine page3[] = {
		{ 1, 11, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[93].c_str() }, // "Macintosh version by"
		{ 0, 25, kStyleHeader, Graphics::kTextAlignCenter, _strsStrings[95].c_str() }, // "Eric Johnston"
		{ 0, 49, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[94].c_str() }, // "Macintosh scripting by"
		{ 1, 63, kStyleHeader, Graphics::kTextAlignCenter, _strsStrings[96].c_str() }, // "Ron Baldwin"
		TEXT_END_MARKER
	};

	const TextLine page4[] = {
		{ 0, 26, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[97].c_str() }, // "Original game created by"
		{ 1, 40, kStyleHeader, Graphics::kTextAlignCenter, _strsStrings[98].c_str() }, // "Brian Moriarty"
		TEXT_END_MARKER
	};

	const TextLine page5[] = {
		{ 1, 11, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[99].c_str() }, // "Produced by"
		{ 0, 25, kStyleHeader, Graphics::kTextAlignCenter, _strsStrings[101].c_str() }, // "Gregory D. Hammond"
		{ 0, 49, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[100].c_str() }, // "Macintosh Version Produced by"
		{ 1, 63, kStyleHeader, Graphics::kTextAlignCenter, _strsStrings[102].c_str() }, // "David Fox"
		TEXT_END_MARKER
	};

	const TextLine page6[] = {
		{ 1, 6, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[103].c_str() }, // "SCUMM Story System"
		{ 1, 16, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[104].c_str() }, // "created by"
		{ 97, 35, kStyleHeader, Graphics::kTextAlignLeft, _strsStrings[106].c_str() }, // "Ron Gilbert"
		{ 1, 51, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[105].c_str() }, // "and"
		{ 122, 65, kStyleHeader, Graphics::kTextAlignLeft, _strsStrings[107].c_str() }, // "Aric Wilmunder"
		TEXT_END_MARKER
	};

	const TextLine page7[] = {
		{ 1, 16, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[108].c_str() }, // "Stumped?  Loom hint books are available!"
		{ 76, 33, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[111].c_str() }, // "In the U.S. call"
		{ 150, 34, kStyleBold, Graphics::kTextAlignLeft, _strsStrings[109].c_str() }, // "1 (800) STAR-WARS"
		{ 150, 43, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[113].c_str() }, // "that\xD5s  1 (800) 782-7927"
		{ 80, 63, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[112].c_str() }, // "In Canada call"
		{ 150, 64, kStyleBold, Graphics::kTextAlignLeft, _strsStrings[110].c_str() }, // "1 (800) 828-7927"
		TEXT_END_MARKER
	};

	const TextLine page8[] = {
		{ 1, 11, kStyleBold, Graphics::kTextAlignCenter, _strsStrings[114].c_str() }, // "Need a hint NOW?  Having problems?"
		{ 81, 25, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[117].c_str() }, // "For technical support call"
		{ 205, 26, kStyleBold, Graphics::kTextAlignLeft, _strsStrings[115].c_str() }, // "1 (415) 721-3333"
		{ 137, 35, kStyleRegular, Graphics::kTextAlignLeft, _strsStrings[118].c_str() }, // "For hints call"

		{ 205, 36, kStyleBold, Graphics::kTextAlignLeft, _strsStrings[116].c_str() }, // "1 (900) 740-JEDI"
		{ 1, 50, kStyleRegular, Graphics::kTextAlignCenter, _strsStrings[119].c_str() }, // "The charge for the hint line is 75\xA2 per minute."
		{ 1, 60, kStyleRegular, Graphics::kTextAlignCenter, _strsStrings[120].c_str() }, // "(You must have your parents\xD5 permission to"
		{ 1, 70, kStyleRegular, Graphics::kTextAlignCenter, _strsStrings[121].c_str() }, // "call this number if you are under 18.)"
		TEXT_END_MARKER
	};

	struct AboutPage {
		const TextLine *text;
		int waitFrames;
	};

	AboutPage aboutPages[] = {
		{ nullptr,  60 },	// ~3 seconds
		{ page1,    40 },	// ~2 seconds
		{ page2,   130 },	// ~6.5 seconds
		{ page3,    80 },	// ~4 seconds
		{ page4,    80 },
		{ page5,    80 },
		{ page6,    80 },
		{ page7,   260 },	// ~13 seconds
		{ page8,     0 }
	};

	int page = 0;

	// I've based the animation speed on what it looks like when Mini vMac
	// emulates an old black-and-white Mac at normal speed. It looks a bit
	// different in Basilisk II, but that's probably because it emulates a
	// much faster Mac.
	//
	// The animation is either either growing or shrinking, depending on
	// if growth is positive or negative. During each scene, the animation
	// may reach its smallest point, at which time it bounces back. When
	// it reaches its outer limit, the scene ends.

	window->show();

	int scene = 0;
	DelayStatus status = kDelayDone;

	Common::Rect r(0, 0, 404, 154);
	int growth = -2;
	int pattern;
	bool darkenOnly = false;
	int waitFrames = 0;

	int innerBounce = 72;
	int targetTop = 48;
	int targetGrowth = 2;

	bool changeScene = false;
	bool fastForward = false;

	while (!_vm->shouldQuit()) {
		if ((scene % 2) == 0) {
			// This appears to be pixel perfect or at least nearly
			// so for the outer layers, but breaks down slightly
			// near the middle.
			//
			// Also, the original does an inexplicable skip in the
			// first animation that I haven't bothered to
			// implement. I don't know if it was intentional or
			// not, but I think it looks awkward. And I wasn't able
			// to get it quite right anyway.

			pattern = (r.top / 2) % 8;

			if (pattern > 4)
				darkenOnly = false;

			Graphics::drawRoundRect(r, 7, pattern, true, darkenOnly ? MacDialogWindow::plotPatternDarkenOnly : MacDialogWindow::plotPattern, window);

			if (!fastForward)
				window->markRectAsDirty(r);

			if (r.top == targetTop && growth == targetGrowth) {
				changeScene = true;
			} else {
				r.grow(growth);

				if (growth < 0 && r.top >= innerBounce)
					growth = -growth;
			}
		} else {
			if (--waitFrames <= 0)
				changeScene = true;
		}

		if (!fastForward) {
			window->update();
			status = delay(50);
		}

		if (status == kDelayInterrupted)
			fastForward = true;

		if (status == kDelayAborted)
			break;

		if (changeScene) {
			changeScene = false;
			scene++;

			// Animations happen on even-numbered scenes. All
			// animations start in an inwards direction.
			//
			// Odd-numbered scenes are the text pages where it
			// waits for a bit before continuing. This is where
			// fast-forwarding (by clicking) stops. Unlike Last
			// Crusade, we can't just skip the animation because
			// everything has to be drawn. (Well, some could
			// probably be skipped, but I doubt it's worth the
			// trouble to do so.)

			if ((scene % 2) == 0)
				growth = -2;
			else {
				fastForward = false;
				darkenOnly = true;

				if (aboutPages[page].text)
					window->drawTexts(r, aboutPages[page].text);

				waitFrames = aboutPages[page].waitFrames;
				page++;
			}

			switch (scene) {
			case 1:
				window->drawSprite(lucasFilm, 134, 61);
				break;

			case 4:
				// All subsequent text pages are larger, which
				// we compensate by making the inner bounce
				// happen earlier.

				innerBounce -= 8;
				targetTop -= 16;
				break;

			case 5:
				window->drawSprite(loom, 95, 38);
				break;
			}

			window->update(true);

			if (scene >= 17)
				break;
		}
	}

	if (status != kDelayAborted)
		delay();

	_windowManager->popCursor();

	lucasFilm->free();
	loom->free();

	delete lucasFilm;
	delete loom;
	delete window;
}

void MacLoomGui::runDraftsInventory() {
	char notesBuf[6];
	const char *names[18] = {
		"Drafts",
		"Opening:", "Straw Into Gold:", "Dyeing:",
		"Night Vision:", "Twisting:", "Sleep:",
		"Emptying:", "Invisibility:", "Terror:",
		"Sharpening:", "Reflection:", "Healing:",
		"Silence:", "Shaping:", "Unmaking:",
		"Transcendence:",
		"Unknown:"
	};

	const char *notes = "cdefgabC";

	// ACT 1: Draw the Mac dialog window
	MacGuiImpl::MacDialogWindow *window = createWindow(Common::Rect(110, 20, 540, 252));
	const Graphics::Font *font = getFont(kSystemFont);

	Graphics::Surface *s = window->innerSurface();

	// ACT 2: Draw the drafts text
	//
	// Drafts are stored in SCUMM global variables; we choose the appropriate
	// first entry in the variables at which these drafts start.
	int base = 55;

	// TODO: Can these be drawn in different styles? (e.g. Checkerboard)
	Color unlockedColor = kBlack;
	Color inactiveColor = kBlack;
	Color newDraftColor = kBlack;

	for (int i = 0; i < 16; i++) {
		int draft = _vm->_scummVars[base + i * 2];

		// In which row are we rendering our text?
		int heightMultiplier = i < 8 ? i : (i % 8);
		int textHeight = 24;

		// Has the draft been unlocked by the player?
		//int titleColor = (draft & 0x2000) ? unlockedColor : inactiveColor;

		// Has the new draft been used at least once?
		int notesColor = (draft & 0x4000) ? unlockedColor : newDraftColor;

		// Has the draft been unlocked? Great: put it in our text buffer
		// otherwise just prepare to render the "????" string.
		if (draft & 0x2000) {
			Common::sprintf_s(notesBuf, sizeof(notesBuf), "%c%c%c%c",
							  notes[draft & 0x0007],
							  notes[(draft & 0x0038) >> 3],
							  notes[(draft & 0x01c0) >> 6],
							  notes[(draft & 0x0e00) >> 9]);
		} else {
			notesColor = inactiveColor;
			Common::sprintf_s(notesBuf, sizeof(notesBuf), "????");
		}

		// Where are we positioning the text?
		// Left column or right column?
		int xPos = i < 8 ? 40 : 260;

		// Draw the titles of the drafts...
		if (draft & 0x2000) {
			font->drawString(s, names[i + 1], xPos - 20, 24 + textHeight * heightMultiplier, s->w, notesColor); // FIXME: titleColor, not notesColor?
		} else {
			// Draw "Unknown:" as the title of the draft
			font->drawString(s, names[17], xPos - 20, 24 + textHeight * heightMultiplier, s->w, notesColor); // FIXME: titleColor, not notesColor?
		}

		// Draw the notes of the draft...
		font->drawString(s, notesBuf, xPos + 100, 24 + textHeight * heightMultiplier, s->w, notesColor);
	}

	// Draw "Drafts" on top of the dialog
	font->drawString(s, names[0], 0, 4, s->w, kBlack, Graphics::kTextAlignCenter);

	// Draw a vertical line to separate the two columns
	s->vLine(210, 44, 184, kBlack);

	// Update the screen with all the new stuff!
	window->show();
	delay();
	delete window;
}

// A standard file picker dialog doesn't really make sense in ScummVM, so we
// make something that just looks similar to one.

bool MacLoomGui::runOpenDialog(int &saveSlotToHandle) {
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

	drawFakePathList(window, Common::Rect(14, 8, 232, 26), _folderIcon, "Loom", Graphics::kTextAlignLeft);
	drawFakeDriveLabel(window, Common::Rect(242, 10, 340, 28), _hardDriveIcon, "ScummVM", Graphics::kTextAlignLeft);

	window->addListBox(Common::Rect(14, 31, 232, 161), savegameNames, true);

	window->setDefaultWidget(0);

	// When quitting, the default action is to not open a saved game
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0 || clicked == 3) {
			saveSlotToHandle =
				window->getWidgetValue(3) < ARRAYSIZE(slotIds) ?
				slotIds[window->getWidgetValue(3)] : -1;
			ret = true;
			break;
		}

		if (clicked == 1)
			break;
	}

	delete window;
	return ret;
}

bool MacLoomGui::runSaveDialog(int &saveSlotToHandle, Common::String &name) {
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

	Common::String saveGameFileAsResStr = _strsStrings[17].c_str();
	Common::String gameFileResStr = _strsStrings[18].c_str();

	int firstAvailableSlot = -1;
	for (int i = 1; i < ARRAYSIZE(busySlots); i++) { // Skip the autosave slot
		if (!busySlots[i]) {
			firstAvailableSlot = i;
			break;
		}
	}

	drawFakePathList(window, Common::Rect(14, 8, 232, 26), _folderIcon, "Loom", Graphics::kTextAlignLeft);
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

bool MacLoomGui::runOptionsDialog() {
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

void MacLoomGui::resetAfterLoad() {
	reset();

	// We used to use verb 53 for the Loom practice box, and while it's
	// still the verb we pretend to use when clicking on it we no longer
	// use the actual verb slot.
	//
	// Apparently the practice box isn't restored on saving, so it seems
	// that savegame compatibility isn't broken. And if it is, it happened
	// shortly after the savegame version was increased for other reasons,
	// so the damage would be very limited.

	for (int i = 0; i < _vm->_numVerbs; i++) {
		if (_vm->_verbs[i].verbid == 53)
			_vm->killVerb(i);
	}
}

void MacLoomGui::update(int delta) {
	// Unlike the PC version, the Macintosh version of Loom appears to
	// hard-code the drawing of the practice mode box. This is handled by
	// script 27 in both versions, but whereas the PC version draws the
	// notes, the Mac version just sets variables 50 and 54.
	//
	// In this script, the variables are set to the same value but it
	// appears that only variable 50 is cleared when the box is supposed to
	// disappear. I don't know what the purpose of variable 54 is.
	//
	// Variable 128 is the game difficulty:
	//
	// 0 - Practice
	// 1 - Standard
	// 2 - Expert
	//
	// Note that the practice mode box is never inscribed on the "Mac
	// screen" surface. It's drawn last on every update, so it floats
	// above everything else.

	int notes = _vm->VAR(50);

	if (_vm->VAR(128) == 0) {
		if (notes) {
			int w = 64;
			int h = 24;

			bool bw = (_vm->_renderMode == Common::kRenderMacintoshBW);

			if (!_practiceBox) {
				debug(1, "MacLoomGui: Creating practice mode box");

				_practiceBox = new Graphics::Surface();
				_practiceBox->create(w, h, Graphics::PixelFormat::createFormatCLUT8());

				_practiceBox->fillRect(Common::Rect(w, h), kBlack);

				Color color = bw ? kWhite : kLightGray;

				_practiceBox->hLine(2, 1, w - 3, color);
				_practiceBox->hLine(2, h - 2, w - 3, color);
				_practiceBox->vLine(1, 2, h - 3, color);
				_practiceBox->vLine(w - 2, 2, h - 3, color);
				_practiceBoxNotes = 0;
			}

			if (notes != _practiceBoxNotes) {
				debug(1, "MacLoomGui: Drawing practice mode notes");

				_practiceBoxNotes = notes;

				_practiceBox->fillRect(Common::Rect(2, 2, w - 2, h - 2), kBlack);

				const Graphics::Font *font = getFont(kLoomFontLarge);
				Color colors[] = { kRed, kBrightRed, kBrightYellow, kBrightGreen, kBrightCyan, kCyan, kBrightBlue, kWhite };

				for (int i = 0; i < 4; i++) {
					int note = (notes >> (4 * i)) & 0x0F;

					if (note >= 2 && note <= 9) {
						font->drawChar(_practiceBox, 14 + note, 9 + i * 13, 5, bw ? kWhite : colors[note - 2]);
					}
				}
			}

			_system->copyRectToScreen(_practiceBox->getBasePtr(0, 0), _practiceBox->pitch, _practiceBoxPos.x, _practiceBoxPos.y, w, h);
		} else {
			if (_practiceBox) {
				debug(1, "MacLoomGui: Deleting practice mode box");

				_system->copyRectToScreen(_surface->getBasePtr(_practiceBoxPos.x, _practiceBoxPos.y), _surface->pitch, _practiceBoxPos.x, _practiceBoxPos.y, _practiceBox->w, _practiceBox->h);

				_practiceBox->free();
				delete _practiceBox;
				_practiceBox = nullptr;
			}
		}
	}
}

bool MacLoomGui::handleEvent(Common::Event event) {
	if (MacGuiImpl::handleEvent(event))
		return true;

	if (_vm->isPaused())
		return false;

	if (!_practiceBox || _vm->_userPut <= 0)
		return false;

	// Perhaps the silliest feature in Mac Loom, that literally only one
	// person has ever asked for: You can drag the Loom practice box.
	//
	// The game will freeze while the button is held down, but that's how
	// the original acted as well. Should sounds keep playing? I don't know
	// if that situation can even occur. I think it's nicer to let them
	// play if it does.

	if (event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	Common::Rect bounds;

	bounds.left = _practiceBoxPos.x;
	bounds.top = _practiceBoxPos.y;
	bounds.right = _practiceBoxPos.x + _practiceBox->w;
	bounds.bottom = _practiceBoxPos.y + _practiceBox->h;

	if (!bounds.contains(event.mouse))
		return false;

	int clickX = event.mouse.x;
	int clickY = event.mouse.y;
	bool dragMode = false;

	while (!_vm->shouldQuit()) {
		bool dragging = false;
		int dragX = 0;
		int dragY = 0;

		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONUP:
				if (!dragMode)
					_vm->runInputScript(kVerbClickArea, 53, 1);
				return true;

			case Common::EVENT_MOUSEMOVE:
				if (ABS(event.mouse.x - clickX) >= 3 || ABS(event.mouse.y - clickY) >= 3)
					dragMode = true;

				if (dragMode) {
					dragging = true;
					dragX = event.mouse.x;
					dragY = event.mouse.y;
				}

				break;

			default:
				break;
			}
		}

		if (dragging) {
			// How much has the mouse moved since the initial
			// click? Calculate new position from that.

			int newX = bounds.left + (dragX - clickX);
			int newY = bounds.top + (dragY - clickY);

			// The box has to stay completely inside the screen.
			// Also, things get weird if you move the box into the
			// menu hotzone, so don't allow that.

			int maxY = _surface->h - _practiceBox->h - 2 * _vm->_screenDrawOffset;
			int minY = 2 * _vm->_screenDrawOffset;

			if (_vm->isUsingOriginalGUI() && minY < 23)
				minY = 23;

			newX = CLIP(newX, 0, _surface->w - _practiceBox->w);
			newY = CLIP(newY, minY, maxY);

			// For some reason, X coordinates can only change in
			// increments of 16 pixels. As an enhancement, we allow
			// any X coordinate.

			if (!_vm->enhancementEnabled(kEnhUIUX))
				newX &= ~0xF;

			if (newX != _practiceBoxPos.x || newY != _practiceBoxPos.y) {
				int w = _practiceBox->w;
				int h = _practiceBox->h;

				// The old and new rect will almost certainly
				// overlap, so it's possible to optimize this.
				// But increasing the delay in the event loop
				// was a better optimization than removing one
				// of the copyRectToScreen() calls completely,
				// so I doubt it's worth it.

				_system->copyRectToScreen(_surface->getBasePtr(_practiceBoxPos.x, _practiceBoxPos.y), _surface->pitch, _practiceBoxPos.x, _practiceBoxPos.y, w, h);
				_system->copyRectToScreen(_practiceBox->getBasePtr(0, 0), _practiceBox->pitch, newX, newY, w, h);

				_practiceBoxPos = Common::Point(newX, newY);
			}

			_system->delayMillis(20);
			_system->updateScreen();
		}
	}

	return false;
}

} // End of namespace Scumm
