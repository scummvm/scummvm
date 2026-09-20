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

#include "access/noctropolis/noctropolis_scripts.h"
#include "access/noctropolis/noctropolis_game.h"
#include "access/noctropolis/noctropolis_room.h"
#include "access/noctropolis/noctropolis_resources.h"

namespace Access {

namespace Noctropolis {

NoctropolisScripts::NoctropolisScripts(NoctropolisEngine *vm) : Scripts(vm),
_keepAskPosition(false) {
}

void NoctropolisScripts::setOpcodes() {
	#define Opcode(func) Common::SharedPtr<Common::Functor0<void>>( \
		new Common::Functor0Mem<void, NoctropolisScripts>(this, &NoctropolisScripts::func))

	Scripts::setOpcodes();

	_opcodes.resize(96);

	// Modify command list for Noctropolis
	_opcodes[1] = Opcode(cmdEndObject_v3);
	_opcodes[6] = Opcode(cmdJumpUse_v3);
	_opcodes[7] = Opcode(cmdJumpTalk_v3);
	_opcodes[9] = Opcode(cmdPrint_v2);
	_opcodes[18] = Opcode(cmdSetCoords);
	_opcodes[33] = Opcode(cmdJumpGoto_v3);
	_opcodes[34] = Opcode(cmdSetVideo_v3);
	_opcodes[45] = Opcode(cmdDoTravel_Noct);
	_opcodes[50] = Opcode(cmdCharSpeak_v3);
	_opcodes[51] = Opcode(cmdPlayerSpeak);
	_opcodes[52] = Opcode(cmdPlayerChoice);
	// 52 to 57 unchanged
	_opcodes[57] = Opcode(cmdDigitalPlay);
	_opcodes[60] = Opcode(cmdFillSound);
	_opcodes[61] = Opcode(cmdBD);
	_opcodes[62] = Opcode(cmdPlayVid1);
	_opcodes[63] = Opcode(cmdNull); // empty function CmdPRINTWITHOUT
	_opcodes[64] = Opcode(cmdDispAbout_v3);
	_opcodes[71] = Opcode(cmdFadeOut_v3);
	_opcodes[74] = Opcode(cmdCharWait);
	_opcodes[75] = Opcode(cmdUndoText);
	_opcodes[76] = Opcode(cmdResetAnim);
	_opcodes[77] = Opcode(cmdWalkTo);
	_opcodes[78] = Opcode(cmdWalkCheck);
	_opcodes[79] = Opcode(cmdSoundEnd);
	_opcodes[80] = Opcode(cmdFadeWhite);
	_opcodes[81] = Opcode(cmdGotoFrame);
	_opcodes[82] = Opcode(cmdPlayerScale);
	_opcodes[83] = Opcode(cmdRestoreBlock);
	_opcodes[84] = Opcode(cmdCopyScnBuf);
	_opcodes[85] = Opcode(cmdStilWalkTo);
	_opcodes[86] = Opcode(cmdStilWalkCheck);
	_opcodes[87] = Opcode(cmdStilOff);
	_opcodes[88] = Opcode(cmdStilOn);
	_opcodes[89] = Opcode(cmdReturnExit);
	_opcodes[90] = Opcode(cmdSetStilCoords);
	_opcodes[91] = Opcode(cmdSetPlayerDir);
	_opcodes[92] = Opcode(cmdSetStilDir);
	_opcodes[93] = Opcode(cmdStilScale);
	_opcodes[94] = Opcode(cmdLockInterface);
	_opcodes[95] = Opcode(cmdUnlockInterface);
}


static int COMMAND_ARG_SIZES[] = {
	-1, 0, 2, 2, 2, 2, 2, 2, 2, -1,
	0, 1, 2, 6, 2, 1, 2, 6, 4, 1,
	2, 6, 4, 1, 0, 0, 10, 1, 0, 2,
	4, 6, 2, 2, 8, 0, 8, 0, 0, 4,
	0, 2, 8, 0, 0, 0, 6, 6, 6, 0,
	-2, -2, -3, 2, 8, 4, 4, 0, 0, 2,
	0, 10, 0, -1, 0, 2, 6, 8, 0, 0,
	1, 1, 0, 0, -2, 0, 1, 6, 2, 2,
	0, 4, 2, 8, 0, 6, 2, 0, 0, 0,
	4, 1, 1, 2, 0, 0,
};

void NoctropolisScripts::skipCommand(byte cmd) {
	assert(cmd < ARRAYSIZE(COMMAND_ARG_SIZES));
	int argsize = COMMAND_ARG_SIZES[cmd];
	switch (argsize) {
	case -1:
		_data->readString();
		break;
	case -2:
		_data->readUint16LE();
		_data->readUint16LE();
		_data->readString();
		break;
	case -3:
		_data->readUint16LE();
		_data->readUint16LE();
		_data->readString();
		_data->readString();
		_data->readString();
		_data->readString();
		_data->readString();
		_data->readString();
		break;
	default:
		_data->seek(argsize, SEEK_CUR);
	}
}

//
// Unlike the previous games which just stop at any 0xE0 (SCRIPT_START_BYTE),
// Noctropolis skips to the next command using argument size.  This avoids
// mistakes from the 0xE0 appearing accidentally in arguments.
//
void NoctropolisScripts::searchForSequence() {
	assert(_data);

	_data->seek(0);
	int sequenceId;
	while (_data->pos() < _data->size()) {
		byte command = _data->readByte();
		if (command == 0xe0) {
			sequenceId = _data->readUint16LE();
			if (sequenceId == _sequence)
				break;
		} else {
			skipCommand(command ^ 0x80);
		}
	}
	if (_data->pos() == _data->size()) {
		warning("Hit EOS when searching for sequence %d", _sequence);
		_endFlag = true;
	}
}

void NoctropolisScripts::cmdEndObject_v3() {
	if (_vm->_room->_selectCommand == Noctropolis::kNoctCmdTalk && _vm->_boxSelectY == 0x62) {

		// Note: removed "DARK/" from these..
		const char *vidfile = "VID/A4IN00.VID";
		if (_vm->_flags[91] == 2) {
			vidfile = "VID/A4IN26.VID";
		}

		if (_vm->_textFlag) {
			Noctropolis::NoctropolisResources *res = (Noctropolis::NoctropolisResources *)_vm->_res;
			const char *subtitle = res->getEndMessage();
			if (_vm->_flags[91] == 2) {
				subtitle = res->getStilMessage();
			}
			_vm->_bubbleBox->_type = (BoxType)(kTextBoxNoctCaption | kTextBoxNoctPlain);
			_vm->_bubbleBox->placeBubble(subtitle);
		}

		debug("cmdEndObject_v3: TODO: duck music volume to 50%% here.");

		VideoPlayer_v2 vidPlayer(_vm);
		vidPlayer.VideoPlayer::setVideo(_vm->_screen, Common::Point(100, 100), Common::Path(vidfile), 0);
		vidPlayer.playToEnd();

		_vm->_bubbleBox->clearBubbles();

	} else {
		const char *msg = _vm->_res->getGeneralMessage(_vm->_room->_selectCommand);
		debugC(1, kDebugScripts, "cmdEndObject_v3(msg=\"%s\")", msg);

		_vm->_bubbleBox->_type = (BoxType)(kTextBoxNoctCaption | kTextBoxNoctCenter);
		if (_vm->_player->_roomNumber <= 4)
			_vm->_bubbleBox->_type = (BoxType)(_vm->_bubbleBox->_type | kTextBoxNoctPlain);

		printString(msg);
	}
	_continuenceFlag = true;
	_endFlag = true;
	_returnCode = 0;
}

void NoctropolisScripts::cmdJumpUse_v3() {
	debugCN(1, kDebugScripts, "cmdJumpUse_v3(selectCommand=%d)", _vm->_room->_selectCommand);
	if (_vm->_room->_selectCommand == Noctropolis::kNoctCmdUse) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void NoctropolisScripts::cmdJumpTalk_v3() {
	debugCN(1, kDebugScripts, "cmdJumpTalk_v3(selectCommand=%d)", _vm->_room->_selectCommand);
	if (_vm->_room->_selectCommand == Noctropolis::kNoctCmdTalk) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void NoctropolisScripts::cmdJumpGoto_v3() {
	debugCN(1, kDebugScripts, "cmdJumpGoto_v3(selectCommand=%d)", _vm->_room->_selectCommand);
	if (_vm->_room->_selectCommand == Noctropolis::kNoctCmdGoto) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void NoctropolisScripts::cmdSetVideo_v3() {
	Common::Point pt;
	short rawx = _data->readSint16LE();
	pt.y = _data->readSint16LE();
	int cellIndex = _data->readUint16LE();
	int noFrame = _data->readUint16LE();

	if (cellIndex > 0x3f)
		error("Invalid room video number %d", cellIndex);

	const int roomNum = _vm->_player->_roomNumber;
#if 0
	bool flag;

	if ((roomNum == 0x1e && cellIndex == 0) || roomNum == 0x21 || (roomNum == 0x36 && cellIndex < 32))
		flag = true;
	else
		flag = false;
#endif

	if (roomNum == 0x1b || roomNum == 0x1e)
		pt.x = rawx + -5;
	else
		pt.x = rawx;

	debugC(1, kDebugScripts, "cmdSetVideo_v3(x=%d, y=%d, cellIndex=%d, noFrame=%d)", pt.x, pt.y, cellIndex, noFrame);

	Common::Path vidpath(_vm->_extraCells[cellIndex]._vidFilename);
	if (!_vm->_files->existFile(vidpath)) {
		// Try removing "DARK/" from video file names -
		// for the demo the are in DEMO/
		Common::StringArray parts = vidpath.splitComponents();
		vidpath = Common::Path::joinComponents(parts.begin() + 1, parts.end());
	}
	_vm->_video->setVideo(_vm->_screen, pt, vidpath, 0);

	((VideoPlayer_v2 *)_vm->_video)->setDrawBorder(noFrame == 0);

	if (cellIndex == 1 && roomNum == 0x36)
		_vm->_screen->setIconPalette();
}

void NoctropolisScripts::cmdCharSpeak_v3() {
	int16 x = _data->readUint16LE();
	int16 y = _data->readUint16LE();
	_charsOrg = Common::Point(x, y);
	_vm->_screen->_printOrg = _charsOrg;
	_vm->_screen->_printStart = _charsOrg;

	//debug("cmdCharSpeak_v3: duck music volume to 50%% here.");

	Common::String str = _data->readString();
	debugC(1, kDebugScripts, "cmdCharSpeak(%d, %d, str=\"%s\")", x, y, str.c_str());
	if (_vm->_textFlag) {
		_vm->_bubbleBox->_type = (BoxType)(kTextBoxNoctCaption | kTextBoxNoctPlain);
		_vm->_bubbleBox->placeBubble(str);
	}
	findNull();
}

void NoctropolisScripts::cmdDispAbout_v3() {
	// Graphical ask box for Noctropolis
	debugC(1, kDebugScripts, "cmdDispAbout_v3()");

	static constexpr struct { int16 x1, y1, x2, y2; } askItemPos[] = {
		{84, 0, 126, 48},
		{42, 24, 84, 72},
		{126, 24, 168, 72},
		{0, 48, 42, 96},
		{84, 48, 126, 96},
		{168, 48, 210, 96},
		{42, 72, 84, 120},
		{126, 72, 168, 120},
		{18, 102, 40, 120},
		{186, 102, 208, 120}
	};
	bool menuUpArrow = false;
	bool menuDownArrow = false;
	int hoveredItem = -1, slotIndex;
	byte slotItems[20];
	Common::Array<byte> items;
	int selectedItem = -2;
	bool needRedraw = true;

	Common::Path path = ((Noctropolis::NoctropolisResources *)_vm->_res)->translatePath("DARK/ASK.AP");
	Resource *spriteData = _vm->_files->loadRawFile(path);
	SpriteResource *askSprites = new SpriteResource(_vm, spriteData);
	delete spriteData;

	if (!_keepAskPosition) {
		for (int i = 0; i < 40; i++)
			_vm->_asked[i] = false;

		int16 warpMouseX, warpMouseY;
		_askBase.x = _vm->_events->clipMouseCenter(_vm->_events->_mousePos.x, 226, 640, warpMouseX);
		_askBase.y = _vm->_events->clipMouseCenter(_vm->_events->_mousePos.y, 145, 400, warpMouseY);

		_keepAskPosition = true;
		if (warpMouseX != _vm->_events->_mousePos.x || warpMouseY != _vm->_events->_mousePos.y)
			g_system->warpMouse(warpMouseX, warpMouseY);
	}

	const Common::Point base = _askBase;

	// Build the list of available ask items
	for (int i = 0; i < 40; i++) {
		if (_vm->_ask[i] == 1)
			items.push_back(i);
	}

	// Grab screen background
	_vm->_screen->saveBlock(Common::Rect(base, 226, 145));
	// Draw ask panel
	_vm->_screen->plotImage(askSprites, 0, Common::Point(base.x, base.y + 96));

	_vm->_events->setCursor(CURSOR_ARROW);

	const Font *font = _vm->_fonts.getFont(1);
	Font::_fontColors[0] = 246;
	Font::_fontColors[1] = 238;

	Noctropolis::NoctropolisResources *res = (Noctropolis::NoctropolisResources *)_vm->_res;

	while (selectedItem == -2 && !_vm->shouldQuitOrRestart()) {
		int item = -1;

		if (needRedraw) {
			hoveredItem = -1;
			menuUpArrow = false;
			menuDownArrow = false;
			slotIndex = 0;
			if (items.size() > 0) {
				// Draw "Up" arrow if required
				menuUpArrow = _vm->_startAboutItem > 0;
				_vm->_screen->plotImage(askSprites, menuUpArrow ? 1 : 3, Common::Point(base.x + 18, base.y + 102));
				// Draw ask items
				for (uint i = 0; i < 8 && _vm->_startAboutItem + i < items.size(); i++) {
					slotItems[slotIndex] = items[_vm->_startAboutItem + i];
					_vm->_screen->plotImage(askSprites, slotItems[slotIndex] + 6, Common::Point(base.x + askItemPos[slotIndex].x1, base.y + askItemPos[slotIndex].y1));
					if (_vm->_asked[slotItems[slotIndex]]) {
						_vm->_screen->plotImage(askSprites, 5, Common::Point(base.x + askItemPos[slotIndex].x1, base.y + askItemPos[slotIndex].y1));
					}
					slotIndex++;
				}
				// Draw "Down" arrow if required
				menuDownArrow = (int)items.size() > _vm->_startAboutItem + 8;
				_vm->_screen->plotImage(askSprites, menuDownArrow ? 2 : 3, Common::Point(base.x + 186, base.y + 102));
			}
			// Draw remaining empty slots if any
			while (slotIndex < 8) {
				slotItems[slotIndex] = 0;
				_vm->_screen->plotImage(askSprites, 4, Common::Point(base.x + askItemPos[slotIndex].x1, base.y + askItemPos[slotIndex].y1));
				slotIndex++;
			}
			needRedraw = false;
		}

		// Find at which slot the mouse is
		for (int i = 0; i < 10; i++) {
			if (_vm->_events->_mousePos.x >= base.x + askItemPos[i].x1 && _vm->_events->_mousePos.x <= base.x + askItemPos[i].x2 &&
				_vm->_events->_mousePos.y >= base.y + askItemPos[i].y1 && _vm->_events->_mousePos.y <= base.y + askItemPos[i].y2) {
				item = i;
				break;
			}
		}

		if (hoveredItem != item) {
			_vm->_screen->fillRect(Common::Rect(Common::Point(base.x + 41, base.y + 128), 150, 8), 246);
			if ((item == 8 && menuUpArrow) || (item == 9 && menuDownArrow)) {
				font->drawString(_vm->_screen, res->getMoreItemsText(), Common::Point(base.x + 41, base.y + 128));
			} else if (item >= 0 && item < 8) {
				font->drawString(_vm->_screen, res->getAskItem(slotItems[item]), Common::Point(base.x + 41, base.y + 128));
			}
			hoveredItem = item;
		}

		// Support the mouse wheel for scrolling through the ask items
		if (menuUpArrow && _vm->_events->_wheelUp) {
			_vm->_startAboutItem--;
			needRedraw = true;
		} else if (menuDownArrow && _vm->_events->_wheelDown) {
			_vm->_startAboutItem++;
			needRedraw = true;
		}

		if (_vm->_events->_leftButton) {
			_vm->_events->debounceLeft();
			if (hoveredItem == 8 && menuUpArrow) {
				_vm->_startAboutItem--;
				needRedraw = true;
			} else if (hoveredItem == 9 && menuDownArrow) {
				_vm->_startAboutItem++;
				needRedraw = true;
			} else if (hoveredItem >= 0 && hoveredItem < 8) {
				if (_vm->_startAboutItem + hoveredItem > (int)items.size()) {
					// An empty slot was clicked
					selectedItem = -1;
				} else {
					selectedItem = slotItems[hoveredItem];
					_vm->_asked[selectedItem] = true;
				}
			}
		} else if (_vm->_events->_rightButton) {
			_vm->_events->debounceRight();
			selectedItem = 255;
			_keepAskPosition = false;
		}

		_vm->_events->pollEventsAndWait();
	}

	_vm->_useItem = selectedItem;

	// Restore screen background
	_vm->_screen->restoreBlock();

	_continuenceFlag = true;
}


void NoctropolisScripts::cmdFadeOut_v3() {
	byte flag = _data->readByte();
	debugC(1, kDebugScripts, "cmdFadeOut_v3(%d)", flag);
	if (flag)
		_vm->_screen->forceFadeIn();
	else
		_vm->_screen->forceFadeOut();
	_continuenceFlag = true;
}

void NoctropolisScripts::cmdDoTravel_Noct() {
	((Noctropolis::NoctropolisEngine *)_vm)->doTravel();
	cmdRetPos();
}

void NoctropolisScripts::cmdDigitalPlay() {
	// Wait until the current sound has finished playing, then
	// if subtitles are enabled wait for input
	debugC(1, kDebugScripts, "cmdDigitalPlay()");
	while (_vm->_sound->isSFXPlaying() && !_vm->shouldQuit()) {
		_vm->_events->pollEventsAndWait();
	}

	_continuenceFlag = true;
}

void NoctropolisScripts::cmdFillSound() {
	debugC(1, kDebugScripts, "cmdFillSound()");
	while (_vm->_sound->isSFXPlaying() && !_vm->shouldQuit()) {
		_vm->_events->pollEventsAndWait();
		Common::CustomEventType action;
		if (_vm->_events->getAction(action) && action == kActionSkip) {
			_vm->_sound->stopSound();
			_vm->_events->zeroKeysActions();
			break;
		}
	}
	_continuenceFlag = true;
}

void NoctropolisScripts::cmdPlayVid1() {
	// TODO: should this mark the video finished if it's not loaded?
	_vm->_video->playVideo();
	_continuenceFlag = true;
}

void NoctropolisScripts::cmdCharWait() {
	int x = _data->readSint16LE();
	int y = _data->readUint16LE();
	Common::String msg = _data->readString();
	debugC(1, kDebugScripts, "cmdCharWait(%d, %d, '%s')", x, y, msg.c_str());

	printString(msg);
	_continuenceFlag = true;
}

void NoctropolisScripts::cmdUndoText() {
	debugC(1, kDebugScripts, "cmdUndoText()");

	// TODO: Restore music volume to 100%

	if (_vm->_textFlag)
		_vm->_bubbleBox->clearBubbles();
}

void NoctropolisScripts::cmdResetAnim() {
	byte animNum = _data->readByte();
	debugC(1, kDebugScripts, "cmdResetAnim(%d)", animNum);
	Animation *anim = _vm->_animation->findAnimation(animNum);
	if (!anim)
		error("cmdResetAnim: Invalid anim num %d", animNum);

	anim->_countdownTicks = anim->_initialTicks;
	anim->_frameNumber = 0;
	if (anim->_type == 3 || anim->_type == 4) {
		anim->_currentLoopCount = anim->_loopCount;
	} else {
		anim->_currentLoopCount = 0;
	}
}

void NoctropolisScripts::cmdWalkTo() {
	int16 x = _data->readSint16LE();
	int16 y = _data->readSint16LE();
	int16 dir = _data->readSint16LE();
	debugC(1, kDebugScripts, "cmdWalkTo(%d, %d, %d)", x, y, dir);

	_vm->_player->_moveTo.x = x;
	_vm->_player->_moveTo.y = y;
	_vm->_player->setDirFromScript(dir);
	_vm->_player->_move = (Direction)dir;
	_vm->_player->_playerMove = true;
}

void NoctropolisScripts::cmdWalkCheck() {
	debugCN(1, kDebugScripts, "cmdWalkCheck()");
	if (!_vm->_player->_playerMove) {
		// Walk is done
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}


void NoctropolisScripts::cmdPlayerScale() {
	_vm->_manScaleOff = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdPlayerScale(%d)", _vm->_manScaleOff);
}

void NoctropolisScripts::cmdPlayerSpeak() {
	int16 x = _data->readUint16LE();
	int16 y = _data->readUint16LE();

	const char *title = _vm->_res->getEgoName();
	Common::String str = _data->readString();
	debugC(1, kDebugScripts, "cmdPlayerSpeak(%d, %d, \"%s\", \"%s\")", x, y, title, str.c_str());

	_charsOrg = Common::Point(x, y);
	_vm->_screen->_printOrg = _charsOrg;
	_vm->_screen->_printStart = _charsOrg;
	_vm->_bubbleBox->_type = (BoxType)(kTextBoxNoctCaption | kTextBoxNoctPlain);
	_vm->_bubbleBox->_bubbleDisplStr = title;

	_vm->_bubbleBox->placeBubble(str);
	_continuenceFlag = true;
	findNull();

	while (!_vm->shouldQuitOrRestart() && !_vm->_events->isKeyActionMousePressed()) {
		_vm->_events->pollEventsAndWait();
	}

	_vm->_events->debounceLeft();
	_vm->_events->zeroKeysActions();
	_vm->_bubbleBox->clearBubbles();
}


void NoctropolisScripts::cmdPlayerChoice() {
	// This is similar to cmdTexChoice but used in Noctropolis
	assert(_vm->gameIs(kGameNoctropolis));

	int16 x = _data->readUint16LE();
	int16 startY = _data->readUint16LE();

	Common::Array<Common::String> choiceStrs;
	for (int i = 0; i < 6; i++) {
		Common::String tmpStr;
		tmpStr = _data->readString();
		if (!tmpStr.empty())
			choiceStrs.push_back(tmpStr);
	}

	debugC(1, kDebugScripts, "cmdPlayerChoice(%d, %d, ..(%d choices)..)", x, startY, choiceStrs.size());

	_vm->_screen->_printStart = _vm->_screen->_printOrg = Common::Point(x, startY);

	_vm->_events->setCursor(CURSOR_ARROW);
	const char *respTitle = ((Noctropolis::NoctropolisResources *)_vm->_res)->getResponseTitle();
	Common::Array<Common::Rect> responseCoords;

	for (uint i = 0; i < choiceStrs.size(); i++) {
		_vm->_bubbleBox->_bubbleDisplStr = Common::String::format(respTitle, i + 1);
		_vm->_bubbleBox->_type = (BoxType)(kTextBoxNoctCaption | kTextBoxNoctPlain);
		_vm->_bubbleBox->calcBubble(choiceStrs[i]);
		_vm->_bubbleBox->printBubble(choiceStrs[i]);
		responseCoords.push_back(_vm->_bubbleBox->_bounds);
		_vm->_screen->_printOrg.y = _vm->_bubbleBox->_bounds.bottom + 10;
		_vm->_screen->_printOrg.x = _vm->_screen->_printStart.x;
	}

	_vm->_bubbleBox->_bubbleDisplStr = _vm->_bubbleBox->_bubbleTitle;

	int choice = -1;
	do {
		_vm->_events->pollEvents();
		if (_vm->shouldQuit())
			return;

		if (_vm->_events->_leftButton) {
			_vm->_events->debounceLeft();
			choice = _vm->_events->checkMouseBox1(responseCoords);
		}
	} while (choice == -1);

	_choice = choice + 1;
	_vm->_bubbleBox->clearBubbles();

	_continuenceFlag = true;
}

void NoctropolisScripts::cmdSoundEnd() {
	debugCN(1, kDebugScripts, "cmdSoundEnd()");
	if (!_vm->_sound->isSFXPlaying()) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void NoctropolisScripts::cmdFadeWhite() {
	debugC(1, kDebugScripts, "cmdFadeWhite()");
	_vm->_screen->forceFadeWhite();
	_continuenceFlag = true;
}

void NoctropolisScripts::cmdGotoFrame() {
	uint16 animNum = _data->readUint16LE();
	uint16 frameNum = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdGotoFrame(%d, %d)", animNum, frameNum);
	Animation *anim = _vm->_animation->findAnimation(animNum);
	if (!anim)
		error("cmdGotoFrame: Invalid anim num %d", animNum);
	anim->_frameNumber = frameNum;
}

void NoctropolisScripts::cmdRestoreBlock() {
	int16 x = _data->readSint16LE();
	int16 y = _data->readSint16LE();
	int16 w = _data->readSint16LE();
	int16 h = _data->readSint16LE();
	debugC(1, kDebugScripts, "cmdRestoreBlock(%d, %d, %d, %d)", x, y, w, h);
	Common::Rect r(Common::Point(x, y), w, h);
	r.translate(-_vm->_screen->_windowXAdd, -_vm->_screen->_windowYAdd);
	r.clip(_vm->_screen->w, _vm->_screen->h);
	_vm->_screen->copyBlock(&_vm->_buffer2, r);

	// Remake does this, but it doesn't use same buffer setup..
	//_vm->clearPlotImagesIn(x, y, w, h);
	//_vm->clearPlotVidsIn(x, y, w, h);
}

void NoctropolisScripts::cmdCopyScnBuf() {
	debugC(1, kDebugScripts, "cmdCopyScnBuf()");

	// Copy the screen to the buffer, applying windowing offsets

	//
	// WORKAROUND: This causes double-drawing in the noct credits?
	// Just skip for now which causes a small graphical glitch after
	// showing the colored bars video.
	//
	if (_vm->_player->_roomNumber == 0x61)
		return;

	Common::Rect src(_vm->_screen->w, _vm->_screen->h);
	const Common::Rect screenSize = src;
	src.translate(_vm->_screen->_windowXAdd, _vm->_screen->_windowYAdd + _vm->_screen->_screenYOff);
	src.clip(screenSize);
	Common::Rect dest = src;
	dest.translate(-dest.left, -dest.top);
	_vm->_buffer2.blitFrom(*_vm->_screen, src, dest);
}

void NoctropolisScripts::cmdStilWalkTo() {
	int16 x = _data->readSint16LE();
	int16 y = _data->readSint16LE();
	int16 dir = _data->readSint16LE();
	debugC(1, kDebugScripts, "cmdStilWalkTo(%d, %d, %d)", x, y, dir);

	((Noctropolis::NoctropolisEngine*)_vm)->_stil->_moveTo.x = x;
	((Noctropolis::NoctropolisEngine*)_vm)->_stil->_moveTo.y = y;
	((Noctropolis::NoctropolisEngine*)_vm)->_stil->setDirFromScript(dir);
	((Noctropolis::NoctropolisEngine*)_vm)->_stil->_move = (Direction)dir;
	((Noctropolis::NoctropolisEngine*)_vm)->_stil->_playerMove = true;
}

void NoctropolisScripts::cmdStilWalkCheck() {
	debugCN(1, kDebugScripts, "cmdStilWalkCheck()");
	if (!((Noctropolis::NoctropolisEngine*)_vm)->_stil->_playerMove) {
		debugC(1, kDebugScripts, " -> True");
		cmdGoto();
	} else {
		debugC(1, kDebugScripts, " -> False");
		_data->skip(2);
	}
}

void NoctropolisScripts::cmdStilOff() {
	debugC(1, kDebugScripts, "cmdStilOff()");
	((Noctropolis::NoctropolisEngine *)_vm)->_stil->_playerOff = true;
}

void NoctropolisScripts::cmdStilOn() {
	debugC(1, kDebugScripts, "cmdStilOn()");
	((Noctropolis::NoctropolisEngine *)_vm)->_stil->_playerOff = false;
}

void NoctropolisScripts::cmdReturnExit() {
	debugC(1, kDebugScripts, "cmdReturnExit()");
	_vm->_exitBox = true;
	_endFlag = true;
	_returnCode = 0;
}

void NoctropolisScripts::cmdSetCoords() {
	const int x = _data->readSint16LE();
	const int y = _data->readSint16LE();
	debugC(1, kDebugScripts, "cmdSetCoords(x=%d, y=%d)", x, y);
	_vm->_player->_rawPlayer.x = x;
	_vm->_player->_moveTo.x = x;
	_vm->_player->checkScroll();
	bool hscroll = _vm->_player->_scrollFlag;
	_vm->_player->_rawPlayer.y = y;
	_vm->_player->_moveTo.y = y;
	_vm->_player->checkScroll();
	_vm->_player->_scrollFlag |= hscroll;
}

void NoctropolisScripts::cmdSetStilCoords() {
	const int x = _data->readSint16LE();
	const int y = _data->readSint16LE();
	debugC(1, kDebugScripts, "cmdSetStilCoords(x=%d, y=%d)", x, y);
	Player *stil = ((Noctropolis::NoctropolisEngine *)_vm)->_stil;
	stil->_rawPlayer.x = x;
	stil->_moveTo.x = x;
	stil->_rawPlayer.y = y;
	stil->_moveTo.y = y;
}

void NoctropolisScripts::cmdSetPlayerDir() {
	byte dir = _data->readByte();
	debugC(1, kDebugScripts, "cmdSetPlayerDir(%d)", dir);
	_vm->_player->setDirFromScript(dir);
}

void NoctropolisScripts::cmdSetStilDir() {
	byte dir = _data->readByte();
	debugC(1, kDebugScripts, "cmdSetStilDir(%d)", dir);
	((Noctropolis::NoctropolisEngine *)_vm)->_stil->setDirFromScript(dir);
}

void NoctropolisScripts::cmdStilScale() {
	short scale = _data->readUint16LE();
	debugC(1, kDebugScripts, "cmdStilScale(%d)", scale);
	_vm->_stilScaleOff = scale;
}

void NoctropolisScripts::cmdLockInterface() {
	debugC(1, kDebugScripts, "cmdLockInterface()");
	_vm->_events->_interfaceOff = true;
	_vm->_events->setCursor(CURSOR_DARK_ANKH);
}

void NoctropolisScripts::cmdUnlockInterface() {
	debugC(1, kDebugScripts, "cmdUnlockInterface()");
	_vm->_events->_interfaceOff = false;
	debug("TODO: cmdUnlockInterface - restore cursor");
	_vm->_events->setCursor(CURSOR_ARROW);
}


bool NoctropolisScripts::executeSpecial(int commandIndex, int param1, int param2) {
	NoctropolisEngine *vm = ((NoctropolisEngine *)_vm);
	bool result = false;
	switch (commandIndex) {
	case 1:
		// Clear screen and restore pal here??
		vm->_screen->fadeOutThenClearAndSetPal();
		vm->_buffer2.clear();
		result = true;
		if (_vm->_player->_roomNumber == 54) {
			warning("TODO: Implement special pal hack for fade in room 54?");
			/*
			NoctPal_ClearRawPalette();
			pbVar4 = GetRawPalette();
			uVar3 = GetRawPaletteHash();
			RB_GetPlayfieldTexture(pbVar4,uVar3);
			NoctRoomEngine::clearPlotVids(&_gNoctRoomEngine);
			*/
		}
		break;
	case 3:
		vm->displayPegsTick();
		break;
	case 4:
		vm->movePeg(param1); // todo: should it be param2?
		break;
	case 5:
		vm->playSuccubusAttack();
		break;
	case 6: // Set player model
		((NoctropolisRoom *)vm->_room)->loadPlayer1();
		_vm->_player->load();
		_vm->_screen->setManPalette();
		_vm->_screen->setPalette();
		break;
	case 7:
		vm->_midi->loadMusic(98, param1);
		break;
	case 8:
		vm->_screen->setDarkPalette(param1 | (param1 << 8), 0, 236);
		break;
	case 9:
		vm->_screen->setDarkPalette(0x3200, 192, 43);
		break;
	case 10:
		vm->playStilMorph();
		break;
	case 11:
		vm->showComicCover();
		break;
	case 12:
		vm->playSuccubusSplit();
		break;
	case 13:
		vm->shotoMeanwhile();
		break;
	case 14:
		// Load some temp music
		_vm->_midi->newMusic(param1, 0);
		break;
	case 15:
		// Resume previous music
		if (!_vm->_midi->isPlaying())
			_vm->_midi->newMusic(0, 1);
		break;
	case 16:
		// This gets called at the start of the end credits
		((VideoPlayer_v2 *)_vm->_video)->setVideoPalNow();
		break;
	case 17:
		vm->showNightdiveCredits();
		_vm->_restartFl = true;
		break;
	case 18:
		// End of demo
		vm->doUpsell();
		_vm->quitGame();
		break;
	case 19:
		warning("TODO: work our correct step on Special 19 (flashpalette)");
		_vm->_screen->flashPalette(20);
		break;
	case 20:
		vm->doLastComic();
		result = true;
		break;
	case 21:
		vm->doSpecialComic();
		result = true;
		break;
	default:
		error("Unknown special %d in NoctropolisScripts::executeSpecial", commandIndex);
	}
	return result;
}


} // end namespace Noctropolis

} // end namespace Access
