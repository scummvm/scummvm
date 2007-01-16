/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2003 Sarien Team
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
 * $URL$
 * $Id$
 *
 */

/*
 * Savegame support by Vasyl Tsvirkunov <vasyl@pacbell.net>
 * Multi-slots by Claudio Matsuoka <claudio@helllabs.org>
 */

#include "common/stdafx.h"
#include "common/file.h"

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/sprite.h"
#include "agi/text.h"
#include "agi/keyboard.h"
#include "agi/menu.h"

#define SAVEGAME_VERSION 2

/*
 * Version 0 (Sarien): view table has 64 entries
 * Version 1 (Sarien): view table has 256 entries (needed in KQ3)
 * Version 2 (ScummVM): first ScummVM version
 */

namespace Agi {

static const uint32 AGIflag=MKID_BE('AGI:');

int AgiEngine::saveGame(const char *fileName, const char *description) {
	char gameIDstring[8]="gameIDX";
	int i;
	struct ImageStackElement *ptr = _imageStack;
	Common::OutSaveFile *out;

	debugC(3, kDebugLevelMain | kDebugLevelSavegame, "AgiEngine::saveGame(%s, %s)", fileName, description);
	if (!(out = _saveFileMan->openForSaving(fileName))) {
		warning("Can't create file '%s', game not saved", fileName);
		return errBadFileOpen;
	} else {
		debugC(3, kDebugLevelMain | kDebugLevelSavegame, "Successfully opened %s for writing", fileName);
	}

	out->writeUint32BE(AGIflag);
	out->write(description, 31);

	out->writeByte(SAVEGAME_VERSION);
	debugC(5, kDebugLevelMain | kDebugLevelSavegame, "Writing save game version (%d)", SAVEGAME_VERSION);

	out->writeByte(_game.state);
	debugC(5, kDebugLevelMain | kDebugLevelSavegame, "Writing game state (%d)", _game.state);
	
	strcpy(gameIDstring, _game.id);
	out->write(gameIDstring, 8);
	debugC(5, kDebugLevelMain | kDebugLevelSavegame, "Writing game id (%s, %s)", gameIDstring, _game.id);

	for (i = 0; i < MAX_FLAGS; i++)
		out->writeByte(_game.flags[i]);
	for (i = 0; i < MAX_VARS; i++)
		out->writeByte(_game.vars[i]);

	out->writeSint16BE((int8)_game.horizon);
	out->writeSint16BE((int16)_game.lineStatus);
	out->writeSint16BE((int16)_game.lineUserInput);
	out->writeSint16BE((int16)_game.lineMinPrint);

	out->writeSint16BE((int16)_game.inputMode);
	out->writeSint16BE((int16)_game.lognum);

	out->writeSint16BE((int16)_game.playerControl);
	out->writeSint16BE((int16)_game.quitProgNow);
	out->writeSint16BE((int16)_game.statusLine);
	out->writeSint16BE((int16)_game.clockEnabled);
	out->writeSint16BE((int16)_game.exitAllLogics);
	out->writeSint16BE((int16)_game.pictureShown);
	out->writeSint16BE((int16)_game.hasPrompt);
	out->writeSint16BE((int16)_game.gameFlags);

	out->writeSint16BE((int16)_game.inputEnabled);

	for (i = 0; i < _HEIGHT; i++)
		out->writeByte(_game.priTable[i]);
	
	out->writeSint16BE((int16)_game.gfxMode);
	out->writeByte(_game.cursorChar);
	out->writeSint16BE((int16)_game.colorFg);
	out->writeSint16BE((int16)_game.colorBg);

	/* game.hires */
	/* game.sbuf */
	/* game.ego_words */
	/* game.num_ego_words */

	out->writeSint16BE((int16)_game.numObjects);
	for (i = 0; i < (int16)_game.numObjects; i++)
		out->writeSint16BE((int16)objectGetLocation(i));

	/* game.ev_keyp */
	for (i = 0; i < MAX_STRINGS; i++)
		out->write(_game.strings[i], MAX_STRINGLEN);

	/* record info about loaded resources */
	for (i = 0; i < MAX_DIRS; i++) {
		out->writeByte(_game.dirLogic[i].flags);
		out->writeSint16BE((int16)_game.logics[i].sIP);
		out->writeSint16BE((int16)_game.logics[i].cIP);
	}
	for (i = 0; i < MAX_DIRS; i++)
		out->writeByte(_game.dirPic[i].flags);
	for (i = 0; i < MAX_DIRS; i++)
		out->writeByte(_game.dirView[i].flags);
	for (i = 0; i < MAX_DIRS; i++)
		out->writeByte(_game.dirSound[i].flags);

	/* game.pictures */
	/* game.logics */
	/* game.views */
	/* game.sounds */

	for (i = 0; i < MAX_VIEWTABLE; i++) {
		VtEntry *v = &_game.viewTable[i];

		out->writeByte(v->stepTime);
		out->writeByte(v->stepTimeCount);
		out->writeByte(v->entry);
		out->writeSint16BE(v->xPos);
		out->writeSint16BE(v->yPos);
		out->writeByte(v->currentView);

		/* v->view_data */

		out->writeByte(v->currentLoop);
		out->writeByte(v->numLoops);

		/* v->loop_data */

		out->writeByte(v->currentCel);
		out->writeByte(v->numCels);

		/* v->cel_data */
		/* v->cel_data_2 */

		out->writeSint16BE(v->xPos2);
		out->writeSint16BE(v->yPos2);

		/* v->s */

		out->writeSint16BE(v->xSize);
		out->writeSint16BE(v->ySize);
		out->writeByte(v->stepSize);
		out->writeByte(v->cycleTime);
		out->writeByte(v->cycleTimeCount);
		out->writeByte(v->direction);

		out->writeByte(v->motion);
		out->writeByte(v->cycle);
		out->writeByte(v->priority);

		out->writeUint16BE(v->flags);

		out->writeByte(v->parm1);
		out->writeByte(v->parm2);
		out->writeByte(v->parm3);
		out->writeByte(v->parm4);
	}

	/* Save image stack */

	for (i = 0; i < _imageStackPointer; i++) {
		ptr = &_imageStack[i];
		out->writeByte(ptr->type);
		out->writeSint16BE(ptr->parm1);
		out->writeSint16BE(ptr->parm2);
		out->writeSint16BE(ptr->parm3);
		out->writeSint16BE(ptr->parm4);
		out->writeSint16BE(ptr->parm5);
		out->writeSint16BE(ptr->parm6);
		out->writeSint16BE(ptr->parm7);
	}
	out->writeByte(0);

	out->flush();
	if (out->ioFailed())
		warning("Can't write file '%s'. (Disk full?)", fileName);
	else
		debugC(1, kDebugLevelMain | kDebugLevelSavegame, "Saved game %s in file %s", description, fileName);
	
	delete out;
	debugC(3, kDebugLevelMain | kDebugLevelSavegame, "Closed %s", fileName);
	return errOK;
}

int AgiEngine::loadGame(const char *fileName) {
	char description[31], saveVersion, loadId[8];
	int i, vtEntries = MAX_VIEWTABLE;
	uint8 t;
	int16 parm[7];
	Common::InSaveFile *in;

	debugC(3, kDebugLevelMain | kDebugLevelSavegame, "AgiEngine::loadGame(%s)", fileName);

	if (!(in = _saveFileMan->openForLoading(fileName))) {
		warning("Can't open file '%s', game not loaded", fileName);
		return errBadFileOpen;
	} else {
		debugC(3, kDebugLevelMain | kDebugLevelSavegame, "Successfully opened %s for reading", fileName);
	}

	uint32 typea = in->readUint32BE();
	if (typea == AGIflag) {
		debugC(6, kDebugLevelMain | kDebugLevelSavegame, "Has AGI flag, good start");
	} else {
		warning("This doesn't appear to be an AGI savegame, game not restored");
		delete in;	
		return errOK;
	} 

	in->read(description, 31);
	
	debugC(6, kDebugLevelMain | kDebugLevelSavegame, "Description is: %s", description);
	
	saveVersion = in->readByte();
	if (saveVersion != SAVEGAME_VERSION)
		warning("Old save game version (%d, current version is %d). Will try and read anyway, but don't be surprised if bad things happen", saveVersion, SAVEGAME_VERSION);

	_game.state = in->readByte();
	
	in->read(loadId, 8);
	if (strcmp(loadId, _game.id)) {
		delete in;	
		warning("This save seems to be from a different AGI game (save from %s, running %s), not loaded", loadId, _game.id);
		return errBadFileOpen;
	}

	for (i = 0; i < MAX_FLAGS; i++)
		_game.flags[i] = in->readByte();
	for (i = 0; i < MAX_VARS; i++)
		_game.vars[i] = in->readByte();

	_game.horizon = in->readSint16BE();
	_game.lineStatus = in->readSint16BE();
	_game.lineUserInput = in->readSint16BE();
	_game.lineMinPrint = in->readSint16BE();
	
	/* These are never saved */
	_game.cursorPos = 0;
	_game.inputBuffer[0] = 0;
	_game.echoBuffer[0] = 0;
	_game.keypress = 0;

	_game.inputMode = in->readSint16BE();
	_game.lognum = in->readSint16BE();

	_game.playerControl = in->readSint16BE();
	_game.quitProgNow = in->readSint16BE();
	_game.statusLine = in->readSint16BE();
	_game.clockEnabled = in->readSint16BE();
	_game.exitAllLogics = in->readSint16BE();
	_game.pictureShown = in->readSint16BE();
	_game.hasPrompt = in->readSint16BE();
	_game.gameFlags = in->readSint16BE();
	_game.inputEnabled = in->readSint16BE();

	for (i = 0; i < _HEIGHT; i++)
		_game.priTable[i] = in->readByte();

	if (_game.hasWindow)
		closeWindow();

	_game.msgBoxTicks = 0;
	_game.block.active = false;
	/* game.window - fixed by close_window() */
	/* game.has_window - fixed by close_window() */

	_game.gfxMode = in->readSint16BE();
	_game.cursorChar = in->readByte();
	_game.colorFg = in->readSint16BE();
	_game.colorBg = in->readSint16BE();

	/* game.hires - rebuilt from image stack */
	/* game.sbuf - rebuilt from image stack */

	/* game.ego_words - fixed by clean_input */
	/* game.num_ego_words - fixed by clean_input */

	_game.numObjects = in->readSint16BE();
	for (i = 0; i < (int16)_game.numObjects; i++)
		objectSetLocation(i, in->readSint16BE());

	/* Those are not serialized */
	for (i = 0; i < MAX_DIRS; i++) {
		_game.evKeyp[i].occured = false;
	}

	for (i = 0; i < MAX_STRINGS; i++)
		in->read(_game.strings[i], MAX_STRINGLEN);

	for (i = 0; i < MAX_DIRS; i++) {
		if (in->readByte() & RES_LOADED)
			agiLoadResource(rLOGIC, i);
		else
			agiUnloadResource(rLOGIC, i);
		_game.logics[i].sIP = in->readSint16BE();
		_game.logics[i].cIP = in->readSint16BE();
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (in->readByte() & RES_LOADED)
			agiLoadResource(rPICTURE, i);
		else
			agiUnloadResource(rPICTURE, i);
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (in->readByte() & RES_LOADED)
			agiLoadResource(rVIEW, i);
		else
			agiUnloadResource(rVIEW, i);
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (in->readByte() & RES_LOADED)
			agiLoadResource(rSOUND, i);
		else
			agiUnloadResource(rSOUND, i);
	}

	/* game.pictures - loaded above */
	/* game.logics - loaded above */
	/* game.views - loaded above */
	/* game.sounds - loaded above */

	for (i = 0; i < vtEntries; i++) {
		struct VtEntry *v = &_game.viewTable[i];

		v->stepTime = in->readByte();
		v->stepTimeCount = in->readByte();
		v->entry = in->readByte();
		v->xPos = in->readSint16BE();
		v->yPos = in->readSint16BE();
		v->currentView = in->readByte();

		/* v->view_data - fixed below  */

		v->currentLoop = in->readByte();
		v->numLoops = in->readByte();

		/* v->loop_data - fixed below  */

		v->currentCel = in->readByte();
		v->numCels = in->readByte();

		/* v->cel_data - fixed below  */
		/* v->cel_data_2 - fixed below  */

		v->xPos2 = in->readSint16BE();
		v->yPos2 = in->readSint16BE();

		/* v->s - fixed below */

		v->xSize = in->readSint16BE();
		v->ySize = in->readSint16BE();
		v->stepSize = in->readByte();
		v->cycleTime = in->readByte();
		v->cycleTimeCount = in->readByte();
		v->direction = in->readByte();

		v->motion = in->readByte();
		v->cycle = in->readByte();
		v->priority = in->readByte();

		v->flags = in->readUint16BE();

		v->parm1 = in->readByte();
		v->parm2 = in->readByte();
		v->parm3 = in->readByte();
		v->parm4 = in->readByte();
	}
	for (i = vtEntries; i < MAX_VIEWTABLE; i++) {
		memset(&_game.viewTable[i], 0, sizeof(struct VtEntry));
	}

	/* Fix some pointers in viewtable */

	for (i = 0; i < MAX_VIEWTABLE; i++) {
		struct VtEntry *v = &_game.viewTable[i];

		if (_game.dirView[v->currentView].offset == _EMPTY)
			continue;

		if (!(_game.dirView[v->currentView].flags & RES_LOADED))
			agiLoadResource(rVIEW, v->currentView);

		setView(v, v->currentView);	/* Fix v->view_data */
		setLoop(v, v->currentLoop);	/* Fix v->loop_data */
		setCel(v, v->currentCel);	/* Fix v->cel_data */
		v->celData2 = v->celData;
		v->s = NULL;	/* not sure if it is used... */
	}

	_sprites->eraseBoth();

	/* Clear input line */
	_gfx->clearScreen(0);
	writeStatus();

	/* Recreate background from saved image stack */
	clearImageStack();
	while ((t = in->readByte()) != 0) {
		for (i = 0; i < 7; i++)
			parm[i] = in->readSint16BE();
		replayImageStackCall(t, parm[0], parm[1], parm[2],
				parm[3], parm[4], parm[5], parm[6]);
	}

	delete in;	
	debugC(3, kDebugLevelMain | kDebugLevelSavegame, "Closed %s", fileName);

	setflag(fRestoreJustRan, true);

	_game.hasPrompt = 0;	/* force input line repaint if necessary */
	cleanInput();

	_sprites->eraseBoth();
	_sprites->blitBoth();
	_sprites->commitBoth();
	_picture->showPic();
	_gfx->doUpdate();

	return errOK;
}

#define NUM_SLOTS 12

const char *AgiEngine::getSavegameFilename(int num) {
	static char saveLoadSlot[12];
	sprintf(saveLoadSlot, "%s.%.3d", _targetName.c_str(), num);
	return saveLoadSlot;
}

int AgiEngine::selectSlot() {
	int i, key, active = 0;
	int rc = -1;
	int hm = 2, vm = 3;	/* box margins */
	char desc[NUM_SLOTS][40];

	for (i = 0; i < NUM_SLOTS; i++) {
		char fileName[MAX_PATH];
		Common::InSaveFile *in;
		
		debugC(4, kDebugLevelMain | kDebugLevelSavegame, "Game id seems to be %s", _targetName.c_str());
		sprintf(fileName, "%s", getSavegameFilename(i));
		if (!(in = _saveFileMan->openForLoading(fileName))) {
			debugC(4, kDebugLevelMain | kDebugLevelSavegame, "File %s does not exist", fileName);
			strcpy(desc[i], "          (empty slot)");
		} else {
			debugC(4, kDebugLevelMain | kDebugLevelSavegame, "Successfully opened %s for reading", fileName);
			uint32 type = in->readUint32BE();
			if (type == AGIflag) {
				debugC(6, kDebugLevelMain | kDebugLevelSavegame, "Has AGI flag, good start");
				in->read(desc[i], 31);
			} else {
				warning("This doesn't appear to be an AGI savegame");
				strcpy(desc[i], "(corrupt file)");
			} 

			delete in;
		}
	}

	for (;;) {
		char dstr[64];
		for (i = 0; i < NUM_SLOTS; i++) {
			sprintf(dstr, "[%-32.32s]", desc[i]);
			printText(dstr, 0, hm + 1, vm + 4 + i,
					(40 - 2 * hm) - 1, i == active ? MSG_BOX_COLOUR : MSG_BOX_TEXT,
					i == active ? MSG_BOX_TEXT : MSG_BOX_COLOUR);

		}

		_gfx->pollTimer();	/* msdos driver -> does nothing */
		key = doPollKeyboard();
		switch (key) {
		case KEY_ENTER:
			rc = active;
			strncpy(_game.strings[MAX_STRINGS], desc[i], MAX_STRINGLEN);
			goto press;
		case KEY_ESCAPE:
			rc = -1;
			goto getout;
		case BUTTON_LEFT:
			break;
		case KEY_DOWN:
			active++;
			active %= NUM_SLOTS;
			break;
		case KEY_UP:
			active--;
			if (active < 0)
				active = NUM_SLOTS - 1;
			break;
		}
		_gfx->doUpdate();
	}

press:
	debugC(8, kDebugLevelMain | kDebugLevelInput, "Button pressed: %d", rc);

getout:
	closeWindow();
	return rc;
}

int AgiEngine::saveGameDialog() {
	char fileName[MAX_PATH];
	char *desc;
	const char *buttons[] = { "Do as I say!", "I regret", NULL };
	char dstr[200];
	int rc, slot = 0;
	int hm, vm, hp, vp;	
	int w;

	hm = 2;
	vm = 3;
	hp = hm * CHAR_COLS;
	vp = vm * CHAR_LINES;
	w = (40 - 2 * hm) - 1;

	sprintf(fileName, "%s", getSavegameFilename(slot));

	drawWindow(hp, vp, GFX_WIDTH - hp, GFX_HEIGHT - vp);
	printText("Select a slot in which you wish to save the game:",
			0, hm + 1, vm + 1, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);
	printText("Press ENTER to select, ESC cancels",
			0, hm + 1, vm + 17, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);

	slot = selectSlot();
	if (slot < 0)	
		return errOK;

	drawWindow(hp, vp + 5 * CHAR_LINES, GFX_WIDTH - hp,
			GFX_HEIGHT - vp - 9 * CHAR_LINES);
	printText("Enter a description for this game:",
			0, hm + 1, vm + 6, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);
	_gfx->drawRectangle(3 * CHAR_COLS, 11 * CHAR_LINES - 1,
			37 * CHAR_COLS, 12 * CHAR_LINES, MSG_BOX_TEXT);
	_gfx->flushBlock(3 * CHAR_COLS, 11 * CHAR_LINES - 1,
			37 * CHAR_COLS, 12 * CHAR_LINES);

	getString(2, 11, 33, MAX_STRINGS);
	_gfx->printCharacter(3, 11, _game.cursorChar, MSG_BOX_COLOUR, MSG_BOX_TEXT);
	do {
		mainCycle();
	} while (_game.inputMode == INPUT_GETSTRING);
	closeWindow();

	desc = _game.strings[MAX_STRINGS];
	sprintf(dstr, "Are you sure you want to save the game "
			"described as:\n\n%s\n\nin slot %d?\n\n\n", desc, slot);

	rc = selectionBox(dstr, buttons);

	if (rc != 0) {
		messageBox("Game NOT saved.");
		return errOK;
	}

	sprintf(fileName, "%s", getSavegameFilename(slot));
	debugC(8, kDebugLevelMain | kDebugLevelResources, "file is [%s]", fileName);

	saveGame(fileName, desc);

	messageBox("Game saved.");

	return errOK;
}

int AgiEngine::saveGameSimple() {
	char fileName[MAX_PATH];

	sprintf(fileName, "%s", getSavegameFilename(0));
	saveGame(fileName, "Default savegame");

	return errOK;
}

int AgiEngine::loadGameDialog() {
	char fileName[MAX_PATH];
	int rc, slot = 0;
	int hm, vm, hp, vp;	/* box margins */
	int w;

	hm = 2;
	vm = 3;
	hp = hm * CHAR_COLS;
	vp = vm * CHAR_LINES;
	w = (40 - 2 * hm) - 1;

	sprintf(fileName, "%s", getSavegameFilename(slot));

	_sprites->eraseBoth();
	_sound->stopSound();

	drawWindow(hp, vp, GFX_WIDTH - hp, GFX_HEIGHT - vp);
	printText("Select a game which you wish to\nrestore:",
			0, hm + 1, vm + 1, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);
	printText("Press ENTER to select, ESC cancels",
			0, hm + 1, vm + 17, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);

	slot = selectSlot();

	if (slot < 0) {
		messageBox("Game NOT restored.");
		return errOK;
	}

	sprintf(fileName, "%s", getSavegameFilename(slot));

	if ((rc = loadGame(fileName)) == errOK) {
		messageBox("Game restored.");
		_game.exitAllLogics = 1;
		_menu->enableAll();
	} else {
		messageBox("Error restoring game.");
	}

	return rc;
}

int AgiEngine::loadGameSimple() {
	char fileName[MAX_PATH];
	int rc = 0;

	sprintf(fileName, "%s", getSavegameFilename(0));

	_sprites->eraseBoth();
	_sound->stopSound();
	closeWindow();

	if ((rc = loadGame(fileName)) == errOK) {
		messageBox("Game restored.");
		_game.exitAllLogics = 1;
		_menu->enableAll();
	} else {
		messageBox("Error restoring game.");
	}

	return rc;
}

} // End of namespace Agi
