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

#include "common/scummsys.h"
#include "access/access.h"
#include "access/resources.h"
#include "access/scripts.h"
#include "access/martian/martian_resources.h"

namespace Access {

Scripts::Scripts(AccessEngine *vm) : Manager(vm) {
	_resource = nullptr;
	_specialFunction = -1;
	_data = nullptr;
	_sequence = 0;
	_endFlag = false;
	_returnCode = 0;
	_scriptCommand = 0;
	_choice = 0;
	_choiceStart = 0;
	_charsOrg = Common::Point(0, 0);
	_texsOrg = Common::Point(0, 0);
	setOpcodes();
}

Scripts::~Scripts() {
	freeScriptData();
}

void Scripts::setOpcodes() {
	COMMAND_LIST[0] = &Scripts::cmdObject;
	COMMAND_LIST[1] = &Scripts::cmdEndObject;
	COMMAND_LIST[2] = &Scripts::cmdJumpLook;
	COMMAND_LIST[3] = &Scripts::cmdJumpOpen;
	COMMAND_LIST[4] = &Scripts::cmdJumpGet;
	COMMAND_LIST[5] = &Scripts::cmdJumpMove;
	COMMAND_LIST[6] = &Scripts::cmdJumpUse;
	COMMAND_LIST[7] = &Scripts::cmdJumpTalk;
	COMMAND_LIST[8] = &Scripts::cmdNull;
	COMMAND_LIST[9] = &Scripts::cmdPrint_v1;
	COMMAND_LIST[10] = &Scripts::cmdRetPos;
	COMMAND_LIST[11] = &Scripts::cmdAnim;
	COMMAND_LIST[12] = &Scripts::cmdSetFlag;
	COMMAND_LIST[13] = &Scripts::cmdCheckFlag;
	COMMAND_LIST[14] = &Scripts::cmdGoto;
	COMMAND_LIST[15] = &Scripts::cmdAddScore;
	COMMAND_LIST[16] = &Scripts::cmdSetInventory;
	COMMAND_LIST[17] = &Scripts::cmdCheckInventory;
	COMMAND_LIST[18] = &Scripts::cmdSetTex;
	COMMAND_LIST[19] = &Scripts::cmdNewRoom;
	COMMAND_LIST[20] = &Scripts::cmdConverse;
	COMMAND_LIST[21] = &Scripts::cmdCheckFrame;
	COMMAND_LIST[22] = &Scripts::cmdCheckAnim;
	COMMAND_LIST[23] = &Scripts::cmdSnd;
	COMMAND_LIST[24] = &Scripts::cmdRetNeg;
	COMMAND_LIST[25] = &Scripts::cmdRetPos;
	COMMAND_LIST[26] = &Scripts::cmdCheckLoc;
	COMMAND_LIST[27] = &Scripts::cmdSetAnim;
	COMMAND_LIST[28] = &Scripts::cmdDispInv_v1;
	COMMAND_LIST[29] = &Scripts::cmdSetAbout;
	COMMAND_LIST[30] = &Scripts::cmdSetTimer;
	COMMAND_LIST[31] = &Scripts::cmdCheckTimer;
	COMMAND_LIST[32] = &Scripts::cmdSetTravel;
	COMMAND_LIST[33] = &Scripts::cmdJumpGoto;
	COMMAND_LIST[34] = &Scripts::cmdSetVideo;
	COMMAND_LIST[35] = &Scripts::cmdPlayVideo;
	COMMAND_LIST[36] = &Scripts::cmdPlotImage;
	COMMAND_LIST[37] = &Scripts::cmdSetDisplay;
	COMMAND_LIST[38] = &Scripts::cmdSetBuffer;
	COMMAND_LIST[39] = &Scripts::cmdSetScroll;
	COMMAND_LIST[40] = &Scripts::cmdSaveRect;
	COMMAND_LIST[41] = &Scripts::cmdVideoEnded;
	COMMAND_LIST[42] = &Scripts::cmdSetBufVid;
	COMMAND_LIST[43] = &Scripts::cmdPlayBufVid;
	COMMAND_LIST[44] = &Scripts::cmdRemoveLast;
	COMMAND_LIST[45] = &Scripts::cmdDoTravel;
	COMMAND_LIST[46] = &Scripts::cmdCheckAbout;
	COMMAND_LIST[47] = &Scripts::cmdSpecial;
	COMMAND_LIST[48] = &Scripts::cmdSetCycle;
	COMMAND_LIST[49] = &Scripts::cmdCycle;
	COMMAND_LIST[50] = &Scripts::cmdCharSpeak;
	COMMAND_LIST[51] = &Scripts::cmdTexSpeak;
	COMMAND_LIST[52] = &Scripts::cmdTexChoice;
	COMMAND_LIST[53] = &Scripts::cmdWait;
	COMMAND_LIST[54] = &Scripts::cmdSetConPos;
	COMMAND_LIST[55] = &Scripts::cmdCheckVFrame;
	COMMAND_LIST[56] = &Scripts::cmdJumpChoice;
	COMMAND_LIST[57] = &Scripts::cmdReturnChoice;
	COMMAND_LIST[58] = &Scripts::cmdClearBlock;
	COMMAND_LIST[59] = &Scripts::cmdLoadSound;
	COMMAND_LIST[60] = &Scripts::cmdFreeSound;
	COMMAND_LIST[61] = &Scripts::cmdSetVideoSound;
	COMMAND_LIST[62] = &Scripts::cmdPlayVideoSound;
	COMMAND_LIST[63] = &Scripts::cmdPrintWatch;
	COMMAND_LIST[64] = &Scripts::cmdDispAbout;
	COMMAND_LIST[65] = &Scripts::cmdPushLocation_v1;
	COMMAND_LIST[66] = &Scripts::cmdCheckTravel;
	COMMAND_LIST[67] = &Scripts::cmdBlock;
	COMMAND_LIST[68] = &Scripts::cmdPlayerOff;
	COMMAND_LIST[69] = &Scripts::cmdPlayerOn;
	COMMAND_LIST[70] = &Scripts::cmdDead;
	COMMAND_LIST[71] = &Scripts::cmdFadeOut;
	COMMAND_LIST[72] = &Scripts::cmdEndVideo;
	COMMAND_LIST[73] = &Scripts::cmdHelp_v1;
}

void Scripts::setOpcodes_v2() {
	COMMAND_LIST[3] = &Scripts::cmdJumpHelp;
	COMMAND_LIST[9] = &Scripts::cmdPrint_v2;
	COMMAND_LIST[15] = &Scripts::cmdSetInventory;
	COMMAND_LIST[28] = &Scripts::cmdDispInv_v2;
	COMMAND_LIST[29] = &Scripts::cmdSetTimer;
	COMMAND_LIST[32] = &Scripts::cmdJumpGoto;
	COMMAND_LIST[40] = &Scripts::cmdVideoEnded;
	COMMAND_LIST[45] = COMMAND_LIST[46] = &Scripts::cmdSpecial;
	COMMAND_LIST[63] = COMMAND_LIST[64] = COMMAND_LIST[65] = COMMAND_LIST[66] = COMMAND_LIST[67] = &Scripts::cmdPushLocation;
}

void Scripts::setScript(Resource *res, bool restartFlag) {
	debugC(1, kDebugScripts, "setScript(res=%p (%s), restartFlag=%d)", (void *)res, res->getFileName(), restartFlag);
	_resource = res;
	_data = res->_stream;
	_endFlag = restartFlag;
}

void Scripts::freeScriptData() {
	delete _resource;
	_resource = nullptr;
	_data = nullptr;
}

void Scripts::searchForSequence() {
	assert(_data);

	_data->seek(0);
	int sequenceId;
	do {
		while (_data->readByte() != SCRIPT_START_BYTE) {}
		sequenceId = _data->readUint16LE();
	} while (sequenceId != _sequence);
}

void Scripts::charLoop() {
	bool endFlag = _endFlag;
	int64 pos = _data->pos();

	_sequence = 2000;
	searchForSequence();
	_vm->_images.clear();
	_vm->_buffer2.copyBlock(&_vm->_buffer1, Common::Rect(0, 0, _vm->_buffer2.w, _vm->_buffer2.h));
	_vm->_newRects.clear();

	executeScript();
	_vm->plotList1();
	_vm->copyBlocks();

	_data->seek(pos);
	_endFlag = endFlag;
}

void Scripts::clearWatch() {
	_vm->_events->hideCursor();
	_vm->_screen->_orgX1 = 128;
	_vm->_screen->_orgY1 = 57;
	_vm->_screen->_orgX2 = 228;
	_vm->_screen->_orgY2 = 106 + 1;
	_vm->_screen->_lColor = 0;
	_vm->_screen->drawRect();

	_vm->_events->showCursor();
}

void Scripts::printWatch() {
	_vm->_fonts._charSet._lo = 8;
	_vm->_fonts._charSet._hi = 2;
	_vm->_fonts._charFor._lo = 2;
	_vm->_fonts._charFor._hi = 255;
	Font::_fontColors[3] = 2;

	_vm->_screen->_maxChars = 19;
	_vm->_screen->_printOrg = Common::Point(128, 58);
	_vm->_screen->_printStart = Common::Point(128, 58);
	clearWatch();

	Common::String msg = _data->readString();
	Common::String line = "";
	int width = 0;
	bool lastLine;
	do {
		lastLine = _vm->_fonts._font2->getLine(msg, _vm->_screen->_maxChars, line, width, Font::kWidthInChars);

		// Draw the text
		_vm->_fonts._font2->drawString(_vm->_screen, line, _vm->_screen->_printOrg);

		_vm->_screen->_printOrg.y += 6;
		_vm->_screen->_printOrg.x = _vm->_screen->_printStart.x;

		if (_vm->_screen->_printOrg.y == 106) {
			_vm->_events->waitKeyActionMouse();
			clearWatch();
			_vm->_screen->_printOrg.y = _vm->_screen->_printStart.y;
		}
	} while (!lastLine);
	_vm->_events->waitKeyActionMouse();
}

void Scripts::findNull() {
	// No implementation required in ScummVM, the strings in the script files are already skipped by the use of readByte()
}

int Scripts::executeScript() {
	assert(_data);
	_endFlag = false;
	_returnCode = 0;

	debugC(1, kDebugScripts, "** Start script execution (at %d/%d bytes) **", (int)_data->pos(), (int)_data->size());
	do {
		// Get next command, skipping over script start start if it's being pointed to
		while ((_scriptCommand = _data->readByte()) == SCRIPT_START_BYTE)
			_data->skip(2);

		if (_scriptCommand < 0x80)
			error("Unexpected opcode value %d", _scriptCommand);

		debugCN(1, kDebugScripts, "%04X %02X ", (int)_data->pos(), _scriptCommand - 0x80);

		executeCommand(_scriptCommand - 0x80);
	} while (!_endFlag && !_vm->shouldQuitOrRestart());

	if (_data)
		debugC(1, kDebugScripts, "** End script execution (return %d, at %d/%d bytes) **", _returnCode, (int)_data->pos(), (int)_data->size());
	else
		debugC(1, kDebugScripts, "** End script execution (return %d, data cleared) **", _returnCode);

	return _returnCode;
}

void Scripts::executeCommand(int commandIndex) {
	(this->*COMMAND_LIST[commandIndex])();
}

void Scripts::cmdObject() {
	debugC(1, kDebugScripts, "cmdObject()");
	_vm->_bubbleBox->load(_data);
}

void Scripts::cmdEndObject() {
	const char *msg;
	if (_vm->getLanguage() == Common::ES_ESP)
		msg = ESP_GENERAL_MESSAGES[_vm->_room->_selectCommand];
	else
		msg = GENERAL_MESSAGES[_vm->_room->_selectCommand];
	debugC(1, kDebugScripts, "cmdEndObject(msg=\"%s\")", msg);
	if (_vm->getGameID() == kGameMartianMemorandum)
		doCmdPrint_v1(msg);
	else
		printString(msg);
}

void Scripts::cmdJumpLook() {
	debugCN(1, kDebugScripts, "cmdJumpLook(selectCommand=%d)", _vm->_room->_selectCommand);
	if (_vm->_room->_selectCommand == 0) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdJumpOpen() {
	debugCN(1, kDebugScripts, "cmdJumpOpen(selectCommand=%d)", _vm->_room->_selectCommand);
	if (_vm->_room->_selectCommand == 1) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdJumpHelp() {
	debugCN(1, kDebugScripts, "cmdJumpHelp(selectCommand=%d)", _vm->_room->_selectCommand);
	if (_vm->_room->_selectCommand == 8) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdJumpGet() {
	debugCN(1, kDebugScripts, "cmdJumpGet(selectCommand=%d)", _vm->_room->_selectCommand);
	if (_vm->_room->_selectCommand == 3) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdJumpMove() {
	debugCN(1, kDebugScripts, "cmdJumpMove(selectCommand=%d)", _vm->_room->_selectCommand);
	if (_vm->_room->_selectCommand == 2) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdJumpUse() {
	debugCN(1, kDebugScripts, "cmdJumpUse(selectCommand=%d)", _vm->_room->_selectCommand);
	if (_vm->_room->_selectCommand == 4) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdJumpTalk() {
	debugCN(1, kDebugScripts, "cmdJumpTalk(selectCommand=%d)", _vm->_room->_selectCommand);
	if (_vm->_room->_selectCommand == 6) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdNull() {
	debugC(1, kDebugScripts, "cmdNull()");
}

#define PRINT_TIMER 25

void Scripts::cmdPrint_v2() {
	// Get a text line for display
	Common::String msg = _data->readString();
	debugC(1, kDebugScripts, "cmdPrint_v2(msg=\"%s\")", msg.c_str());
	printString(msg);
}

void Scripts::doCmdPrint_v1(const Common::String &msg) {
	_vm->_screen->_printOrg = Common::Point(20, 42);
	_vm->_screen->_printStart = Common::Point(20, 32);
	_vm->_bubbleBox->placeBubble(msg);
	_vm->_events->waitKeyActionMouse();
	_vm->_events->hideCursor();
	_vm->_screen->restoreBlock();
	_vm->_events->showCursor();
	findNull();
}

void Scripts::cmdPrint_v1() {
	Common::String msg = _data->readString();
	debugC(1, kDebugScripts, "cmdPrint_v1(msg=\"%s\")", msg.c_str());
	doCmdPrint_v1(msg);
}

void Scripts::printString(const Common::String &msg) {
	if (_vm->getGameID() != kGameMartianMemorandum) {
		_vm->_screen->_printOrg = Common::Point(20, 42);
		_vm->_screen->_printStart = Common::Point(20, 42);
		_vm->_timers[PRINT_TIMER]._timer = 50;
		_vm->_timers[PRINT_TIMER]._initTm = 50;
		++_vm->_timers[PRINT_TIMER]._flag;
	}

	// Display the text in a bubble, and wait for a keypress or mouse click
	_vm->_bubbleBox->placeBubble(msg);
	_vm->_events->waitKeyActionMouse();

	// Wait until the bubble display is expired
	while (!_vm->shouldQuit() && _vm->_timers[PRINT_TIMER]._flag) {
		_vm->_events->pollEvents();
	}

	// Restore the original screen over the text bubble
	_vm->_screen->restoreBlock();
}

void Scripts::cmdRetPos() {
	debugC(1, kDebugScripts, "cmdRetPos()");
	_endFlag = true;
	_returnCode = 0;
}

void Scripts::cmdAnim() {
	int animId = _data->readByte();
	debugC(1, kDebugScripts, "cmdAnim(animId=%d)", animId);
	_vm->_animation->animate(animId);
}

void Scripts::cmdSetFlag() {
	int flagNum = _data->readByte();
	byte flagVal = _data->readByte();
	debugC(1, kDebugScripts, "cmdSetFlag(flagNum=%d, flagVal=%d)", flagNum, flagVal);

	_vm->_flags[flagNum] = flagVal;
}

void Scripts::cmdCheckFlag() {
	int flagNum = _data->readUint16LE();
	int flagVal = _data->readUint16LE();
	debugCN(1, kDebugScripts, "cmdCheckFlag(flagNum=%d, flagVal=%d)", flagNum, flagVal);
	assert(flagNum < 256);

	//
	// Although this opcode takes an int16, only the byte value (AL) is compared
	// to flags in both MM and Amazon. This is important as some scripts compare
	// against 255 to check for -1, so we need to make the casting the same.
	//
	if ((byte)_vm->_flags[flagNum] == (byte)flagVal) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdGoto() {
	_sequence = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdGoto(sequence=%d)", _sequence);
	searchForSequence();
}

void Scripts::cmdAddScore() {
	debugC(1, kDebugScripts, "cmdAddScore()");
	_data->skip(1);
}

void Scripts::cmdSetInventory() {
	int itemId = _data->readByte();
	int itemVal = _data->readByte();
	debugC(1, kDebugScripts, "cmdSetInventory(itemId=%d, itemVal=%d)", itemId, itemVal);
	(*_vm->_inventory)[itemId] = itemVal;
	_vm->_inventory->_startInvItem = 0;
	_vm->_inventory->_startInvBox = 0;
	_vm->_inventory->_invChangeFlag = true;
}

void Scripts::cmdCheckInventory() {
	int itemId = _data->readUint16LE();
	int itemVal = _data->readUint16LE();
	debugCN(1, kDebugScripts, "cmdCheckInventory(itemId=%d, itemVal=%d)", itemId, itemVal);
	if ((byte)(*_vm->_inventory)[itemId] == (byte)itemVal) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdSetTex() {
	const int xStart = _data->readSint16LE();
	const int yStart = _data->readSint16LE();
	debugC(1, kDebugScripts, "cmdSetTex(xStart=%d, yStart=%d)", xStart, yStart);
	_vm->_player->_playerDirection = RIGHT;
	int posX = xStart - (_vm->_player->_playerOffset.x / 2);
	if (posX <= _vm->_player->_rawPlayer.x)
		_vm->_player->_playerDirection = LEFT;

	_vm->_player->_rawPlayer.x = posX;
	_vm->_player->checkScroll();
	bool scrlTemp = _vm->_player->_scrollFlag;

	_vm->_player->_playerDirection = UP;
	int posY = yStart;
	if (posY <= _vm->_player->_rawPlayer.y)
		_vm->_player->_playerDirection = DOWN;

	_vm->_player->_rawPlayer.y = posY;
	_vm->_player->_frame = 5;
	_vm->_player->checkScroll();

	_vm->_player->_scrollFlag |= scrlTemp;

	_vm->_player->_position = Common::Point(_vm->_player->_rawPlayer.x, _vm->_player->_rawPlayer.y - _vm->_player->_playerOffset.y);
	_vm->_player->_offsetY = _vm->_player->_playerOffset.y;
	_vm->_player->_spritesPtr = _vm->_player->_playerSprites;
	_vm->_player->_frameNumber = _vm->_player->_frame;

	_vm->_room->setWallCodes();
}

#define CURRENT_ROOM 0xFF

void Scripts::cmdNewRoom() {
	int roomNumber = _data->readByte();
	debugC(1, kDebugScripts, "cmdNewRoom(roomNumber=%d)", roomNumber);
	if (roomNumber != CURRENT_ROOM)
		_vm->_player->_roomNumber = roomNumber;

	_vm->_room->_function = FN_CLEAR1;
	_vm->freeChar();
	_vm->_converseMode = 0;
	cmdRetPos();
}

void Scripts::converse1(int val) {
	_vm->_conversation = val;
	_vm->_room->clearRoom();
	_vm->freeChar();
	_vm->_char->loadChar(_vm->_conversation);
	_vm->_events->setCursor(CURSOR_ARROW);

	_vm->_images.clear();
	_vm->_oldRects.clear();
	// game also clears screen sprites here which is redundant, done by clearRoom above.
	_sequence = 0;
	searchForSequence();

	if (_vm->_screen->_vesaMode) {
		// This is not in MM, but _vesaMode is also never set there
		_vm->_converseMode = 1;
	}
}

void Scripts::cmdConverse() {
	int val = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdConverse(val=%d)", val);
	converse1(val);
}

void Scripts::cmdCheckFrame() {
	int id = _data->readUint16LE();
	Animation *anim = _vm->_animation->findAnimation(id);

	int frame = _data->readUint16LE();
	debugCN(1, kDebugScripts, "cmdCheckFrame(id=%d, frame=%d)", id, frame);
	if (anim->_frameNumber == frame) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdCheckAnim() {
	int id = _data->readUint16LE();
	Animation *anim = _vm->_animation->findAnimation(id);
	debugCN(1, kDebugScripts, "cmdCheckAnim(id=%d)", id);
	if (anim->_currentLoopCount == -1) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdSnd() {
	int id = _data->readByte();
	debugC(1, kDebugScripts, "cmdSnd(id=%d)", id);
	_vm->_sound->playSound(id);
}

void Scripts::cmdRetNeg() {
	debugC(1, kDebugScripts, "cmdRetNeg()");
	_endFlag = true;
	_returnCode = -1;
}

void Scripts::cmdCheckLoc() {
	int minX = _data->readUint16LE();
	int minY = _data->readUint16LE();
	int maxX = _data->readUint16LE();
	int maxY = _data->readUint16LE();
	debugCN(1, kDebugScripts, "cmdCheckLoc(minX=%d, minY=%d, maxX=%d, maxY=%d)", minX, minY, maxX, maxY);

	int curX = _vm->_player->_rawPlayer.x + _vm->_player->_playerOffset.x;
	int curY = _vm->_player->_rawPlayer.y;

	if ((curX >= minX) && (curX <= maxX) && (curY >= minY) && (curY <= maxY)) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdSetAnim() {
	int animId = _data->readByte();
	debugC(1, kDebugScripts, "cmdSetAnim(animId=%d)", animId);
	Animation *anim = _vm->_animation->setAnimation(animId);

	if (anim)
		_vm->_animation->setAnimTimer(anim);
}

void Scripts::cmdDispInv_v1() {
	debugC(1, kDebugScripts, "cmdDispInv_v1()");
	_vm->_inventory->displayInv();
}

void Scripts::cmdDispInv_v2() {
	debugC(1, kDebugScripts, "cmdDispInv_v2()");
	_vm->_inventory->newDisplayInv();
	_vm->_events->forceSetCursor(CURSOR_ARROW);
}

void Scripts::cmdSetAbout() {
	int idx = _data->readByte();
	byte val = _data->readByte();

	if (idx >= ARRAYSIZE(_vm->_ask))
		error("Invalid index %d in cmdSetAbout", idx);

	debugC(1, kDebugScripts, "cmdSetAbout(idx=%d, val=%d)", idx, val);
	_vm->_ask[idx] = val;
	_vm->_startAboutBox = _vm->_startAboutItem = 0;
}

void Scripts::cmdSetTimer() {
	int idx = _data->readUint16LE();
	int val = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdSetTimer(idx=%d, val=%d)", idx, val);
	++_vm->_timers[idx]._flag;
	_vm->_timers[idx]._timer = val;
	_vm->_timers[idx]._initTm = val;

	_vm->_events->debounceLeft();
	_vm->_events->zeroKeysActions();
}

void Scripts::cmdCheckTimer() {
	int idx = _data->readUint16LE();
	debugCN(1, kDebugScripts, "cmdCheckTimer(idx=%d)", idx);

	_vm->_canSaveLoad = true;
	_vm->_events->pollEvents();
	_vm->_canSaveLoad = false;

	// Since the ScummVM debugger can be launched from the above point, we need
	// to check whether the script needs to be ended here, since some commands,
	// like the scene command, can change the current script
	if (_endFlag)
		return;

	if (_vm->getGameID() == kGameAmazon && idx == 9 && _vm->_events->isKeyActionPending()) {
		_vm->_events->zeroKeysActions();
		_vm->_timers[9]._timer = 0;
		_vm->_timers[9]._flag = 0;
	}

	byte val = (byte)_data->readUint16LE();

	if (val != 0 && val != 1)
        warning("Invalid check value %d in cmdCheckTimer - expect only 1 or 0??", val);

	if (_vm->_timers[idx]._flag == val) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdSetTravel() {
	int idx = _data->readByte();
	byte dest = _data->readByte();
	debugC(1, kDebugScripts, "cmdSetTravel(idx=%d, dest=%d)", idx, dest);
	_vm->_travel[idx] = dest;
	_vm->_startTravelItem = _vm->_startTravelBox = 0;
}

void Scripts::cmdJumpGoto() {
	debugCN(1, kDebugScripts, "cmdJumpGoto(selectCommand=%d)", _vm->_room->_selectCommand);
	if (_vm->_room->_selectCommand == 5) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdSetVideo() {
	Common::Point pt;
	pt.x = _data->readSint16LE();
	pt.y = _data->readSint16LE();
	int cellIndex = _data->readUint16LE();
	int rate = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdSetVideo(x=%d, y=%d, cellIndex=%d, rate=%d)", pt.x, pt.y, cellIndex, rate);
	_vm->_video->setVideo(_vm->_screen, pt, _vm->_extraCells[cellIndex]._vid, rate);
}

void Scripts::cmdPlayVideo() {
	debugC(1, kDebugScripts, "cmdPlayVideo()");
	_vm->_video->playVideo();
}

void Scripts::cmdPlotImage() {
	_vm->_destIn = _vm->_current;

	int destX = _data->readUint16LE();
	int destY = _data->readUint16LE();
	int objId = _data->readUint16LE();
	int imgId = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdPlotImage(destX=%d, destY=%d, objId=%d, imgId=%d)", destX, destY, objId, imgId);

	_vm->_current->plotImage(_vm->_objectsTable[objId], imgId, Common::Point(destX, destY));
}

void Scripts::cmdSetDisplay() {
	debugC(1, kDebugScripts, "cmdSetDisplay()");
	_vm->_screen->setDisplayScan();
	_vm->_current = _vm->_screen;
}

void Scripts::cmdSetBuffer() {
	debugC(1, kDebugScripts, "cmdSetBuffer()");
	_vm->_current = &_vm->_buffer2;
}

void Scripts::cmdSetScroll() {
	int scrollCol = _data->readUint16LE();
	int scrollRow = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdSetScroll(scrollCol=%d, scrollRow=%d)", scrollCol, scrollRow);
	_vm->_scrollCol = scrollCol;
	_vm->_scrollRow = scrollRow;
	_vm->_scrollX = 0;
	_vm->_scrollY = 0;
}

void Scripts::cmdSaveRect() {
	debugC(1, kDebugScripts, "cmdSaveRect()");
	int x = _vm->_screen->_lastBoundsX;
	int y = _vm->_screen->_lastBoundsY;
	int w = _vm->_screen->_lastBoundsW;
	int h = _vm->_screen->_lastBoundsH;
	assert(x >= 0 && y >= 0 && x < 320 && y < 200 && w > 0 && h > 0 && w <= 320 && h <= 200);
	_vm->_newRects.push_back(Common::Rect(x, y, x + w, y + h));
}

void Scripts::cmdVideoEnded() {
	debugCN(1, kDebugScripts, "cmdVideoEnded()");
	_vm->_events->pollEventsAndWait();

	// Slight enhancement over original - add ability to skip unskippable videos.
	bool skipVideo = false;
	Common::CustomEventType action;
	if (_vm->_events->getAction(action) && action == kActionSkip) {
		skipVideo = true;
		_vm->_sound->stopSound();
		_vm->_events->zeroKeysActions();
	}

	if (_vm->_video->_videoEnd || skipVideo) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdSetBufVid() {
	int vidX = _data->readUint16LE();
	int vidY = _data->readUint16LE();
	int idx = _data->readUint16LE();
	int rate = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdSetBufVid(vidX=%d, vidY=%d, idx=%d, rate=%d)", vidX, vidY, idx, rate);
	_vm->_vidX = vidX;
	_vm->_vidY = vidY;
	_vm->_video->setVideo(&_vm->_vidBuf, Common::Point(0, 0), FileIdent(_vm->_extraCells[idx]._vid._fileNum, _vm->_extraCells[idx]._vid._subfile), rate);
}

void Scripts::cmdPlayBufVid() {
	debugC(1, kDebugScripts, "cmdPlayBufVid()");
	_vm->_video->playVideo();
	_vm->_video->copyVideo();
}

void Scripts::cmdRemoveLast() {
	debugC(1, kDebugScripts, "cmdRemoveLast()");
	_vm->_animation->popBackTimer();
}

void Scripts::cmdDoTravel() {
	// MM only
	debugC(1, kDebugScripts, "cmdDoTravel()");
	while (true) {
		_vm->_travelBox->getList(Martian::TRAVDATA, _vm->_travel);
		int btnSelected = 0;
		int boxX = _vm->_travelBox->doBox_v1(_vm->_startTravelItem, _vm->_startTravelBox, btnSelected);
		_vm->_startTravelItem = _vm->_boxDataStart;
		_vm->_startTravelBox = _vm->_boxSelectY;

		if (boxX == -1)
			btnSelected = 2;

		if (btnSelected != 2) {
			int newRoomNum = _vm->_travelBox->_tempListIdx[boxX];
			if (!_vm->_cheatFl && (Martian::CAN_TRAVEL_MATRIX[newRoomNum] != _vm->_flags[171])) {
				_vm->_bubbleBox->_bubbleDisplStr = "TRAVEL";
				doCmdPrint_v1(_vm->_res->CANT_GET_THERE);
				continue;
			}
			if (_vm->_player->_roomNumber != newRoomNum) {
				_vm->_room->_function = FN_CLEAR1;
				if (_vm->_res->ROOMTBL[newRoomNum]._travelPos.x == -1) {
					// For x == -1, the y value is a script Id, not a co-ordinate
					_vm->_room->_conFlag = true;
					_vm->_player->_roomNumber = -1;
					_vm->_scripts->converse1(_vm->_res->ROOMTBL[newRoomNum]._travelPos.y);
					return;
				}
				_vm->_player->_roomNumber = newRoomNum;
				_vm->_player->_rawPlayer = _vm->_res->ROOMTBL[newRoomNum]._travelPos;
				cmdRetPos();
				return;
			}
		}

		if (_vm->_player->_roomNumber == -1)
			continue;

		return;
	}
}

void Scripts::cmdHelp_v1() {
	debugC(1, kDebugScripts, "cmdHelp_v1()");
	int idx = 0;
	for (int i = 0; i < 40; i++) {
		byte c = _data->readByte();
		if (c != 0xFF) {
			Common::String tmpStr = c + _data->readString();
			if (Martian::HELP[i]) {
				_vm->_helpBox->_tempList[idx] = tmpStr;
				_vm->_helpBox->_tempListIdx[idx] = i;
				++idx;
			}
		} else
			break;
	}
	_vm->_helpBox->_tempList[idx] = "";

	int btnSelected = 0;
	int boxX = _vm->_helpBox->doBox_v1(0, 0, btnSelected);

	if (boxX == -1)
		btnSelected = 2;

	if (btnSelected != 2)
		_vm->_useItem = _vm->_helpBox->_tempListIdx[boxX];
	else
		_vm->_useItem = -1;
}

void Scripts::cmdCheckAbout() {
	int idx = _data->readSint16LE();
	int16 val = _data->readSint16LE();
	debugCN(1, kDebugScripts, "cmdCheckAbout(idx=%d, val=%d)", idx, val);

	if (idx < 0 || idx >= ARRAYSIZE(_vm->_ask))
		error("Invalid index %d in cmdCheckAbout", idx);

	if ((byte)_vm->_ask[idx] == (byte)val) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdSpecial() {
	_specialFunction = _data->readUint16LE();
	int p1 = _data->readUint16LE();
	int p2 = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdSpecial(specialFunction=%d, p1=%d, p2=%d)", _specialFunction, p1, p2);

	if (_specialFunction == 1 || _vm->getGameID() == kGameMartianMemorandum) {
		if (_vm->getGameID() == kGameAmazon && _vm->_establishTable[p2])
			return;

		_vm->_screen->savePalette();
	}

	executeSpecial(_specialFunction, p1, p2);

	if (_specialFunction == 1 || _vm->getGameID() == kGameMartianMemorandum) {
		_vm->_screen->restorePalette();
		_vm->_room->_function = FN_RELOAD;

		// WORKAROUND: This fixes scene establishment being re-shown
		// when restoring savegames in rooms which have one
		if (_vm->getGameID() == kGameAmazon && !_vm->isCD())
			_vm->_establishTable[p2] = true;
	}
}

void Scripts::cmdSetCycle() {
	int startCycle = _data->readUint16LE();
	int endCycle = _data->readUint16LE();
	int timer = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdSetCycle(startCycle=%d, endCycle=%d, timer=%d)", startCycle, endCycle, timer);
	_vm->_screen->setPaletteCycle(startCycle, endCycle, timer);
}

void Scripts::cmdCycle() {
	debugC(1, kDebugScripts, "cmdCycle()");
	if (_vm->_startup == -1)
		_vm->_screen->cyclePaletteForward();
}

void Scripts::cmdCharSpeak() {
	_vm->_screen->_printOrg = _charsOrg;
	_vm->_screen->_printStart = _charsOrg;

	Common::String str = _data->readString();
	debugC(1, kDebugScripts, "cmdCharSpeak(str=\"%s\")", str.c_str());
	_vm->_bubbleBox->placeBubble(str);
	findNull();
}

void Scripts::cmdTexSpeak() {
	_vm->_screen->_printOrg = _texsOrg;
	_vm->_screen->_printStart = _texsOrg;
	_vm->_screen->_maxChars = (_vm->getGameID() == kGameMartianMemorandum) ? 23 : 20;

	Common::String str = _data->readString();
	debugC(1, kDebugScripts, "cmdTexSpeak(str=\"%s\")", str.c_str());

	_vm->_bubbleBox->_bubbleDisplStr = _vm->_res->getEgoName();
	_vm->_bubbleBox->placeBubble1(str);
	findNull();
}

void Scripts::cmdTexChoice() {
	debugC(1, kDebugScripts, "cmdTexChoice()");

	_vm->_oldRects.clear();
	_choiceStart = _data->pos() - 1;
	_vm->_fonts._charSet._lo = 1;
	_vm->_fonts._charSet._hi = 8;
	_vm->_fonts._charFor._hi = 255;

	if (_vm->getGameID() == kGameMartianMemorandum) {
		_vm->_fonts._charFor._lo = 247;
		_vm->_screen->_maxChars = 23;
	} else {
		_vm->_fonts._charFor._lo = 55;
		_vm->_screen->_maxChars = 20;
	}

	_vm->_screen->_printOrg = _texsOrg;
	_vm->_screen->_printStart = _texsOrg;

	_vm->_bubbleBox->clearBubbles();
	_vm->_bubbleBox->_bubbleDisplStr = "RESPONSE 1";

	Common::String tmpStr = _data->readString();

	_vm->_bubbleBox->calcBubble(tmpStr);
	_vm->_bubbleBox->printBubble(tmpStr);

	const int responseYOff = (_vm->getGameID() == kGameMartianMemorandum) ? 20 : 11;

	Common::Array<Common::Rect> responseCoords;
	responseCoords.push_back(_vm->_bubbleBox->_bounds);
	_vm->_screen->_printOrg.y = _vm->_bubbleBox->_bounds.bottom + responseYOff;

	findNull();

	tmpStr = _data->readString();

	if (!tmpStr.empty()) {
		_vm->_bubbleBox->_bubbleDisplStr = "RESPONSE 2";
		_vm->_bubbleBox->calcBubble(tmpStr);
		_vm->_bubbleBox->printBubble(tmpStr);
		responseCoords.push_back(_vm->_bubbleBox->_bounds);
		_vm->_screen->_printOrg.y = _vm->_bubbleBox->_bounds.bottom + responseYOff;
	}

	findNull();

	bool choice3Fl = false;
	tmpStr = _data->readString();

	if (!tmpStr.empty()) {
		_vm->_bubbleBox->_bubbleDisplStr = "RESPONSE 3";
		_vm->_bubbleBox->calcBubble(tmpStr);
		_vm->_bubbleBox->printBubble(tmpStr);
		responseCoords.push_back(_vm->_bubbleBox->_bounds);
		_vm->_screen->_printOrg.y = _vm->_bubbleBox->_bounds.bottom + responseYOff;
	}

	findNull();

	int choice = -1;
	do {
		_vm->_events->pollEvents();
		if (_vm->shouldQuit())
			return;

		charLoop();

		_vm->_bubbleBox->_bubbleDisplStr = _vm->_bubbleBox->_bubbleTitle;
		if (_vm->_events->_leftButton) {
			if (_vm->_events->_mouseRow >= ((_vm->getGameID() == kGameMartianMemorandum) ? 23 : 22)) {
				_vm->_events->debounceLeft();
				int x = _vm->_events->_mousePos.x;
				choice = _vm->_res->inButtonXRange(x);
			} else {
				_vm->_events->debounceLeft();
				choice = _vm->_events->checkMouseBox1(responseCoords);
			}
		}
	} while ((choice == -1) || ((choice == 2) && choice3Fl));

	_choice = choice + 1;
	_vm->_bubbleBox->clearBubbles();
}

void Scripts::cmdWait() {
	int time = _data->readSint16LE();
	debugC(1, kDebugScripts, "cmdWait(time=%d)", time);
	_vm->_timers[3]._timer = time;
	_vm->_timers[3]._initTm = time;
	_vm->_timers[3]._flag++;
	_vm->_events->zeroKeysActions();

	while (!_vm->shouldQuit() && !_vm->_events->isKeyActionMousePressed() &&
			_vm->_timers[3]._flag) {
		_vm->_midi->midiRepeat();
		charLoop();

		_vm->_events->pollEventsAndWait();
	}

	_vm->_events->debounceLeft();
	_vm->_events->zeroKeysActions();
}

void Scripts::cmdSetConPos() {
	int x = _data->readSint16LE();
	int y = _data->readSint16LE();
	debugC(1, kDebugScripts, "cmdSetConPos(x=%d, y=%d)", x, y);
	_charsOrg = Common::Point(x, y);

	x = _data->readSint16LE();
	y = _data->readSint16LE();
	_texsOrg = Common::Point(x, y);
}

void Scripts::cmdCheckVFrame() {
	int vframe = _data->readSint16LE();
	debugCN(1, kDebugScripts, "cmdCheckVFrame(vframe=%d)", vframe);
	if (_vm->_video->_videoFrame == vframe) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdJumpChoice() {
	int val = _data->readUint16LE();
	debugCN(1, kDebugScripts, "cmdJumpChoice(val=%d, choice=%d)", val, _choice);
	if ((byte)val == (byte)_choice) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdReturnChoice() {
	debugC(1, kDebugScripts, "cmdReturnChoice()");
	_data->seek(_choiceStart);
}

void Scripts::cmdClearBlock() {
	debugC(1, kDebugScripts, "cmdClearBlock()");
	_vm->_screen->restoreBlock();
}

void Scripts::cmdLoadSound() {
	int idx = _data->readSint16LE();
	debugC(1, kDebugScripts, "cmdLoadSound(idx=%d)", idx);
	_vm->_sound->_soundTable.clear();
	Resource *sound = _vm->_files->loadFile(_vm->_extraCells[idx]._vidSound);
	_vm->_sound->_soundTable.push_back(SoundEntry(sound, 1));
}

void Scripts::cmdFreeSound() {
	debugC(1, kDebugScripts, "cmdFreeSound()");
	SoundManager &sound = *_vm->_sound;

	if (sound._soundTable.size() > 0 && sound._soundTable[0]._res) {
		// Keep doing char display loop if playing sound for it
		do {
			if (_vm->getGameID() == kGameAmazon && _vm->_flags[236] == 1)
				charLoop();

			_vm->_events->pollEvents();
		} while (!_vm->shouldQuit() && sound.isSFXPlaying());

		// Free the sounds
		sound.freeSounds();
	}
}

void Scripts::cmdSetVideoSound() {
	debugC(1, kDebugScripts, "cmdSetVideoSound()");
	uint32 startPos = _data->pos();
	_data->skip(4);
	cmdLoadSound();
	_data->seek(startPos);
	cmdSetVideo();

	_vm->_video->_soundFrame = _data->readUint16LE();
	_vm->_video->_soundFlag = false;
}

void Scripts::cmdPlayVideoSound() {
	debugC(1, kDebugScripts, "cmdPlayVideoSound()");
	_vm->_video->playVideo();
	if (_vm->_video->_soundFrame == _vm->_video->_videoFrame &&
			!_vm->_video->_soundFlag) {
		_vm->_sound->playSound(0);
		_vm->_video->_soundFlag = true;
	}

	_vm->_events->pollEventsAndWait();
}

void Scripts::cmdPrintWatch() {
	debugC(1, kDebugScripts, "cmdPrintWatch()");
	printWatch();
	findNull();
}

void Scripts::cmdDispAbout() {
	debugC(1, kDebugScripts, "cmdDispAbout()");
	_vm->_aboutBox->getList(Martian::ASK_TBL, _vm->_ask);
	int btnSelected = 0;
	int boxX = _vm->_aboutBox->doBox_v1(_vm->_startAboutItem, _vm->_startAboutBox, btnSelected);
	_vm->_startAboutItem = _vm->_boxDataStart;
	_vm->_startAboutBox = _vm->_boxSelectY;
	if (boxX == -1)
		btnSelected = 2;
	if (btnSelected == 2)
		_vm->_useItem = -1;
	else
		_vm->_useItem = _vm->_aboutBox->_tempListIdx[boxX];
}

void Scripts::cmdPushLocation_v1() {
	// MM only
	debugC(1, kDebugScripts, "cmdPushLocation_v1()");
	_choiceStart = _data->pos() + 2;
	cmdGoto();
}

void Scripts::cmdPushLocation() {
	// TODO: Double-check this code.  It doesn't seem to match function
	//   sub13BC0 in Amazon?
	debugC(1, kDebugScripts, "cmdPushLocation()");
	_choiceStart = _data->pos() - 1;
}

void Scripts::cmdCheckTravel() {
	int idx = _data->readSint16LE();
	uint16 val = _data->readUint16LE();
	debugCN(1, kDebugScripts, "cmdCheckTravel(idx=%d, val=%d)", idx, val);

	if ((byte)_vm->_travel[idx] == (byte)val) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void Scripts::cmdBlock() {
	int x = _data->readSint16LE();
	int y = _data->readUint16LE();
	int w = _data->readSint16LE();
	int h = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdBlock(x=%d, y=%d, w=%d, h=%d)", x, y, w, h);
	_vm->_screen->blitFrom(_vm->_buffer2, Common::Rect(x, y, x + w, y + h), Common::Point(x, y));
}

void Scripts::cmdPlayerOff() {
	debugC(1, kDebugScripts, "cmdPlayerOff()");
	_vm->_player->_playerOff = true;
}

void Scripts::cmdPlayerOn() {
	debugC(1, kDebugScripts, "cmdPlayerOn()");
	_vm->_player->_playerOff = false;
}

void Scripts::cmdDead() {
	int deathId = _data->readByte();
	debugC(1, kDebugScripts, "cmdDead(deathId=%d)", deathId);
	_vm->dead(deathId);
}

void Scripts::cmdFadeOut() {
	debugC(1, kDebugScripts, "cmdFadeOut()");
	_vm->_screen->forceFadeOut();
}

void Scripts::cmdEndVideo() {
	debugC(1, kDebugScripts, "cmdEndVideo()");
	_vm->_video->closeVideo();
	_vm->_video->_videoEnd = true;
}

} // End of namespace Access
