/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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

// Item script opcodes for Simon1/Simon2

#include "common/stdafx.h"
#include "simon/simon.h"
#include "simon/intern.h"

#include "common/system.h"

#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

namespace Simon {

int SimonEngine::runScript() {
	byte opcode;
	bool flag, condition;

	do {
		if (_continousMainScript)
			dumpOpcode(_codePtr);

		opcode = getByte();
		if (opcode == 0xFF)
			return 0;

		if (_runScriptReturn1)
			return 1;

		/* Invert condition? */
		flag = false;
		if (opcode == 0) {
			flag = true;
			opcode = getByte();
			if (opcode == 0xFF)
				return 0;
		}

		condition = true;

		switch (opcode) {
		case 1:{										/* ptrA parent is */
				condition = (getItem1Ptr()->parent == getNextItemID());
			}
			break;

		case 2:{										/* ptrA parent is not */
				condition = (getItem1Ptr()->parent != getNextItemID());
			}
			break;

		case 5:{										/* parent is 1 */
				condition = (getNextItemPtr()->parent == getItem1ID());
			}
			break;

		case 6:{										/* parent isnot 1 */
				condition = (getNextItemPtr()->parent != getItem1ID());
			}
			break;

		case 7:{										/* parent is */
				Item *item = getNextItemPtr();
				condition = (item->parent == getNextItemID());
			}
			break;

		case 11:{									/* is zero */
				condition = (getNextVarContents() == 0);
			}
			break;

		case 12:{									/* isnot zero */
				condition = (getNextVarContents() != 0);
			}
			break;

		case 13:{									/* equal */
				uint tmp = getNextVarContents();
				condition = (tmp == getVarOrWord());
			}
			break;

		case 14:{									/* not equal */
				uint tmp = getNextVarContents();
				condition = (tmp != getVarOrWord());
			}
			break;

		case 15:{									/* is greater */
				uint tmp = getNextVarContents();
				condition = (tmp > getVarOrWord());
			}
			break;

		case 16:{									/* is less */
				uint tmp = getNextVarContents();
				condition = (tmp < getVarOrWord());
			}
			break;

		case 17:{									/* is eq f */
				uint tmp = getNextVarContents();
				condition = (tmp == getNextVarContents());
			}
			break;

		case 18:{									/* is not equal f */
				uint tmp = getNextVarContents();
				condition = (tmp != getNextVarContents());
			}
			break;

		case 19:{									/* is greater f */
				uint tmp = getNextVarContents();
				condition = (tmp < getNextVarContents());
			}
			break;

		case 20:{									/* is less f */
				uint tmp = getNextVarContents();
				condition = (tmp > getNextVarContents());
			}
			break;

		case 23:{
				condition = o_chance(getVarOrWord());
			}
			break;

		case 25:{									/* is room */
				condition = isRoom(getNextItemPtr());
			}
			break;

		case 26:{									/* is object */
				condition = isObject(getNextItemPtr());
			}
			break;

		case 27:{									/* item state is */
				Item *item = getNextItemPtr();
				condition = ((uint) item->state == getVarOrWord());
			}
			break;

		case 28:{									/* item has prop */
				SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
				byte num = getVarOrByte();
				condition = subObject != NULL && (subObject->objectFlags & (1 << num)) != 0;
			} break;

		case 31:{									/* set no parent */
				setItemParent(getNextItemPtr(), NULL);
			}
			break;

		case 33:{									/* set item parent */
				Item *item = getNextItemPtr();
				setItemParent(item, getNextItemPtr());
			}
			break;

		case 36:{									/* copy var */
				uint value = getNextVarContents();
				writeNextVarContents(value);
			}
			break;

		case 37:{
				if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2)
					goto invalid_opcode;

				getVarOrByte();
				_runScriptReturn1 = 1;
			}
			break;

		case 41:{									/* zero var */
				writeNextVarContents(0);
			}
			break;

		case 42:{									/* set var */
				uint var = getVarOrByte();
				writeVariable(var, getVarOrWord());
			}
			break;

		case 43:{									/* add */
				uint var = getVarOrByte();
				writeVariable(var, readVariable(var) + getVarOrWord());
			}
			break;

		case 44:{									/* sub */
				uint var = getVarOrByte();
				writeVariable(var, readVariable(var) - getVarOrWord());
			}
			break;

		case 45:{									/* add f */
				uint var = getVarOrByte();
				writeVariable(var, readVariable(var) + getNextVarContents());
			}
			break;

		case 46:{									/* sub f */
				uint var = getVarOrByte();
				writeVariable(var, readVariable(var) - getNextVarContents());
			}
			break;

		case 47:{									/* mul */
				uint var = getVarOrByte();
				writeVariable(var, readVariable(var) * getVarOrWord());
			}
			break;

		case 48:{									/* div */
				uint var = getVarOrByte();
				int value = getVarOrWord();
				if (value == 0)
					error("Division by zero in div");
				writeVariable(var, readVariable(var) / value);
			}
			break;

		case 49:{									/* mul f */
				uint var = getVarOrByte();
				writeVariable(var, readVariable(var) * getNextVarContents());
			}
			break;

		case 50:{									/* div f */
				uint var = getVarOrByte();
				int value = getNextVarContents();
				if (value == 0)
					error("Division by zero in div f");
				writeVariable(var, readVariable(var) / value);
			}
			break;

		case 51:{									/* mod */
				uint var = getVarOrByte();
				int value = getVarOrWord();
				if (value == 0)
					error("Division by zero in mod");
				writeVariable(var, readVariable(var) % value);
			}
			break;

		case 52:{									/* mod f */
				uint var = getVarOrByte();
				int value = getNextVarContents();
				if (value == 0)
					error("Division by zero in mod f");
				writeVariable(var, readVariable(var) % value);
			}
			break;

		case 53:{									/* random */
				uint var = getVarOrByte();
				uint value = (uint16)getVarOrWord();

				// Disable random in simon1amiga for now
				// Since copy protection screen is currently unreadable
				if (getPlatform() == Common::kPlatformAmiga)
					writeVariable(var, 4);
				else
					writeVariable(var, _rnd.getRandomNumber(value - 1));
			}
			break;

		case 55:{									/* set itemA parent */
				setItemParent(getItem1Ptr(), getNextItemPtr());
			}
			break;

		case 56:{									/* set child2 fr bit */
				SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
				int value = getVarOrByte();
				if (subObject != NULL && value >= 0x10)
					subObject->objectFlags |= 1 << value;
			}
			break;

		case 57:{									/* clear child2 fr bit */
				SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
				int value = getVarOrByte();
				if (subObject != NULL && value >= 0x10)
					subObject->objectFlags &= ~(1 << value);
			}
			break;

		case 58:{									/* make siblings */
				Item *item = getNextItemPtr();
				setItemParent(item, derefItem(getNextItemPtr()->parent));
			}
			break;

		case 59:{									/* item inc state */
				Item *item = getNextItemPtr();
				if (item->state <= 30000)
					setItemState(item, item->state + 1);
			}
			break;

		case 60:{									/* item dec state */
				Item *item = getNextItemPtr();
				if (item->state >= 0)
					setItemState(item, item->state - 1);
			}
			break;

		case 61:{									/* item set state */
				Item *item = getNextItemPtr();
				int value = getVarOrWord();
				if (value < 0)
					value = 0;
				if (value > 30000)
					value = 30000;
				setItemState(item, value);
			}
			break;

		case 62:{									/* show int */
				showMessageFormat("%d", getNextVarContents());
			}
			break;

		case 63:{									/* show string nl */
				showMessageFormat("%s\n", getStringPtrByID(getNextStringID()));
			}
			break;

		case 64:{									/* show string */
				showMessageFormat("%s", getStringPtrByID(getNextStringID()));
			}
			break;

		case 65:{									/* add hit area */
				int id = getVarOrWord();
				int x = getVarOrWord();
				int y = getVarOrWord();
				int w = getVarOrWord();
				int h = getVarOrWord();
				int number = getVarOrByte();
				if (number < 20)
					addNewHitArea(id, x, y, w, h, (number << 8) + 129, 0xD0, _dummyItem2);
			}
			break;

		case 66:{									/* set item name */
				uint var = getVarOrByte();
				uint stringId = getNextStringID();
				if (var < 20)
					_stringIdArray2[var] = stringId;
			}
			break;

		case 67:{									/* set item description */
				uint var = getVarOrByte();
				uint stringId = getNextStringID();
				if (getFeatures() & GF_TALKIE) {
					uint speechId = getNextWord();
					if (var < 20) {
						_stringIdArray3[var] = stringId;
						_speechIdArray4[var] = speechId;
					}
				} else {
					if (var < 20) {
						_stringIdArray3[var] = stringId;
					}
				}
			}
			break;

		case 68:{									/* exit interpreter */
				shutdown();
			}
			break;

		case 69:{									/* return 1 */
				return 1;
			}

		case 70:{									/* show string from array */
				const char *str = (const char *)getStringPtrByID(_stringIdArray3[getVarOrByte()]);

				if (getGameType() == GType_SIMON2) {
					writeVariable(51, strlen(str) / 53 * 8 + 8);
				}

				showMessageFormat("%s\n", str);
			}
			break;

		case 71:{									/* start subroutine */
				Subroutine *sub = getSubroutineByID(getVarOrWord());
				if (sub != NULL)
					startSubroutine(sub);
			}
			break;

		case 76:{									/* add timeout */
				uint timeout = getVarOrWord();
				addTimeEvent(timeout, getVarOrWord());
			}
			break;

		case 77:{									/* has item minus 1 */
				condition = _subjectItem != NULL;
			}
			break;

		case 78:{									/* has item minus 3 */
				condition = _objectItem != NULL;
			}
			break;

		case 79:{									/* childstruct fr2 is */
				SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
				uint stringId = getNextStringID();
				condition = (subObject != NULL) && subObject->objectName == stringId;
			}
			break;

		case 80:{									/* item equal */
				condition = getNextItemPtr() == getNextItemPtr();
			}
			break;

		case 82:{									/* debug opcode */
				getVarOrByte();
			}
			break;

		case 83:{									/* restart subroutine */
				if (getGameType() == GType_SIMON2 || getGameType() == GType_FF)
					o_83_helper();
				return -10;
			}

		case 87:{									/* comment */
				getNextStringID();
			}
			break;

		case 88:{									/* stop animation */
				_lockWord |= 0x10;
			}
			break;

		case 89:{									/* restart animation */
				_lockWord &= ~0x10;
			}
			break;

		case 90:{									/* set minusitem to parent */
				Item *item = derefItem(getNextItemPtr()->parent);
				switch (getVarOrByte()) {
				case 0:
					_objectItem = item;
					break;
				case 1:
					_subjectItem = item;
					break;
				default:
					error("set minusitem to parent, invalid subcode");
				}
			}
			break;

		case 91:{									/* set minusitem to sibling */
				Item *item = derefItem(getNextItemPtr()->sibling);
				switch (getVarOrByte()) {
				case 0:
					_objectItem = item;
					break;
				case 1:
					_subjectItem = item;
					break;
				default:
					error("set minusitem to sibling, invalid subcode");
				}
			}
			break;

		case 92:{									/* set minusitem to child */
				Item *item = derefItem(getNextItemPtr()->child);
				switch (getVarOrByte()) {
				case 0:
					_objectItem = item;
					break;
				case 1:
					_subjectItem = item;
					break;
				default:
					error("set minusitem to child, invalid subcode");
				}
			}
			break;

		case 96:{
				uint val = getVarOrWord();
				o_set_video_mode(getVarOrByte(), val);
			}
			break;

		case 97:{									/* load vga */
				o_loadZone(getVarOrWord());
			}
			break;

		case 98:{									/* start vga */
				uint vga_res, vgaSpriteId, windowNum, x, y, palette;
				if (getGameType() == GType_SIMON1) {
					vgaSpriteId = getVarOrWord();
					vga_res = vgaSpriteId / 100;
				} else {
					vga_res = getVarOrWord();
					vgaSpriteId = getVarOrWord();
				}
				windowNum = getVarOrByte();
				x = getVarOrWord();
				y = getVarOrWord();
				palette = getVarOrWord();
				loadSprite(windowNum, vga_res, vgaSpriteId, x, y, palette);
			}
			break;

		case 99:{									/* kill sprite */
				if (getGameType() == GType_SIMON1) {
					o_kill_sprite_simon1(getVarOrWord());
				} else {
					uint a = getVarOrWord();
					uint b = getVarOrWord();
					o_kill_sprite_simon2(a, b);
				}
			}
			break;

		case 100:{									/* vga reset */
				o_vga_reset();
			}
			break;

		case 101:{
				uint num = getVarOrByte();
				uint x = getVarOrWord();
				uint y = getVarOrWord();
				uint w = getVarOrWord();
				uint h = getVarOrWord();
				uint flags = getVarOrWord();
				uint fill_color = getVarOrWord();
				o_defineWindow(num, x, y, w, h, flags, fill_color, 0);
			}
			break;

		case 102:{
				changeWindow(getVarOrByte() & 7);
			}
			break;

		case 103:{
				o_unk_103();
			}
			break;

		case 104:{
				closeWindow(getVarOrByte() & 7);
			}
			break;

		case 107:{									/* add item hitarea */
				uint flags = 0;
				uint id = getVarOrWord();
				uint params = id / 1000;
				uint x, y, w, h, verb;
				Item *item;

				id = id % 1000;

				if (params & 1)
					flags |= 8;
				if (params & 2)
					flags |= 4;
				if (params & 4)
					flags |= 0x80;
				if (params & 8)
					flags |= 1;
				if (params & 16)
					flags |= 0x10;

				x = getVarOrWord();
				y = getVarOrWord();
				w = getVarOrWord();
				h = getVarOrWord();
				item = getNextItemPtrStrange();
				verb = getVarOrWord();
				if (x >= 1000) {
					verb += 0x4000;
					x -= 1000;
				}
				addNewHitArea(id, x, y, w, h, flags, verb, item);
			}
			break;

		case 108:{									/* delete hitarea */
				delete_hitarea(getVarOrWord());
			}
			break;

		case 109:{									/* clear hitarea bit 0x40 */
				clear_hitarea_bit_0x40(getVarOrWord());
			}
			break;

		case 110:{									/* set hitarea bit 0x40 */
				set_hitarea_bit_0x40(getVarOrWord());
			}
			break;

		case 111:{									/* set hitarea xy */
				uint hitarea_id = getVarOrWord();
				uint x = getVarOrWord();
				uint y = getVarOrWord();
				moveBox(hitarea_id, x, y);
			}
			break;

		case 114:{
				Item *item = getNextItemPtr();
				uint num = getVarOrByte();
				mouseOff();
				drawIconArray(num, item, 0, 0);
				mouseOn();
			}
			break;

		case 115:{									/* item has flag */
				Item *item = getNextItemPtr();
				condition = (item->classFlags & (1 << getVarOrByte())) != 0;
			}
			break;

		case 116:{									/* item set flag */
				Item *item = getNextItemPtr();
				item->classFlags |= (1 << getVarOrByte());
			}
			break;

		case 117:{									/* item clear flag */
				Item *item = getNextItemPtr();
				item->classFlags &= ~(1 << getVarOrByte());
			}
			break;

		case 119:{									/* wait vga */
				uint var = getVarOrWord();
				_scriptVar2 = (var == 200);

				if (var != 200 || !_skipVgaWait)
					o_waitForSync(var);
				_skipVgaWait = false;
			}
			break;

		case 120:{
				o_sync(getVarOrWord());
			}
			break;

		case 121:{									/* set vga item */
				uint slot = getVarOrByte();
				_vcItemArray[slot] = getNextItemPtr();
			}
			break;

		case 122:{									/* oracle text down */
				if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2)
					goto invalid_opcode;

				oracleTextDown();
			}
			break;

		case 123:{									/* oracle text down */
				if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2)
					goto invalid_opcode;

				oracleTextUp();
			}
			break;

		case 124:{									/* if time */
				if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2)
					goto invalid_opcode;

				uint time = getVarOrWord();
				condition = 1;
				warning("STUB: script opcode 124 (%d)", time);
			}
			break;

		case 125:{									/* item is sibling with item 1 */
				Item *item = getNextItemPtr();
				condition = (getItem1Ptr()->parent == item->parent);
			}
			break;

		case 126:{
				Item *item = getNextItemPtr();
				uint num = getVarOrByte();
				uint a = 1 << getVarOrByte();
				mouseOff();
				drawIconArray(num, item, 1, a);
				mouseOn();
			}
			break;

		case 127:{									/* deals with music */
				o_playMusic();
			}
			break;

		case 128:{									/* dummy instruction */
				getVarOrWord();
			}
			break;

		case 129:{									/* dummy instruction */
				getVarOrWord();
				condition = true;
			}
			break;

		case 130:{									/* set adj noun */
				uint var = getVarOrByte();
				if (var == 1) {
					_scriptAdj1 = getNextWord();
					_scriptNoun1 = getNextWord();
				} else {
					_scriptAdj2 = getNextWord();
					_scriptNoun2 = getNextWord();
				}
			}
			break;

		case 131:{
				warning("STUB: script opcode 131");
			}
			break;

		case 132:{									/* save game */
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
				o_saveGame();
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
			}
			break;

		case 133:{									/* load game */
				if (getGameType() == GType_FF) {
					loadGame(readVariable(55));
				} else {
					_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
					o_loadGame();
					_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
				}
			}
			break;

		case 134:{									/* dummy opcode? */
				if (getGameType() == GType_FF) {
					listSaveGames(1);
				} else {
					midi.stop();
					_lastMusicPlayed = -1;
				}
			}
			break;

		case 135:{									/* quit if user presses y */
				if (getGameType() == GType_FF) {
					// Switch CD
					debug(1, "Switch to CD number %d", readVariable(97));
				} else {
					_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
					o_confirmQuit();
					_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
				}
			}
			break;

		case 136:{									/* set var to item unk3 */
				Item *item = getNextItemPtr();
				writeNextVarContents(item->state);
			}
			break;

		case 137:{
				o_restoreIconArray(getVarOrByte());
			}
			break;

		case 138:{									/* vga pointer op 4 */
				o_freezeBottom();
			}
			break;

		case 139:{									/* set parent special */
				Item *item = getNextItemPtr();
				_noParentNotify = true;
				setItemParent(item, getNextItemPtr());
				_noParentNotify = false;
			}
			break;

		case 140:{									/* del te and add one */
				killAllTimers();
				addTimeEvent(3, 0xA0);
			}
			break;

		case 141:{									/* set m1 or m3 */
				uint which = getVarOrByte();
				Item *item = getNextItemPtr();
				if (which == 1) {
					_subjectItem = item;
				} else {
					_objectItem = item;
				}
			}
			break;

		case 142:{									/* is hitarea 0x40 clear */
				condition = is_hitarea_0x40_clear(getVarOrWord());
			}
			break;

		case 143:{									/* start item sub */
				SubRoom *subRoom = (SubRoom *)findChildOfType(getNextItemPtr(), 1);
				if (subRoom != NULL) {
					Subroutine *sub = getSubroutineByID(subRoom->subroutine_id);
					if (sub)
						startSubroutine(sub);
				}
			}
			break;

		case 151:{									/* set array6 to item */
				uint var = getVarOrByte();
				Item *item = getNextItemPtr();
				_itemArray6[var] = item;
			}
			break;

		case 152:{									/* set m1 or m3 to array6 */
				Item *item = _itemArray6[getVarOrByte()];
				uint var = getVarOrByte();
				if (var == 1) {
					_subjectItem = item;
				} else {
					_objectItem = item;
				}
			}
			break;

		case 153:{									/* set bit */
				setBitFlag(getVarOrByte(), true);
				break;
			}

		case 154:{									/* clear bit */
				setBitFlag(getVarOrByte(), false);
				break;
			}

		case 155:{									/* is bit clear */
				condition = !getBitFlag(getVarOrByte());
			}
			break;

		case 156:{									/* is bit set */
				uint bit = getVarOrByte();
				if (getGameType() == GType_SIMON1 && _subroutine == 2962 && bit == 63) {
					bit = 50;
				}
				condition = getBitFlag(bit);
			}
			break;

		case 157:{									/* get item int prop */
				Item *item = getNextItemPtr();
				SubObject *subObject = (SubObject *)findChildOfType(item, 2);
				uint prop = getVarOrByte();

				if (subObject != NULL && subObject->objectFlags & (1 << prop) && prop < 16) {
					uint offs = getOffsetOfChild2Param(subObject, 1 << prop);
					writeNextVarContents(subObject->objectFlagValue[offs]);
				} else {
					writeNextVarContents(0);
				}
			}
			break;

		case 158:{									/* set item prop */
				Item *item = getNextItemPtr();
				SubObject *subObject = (SubObject *)findChildOfType(item, 2);
				uint prop = getVarOrByte();
				int value = getVarOrWord();

				if (subObject != NULL && subObject->objectFlags & (1 << prop) && prop < 16) {
					uint offs = getOffsetOfChild2Param(subObject, 1 << prop);
					subObject->objectFlagValue[offs] = value;
				}
			}
			break;

		case 160:{
				o_setTextColor(getVarOrByte());
			}
			break;

		case 161:{									/* setup text */
				TextLocation *tl = getTextLocation(getVarOrByte());

				tl->x = getVarOrWord();
				if (getGameType() == GType_FF)
					tl->y = getVarOrWord();
				else
					tl->y = getVarOrByte();
				tl->width = getVarOrWord();
			}
			break;

		case 162:{									/* print string */
				o_printStr();
			}
			break;

		case 163:{									/* play sound */
				o_playSFX(getVarOrWord());
			}
			break;

		case 164:{
				_showPreposition = true;
				o_setup_cond_c();
				_showPreposition = false;
			}
			break;

		case 165:{									/* item unk1 unk2 is */
				Item *item = getNextItemPtr();
				int16 a = getNextWord(), b = getNextWord();
				condition = (item->adjective == a && item->noun == b);
			} break;

		case 166:{									/* set bit2 */
				setBitFlag(256 + getVarOrByte(), true);
			}
			break;

		case 167:{									/* clear bit2 */
				setBitFlag(256 + getVarOrByte(), false);
			}
			break;

		case 168:{									/* is bit2 clear */
				condition = !getBitFlag(256 + getVarOrByte());
			}
			break;

		case 169:{									/* is bit2 set */
				condition = getBitFlag(256 + getVarOrByte());
			}
			break;

		case 171:{									/* oracle hyperlink on */
				if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2)
					goto invalid_opcode;

				hyperLinkOn(getVarOrWord());
			}
			break;

		case 172:{									/* oracle hyperlink off */
				if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2)
					goto invalid_opcode;

				hyperLinkOff();
			}
			break;

		case 173:{
				warning("STUB: script opcode 173");
			}
			break;

		case 175:{									/* vga pointer op 1 */
				o_lockZone();
			}
			break;

		case 176:{									/* vga pointer op 2 */
				o_unlockZone();
			}
			break;

		case 177:{									/* inventory descriptions */
				o_inventory_descriptions();
			}
			break;

		case 178:{									/* path find */
				uint a = getVarOrWord();
				uint b = getVarOrWord();
				uint c = getVarOrByte();
				uint d = getVarOrByte();
				o_pathfind(a, b, c, d);
			}
			break;

		case 179:{									/* conversation responses */
				uint vgaSpriteId = getVarOrByte();				/* and room descriptions */
				uint color = getVarOrByte();
				uint stringId = getVarOrByte();
				uint speechId = 0;

				const char *string_ptr = (const char *)getStringPtrByID(_stringIdArray3[stringId]);
				TextLocation *tl = getTextLocation(vgaSpriteId);
				if (getFeatures() & GF_TALKIE)
					speechId = _speechIdArray4[stringId];

				if (_speech && speechId != 0)
					playSpeech(speechId, vgaSpriteId);
				if (string_ptr != NULL && _subtitles)
					printText(vgaSpriteId, color, string_ptr, tl->x, tl->y, tl->width);
			}
			break;

		case 180:{									/* force mouseOn */
				o_mouseOn();
			}
			break;

		case 181:{									/* force mouseOff */
				o_mouseOff();
				if (getGameType() == GType_SIMON2) {
					changeWindow(1);
					showMessageFormat("\xC");
				}
			}
			break;

		case 182:{									/* load beard */
				if (getGameType() == GType_FF) {
					// Load video file
					debug(1,"Load video file: %s", getStringPtrByID(getNextStringID()));
				} else if (getGameType() == GType_SIMON2) {
					goto invalid_opcode;
				} else {
					o_loadBeard();
				}
			}
			break;

		case 183:{									/* unload beard */
				if (getGameType() == GType_FF) {
					// Play video
					debug(1, "Play video");
				} else if (getGameType() == GType_SIMON2) {
					goto invalid_opcode;
				} else {
					o_unloadBeard();
				}
			}
			break;

		case 184:{									/* clear vgapointer entry */
				o_unloadZone(getVarOrWord());
			}
			break;

		case 185:{									/* load sound files */
				if (getGameType() == GType_SIMON2)
					goto invalid_opcode;

				_soundFileId = getVarOrWord();
				if (getPlatform() == Common::kPlatformAmiga && getFeatures() & GF_TALKIE) {
					char buf[10];
					sprintf(buf, "%d%s", _soundFileId, "Effects");
					_sound->readSfxFile(buf);
					sprintf(buf, "%d%s", _soundFileId, "simon");
					_sound->readVoiceFile(buf);
				}

			}
			break;

		case 186:{									/* vga pointer op 3 */
				o_unfreezeBottom();
			}
			break;

		case 187:{									/* fade to black */
				if (getGameType() == GType_FF) {
					warning("STUB: script opcode 187");
				} else if (getGameType() == GType_SIMON2) {
					goto invalid_opcode;
				} else {
					o_fadeToBlack();
				}
			}
			break;

		case 188:									/* string2 is */
			if (getGameType() == GType_SIMON1)
				goto invalid_opcode;
			{
				uint i = getVarOrByte();
				uint str = getNextStringID();
				condition = (str < 20 && _stringIdArray2[i] == str);
			}
			break;

		case 189:{									/* clear_op189_flag */
				if (getGameType() == GType_SIMON1)
					goto invalid_opcode;
				_marks = 0;
			}
			break;

		case 190:{
				uint i;
				if (getGameType() == GType_SIMON1)
					goto invalid_opcode;
				i = getVarOrByte();
				if (!(_marks & (1 << i)))
					o_waitForMark(i);
			}
			break;

		// Feeble opcodes
		case 191:
			if (getBitFlag(83)) {
				_PVCount1 = 0;
				_GPVCount1 = 0;
			} else {
				_PVCount = 0;
				_GPVCount = 0;
			}
			break;

		case 192:{
				uint8 a = getVarOrByte();
				uint8 b = getVarOrByte();
				uint8 c = getVarOrByte();
				uint8 d = getVarOrByte();
				if (getBitFlag(83)) {
					_pathValues1[_PVCount1++] = a;
					_pathValues1[_PVCount1++] = b;
					_pathValues1[_PVCount1++] = c;
					_pathValues1[_PVCount1++] = d;
				} else {
					_pathValues[_PVCount++] = a;
					_pathValues[_PVCount++] = b;
					_pathValues[_PVCount++] = c;
					_pathValues[_PVCount++] = d;
				}
			}
			break;

		case 193:
			// pause clock
			warning("STUB: script opcode 193");
			break;

		case 194:
			// resume clock
			warning("STUB: script opcode 194");
			break;

		case 195:{
				// Set palette colour?
				uint blue = getVarOrByte();
				uint green = getVarOrByte();
				uint red = getVarOrByte();
				uint color = getVarOrByte();
				warning("STUB: script opcode 195 (%d, %d, %d, %d)", blue, green, red, color);
			}
			break;

		case 196:{									/* set bit3 */
				setBitFlag(512 + getVarOrByte(), true);
			}
			break;

		case 197:{									/* clear bit3 */
				setBitFlag(512 + getVarOrByte(), false);
			}
			break;

		case 198:{									/* is bit3 clear */
				condition = !getBitFlag(512 + getVarOrByte());
			}
			break;

		case 199:{									/* is bit3 set */
				condition = getBitFlag(512 + getVarOrByte());
			}
			break;

		default:
		invalid_opcode:;
			error("Invalid opcode '%d'", opcode);
		}

	} while (condition != flag);

	return 0;
}

int SimonEngine::startSubroutine(Subroutine *sub) {
	int result = -1;
	SubroutineLine *sl;
	const byte *old_code_ptr;

	if (_startMainScript)
		dumpSubroutine(sub);

	old_code_ptr = _codePtr;

	if (++_recursionDepth > 40)
		error("Recursion error");

	sl = (SubroutineLine *)((byte *)sub + sub->first);

	while ((byte *)sl != (byte *)sub) {
		if (checkIfToRunSubroutineLine(sl, sub)) {
			result = 0;
			_codePtr = (byte *)sl;
			if (sub->id)
				_codePtr += 2;
			else
				_codePtr += 8;

			if (_continousMainScript)
				fprintf(_dumpFile, "; %d\n", sub->id);
			result = runScript();
			if (result != 0) {
				/* result -10 means restart subroutine */
				if (result == -10) {
					delay(0);							/* maybe leave control to the VGA */
					sl = (SubroutineLine *)((byte *)sub + sub->first);
					continue;
				}
				break;
			}
		}
		sl = (SubroutineLine *)((byte *)sub + sl->next);
	}

	_codePtr = old_code_ptr;

	_recursionDepth--;
	return result;
}

int SimonEngine::startSubroutineEx(Subroutine *sub) {
	return startSubroutine(sub);
}

bool SimonEngine::checkIfToRunSubroutineLine(SubroutineLine *sl, Subroutine *sub) {
	if (sub->id)
		return true;

	if (sl->verb != -1 && sl->verb != _scriptVerb &&
			(sl->verb != -2 || _scriptVerb != -1))
		return false;

	if (sl->noun1 != -1 && sl->noun1 != _scriptNoun1 &&
			(sl->noun1 != -2 || _scriptNoun1 != -1))
		return false;

	if (sl->noun2 != -1 && sl->noun2 != _scriptNoun2 &&
			(sl->noun2 != -2 || _scriptNoun2 != -1))
		return false;

	return true;
}

void SimonEngine::o_83_helper() {
		if (_exitCutscene) {
			if (getBitFlag(9)) {
				endCutscene();
			}
		} else {
			processSpecialKeys();
		}
}

void SimonEngine::o_waitForMark(uint i) {
	_exitCutscene = false;
	while (!(_marks & (1 << i))) {
		if (_exitCutscene) {
			if (getBitFlag(9)) {
				endCutscene();
				break;
			}
		} else {
			processSpecialKeys();
		}

		delay(10);
	}
}


bool SimonEngine::o_chance(uint a) {
	if (a == 0)
		return 0;

	if (a == 100)
		return 1;

	a += _scriptUnk1;
	if (a <= 0) {
		_scriptUnk1 = 0;
		return 0;
	}

	if ((uint)_rnd.getRandomNumber(99) < a) {
		if (_scriptUnk1 <= 0)
			_scriptUnk1 -= 5;
		else
			_scriptUnk1 = 0;
		return 1;
	}

	if (_scriptUnk1 >= 0)
		_scriptUnk1 += 5;
	else
		_scriptUnk1 = 0;

	return 0;
}

void SimonEngine::o_inventory_descriptions() {
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	const char *string_ptr = NULL;
	TextLocation *tl = NULL;
	char buf[256];

	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	if (subObject != NULL && subObject->objectFlags & kOFText) {
		string_ptr = (const char *)getStringPtrByID(subObject->objectFlagValue[0]);
		tl = getTextLocation(vgaSpriteId);
	}

	if ((getGameType() == GType_SIMON2) && (getFeatures() & GF_TALKIE)) {
		if (subObject != NULL && subObject->objectFlags & kOFVoice) {
			uint speechId = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFVoice)];

			if (subObject->objectFlags & kOFNumber) {
				uint speechIdOffs = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)];

				if (speechId == 116)
					speechId = speechIdOffs + 115;
				if (speechId == 92)
					speechId = speechIdOffs + 98;
				if (speechId == 99)
					speechId = 9;
				if (speechId == 97) {
					switch (speechIdOffs) {
					case 12:
						speechId = 109;
						break;
					case 14:
						speechId = 108;
						break;
					case 18:
						speechId = 107;
						break;
					case 20:
						speechId = 106;
						break;
					case 22:
						speechId = 105;
						break;
					case 28:
						speechId = 104;
						break;
					case 90:
						speechId = 103;
						break;
					case 92:
						speechId = 102;
						break;
					case 100:
						speechId = 51;
						break;
					default:
						error("o_177: invalid case %d", speechIdOffs);
					}
				}
			}

			if (_speech)
				playSpeech(speechId, vgaSpriteId);
		}

	} else if (getFeatures() & GF_TALKIE) {
		if (subObject != NULL && subObject->objectFlags & kOFVoice) {
			uint offs = getOffsetOfChild2Param(subObject, kOFVoice);
			playSpeech(subObject->objectFlagValue[offs], vgaSpriteId);
		} else if (subObject != NULL && subObject->objectFlags & kOFNumber) {
			uint offs = getOffsetOfChild2Param(subObject, kOFNumber);
			playSpeech(subObject->objectFlagValue[offs] + 3550, vgaSpriteId);
		}
	}

	if (subObject != NULL && (subObject->objectFlags & kOFText) && _subtitles) {
		if (subObject->objectFlags & kOFNumber) {
			sprintf(buf, "%d%s", subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)], string_ptr);
			string_ptr = buf;
		}
		if (string_ptr != NULL)
			printText(vgaSpriteId, color, string_ptr, tl->x, tl->y, tl->width);
	}
}

void SimonEngine::o_confirmQuit() {
	// If all else fails, use English as fallback.
	byte keyYes = 'y';
	byte keyNo = 'n';

	switch (_language) {
	case Common::RU_RUS:
		break;
	case Common::PL_POL:
		keyYes = 't';
		break;
	case Common::HB_ISR:
		keyYes = 'f';
		break;
	case Common::ES_ESP:
		keyYes = 's';
		break;
	case Common::IT_ITA:
		keyYes = 's';
		break;
	case Common::FR_FRA:
		keyYes = 'o';
		break;
	case Common::DE_DEU:
		keyYes = 'j';
		break;
	default:
		break;
	}

	for (;;) {
		delay(1);
#ifdef _WIN32_WCE
		if (isSmartphone()) {
			if (_keyPressed) {
				if (_keyPressed == 13)
					shutdown();
				else
					break;
			}
		}
#endif
		if (_keyPressed == keyYes)
			shutdown();
		else if (_keyPressed == keyNo)
			break;
	}
}

void SimonEngine::o_restoreIconArray(uint num) {
	WindowBlock *window;

	window = _windowArray[num & 7];
	if (window->iconPtr == NULL)
		return;
	drawIconArray(num, window->iconPtr->itemRef, window->iconPtr->line, window->iconPtr->classMask);
}

void SimonEngine::o_freezeBottom() {
	_vgaBufStart = _vgaBufFreeStart;
	_vgaFileBufOrg = _vgaBufFreeStart;
}

void SimonEngine::o_unfreezeBottom() {
	_vgaBufFreeStart = _vgaFileBufOrg2;
	_vgaBufStart = _vgaFileBufOrg2;
	_vgaFileBufOrg = _vgaFileBufOrg2;
}

void SimonEngine::o_lockZone() {
	_vgaBufStart = _vgaBufFreeStart;
}

void SimonEngine::o_unlockZone() {
	_vgaBufFreeStart = _vgaFileBufOrg;
	_vgaBufStart = _vgaFileBufOrg;
}

void SimonEngine::o_playMusic() {
	int music = getVarOrWord();
	int track = getVarOrWord();

	// Jamieson630:
	// This appears to be a "load or play music" command.
	// The music resource is specified, and optionally
	// a track as well. Normally we see two calls being
	// made, one to load the resource and another to
	// actually start a track (so the resource is
	// effectively preloaded so there's no latency when
	// starting playback).
	if (getGameType() == GType_SIMON2) {
		int loop = getVarOrByte();

		midi.setLoop (loop != 0);
		if (_lastMusicPlayed != music)
			_nextMusicToPlay = music;
		else
			midi.startTrack (track);
	} else {
		if (music != _lastMusicPlayed) {
			_lastMusicPlayed = music;
			loadMusic (music);
			midi.startTrack (track);
		}
	}
}

void SimonEngine::o_sync(uint a) {
	uint16 id = to16Wrapper(a);
	_lockWord |= 0x8000;
	_vcPtr = (byte *)&id;
	vc15_wakeup_id();
	_lockWord &= ~0x8000;
}

void SimonEngine::o_playSFX(uint sound_id) {
	if (getGameId() == GID_SIMON1DOS)
		playSting(sound_id);
	else
		_sound->playEffects(sound_id);
}

void SimonEngine::o_setTextColor(uint color) {
	WindowBlock *window;

	window = _windowArray[_curWindow];
	window->text_color = color;
}

void SimonEngine::o_unk_103() {
	mouseOff();
	removeIconArray(_curWindow);
	if (getGameType() == GType_FF)
		showMessageFormat("\x0E");
	else
		showMessageFormat("\x0C");
	mouseOn();
}

void SimonEngine::o_kill_sprite_simon1(uint a) {
	uint16 b = to16Wrapper(a);
	_lockWord |= 0x8000;
	_vcPtr = (byte *)&b;
	vc60_killSprite();
	_lockWord &= ~0x8000;
}

void SimonEngine::o_kill_sprite_simon2(uint a, uint b) {
	uint16 items[2];

	items[0] = to16Wrapper(a);
	items[1] = to16Wrapper(b);

	_lockWord |= 0x8000;
	_vcPtr = (byte *)&items;
	vc60_killSprite();
	_lockWord &= ~0x8000;
}

/* OK */
void SimonEngine::o_defineWindow(uint num, uint x, uint y, uint w, uint h, uint flags, uint fill_color, uint text_color) {
	num &= 7;

	if (_windowArray[num])
		closeWindow(num);

	_windowArray[num] = openWindow(x, y, w, h, flags, fill_color, text_color);

	if (num == _curWindow) {
		_textWindow = _windowArray[num];
		if (getGameType() == GType_FF)
			showmessage_helper_3(_textWindow->textColumn, _textWindow->width);
		else
			showmessage_helper_3(_textWindow->textLength, _textWindow->textMaxLength);
	}
}

} // End of namespace Simon
