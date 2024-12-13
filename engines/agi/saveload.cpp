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

//
// Savegame support by Vasyl Tsvirkunov <vasyl@pacbell.net>
// Multi-slots by Claudio Matsuoka <claudio@helllabs.org>
//

#include "common/file.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "gui/saveload.h"

#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/text.h"
#include "agi/sprite.h"
#include "agi/keyboard.h"
#include "agi/menu.h"
#include "agi/systemui.h"
#include "agi/words.h"

#define SAVEGAME_CURRENT_VERSION 11

//
// Version 0 (Sarien):   view table has 64 entries
// Version 1 (Sarien):   view table has 256 entries (needed in KQ3)
// Version 2 (ScummVM):  first ScummVM version
// Version 3 (ScummVM):  added AGIPAL save/load support
// Version 4 (ScummVM):  added thumbnails and save creation date/time
// Version 5 (ScummVM):  Added game md5
// Version 6 (ScummVM):  Added game played time
// Version 7 (ScummVM):  Added controller key mappings
//                        required for some games for quick-loading from ScummVM main menu
//                        for games, that do not set all key mappings right at the start
//                       Added automatic save data (for command SetSimple)
// Version 8 (ScummVM):  Added Hold-Key-Mode boolean
//                        required for at least Mixed Up Mother Goose
//                        gets set at the start of the game only
// Version 9 (ScummVM):  Added seconds to saved game time stamp
// Version 10 (ScummVM): Added priorityTableSet boolean

namespace Agi {

static const uint32 AGIflag = MKTAG('A', 'G', 'I', ':');

int AgiEngine::saveGame(const Common::String &fileName, const Common::String &descriptionString) {
	char gameIDstring[8] = "gameIDX";
	int i;
	Common::OutSaveFile *out;
	int result = errOK;

	debugC(3, kDebugLevelSavegame, "AgiEngine::saveGame(%s, %s)", fileName.c_str(), descriptionString.c_str());
	if (!(out = _saveFileMan->openForSaving(fileName))) {
		warning("Can't create file '%s', game not saved", fileName.c_str());
		return errBadFileOpen;
	} else {
		debugC(3, kDebugLevelSavegame, "Successfully opened %s for writing", fileName.c_str());
	}

	out->writeUint32BE(AGIflag);

	const char *descriptionStringC;
	Common::U32String hebDesc;
	if (_game._vm->getLanguage() != Common::HE_ISR) {
		descriptionStringC = descriptionString.c_str();
	} else {
		hebDesc = descriptionString.substr(0, SAVEDGAME_DESCRIPTION_LEN / 2 - 3).decode(Common::kWindows1255);
		descriptionStringC = hebDesc.encode(Common::kUtf8).c_str();
	}

	// Write description of saved game, limited to SAVEDGAME_DESCRIPTION_LEN characters + terminating NUL
	char description[SAVEDGAME_DESCRIPTION_LEN + 1];

	memset(description, 0, sizeof(description));
	Common::strlcpy(description, descriptionStringC, SAVEDGAME_DESCRIPTION_LEN);
	assert(SAVEDGAME_DESCRIPTION_LEN + 1 == 31); // safety
	out->write(description, 31);

	out->writeByte(SAVEGAME_CURRENT_VERSION);
	debugC(5, kDebugLevelSavegame, "Writing save game version (%d)", SAVEGAME_CURRENT_VERSION);

	// Thumbnail
	Graphics::saveThumbnail(*out);

	// Creation date/time
	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = ((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF);
	uint32 playTime = g_engine->getTotalPlayTime() / 1000;

	out->writeUint32BE(saveDate);
	debugC(5, kDebugLevelSavegame, "Writing save date (%d)", saveDate);
	out->writeUint16BE(saveTime);
	debugC(5, kDebugLevelSavegame, "Writing save time (%d)", saveTime);
	// Version 9+: save seconds of current time as well
	out->writeByte(curTime.tm_sec & 0xFF);
	out->writeUint32BE(playTime);
	debugC(5, kDebugLevelSavegame, "Writing play time (%d)", playTime);

	out->writeByte(2); // was _game.state, 2 = STATE_RUNNING

	Common::strlcpy(gameIDstring, _game.id, 8);
	out->write(gameIDstring, 8);
	debugC(5, kDebugLevelSavegame, "Writing game id (%s, %s)", gameIDstring, _game.id);

	const char *tmp = getGameMD5();
	// As reported in bug report #4582 "AGI: Crash when saving fallback-matched game"
	// getGameMD5 will return NULL for fallback matched games. Since there is also no
	// filename available we can not compute any MD5 here either. Thus we will just set
	// the MD5 sum in the savegame to all zero, when getGameMD5 returns NULL.
	if (!tmp) {
		for (i = 0; i < 32; ++i)
			out->writeByte(0);
	} else {
		for (i = 0; i < 32; ++i)
			out->writeByte(tmp[i]);
	}

	// Version 7+: Save automatic saving state (set.simple opcode)
	out->writeByte(_game.automaticSave);
	out->write(_game.automaticSaveDescription, 31);

	// touch VM_VAR_SECONDS, so that it gets updated
	getVar(VM_VAR_SECONDS);

	for (i = 0; i < MAX_FLAGS; i++)
		out->writeByte(_game.flags[i]);
	for (i = 0; i < MAX_VARS; i++)
		out->writeByte(_game.vars[i]);

	out->writeSint16BE((int16)_game.horizon);
	out->writeSint16BE((int16)_text->statusRow_Get());
	out->writeSint16BE((int16)_text->promptRow_Get());
	out->writeSint16BE((int16)_text->getWindowRowMin());

	out->writeSint16BE(1); // was _game.inputMode, we set it to 1, which was INPUTMODE_NORMAL
	out->writeSint16BE((int16)_game.curLogicNr);

	out->writeSint16BE((int16)_game.playerControl);
	out->writeSint16BE((int16)shouldQuit());
	if (_text->statusEnabled()) {
		out->writeSint16BE(0x7FFF);
	} else {
		out->writeSint16BE(0);
	}
	out->writeSint16BE(1); // was clock enabled
	// (previous in-game-timer, in-game-timer is always enabled during the regular game, so need to save/load it)
	out->writeSint16BE((int16)_game.exitAllLogics);
	out->writeSint16BE((int16)_game.pictureShown);
	out->writeSint16BE((int16)_text->promptIsEnabled()); // was "_game.hasPrompt", no longer needed
	out->writeSint16BE(0);	// was _game.gameFlags, no longer needed

	if (_text->promptIsEnabled()) {
		out->writeSint16BE(0x7FFF);
	} else {
		out->writeSint16BE(0);
	}

	for (i = 0; i < SCRIPT_HEIGHT; i++)
		out->writeByte(_gfx->saveLoadGetPriority(i));

	// Version 10+: Save, if priority table got modified (set.pri.base opcode)
	out->writeSint16BE((int16)_gfx->saveLoadWasPriorityTableModified());

	out->writeSint16BE((int16)_game.gfxMode);
	out->writeByte(_text->inputGetCursorChar());
	out->writeSint16BE((int16)_text->charAttrib_GetForeground());
	out->writeSint16BE((int16)_text->charAttrib_GetBackground());

	// game.hires
	// game.sbuf
	// game.ego_words
	// game.num_ego_words

	out->writeSint16BE((int16)_game.numObjects);
	for (i = 0; i < (int16)_game.numObjects; i++)
		out->writeSint16BE((int16)objectGetLocation(i));

	// Version 7+: save controller key mappings
	//  required for games, that do not set all key mappings right at the start
	//  when quick restoring is used from ScummVM menu, only 1 cycle is executed
	for (i = 0; i < MAX_CONTROLLER_KEYMAPPINGS; i++) {
		out->writeUint16BE(_game.controllerKeyMapping[i].keycode);
		out->writeByte(_game.controllerKeyMapping[i].controllerSlot);
	}

	// Version 8+: hold-key-mode
	//  required for at least Mixed Up Mother Goose
	out->writeByte(_keyHoldMode);

	// game.ev_keyp
	for (i = 0; i < MAX_STRINGS; i++)
		out->write(_game.getString(i), MAX_STRINGLEN);

	// record info about loaded resources
	for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++) {
		out->writeByte(_game.dirLogic[i].flags);
		out->writeSint16BE((int16)_game.logics[i].sIP);
		out->writeSint16BE((int16)_game.logics[i].cIP);
	}
	for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++)
		out->writeByte(_game.dirPic[i].flags);
	for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++)
		out->writeByte(_game.dirView[i].flags);
	for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++)
		out->writeByte(_game.dirSound[i].flags);

	// game.pictures
	// game.logics
	// game.views
	// game.sounds

	for (i = 0; i < SCREENOBJECTS_MAX; i++) {
		ScreenObjEntry *screenObj = &_game.screenObjTable[i];

		out->writeByte(screenObj->stepTime);
		out->writeByte(screenObj->stepTimeCount);
		out->writeByte(screenObj->objectNr);
		out->writeSint16BE(screenObj->xPos);
		out->writeSint16BE(screenObj->yPos);
		out->writeByte(screenObj->currentViewNr);

		// v->view_data

		out->writeByte(screenObj->currentLoopNr);
		out->writeByte(screenObj->loopCount);

		// v->loop_data

		out->writeByte(screenObj->currentCelNr);
		out->writeByte(screenObj->celCount);

		// v->cel_data
		// v->cel_data_2

		out->writeSint16BE(screenObj->xPos_prev);
		out->writeSint16BE(screenObj->yPos_prev);

		// v->s

		out->writeSint16BE(screenObj->xSize);
		out->writeSint16BE(screenObj->ySize);
		out->writeByte(screenObj->stepSize);
		out->writeByte(screenObj->cycleTime);
		out->writeByte(screenObj->cycleTimeCount);
		out->writeByte(screenObj->direction);

		out->writeByte(screenObj->motionType);
		out->writeByte(screenObj->cycle);
		// Version 11+: loop_flag, was saved previously under vt.parm1
		out->writeByte(screenObj->loop_flag);
		out->writeByte(screenObj->priority);

		out->writeUint16BE(screenObj->flags);

		// this was done so that saved games compatibility isn't broken
		switch (screenObj->motionType) {
		case kMotionNormal:
			out->writeByte(0);
			out->writeByte(0);
			out->writeByte(0);
			out->writeByte(0);
			break;
		case kMotionWander:
			out->writeByte(screenObj->wander_count);
			out->writeByte(0);
			out->writeByte(0);
			out->writeByte(0);
			break;
		case kMotionFollowEgo:
			out->writeByte(screenObj->follow_stepSize);
			out->writeByte(screenObj->follow_flag);
			out->writeByte(screenObj->follow_count);
			out->writeByte(0);
			break;
		case kMotionEgo:
		case kMotionMoveObj:
			out->writeByte((byte)screenObj->move_x); // problematic! int16 -> byte
			out->writeByte((byte)screenObj->move_y);
			out->writeByte(screenObj->move_stepSize);
			out->writeByte(screenObj->move_flag);
			break;
		default:
			error("unknown motion-type");
		}
	}

	// Save image stack

	for (Common::Stack<ImageStackElement>::size_type j = 0; j < _imageStack.size(); ++j) {
		const ImageStackElement &ise = _imageStack[j];
		out->writeByte(ise.type);
		out->writeSint16BE(ise.parm1);
		out->writeSint16BE(ise.parm2);
		out->writeSint16BE(ise.parm3);
		out->writeSint16BE(ise.parm4);
		out->writeSint16BE(ise.parm5);
		out->writeSint16BE(ise.parm6);
		out->writeSint16BE(ise.parm7);
	}
	out->writeByte(0);

	//Write which file number AGIPAL is using (0 if not being used)
	out->writeSint16BE(_gfx->getAGIPalFileNum());

	out->finalize();
	if (out->err()) {
		warning("Can't write file '%s'. (Disk full?)", fileName.c_str());
		result = errIOError;
	} else
		debugC(1, kDebugLevelSavegame, "Saved game %s in file %s", descriptionString.c_str(), fileName.c_str());

	delete out;
	debugC(3, kDebugLevelSavegame, "Closed %s", fileName.c_str());

	return result;
}

int AgiEngine::loadGame(const Common::String &fileName, bool checkId) {
	char  description[SAVEDGAME_DESCRIPTION_LEN + 1];
	byte  saveVersion = 0;
	char  loadId[8];
	int   i, vtEntries = SCREENOBJECTS_MAX;
	uint8 t;
	int16 parm[7];
	Common::InSaveFile *in;
	bool totalPlayTimeWasSet = false;
	byte oldLoopFlag = 0;

	debugC(3, kDebugLevelSavegame, "AgiEngine::loadGame(%s)", fileName.c_str());

	if (!(in = _saveFileMan->openForLoading(fileName))) {
		warning("Can't open file '%s', game not loaded", fileName.c_str());
		return errBadFileOpen;
	} else {
		debugC(3, kDebugLevelSavegame, "Successfully opened %s for reading", fileName.c_str());
	}

	uint32 typea = in->readUint32BE();
	if (typea == AGIflag) {
		debugC(6, kDebugLevelSavegame, "Has AGI flag, good start");
	} else {
		warning("This doesn't appear to be an AGI savegame, game not restored");
		delete in;
		return errOK;
	}

	assert(SAVEDGAME_DESCRIPTION_LEN + 1 == 31); // safety
	in->read(description, 31); // skip description

	// check, if there is a terminating NUL inside description
	uint16 descriptionPos = 0;
	while (description[descriptionPos]) {
		descriptionPos++;
		if (descriptionPos >= sizeof(description))
			error("saved game description is corrupt");
	}
	debugC(6, kDebugLevelSavegame, "Description is: %s", description);

	saveVersion = in->readByte();
	if (saveVersion < 2)    // is the save game pre-ScummVM?
		warning("Old save game version (%d, current version is %d). Will try and read anyway, but don't be surprised if bad things happen", saveVersion, SAVEGAME_CURRENT_VERSION);

	if (saveVersion < 3)
		warning("This save game contains no AGIPAL data, if the game is using the AGIPAL hack, it won't work correctly");

	if (saveVersion > SAVEGAME_CURRENT_VERSION)
		error("Saved game was created with a newer version of ScummVM. Unable to load.");

	if (saveVersion >= 4) {
		// We don't need the thumbnail here, so just read it and discard it
		Graphics::skipThumbnail(*in);

		in->readUint32BE(); // save date
		in->readUint16BE(); // save time (hour + minute)
		if (saveVersion >= 9) {
			in->readByte(); // save time seconds
		}
		if (saveVersion >= 6) {
			uint32 playTime = in->readUint32BE();
			inGameTimerReset(playTime * 1000);
			totalPlayTimeWasSet = true;
		}
	}

	in->readByte(); // was _game.state, not needed anymore

	in->read(loadId, 8);
	if (strcmp(loadId, _game.id) != 0 && checkId) {
		delete in;
		warning("This save seems to be from a different AGI game (save from %s, running %s), not loaded", loadId, _game.id);
		return errBadFileOpen;
	}

	Common::strlcpy(_game.id, loadId, 8);

	if (saveVersion >= 5) {
		char md5[32 + 1];

		for (i = 0; i < 32; i++) {
			md5[i] = in->readByte();

		}
		md5[i] = 0; // terminate

		// As noted above in AgiEngine::saveGame the MD5 sum field may be all zero
		// when the save was made via a fallback matched game. In this case we will
		// replace the MD5 sum with a nicer string, so that the user can easily see
		// this fact in the debug output. The string saved in "md5" will never match
		// any valid MD5 sum, thus it is safe to do that here.
		if (md5[0] == 0)
			Common::strcpy_s(md5, "fallback matched");

		debug(0, "Saved game MD5: \"%s\"", md5);

		if (!getGameMD5()) {
			warning("Since your game was only detected via the fallback detector, there is no possibility to assure the save is compatible with your game version");

			debug(0, "The game used for saving is \"%s\".", md5);
		} else if (strcmp(md5, getGameMD5()) != 0) {
			warning("Game was saved with different gamedata - you may encounter problems");

			debug(0, "Your game is \"%s\" and save is \"%s\".", getGameMD5(), md5);
		}
	}

	if (saveVersion >= 7) {
		// Restore automatic saving state (set.simple opcode)
		_game.automaticSave = in->readByte();
		in->read(_game.automaticSaveDescription, 31);
	} else {
		_game.automaticSave = false;
		_game.automaticSaveDescription[0] = 0;
	}

	for (i = 0; i < MAX_FLAGS; i++)
		_game.flags[i] = in->readByte();
	for (i = 0; i < MAX_VARS; i++)
		_game.vars[i] = in->readByte();

	if (!totalPlayTimeWasSet) {
		// If we haven't gotten total play time by now, try to calculate it by using VM Variables
		// This will happen for at least saves before version 6
		// Direct access because otherwise we would trigger an update to these variables according to ScummVM total play time
		byte playTimeSeconds = _game.vars[VM_VAR_SECONDS];
		byte playTimeMinutes = _game.vars[VM_VAR_MINUTES];
		byte playTimeHours   = _game.vars[VM_VAR_HOURS];
		byte playTimeDays    = _game.vars[VM_VAR_DAYS];
		uint32 playTime = (playTimeSeconds + (playTimeMinutes * 60) + (playTimeHours * 3600) + (playTimeDays * 86400)) * 1000;

		inGameTimerReset(playTime);
	}

	setVar(VM_VAR_FREE_PAGES, 180); // Set amount of free memory to realistic value (Overwriting the just loaded value)

	_game.horizon = in->readSint16BE();
	_text->statusRow_Set(in->readSint16BE());
	_text->promptRow_Set(in->readSint16BE());
	_text->configureScreen(in->readSint16BE());

	// These are never saved
	_text->promptReset();

	in->readSint16BE(); // was _game.inputMode, not needed anymore

	_game.curLogicNr = in->readSint16BE();

	_game.playerControl = in->readSint16BE();
	if (in->readSint16BE())
		quitGame();
	if (in->readSint16BE()) {
		_text->statusEnable();
	} else {
		_text->statusDisable();
	}
	in->readSint16BE(); // was clock enabled, no longer needed
	_game.exitAllLogics = in->readSint16BE();
	in->readSint16BE(); // was _game.pictureShown
	in->readSint16BE(); // was _game.hasPrompt, no longer needed
	in->readSint16BE();	// was _game.gameFlags, no longer needed
	if (in->readSint16BE()) {
		_text->promptEnable();
	} else {
		_text->promptDisable();
	}

	for (i = 0; i < SCRIPT_HEIGHT; i++)
		_gfx->saveLoadSetPriority(i, in->readByte());

	if (saveVersion >= 10) {
		// Version 10+: priority table was modified by scripts
		int16 priorityTableWasModified = in->readSint16BE();

		if (priorityTableWasModified) {
			_gfx->saveLoadSetPriorityTableModifiedBool(true);
		} else {
			_gfx->saveLoadSetPriorityTableModifiedBool(false);
		}
	} else {
		// Try to figure it out by ourselves
		_gfx->saveLoadFigureOutPriorityTableModifiedBool();
	}

	_text->closeWindow();

	_game.block.active = false;

	_game.gfxMode = in->readSint16BE();
	_text->inputSetCursorChar(in->readByte());

	int16 textForeground = in->readSint16BE();
	int16 textBackground = in->readSint16BE();
	_text->charAttrib_Set(textForeground, textBackground);

	// game.ego_words - fixed by clean_input
	// game.num_ego_words - fixed by clean_input

	_game.numObjects = in->readSint16BE();
	for (i = 0; i < (int16)_game.numObjects; i++)
		objectSetLocation(i, in->readSint16BE());

	// Those are not serialized
	for (i = 0; i < MAX_CONTROLLERS; i++) {
		_game.controllerOccurred[i] = false;
	}

	if (saveVersion >= 7) {
		// For old saves, we just keep the current controllers
		for (i = 0; i < MAX_CONTROLLER_KEYMAPPINGS; i++) {
			_game.controllerKeyMapping[i].keycode = in->readUint16BE();
			_game.controllerKeyMapping[i].controllerSlot = in->readByte();
		}
	}

	if (saveVersion >= 8) {
		// Version 8+: hold-key-mode
		if (in->readByte()) {
			_keyHoldMode = true;
		} else {
			_keyHoldMode = false;
		}
	}

	for (i = 0; i < MAX_STRINGS; i++)
		in->read(_game.strings[i], MAX_STRINGLEN);

	for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++) {
		if (in->readByte() & RES_LOADED)
			loadResource(RESOURCETYPE_LOGIC, i);
		else
			unloadResource(RESOURCETYPE_LOGIC, i);
		_game.logics[i].sIP = in->readSint16BE();
		_game.logics[i].cIP = in->readSint16BE();
	}

	for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++) {
		if (in->readByte() & RES_LOADED)
			loadResource(RESOURCETYPE_PICTURE, i);
		else
			unloadResource(RESOURCETYPE_PICTURE, i);
	}

	for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++) {
		if (in->readByte() & RES_LOADED)
			loadResource(RESOURCETYPE_VIEW, i);
		else
			unloadResource(RESOURCETYPE_VIEW, i);
	}

	for (i = 0; i < MAX_DIRECTORY_ENTRIES; i++) {
		if (in->readByte() & RES_LOADED)
			loadResource(RESOURCETYPE_SOUND, i);
		else
			unloadResource(RESOURCETYPE_SOUND, i);
	}

	// game.pictures - loaded above
	// game.logics - loaded above
	// game.views - loaded above
	// game.sounds - loaded above

	for (i = 0; i < vtEntries; i++) {
		ScreenObjEntry *screenObj = &_game.screenObjTable[i];

		screenObj->stepTime = in->readByte();
		screenObj->stepTimeCount = in->readByte();
		screenObj->objectNr = in->readByte();
		screenObj->xPos = in->readSint16BE();
		screenObj->yPos = in->readSint16BE();
		screenObj->currentViewNr = in->readByte();

		// screenObj->view_data - fixed below

		screenObj->currentLoopNr = in->readByte();
		screenObj->loopCount = in->readByte();

		// screenObj->loop_data - fixed below

		screenObj->currentCelNr = in->readByte();
		screenObj->celCount = in->readByte();

		// screenObj->cel_data - fixed below
		// screenObj->cel_data_2 - fixed below

		screenObj->xPos_prev = in->readSint16BE();
		screenObj->yPos_prev = in->readSint16BE();

		// screenObj->s - fixed below

		screenObj->xSize = in->readSint16BE();
		screenObj->ySize = in->readSint16BE();
		screenObj->stepSize = in->readByte();
		screenObj->cycleTime = in->readByte();
		screenObj->cycleTimeCount = in->readByte();
		screenObj->direction = in->readByte();

		screenObj->motionType = (MotionType)in->readByte();
		screenObj->cycle = (CycleType)in->readByte();
		if (saveVersion >= 11) {
			// Version 11+: loop_flag, was previously vt.parm1
			screenObj->setLoopFlag(in->readByte());
		}
		screenObj->priority = in->readByte();

		screenObj->flags = in->readUint16BE();

		// this was done so that saved games compatibility isn't broken
		switch (screenObj->motionType) {
		case kMotionNormal:
			oldLoopFlag = in->readByte();
			in->readByte();
			in->readByte();
			in->readByte();
			break;
		case kMotionWander:
			screenObj->wander_count = in->readByte();
			in->readByte();
			in->readByte();
			in->readByte();
			oldLoopFlag = screenObj->wander_count;
			break;
		case kMotionFollowEgo:
			screenObj->follow_stepSize = in->readByte();
			screenObj->follow_flag = in->readByte();
			screenObj->follow_count = in->readByte();
			in->readByte();
			oldLoopFlag = screenObj->follow_stepSize;
			break;
		case kMotionEgo:
		case kMotionMoveObj:
			screenObj->move_x = in->readByte(); // problematic! int16 -> byte
			screenObj->move_y = in->readByte();
			screenObj->move_stepSize = in->readByte();
			screenObj->move_flag = in->readByte();
			oldLoopFlag = screenObj->move_x;
			break;
		default:
			error("unknown motion-type");
		}
		if (saveVersion < 11) {
			if (saveVersion < 7) {
				// Recreate loop_flag from motion-type (was previously vt.parm1)
				// vt.parm1 was shared for multiple uses
				screenObj->setLoopFlag(oldLoopFlag);
			} else {
				// for Version 7-10 we can't really do anything, it was not saved
				screenObj->setLoopFlag(0); // set it to 0
			}
		}
	}

	// Fix some pointers in screenObjTable

	for (i = 0; i < SCREENOBJECTS_MAX; i++) {
		ScreenObjEntry *screenObj = &_game.screenObjTable[i];

		if (_game.dirView[screenObj->currentViewNr].offset == _EMPTY)
			continue;

		if (!(_game.dirView[screenObj->currentViewNr].flags & RES_LOADED))
			loadResource(RESOURCETYPE_VIEW, screenObj->currentViewNr);

		setView(screenObj, screenObj->currentViewNr);   // Fix v->view_data
		setLoop(screenObj, screenObj->currentLoopNr);   // Fix v->loop_data
		setCel(screenObj, screenObj->currentCelNr); // Fix v->cel_data
	}

	_sprites->eraseSprites();

	_game.pictureShown = false;

	_gfx->clearDisplay(0, false); // clear display screen, but not copy it to actual screen for now b/c transition

	// Recreate background from saved image stack
	clearImageStack();
	while ((t = in->readByte()) != 0) {
		for (i = 0; i < 7; i++)
			parm[i] = in->readSint16BE();
		replayImageStackCall(t, parm[0], parm[1], parm[2],
		                     parm[3], parm[4], parm[5], parm[6]);
	}

	// Load AGIPAL Data
	if (saveVersion >= 3)
		_gfx->setAGIPal(in->readSint16BE());

	delete in;
	debugC(3, kDebugLevelSavegame, "Closed %s", fileName.c_str());

	setFlag(VM_FLAG_RESTORE_JUST_RAN, true);

	_words->clearEgoWords();

	// don't delay anything right after restoring a game
	artificialDelay_Reset();

	_sprites->eraseSprites();
	_sprites->buildAllSpriteLists();
	_sprites->drawAllSpriteLists();
	_picture->showPictureWithTransition();
	_game.pictureShown = true;
	_text->statusDraw();
	_text->promptRedraw();

	// copy everything over (we should probably only copy over the remaining parts of the screen w/o play screen
	_gfx->copyDisplayToScreen();

	applyVolumeToMixer();

	return errOK;
}

int AgiEngine::scummVMSaveLoadDialog(bool isSave) {
	GUI::SaveLoadChooser *dialog;
	Common::String desc;
	int slot;

	if (isSave) {
		dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

		slot = dialog->runModalWithCurrentTarget();
		desc = dialog->getResultString();

		if (desc.empty()) {
			// create our own description for the saved game, the user didn't enter it
			desc = dialog->createDefaultSaveDescription(slot);
		}

		if (desc.size() > 28)
			desc = Common::String(desc.c_str(), 28);
	} else {
		dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
		slot = dialog->runModalWithCurrentTarget();
	}

	delete dialog;

	if (slot < 0)
		return true;

	if (isSave)
		return doSave(slot, desc);
	else
		return doLoad(slot, false);
}

int AgiEngine::doSave(int slot, const Common::String &desc) {
	Common::String fileName = getSaveStateName(slot);
	debugC(8, kDebugLevelResources, "file is [%s]", fileName.c_str());

	// Make sure all graphics was blitted to screen. This fixes bug
	// #4790: "AGI: Ego partly erased in Load/Save thumbnails"
	_gfx->updateScreen();
//	_gfx->doUpdate();

	return saveGame(fileName, desc);
}

int AgiEngine::doLoad(int slot, bool showMessages) {
	Common::String fileName = getSaveStateName(slot);
	debugC(8, kDebugLevelResources, "file is [%s]", fileName.c_str());

	_sprites->eraseSprites();
	_sound->stopSound();
	_text->closeWindow();

	int result = loadGame(fileName);

	if (result == errOK) {
		_game.exitAllLogics = true;
		_menu->itemEnableAll();
	} else {
		if (showMessages)
			_text->messageBox("Error restoring game.");
	}

	return result;
}

SavedGameSlotIdArray AgiEngine::getSavegameSlotIds() {
	Common::StringArray filenames;
	int16 numberPos = _targetName.size() + 1;
	int16 slotId = 0;
	SavedGameSlotIdArray slotIdArray;

	// search for saved game filenames...
	filenames = _saveFileMan->listSavefiles(_targetName + ".###");

	Common::StringArray::iterator it;
	Common::StringArray::iterator end = filenames.end();

	// convert to lower-case, just to be sure
	for (it = filenames.begin(); it != end; it++) {
		it->toLowercase();
	}
	// sort
	Common::sort(filenames.begin(), filenames.end());

	// now extract slot-Ids
	for (it = filenames.begin(); it != end; it++) {
		slotId = atoi(it->c_str() + numberPos);

		slotIdArray.push_back(slotId);
	}
	return slotIdArray;
}

bool AgiEngine::getSavegameInformation(int16 slotId, Common::String &saveDescription, uint32 &saveDate, uint32 &saveTime, bool &saveIsValid) {
	Common::InSaveFile *in;
	Common::String fileName = getSaveStateName(slotId);
	char saveGameDescription[31];
	int16 curPos = 0;
	byte  saveVersion = 0;

	saveDescription.clear();
	saveDate = 0;
	saveTime = 0;
	saveIsValid = false;

	debugC(4, kDebugLevelSavegame, "Current game id is %s", _targetName.c_str());

	if (!(in = _saveFileMan->openForLoading(fileName))) {
		debugC(4, kDebugLevelSavegame, "File %s does not exist", fileName.c_str());
		return false;

	} else {
		debugC(4, kDebugLevelSavegame, "Successfully opened %s for reading", fileName.c_str());

		uint32 type = in->readUint32BE();

		if (type != AGIflag) {
			warning("This doesn't appear to be an AGI savegame");
			saveDescription += "[ScummVM: not an AGI save]";
			delete in;
			return true;
		}

		debugC(6, kDebugLevelSavegame, "Has AGI flag, good start");
		if (in->read(saveGameDescription, 31) != 31) {
			warning("unexpected EOF");
			delete in;
			saveDescription += "[ScummVM: invalid save]";
			return true;
		}

		for (curPos = 0; curPos < 31; curPos++) {
			if (!saveGameDescription[curPos])
				break;
		}
		if (curPos >= 31) {
			warning("corrupted description");
			delete in;
			saveDescription += "[ScummVM: invalid save]";
			return true;
		}

		saveVersion = in->readByte();
		if (saveVersion > SAVEGAME_CURRENT_VERSION) {
			warning("save from a future ScummVM, not supported");
			delete in;
			saveDescription += "[ScummVM: not supported]";
			return true;
		}

		if (saveVersion >= 4) {
			// We don't need the thumbnail here, so just read it and discard it
			Graphics::skipThumbnail(*in);

			saveDate = in->readUint32BE();
			saveTime = in->readUint16BE() << 8;
			if (saveVersion >= 9) {
				saveTime |= in->readByte(); // add seconds (only available since saved game version 9+)
			}

			// save date is DDMMYYYY, we need a proper format
			byte saveDateDay = saveDate >> 24;
			byte saveDateMonth = (saveDate >> 16) & 0xFF;
			uint16 saveDateYear = saveDate & 0xFFFF;

			saveDate = (saveDateYear << 16) | (saveDateMonth << 8) | saveDateDay;

		} else {
			saveDate = 0;
			saveTime = 0;
		}

		saveDescription += saveGameDescription;
		saveIsValid = true;

		if (_game._vm->getLanguage() == Common::HE_ISR) {
			saveDescription = saveDescription.decode(Common::kUtf8).encode(Common::kWindows1255);
		}

		delete in;
		return true;
	}
}

bool AgiEngine::loadGameAutomatic() {
	int16 automaticRestoreGameSlotId = 0;

	automaticRestoreGameSlotId = _systemUI->figureOutAutomaticRestoreGameSlot(_game.automaticSaveDescription);
	if (automaticRestoreGameSlotId >= 0) {
		if (doLoad(automaticRestoreGameSlotId, true) == errOK) {
			return true;
		}
	}
	return false;
}

bool AgiEngine::loadGameDialog() {
	int16 restoreGameSlotId = 0;

	if (!ConfMan.getBool("originalsaveload"))
		return scummVMSaveLoadDialog(false);

	restoreGameSlotId = _systemUI->askForRestoreGameSlot();
	if (restoreGameSlotId >= 0) {
		if (doLoad(restoreGameSlotId, true) == errOK) {
			return true;
		}
	}
	return errOK;
}

// Try to figure out either the slot, that is currently using the automatic saved game description
// or get a new slot.
// If we fail, return false, so that the regular saved game dialog is called
// Original AGI was limited to 12 saves, we are effectively limited to 100 saves at the moment.
//
// btw. this also means that entering an existent name in Mixed Up Mother Goose will effectively overwrite
// that saved game. This is also what original AGI did.
bool AgiEngine::saveGameAutomatic() {
	int16 automaticSaveGameSlotId = 0;

	automaticSaveGameSlotId = _systemUI->figureOutAutomaticSaveGameSlot(_game.automaticSaveDescription);
	if (automaticSaveGameSlotId >= 0) {
		Common::String slotDescription(_game.automaticSaveDescription);

		// WORKAROUND: Remove window in case one is currently shown, otherwise it would get saved in the thumbnail
		// Happens for Mixed Up Mother Goose. The scripts close the window after saving.
		// Original interpreter obviously did not do this, but original interpreter also did not save thumbnails.
		_text->closeWindow();

		if (doSave(automaticSaveGameSlotId, slotDescription) == errOK) {
			return true;
		}
	}
	return false;
}

bool AgiEngine::saveGameDialog() {
	int16 saveGameSlotId = 0;
	Common::String slotDescription;

	if (!ConfMan.getBool("originalsaveload"))
		return scummVMSaveLoadDialog(true);

	saveGameSlotId = _systemUI->askForSaveGameSlot();
	if (saveGameSlotId >= 0) {
		if (_systemUI->askForSaveGameDescription(saveGameSlotId, slotDescription)) {
			if (doSave(saveGameSlotId, slotDescription) == errOK) {
				return true;
			}
		}
	}
	return false;
}


void AgiEngine::recordImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
									 int16 p4, int16 p5, int16 p6, int16 p7) {
	ImageStackElement pnew;

	pnew.type = type;
	pnew.pad = 0;
	pnew.parm1 = p1;
	pnew.parm2 = p2;
	pnew.parm3 = p3;
	pnew.parm4 = p4;
	pnew.parm5 = p5;
	pnew.parm6 = p6;
	pnew.parm7 = p7;

	_imageStack.push(pnew);
}

void AgiEngine::replayImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
									 int16 p4, int16 p5, int16 p6, int16 p7) {
	switch (type) {
	case ADD_PIC:
		debugC(8, kDebugLevelMain, "--- decoding picture %d ---", p1);
		loadResource(RESOURCETYPE_PICTURE, p1);
		_picture->decodePicture(p1, p2, p3 != 0);
		break;
	case ADD_VIEW:
		loadResource(RESOURCETYPE_VIEW, p1);
		_sprites->addToPic(p1, p2, p3, p4, p5, p6, p7);
		break;
	default:
		break;
	}
}

void AgiEngine::clearImageStack() {
	_imageStack.clear();
}

void AgiEngine::releaseImageStack() {
	_imageStack.clear();
}

void AgiEngine::checkQuickLoad() {
	if (ConfMan.hasKey("save_slot")) {
		Common::String saveNameBuffer = getSaveStateName(ConfMan.getInt("save_slot"));

		_sprites->eraseSprites();
		_sound->stopSound();

		if (loadGame(saveNameBuffer, false) == errOK) {  // Do not check game id
			_game.exitAllLogics = true;
			_menu->itemEnableAll();
		}
	}
}

Common::Error AgiEngine::loadGameState(int slot) {
	Common::String saveLoadSlot = getSaveStateName(slot);

	_sprites->eraseSprites();
	_sound->stopSound();

	if (loadGame(saveLoadSlot) == errOK) {
		_game.exitAllLogics = true;
		_menu->itemEnableAll();
		return Common::kNoError;
	} else {
		return Common::kUnknownError;
	}
}

Common::Error AgiEngine::saveGameState(int slot, const Common::String &description, bool isAutosave) {
	Common::String saveLoadSlot = getSaveStateName(slot);
	if (saveGame(saveLoadSlot, description) == errOK)
		return Common::kNoError;
	else
		return Common::kUnknownError;
}

} // End of namespace Agi
