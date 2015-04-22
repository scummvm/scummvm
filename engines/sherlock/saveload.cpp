/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/saveload.h"
#include "sherlock/graphics.h"
#include "sherlock/sherlock.h"
#include "common/system.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"

namespace Sherlock {

const int ENV_POINTS[6][3] = {
	{ 41, 80, 61 },		// Exit
	{ 81, 120, 101 },	// Load
	{ 121, 160, 141 },	// Save
	{ 161, 200, 181 },	// Up
	{ 201, 240, 221 },	// Down
	{ 241, 280, 261 }	// Quit
};

/*----------------------------------------------------------------*/

SaveManager::SaveManager(SherlockEngine *vm, const Common::String &target) : 
		_vm(vm), _target(target) {
	_saveThumb = nullptr;
	_envMode = SAVEMODE_NONE;
	_justLoaded = false;
	_savegameIndex = 0;
}

SaveManager::~SaveManager() {
	if (_saveThumb) {
		_saveThumb->free();
		delete _saveThumb;
	}
}

/**
 * Shows the in-game dialog interface for loading and saving games
 */
void SaveManager::drawInterface() {
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;

	// Create a list of savegame slots
	createSavegameList();

	screen._backBuffer1.fillRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, CONTROLS_Y + 10), BORDER_COLOR);
	screen._backBuffer1.fillRect(Common::Rect(0, CONTROLS_Y + 10, 2, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	screen._backBuffer1.fillRect(Common::Rect(318, CONTROLS_Y + 10, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	screen._backBuffer1.fillRect(Common::Rect(0, 199, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	screen._backBuffer1.fillRect(Common::Rect(2, CONTROLS_Y + 10, SHERLOCK_SCREEN_WIDTH - 2, SHERLOCK_SCREEN_HEIGHT - 2), INV_BACKGROUND);

	screen.makeButton(Common::Rect(ENV_POINTS[0][0], CONTROLS_Y, ENV_POINTS[0][1], CONTROLS_Y + 10), 
		ENV_POINTS[0][2] - screen.stringWidth("Exit") / 2, "Exit");
	screen.makeButton(Common::Rect(ENV_POINTS[1][0], CONTROLS_Y, ENV_POINTS[1][1], CONTROLS_Y + 10),
		ENV_POINTS[1][2] - screen.stringWidth("Load") / 2, "Load");
	screen.makeButton(Common::Rect(ENV_POINTS[2][0], CONTROLS_Y, ENV_POINTS[2][1], CONTROLS_Y + 10),
		ENV_POINTS[2][2] - screen.stringWidth("Save") / 2, "Save");
	screen.makeButton(Common::Rect(ENV_POINTS[3][0], CONTROLS_Y, ENV_POINTS[3][1], CONTROLS_Y + 10),
		ENV_POINTS[3][2] - screen.stringWidth("Up") / 2, "Up");
	screen.makeButton(Common::Rect(ENV_POINTS[4][0], CONTROLS_Y, ENV_POINTS[4][1], CONTROLS_Y + 10),
		ENV_POINTS[4][2] - screen.stringWidth("Down") / 2, "Down");
	screen.makeButton(Common::Rect(ENV_POINTS[5][0], CONTROLS_Y, ENV_POINTS[5][1], CONTROLS_Y + 10),
		ENV_POINTS[5][2] - screen.stringWidth("Quit") / 2, "Quit");

	if (!_savegameIndex)
		screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), COMMAND_NULL, 0, "Up");

	if (_savegameIndex == MAX_SAVEGAME_SLOTS - 5)
		screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), COMMAND_NULL, 0, "Down");

	for (int idx = _savegameIndex; idx < _savegameIndex + 5; ++idx)
	{
		screen.gPrint(Common::Point(6, CONTROLS_Y + 11 + (idx - _savegameIndex) * 10), 
			INV_FOREGROUND, "%d.", idx + 1);
		screen.gPrint(Common::Point(24, CONTROLS_Y + 11 + (idx - _savegameIndex) * 10), 
			INV_FOREGROUND, "%s", _savegames[idx].c_str());
	}

	if (!ui._windowStyle) {
		screen.slamRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
	} else {
		ui.summonWindow();
	}

	_envMode = SAVEMODE_NONE;
}

/**
 * Build up a savegame list, with empty slots given an explicit Empty message
 */
void SaveManager::createSavegameList() {
	Screen &screen = *_vm->_screen;

	_savegames.clear();
	for (int idx = 0; idx < MAX_SAVEGAME_SLOTS; ++idx)
		_savegames.push_back("-EMPTY-");

	SaveStateList saveList = getSavegameList(_target);
	for (uint idx = 0; idx < saveList.size(); ++idx)
		_savegames[saveList[idx].getSaveSlot()] = saveList[idx].getDescription();

	// Ensure the names will fit on the screen
	for (uint idx = 0; idx < _savegames.size(); ++idx) {
		int width = screen.stringWidth(_savegames[idx]) + 24;
		if (width > 308) {
			// It won't fit in, so remove characters until it does
			do {
				width -= screen.charWidth(_savegames[idx].lastChar());
				_savegames[idx].deleteLastChar();
			} while (width > 300);
		}
	}
}

/**
 * Load a list of savegames
 */
SaveStateList SaveManager::getSavegameList(const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0??", target.c_str());
	SherlockSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort to get the files in numerical order

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < MAX_SAVEGAME_SLOTS) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				readSavegameHeader(in, header);
				saveList.push_back(SaveStateDescriptor(slot, header._saveName));

				header._thumbnail->free();
				delete header._thumbnail;
				delete in;
			}
		}
	}

	return saveList;
}

const char *const SAVEGAME_STR = "SHLK";
#define SAVEGAME_STR_SIZE 4

/**
 * Read in the header information for a savegame
 */
bool SaveManager::readSavegameHeader(Common::InSaveFile *in, SherlockSavegameHeader &header) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];
	header._thumbnail = nullptr;

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header._version = in->readByte();
	if (header._version > SHERLOCK_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0') header._saveName += ch;

	// Get the thumbnail
	header._thumbnail = Graphics::loadThumbnail(*in);
	if (!header._thumbnail)
		return false;

	// Read in save date/time
	header._year = in->readSint16LE();
	header._month = in->readSint16LE();
	header._day = in->readSint16LE();
	header._hour = in->readSint16LE();
	header._minute = in->readSint16LE();
	header._totalFrames = in->readUint32LE();

	return true;
}

/**
 * Write out the header information for a savegame
 */
void SaveManager::writeSavegameHeader(Common::OutSaveFile *out, SherlockSavegameHeader &header) {
	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(SHERLOCK_SAVEGAME_VERSION);

	// Write savegame name
	out->write(header._saveName.c_str(), header._saveName.size());
	out->writeByte('\0');

	// Handle the thumbnail. If there's already one set by the game, create one
	if (!_saveThumb)
		createThumbnail();
	Graphics::saveThumbnail(*out, *_saveThumb);

	_saveThumb->free();
	delete _saveThumb;
	_saveThumb = nullptr;

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
	out->writeUint32LE(_vm->_events->getFrameCounter());
}

/**
 * Creates a thumbnail for the current on-screen contents
 */
void SaveManager::createThumbnail() {
	if (_saveThumb) {
		_saveThumb->free();
		delete _saveThumb;
	}

	uint8 thumbPalette[PALETTE_SIZE];
	_vm->_screen->getPalette(thumbPalette);
	_saveThumb = new Graphics::Surface();
	::createThumbnail(_saveThumb, (const byte *)_vm->_screen->getPixels(), SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT, thumbPalette);
}

/**
 * Return the index of the button the mouse is over, if any
 */
int SaveManager::getHighlightedButton() const {
	Common::Point pt = _vm->_events->mousePos();

	for (int idx = 0; idx < 6; ++idx) {
		if (pt.x > ENV_POINTS[idx][0] && pt.x < ENV_POINTS[idx][1] && pt.y > CONTROLS_Y
				&& pt.y < (CONTROLS_Y + 10))
			return idx;
	}

	return -1;
}

/**
 * Handle highlighting buttons
 */
void SaveManager::highlightButtons(int btnIndex) {
	Screen &screen = *_vm->_screen;
	byte color = (btnIndex == 0) ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND;

	screen.buttonPrint(Common::Point(ENV_POINTS[0][2], CONTROLS_Y), color, 1, "Exit");

	if ((btnIndex == 1) || ((_envMode == 1) && (btnIndex != 2)))
		screen.buttonPrint(Common::Point(ENV_POINTS[1][2], CONTROLS_Y), COMMAND_HIGHLIGHTED, true, "Load");
	else
		screen.buttonPrint(Common::Point(ENV_POINTS[1][2], CONTROLS_Y), COMMAND_FOREGROUND, true, "Load");

	if ((btnIndex == 2) || ((_envMode == 2) && (btnIndex != 1)))
		screen.buttonPrint(Common::Point(ENV_POINTS[2][2], CONTROLS_Y), COMMAND_HIGHLIGHTED, true, "Save");
	else
		screen.buttonPrint(Common::Point(ENV_POINTS[2][2], CONTROLS_Y), COMMAND_FOREGROUND, true, "Save");

	if (btnIndex == 3 && _savegameIndex)
		screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), COMMAND_HIGHLIGHTED, true, "Up");
	else
		if (_savegameIndex)
			screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), COMMAND_FOREGROUND, true, "Up");

	if ((btnIndex == 4) && (_savegameIndex < MAX_SAVEGAME_SLOTS - 5))
		screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), COMMAND_HIGHLIGHTED, true, "Down");
	else if (_savegameIndex < (MAX_SAVEGAME_SLOTS - 5))
		screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), COMMAND_FOREGROUND, true, "Down");

	color = (btnIndex == 5) ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND;
	screen.buttonPrint(Common::Point(ENV_POINTS[5][2], CONTROLS_Y), color, 1, "Quit");
}

/**
 * Load the game in the specified slot
 */
void SaveManager::loadGame(int slot) {
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(
		generateSaveName(slot));
	Common::Serializer s(saveFile, nullptr);

	// Load the savaegame header
	SherlockSavegameHeader header;
	if (!readSavegameHeader(saveFile, header))
		error("Invalid savegame");

	if (header._thumbnail) {
		header._thumbnail->free();
		delete header._thumbnail;
	}

	// Synchronize the savegame data
	synchronize(s);

	delete saveFile;
}

/**
 * Save the game in the specified slot with the given name
 */
void SaveManager::saveGame(int slot, const Common::String &name) {
	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(
		generateSaveName(slot));

	SherlockSavegameHeader header;
	header._saveName = name;
	writeSavegameHeader(out, header);

	Common::Serializer s(nullptr, out);
	synchronize(s);

	out->finalize();
	delete out;
}

/**
 * Support method that generates a savegame name
 * @param slot		Slot number
 */
Common::String SaveManager::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _target.c_str(), slot);
}

/**
 * Synchronize the data for a savegame
 */
void SaveManager::synchronize(Common::Serializer &s) {
	Journal &journal = *_vm->_journal;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;

	int oldFont = screen.fontNumber();

	journal.synchronize(s);
	people.synchronize(s);
	scene.synchronize(s);
	screen.synchronize(s);
	talk.synchronize(s);
	_vm->synchronize(s);

	if (screen.fontNumber() != oldFont)
		journal.resetPosition();
	/*
	char room_flags[MAX_ROOMS * 9];

		*/

	_vm->_loadingSavedGame = true;
	_justLoaded = true;
}

/**
 * Make sure that the selected savegame is on-screen
 */
bool SaveManager::checkGameOnScreen(int slot) {
	Screen &screen = *_vm->_screen;

	// Check if it's already on-screen
	if (slot != -1 && (slot < _savegameIndex || slot >= (_savegameIndex + 5))) {
		_savegameIndex = slot;

		screen._backBuffer1.fillRect(Common::Rect(3, CONTROLS_Y + 11, SHERLOCK_SCREEN_WIDTH - 2,
			SHERLOCK_SCREEN_HEIGHT - 1), INV_BACKGROUND);

		for (int idx = _savegameIndex; idx < (_savegameIndex + 5); ++idx) {
			screen.gPrint(Common::Point(6, CONTROLS_Y + 11 + (idx - _savegameIndex) * 10), 
				INV_FOREGROUND, "%d.", idx + 1);
			screen.gPrint(Common::Point(24, CONTROLS_Y + 11 + (idx - _savegameIndex) * 10),
				INV_FOREGROUND, "%s", _savegames[idx].c_str());
		}
		
		screen.slamRect(Common::Rect(3, CONTROLS_Y + 11, 318, SHERLOCK_SCREEN_HEIGHT));

		byte color = !_savegameIndex ? COMMAND_NULL : COMMAND_FOREGROUND;
		screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), color, 1, "Up");

		color = (_savegameIndex == (MAX_SAVEGAME_SLOTS - 5)) ? COMMAND_NULL : COMMAND_FOREGROUND;
		screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), color, 1, "Down");

		return true;
	}

	return false;
}

bool SaveManager::getFilename(int slot) {
	Events &events = *_vm->_events;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	int xp, yp;
	bool flag = false;
	
	screen.buttonPrint(Common::Point(ENV_POINTS[0][2], CONTROLS_Y), COMMAND_NULL, true, "Exit");
	screen.buttonPrint(Common::Point(ENV_POINTS[1][2], CONTROLS_Y), COMMAND_NULL, true, "Load");
	screen.buttonPrint(Common::Point(ENV_POINTS[2][2], CONTROLS_Y), COMMAND_NULL, true, "Save");
	screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), COMMAND_NULL, true, "Up");
	screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), COMMAND_NULL, true, "Down");
	screen.buttonPrint(Common::Point(ENV_POINTS[5][2], CONTROLS_Y), COMMAND_NULL, true, "Quit");

	Common::String saveName = _savegames[slot];
	if (scumm_stricmp(saveName.c_str(), "-EMPTY-") == 0) {
		// It's an empty slot, so start off with an empty save name
		saveName = "";

		yp = CONTROLS_Y + 12 + (slot - _savegameIndex) * 10;
		screen.vgaBar(Common::Rect(24, yp, 85, yp + 9), INV_BACKGROUND);
	}

	screen.print(Common::Point(6, CONTROLS_Y + 12 + (slot - _savegameIndex) * 10), TALK_FOREGROUND, "%d.", slot + 1);
	screen.print(Common::Point(24, CONTROLS_Y + 12 + (slot - _savegameIndex) * 10), TALK_FOREGROUND, "%s", saveName.c_str());
	xp = 24 + screen.stringWidth(saveName);
	yp = CONTROLS_Y + 12 + (slot - _savegameIndex) * 10;

	int done = 0;
	do {
		while (!_vm->shouldQuit() && !events.kbHit()) {
			scene.doBgAnim();

			if (talk._talkToAbort)
				return false;

			// Allow event processing
			events.pollEventsAndWait();
			events.setButtonState();

			flag = !flag;
			if (flag)
				screen.vgaBar(Common::Rect(xp, yp - 1, xp + 8, yp + 9), INV_FOREGROUND);
			else
				screen.vgaBar(Common::Rect(xp, yp - 1, xp + 8, yp + 9), INV_BACKGROUND);
		}
		if (_vm->shouldQuit())
			return false;

		// Get the next keypress
		Common::KeyState keyState = events.getKey();

		if (keyState.keycode == Common::KEYCODE_BACKSPACE && saveName.size() > 0) {
			// Delete character of save name
			screen.vgaBar(Common::Rect(xp - screen.charWidth(saveName.lastChar()), yp - 1, 
				xp + 8, yp + 9), INV_BACKGROUND);
			xp -= screen.charWidth(saveName.lastChar());
			screen.vgaBar(Common::Rect(xp, yp - 1, xp + 8, yp + 9), INV_FOREGROUND);
			saveName.deleteLastChar();
		}

		if (keyState.keycode == Common::KEYCODE_RETURN)
			done = 1;

		if (keyState.keycode == Common::KEYCODE_ESCAPE) {
			screen.vgaBar(Common::Rect(xp, yp - 1, xp + 8, yp + 9), INV_BACKGROUND);
			done = -1;
		}

		if (keyState.keycode >= ' ' && keyState.keycode <= 'z' && saveName.size() < 50
				&& (xp + screen.charWidth(keyState.keycode)) < 308) {
			screen.vgaBar(Common::Rect(xp, yp - 1, xp + 8, yp + 9), INV_BACKGROUND);
			screen.print(Common::Point(xp, yp), TALK_FOREGROUND, "%c", (char)keyState.keycode);
			xp += screen.charWidth((char)keyState.keycode);
			screen.vgaBar(Common::Rect(xp, yp - 1, xp + 8, yp + 9), INV_FOREGROUND);
			saveName += (char)keyState.keycode;
		}
	} while (!done);

	if (done == 1) {
		// Enter key perssed
		_savegames[slot] = saveName;
	} else {
		done = 0;
		_envMode = SAVEMODE_NONE;
		highlightButtons(-1);
	}

	return done == 1;
}

} // End of namespace Sherlock
