/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Item script opcodes for Simon1/Simon2
#include <ctype.h>

#include "stdafx.h"
#include "simon/simon.h"
#include "simon/intern.h"

#ifdef _WIN32_WCE

extern void force_keyboard(bool);

#endif

namespace Simon {

int SimonEngine::runScript() {
	byte opcode;
	bool flag, condition;

	do {
		if (_continous_mainscript)
			dumpOpcode(_code_ptr);

		opcode = getByte();
		if (opcode == 0xFF)
			return 0;

		if (_run_script_return_1)
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
				condition = o_unk_23(getVarOrWord());
			}
			break;

		case 25:{									/* has child of type 1 */
				condition = hasChildOfType1(getNextItemPtr());
			}
			break;

		case 26:{									/* has child of type 2 */
				condition = hasChildOfType2(getNextItemPtr());
			}
			break;

		case 27:{									/* item unk3 is */
				Item *item = getNextItemPtr();
				condition = ((uint) item->unk3 == getVarOrWord());
			}
			break;

		case 28:{									/* item has prop */
				Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
				byte num = getVarOrByte();
				condition = child != NULL && (child->avail_props & (1 << num)) != 0;
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
				if (_game == GAME_SIMON1AMIGA)
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
				Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
				int value = getVarOrByte();
				if (child != NULL && value >= 0x10)
					child->avail_props |= 1 << value;
			}
			break;

		case 57:{									/* clear child2 fr bit */
				Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
				int value = getVarOrByte();
				if (child != NULL && value >= 0x10)
					child->avail_props &= ~(1 << value);
			}
			break;

		case 58:{									/* make siblings */
				Item *item = getNextItemPtr();
				setItemParent(item, derefItem(getNextItemPtr()->parent));
			}
			break;

		case 59:{									/* item inc unk3 */
				Item *item = getNextItemPtr();
				if (item->unk3 <= 30000)
					setItemUnk3(item, item->unk3 + 1);
			}
			break;

		case 60:{									/* item dec unk3 */
				Item *item = getNextItemPtr();
				if (item->unk3 >= 0)
					setItemUnk3(item, item->unk3 - 1);
			}
			break;

		case 61:{									/* item set unk3 */
				Item *item = getNextItemPtr();
				int value = getVarOrWord();
				if (value < 0)
					value = 0;
				if (value > 30000)
					value = 30000;
				setItemUnk3(item, value);
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
					addNewHitArea(id, x, y, w, h, (number << 8) + 129, 0xD0, _dummy_item_2);
			}
			break;

		case 66:{									/* set item name */
				uint var = getVarOrByte();
				uint string_id = getNextStringID();
				if (var < 20)
					_stringid_array_2[var] = string_id;
			}
			break;

		case 67:{									/* set item description */
				uint var = getVarOrByte();
				uint string_id = getNextStringID();
				if (_game & GF_TALKIE) {
					uint speech_id = getNextWord();
					if (var < 20) {
						_stringid_array_3[var] = string_id;
						_speechid_array_4[var] = speech_id;
					}
				} else {
					if (var < 20) {
						_stringid_array_3[var] = string_id;
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
				const char *str = (const char *)getStringPtrByID(_stringid_array_3[getVarOrByte()]);

				if (_game & GF_SIMON2) {
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
				condition = _subject_item != NULL;
			}
			break;

		case 78:{									/* has item minus 3 */
				condition = _object_item != NULL;
			}
			break;

		case 79:{									/* childstruct fr2 is */
				Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
				uint string_id = getNextStringID();
				condition = (child != NULL) && child->string_id == string_id;
			}
			break;

		case 80:{									/* item equal */
				condition = getNextItemPtr() == getNextItemPtr();
			}
			break;

		case 82:{									/* dummy opcode */
				getVarOrByte();
			}
			break;

		case 83:{									/* restart subroutine */
				if (_game & GF_SIMON2)
					o_83_helper();
				return -10;
			}

		case 87:{									/* dummy opcode */
				getNextStringID();
			}
			break;

		case 88:{									/* or_lock_word */
				_lock_word |= 0x10;
			}
			break;

		case 89:{									/* and lock word */
				_lock_word &= ~0x10;
			}
			break;

		case 90:{									/* set minusitem to parent */
				Item *item = derefItem(getNextItemPtr()->parent);
				switch (getVarOrByte()) {
				case 0:
					_object_item = item;
					break;
				case 1:
					_subject_item = item;
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
					_object_item = item;
					break;
				case 1:
					_subject_item = item;
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
					_object_item = item;
					break;
				case 1:
					_subject_item = item;
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
				ensureVgaResLoadedC(getVarOrWord());
			}
			break;

		case 98:{									/* start vga */
				if (!(_game & GF_SIMON2)) {
					uint vga_sprite_id = getVarOrWord();
					uint b = getVarOrByte();
					uint x = getVarOrWord();
					uint y = getVarOrWord();
					uint base_color = getVarOrWord();
					start_vga_code(b, vga_sprite_id / 100, vga_sprite_id, x, y, base_color);
				} else {
					uint vga_res = getVarOrWord();
					uint vga_sprite_id = getVarOrWord();
					uint b = getVarOrByte();
					uint x = getVarOrWord();
					uint y = getVarOrWord();
					uint base_color = getVarOrWord();
					start_vga_code(b, vga_res, vga_sprite_id, x, y, base_color);
				}
			}
			break;

		case 99:{									/* kill sprite */
				if (!(_game & GF_SIMON2)) {
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
				uint a = getVarOrByte();
				uint b = getVarOrWord();
				uint c = getVarOrWord();
				uint d = getVarOrWord();
				uint e = getVarOrWord();
				uint f = getVarOrWord();
				uint g = getVarOrWord();
				o_unk26_helper(a, b, c, d, e, f, g, 0);
			}
			break;

		case 102:{
				fcs_unk_2(getVarOrByte() & 7);
			}
			break;

		case 103:{
				o_unk_103();
			}
			break;

		case 104:{
				fcs_delete(getVarOrByte() & 7);
			}
			break;

		case 107:{									/* add item hitarea */
				uint flags = 0;
				uint id = getVarOrWord();
				uint params = id / 1000;
				uint x, y, w, h, unk3;
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
				unk3 = getVarOrWord();
				if (x >= 1000) {
					unk3 += 0x4000;
					x -= 1000;
				}
				addNewHitArea(id, x, y, w, h, flags, unk3, item);
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
				set_hitarea_x_y(hitarea_id, x, y);
			}
			break;

		case 114:{
				Item *item = getNextItemPtr();
				uint fcs_index = getVarOrByte();
				lock();
				fcs_unk_proc_1(fcs_index, item, 0, 0);
				unlock();
			}
			break;

		case 115:{									/* item has flag */
				Item *item = getNextItemPtr();
				condition = (item->unk4 & (1 << getVarOrByte())) != 0;
			}
			break;

		case 116:{									/* item set flag */
				Item *item = getNextItemPtr();
				item->unk4 |= (1 << getVarOrByte());
			}
			break;

		case 117:{									/* item clear flag */
				Item *item = getNextItemPtr();
				item->unk4 &= ~(1 << getVarOrByte());
			}
			break;

		case 119:{									/* wait vga */
				uint var = getVarOrWord();
				_scriptvar_2 = (var == 200);

				if (var != 200 || !_skip_vga_wait)
					o_wait_for_vga(var);
				_skip_vga_wait = false;
			}
			break;

		case 120:{
				o_unk_120(getVarOrWord());
			}
			break;

		case 121:{									/* set vga item */
				uint slot = getVarOrByte();
				_vc_item_array[slot] = getNextItemPtr();
			}
			break;

		case 125:{									/* item is sibling with item 1 */
				Item *item = getNextItemPtr();
				condition = (getItem1Ptr()->parent == item->parent);
			}
			break;

		case 126:{
				Item *item = getNextItemPtr();
				uint fcs_index = getVarOrByte();
				uint a = 1 << getVarOrByte();
				lock();
				fcs_unk_proc_1(fcs_index, item, 1, a);
				unlock();
			}
			break;

		case 127:{									/* deals with music */
				o_play_music_resource();
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

		case 130:{									/* set script cond */
				uint var = getVarOrByte();
				getNextWord();
				if (var == 1)
					_script_cond_b = getNextWord();
				else
					_script_cond_c = getNextWord();
			}
			break;

		case 132:{									/* save game */
#ifdef _WIN32_WCE
				force_keyboard(true);
#endif

				o_save_game();

#ifdef _WIN32_WCE
				force_keyboard(false);
#endif
			}
			break;

		case 133:{									/* load game */
#ifdef _WIN32_WCE
				force_keyboard(true);
#endif

				o_load_game();

#ifdef _WIN32_WCE
				force_keyboard(false);
#endif
			}
			break;

		case 134:{									/* dummy opcode? */
				midi.stop();
				_last_music_played = -1;
			}
			break;

		case 135:{									/* quit if user presses y */
#ifdef _WIN32_WCE
				force_keyboard(true);
#endif

				o_quit_if_user_presses_y();

#ifdef _WIN32_WCE
				force_keyboard(false);
#endif
			}
			break;

		case 136:{									/* set var to item unk3 */
				Item *item = getNextItemPtr();
				writeNextVarContents(item->unk3);
			}
			break;

		case 137:{
				o_unk_137(getVarOrByte());
			}
			break;

		case 138:{									/* vga pointer op 4 */
				o_unk_138();
			}
			break;

		case 139:{									/* set parent special */
				Item *item = getNextItemPtr();
				_no_parent_notify = true;
				setItemParent(item, getNextItemPtr());
				_no_parent_notify = false;
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
					_subject_item = item;
				} else {
					_object_item = item;
				}
			}
			break;

		case 142:{									/* is hitarea 0x40 clear */
				condition = is_hitarea_0x40_clear(getVarOrWord());
			}
			break;

		case 143:{									/* start item sub */
				Child1 *child = (Child1 *)findChildOfType(getNextItemPtr(), 1);
				if (child != NULL) {
					Subroutine *sub = getSubroutineByID(child->subroutine_id);
					if (sub)
						startSubroutine(sub);
				}
			}
			break;

		case 151:{									/* set array6 to item */
				uint var = getVarOrByte();
				Item *item = getNextItemPtr();
				_item_array_6[var] = item;
			}
			break;

		case 152:{									/* set m1 or m3 to array6 */
				Item *item = _item_array_6[getVarOrByte()];
				uint var = getVarOrByte();
				if (var == 1) {
					_subject_item = item;
				} else {
					_object_item = item;
				}
			}
			break;

		case 153:{									/* set bit */
				uint bit = getVarOrByte();
				_bit_array[bit >> 4] |= 1 << (bit & 15);
				break;
			}

		case 154:{									/* clear bit */
				uint bit = getVarOrByte();
				_bit_array[bit >> 4] &= ~(1 << (bit & 15));
				break;
			}

		case 155:{									/* is bit clear? */
				uint bit = getVarOrByte();
				condition = (_bit_array[bit >> 4] & (1 << (bit & 15))) == 0;
			}
			break;

		case 156:{									/* is bit set? */
				uint bit = getVarOrByte();
				condition = (_bit_array[bit >> 4] & (1 << (bit & 15))) != 0;
			}
			break;

		case 157:{									/* get item int prop */
				Item *item = getNextItemPtr();
				Child2 *child = (Child2 *)findChildOfType(item, 2);
				uint prop = getVarOrByte();

				if (child != NULL && child->avail_props & (1 << prop) && prop < 16) {
					uint offs = getOffsetOfChild2Param(child, 1 << prop);
					writeNextVarContents(child->array[offs]);
				} else {
					writeNextVarContents(0);
				}
			}
			break;

		case 158:{									/* set item prop */
				Item *item = getNextItemPtr();
				Child2 *child = (Child2 *)findChildOfType(item, 2);
				uint prop = getVarOrByte();
				int value = getVarOrWord();

				if (child != NULL && child->avail_props & (1 << prop) && prop < 16) {
					uint offs = getOffsetOfChild2Param(child, 1 << prop);
					child->array[offs] = value;
				}
			}
			break;

		case 160:{
				o_unk_160(getVarOrByte());
			}
			break;

		case 161:{									/* setup text */
				ThreeValues *tv = getThreeValues(getVarOrByte());

				tv->a = getVarOrWord();
				tv->b = getVarOrByte();
				tv->c = getVarOrWord();
			}
			break;

		case 162:{									/* print string */
				o_print_str();
			}
			break;

		case 163:{									/* play sound */
				o_play_sound(getVarOrWord());
			}
			break;

		case 164:{
				_show_preposition = true;
				o_setup_cond_c();
				_show_preposition = false;
			}
			break;

		case 165:{									/* item unk1 unk2 is */
				Item *item = getNextItemPtr();
				int16 a = getNextWord(), b = getNextWord();
				condition = (item->unk2 == a && item->unk1 == b);
			} break;

		case 166:{									/* set bit2 */
				uint bit = getVarOrByte();
				_bit_array[(bit >> 4) + 16] |= 1 << (bit & 15);
			}
			break;

		case 167:{									/* clear bit2 */
				uint bit = getVarOrByte();
				_bit_array[(bit >> 4) + 16] &= ~(1 << (bit & 15));
			}
			break;

		case 168:{									/* is bit2 clear */
				uint bit = getVarOrByte();
				condition = (_bit_array[(bit >> 4) + 16] & (1 << (bit & 15))) == 0;
			}
			break;

		case 169:{									/* is bit2 set */
				uint bit = getVarOrByte();
				condition = (_bit_array[(bit >> 4) + 16] & (1 << (bit & 15))) != 0;
			}
			break;

		case 175:{									/* vga pointer op 1 */
				o_unk_175();
			}
			break;

		case 176:{									/* vga pointer op 2 */
				o_unk_176();
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
				uint vga_sprite_id = getVarOrByte();				/* and room descriptions */
				uint color = getVarOrByte();
				uint string_id = getVarOrByte();
				uint speech_id = 0;

				const char *string_ptr = (const char *)getStringPtrByID(_stringid_array_3[string_id]);
				ThreeValues *tv = getThreeValues(vga_sprite_id);
				if (_game & GF_TALKIE) 
					speech_id = _speechid_array_4[string_id];

				if (_speech && speech_id != 0)
					talk_with_speech(speech_id, vga_sprite_id);
				if (string_ptr != NULL && _subtitles)
					talk_with_text(vga_sprite_id, color, string_ptr, tv->a, tv->b, tv->c);
			}
			break;

		case 180:{									/* force unlock */
				o_force_unlock();
			}
			break;

		case 181:{									/* force lock */
				o_force_lock();
				if (_game & GF_SIMON2) {
					fcs_unk_2(1);
					showMessageFormat("\xC");
				}
			}
			break;

		case 182:{									/* read vgares 328 */
				if (_game & GF_SIMON2)
					goto invalid_opcode;
				o_read_vgares_328();
			}
			break;

		case 183:{									/* read vgares 23 */
				if (_game & GF_SIMON2)
					goto invalid_opcode;
				o_read_vgares_23();
			}
			break;

		case 184:{									/* clear vgapointer entry */
				o_clear_vgapointer_entry(getVarOrWord());
			}
			break;

		case 185:{									/* midi sfx file number */
				if (_game & GF_SIMON2)
					goto invalid_opcode;
					_sound_file_id = getVarOrWord();
				if (_game == GAME_SIMON1CD32) {
					char buf[10];
					sprintf(buf, "%d%s", _sound_file_id, "Effects");
					_sound->readSfxFile(buf, _gameDataPath);
					sprintf(buf, "%d%s", _sound_file_id, "simon");
					_sound->readVoiceFile(buf, _gameDataPath);
				}

			}
			break;

		case 186:{									/* vga pointer op 3 */
				o_unk_186();
			}
			break;

		case 187:{									/* fade to black */
				if (_game & GF_SIMON2)
					goto invalid_opcode;
				o_fade_to_black();
			}
			break;

		case 188:									/* string2 is */
			if (!(_game & GF_SIMON2))
				goto invalid_opcode;
			{
				uint i = getVarOrByte();
				uint str = getNextStringID();
				condition = (str < 20 && _stringid_array_2[i] == str);
			}
			break;

		case 189:{									/* clear_op189_flag */
				if (!(_game & GF_SIMON2))
					goto invalid_opcode;
				_op_189_flags = 0;
			}
			break;

		case 190:{
				uint i;
				if (!(_game & GF_SIMON2))
					goto invalid_opcode;
				i = getVarOrByte();
				if (!(_op_189_flags & (1 << i)))
					o_190_helper(i);
			}
			break;

		default:
		invalid_opcode:;
			error("Invalid opcode '%d'", opcode);
		}
		delay(1);

	} while (condition != flag);

	return 0;
}

int SimonEngine::startSubroutine(Subroutine *sub) {
	int result = -1;
	SubroutineLine *sl;
	byte *old_code_ptr;

	if (_start_mainscript)
		dumpSubroutine(sub);

	old_code_ptr = _code_ptr;

	if (++_recursion_depth > 40)
		error("Recursion error");

	sl = (SubroutineLine *)((byte *)sub + sub->first);

	while ((byte *)sl != (byte *)sub) {
		if (checkIfToRunSubroutineLine(sl, sub)) {
			result = 0;
			_code_ptr = (byte *)sl;
			if (sub->id)
				_code_ptr += 2;
			else
				_code_ptr += 8;

			if (_continous_mainscript)
				fprintf(_dump_file, "; %d\n", sub->id);
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

	_code_ptr = old_code_ptr;

	_recursion_depth--;
	return result;
}

int SimonEngine::startSubroutineEx(Subroutine *sub) {
	return startSubroutine(sub);
}

bool SimonEngine::checkIfToRunSubroutineLine(SubroutineLine *sl, Subroutine *sub) {
	if (sub->id)
		return true;

	if (sl->cond_a != -1 && sl->cond_a != _script_cond_a &&
			(sl->cond_a != -2 || _script_cond_a != -1))
		return false;

	if (sl->cond_b != -1 && sl->cond_b != _script_cond_b &&
			(sl->cond_b != -2 || _script_cond_b != -1))
		return false;

	if (sl->cond_c != -1 && sl->cond_c != _script_cond_c &&
			(sl->cond_c != -2 || _script_cond_c != -1))
		return false;

	return true;
}

void SimonEngine::o_83_helper() {
		if (_exit_cutscene) {
			if (vc_get_bit(9)) {
				startSubroutine170();
			}
		} else {
			processSpecialKeys();
		}
}

void SimonEngine::o_190_helper(uint i) {
	_exit_cutscene = false;
	while (!(_op_189_flags & (1 << i))) {
		if (_exit_cutscene) {
			if (vc_get_bit(9)) {
				startSubroutine170();
				break;
			}
		} else {
			processSpecialKeys();
		}

		delay(10);
	}
}


bool SimonEngine::o_unk_23(uint a) {
	if (a == 0)
		return 0;

	if (a == 100)
		return 1;

	a += _script_unk_1;
	if (a <= 0) {
		_script_unk_1 = 0;
		return 0;
	}

	if ((uint)_rnd.getRandomNumber(99) < a) {
		if (_script_unk_1 <= 0)
			_script_unk_1 -= 5;
		else
			_script_unk_1 = 0;
		return 1;
	}

	if (_script_unk_1 >= 0)
		_script_unk_1 += 5;
	else
		_script_unk_1 = 0;

	return 0;
}

void SimonEngine::o_inventory_descriptions() {
	uint vga_sprite_id = getVarOrByte();
	uint color = getVarOrByte();
	const char *string_ptr = NULL;
	ThreeValues *tv = NULL;
	char buf[256];

	Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
	if (child != NULL && child->avail_props & 1) {
		string_ptr = (const char *)getStringPtrByID(child->array[0]);
		tv = getThreeValues(vga_sprite_id);
	}

	if ((_game & GF_SIMON2) && (_game & GF_TALKIE)) {
		if (child != NULL && child->avail_props & 0x200) {
			uint speech_id = child->array[getOffsetOfChild2Param(child, 0x200)];

			if (child->avail_props & 0x100) {
				uint speech_id_offs = child->array[getOffsetOfChild2Param(child, 0x100)];

				if (speech_id == 116)
					speech_id = speech_id_offs + 115;
				if (speech_id == 92)
					speech_id = speech_id_offs + 98;
				if (speech_id == 99)
					speech_id = 9;
				if (speech_id == 97) {
					switch (speech_id_offs) {
					case 12:
						speech_id = 109;
						break;
					case 14:
						speech_id = 108;
						break;
					case 18:
						speech_id = 107;
						break;
					case 20:
						speech_id = 106;
						break;
					case 22:
						speech_id = 105;
						break;
					case 28:
						speech_id = 104;
						break;
					case 90:
						speech_id = 103;
						break;
					case 92:
						speech_id = 102;
						break;
					case 100:
						speech_id = 51;
						break;
					default:
						error("o_177: invalid case %d", speech_id_offs);
					}
				}
			}

			if (_speech)
				talk_with_speech(speech_id, vga_sprite_id);
		}

	} else if (_game & GF_TALKIE) {
		if (child != NULL && child->avail_props & 0x200) {
			uint offs = getOffsetOfChild2Param(child, 0x200);
			talk_with_speech(child->array[offs], vga_sprite_id);
		} else if (child != NULL && child->avail_props & 0x100) {
			uint offs = getOffsetOfChild2Param(child, 0x100);
			talk_with_speech(child->array[offs] + 3550, vga_sprite_id);
		}
	}

	if (child != NULL && (child->avail_props & 1) && _subtitles) {
		if (child->avail_props & 0x100) {
			sprintf(buf, "%d%s", child->array[getOffsetOfChild2Param(child, 0x100)], string_ptr);
			string_ptr = buf;
		}
		if (string_ptr != NULL)
			talk_with_text(vga_sprite_id, color, string_ptr, tv->a, tv->b, tv->c);
	}
}

void SimonEngine::o_quit_if_user_presses_y() {
	for (;;) {
		delay(1);
		if (_key_pressed == 'f' && _language == 20) // Hebrew
			shutdown();
		if (_key_pressed == 's' && _language == 5) // Spanish
			shutdown();
		if (_key_pressed == 's' && _language == 3) // Italian
			shutdown();
		if (_key_pressed == 'o' && _language == 2) // French
			shutdown();
		if (_key_pressed == 'j' && _language == 1) // German
			shutdown();
		if (_key_pressed == 'y' && _language == 0) // English
			shutdown();
		if (_key_pressed == 'n')
			goto get_out;
	}
get_out:;
}

void SimonEngine::o_unk_137(uint fcs_index) {
	FillOrCopyStruct *fcs;

	fcs = _fcs_ptr_array_3[fcs_index & 7];
	if (fcs->fcs_data == NULL)
		return;
	fcs_unk_proc_1(fcs_index, fcs->fcs_data->item_ptr, fcs->fcs_data->unk1, fcs->fcs_data->unk2);
}

void SimonEngine::o_unk_138() {
	_vga_buf_start = _vga_buf_free_start;
	_vga_file_buf_org = _vga_buf_free_start;
}

void SimonEngine::o_unk_186() {
	_vga_buf_free_start = _vga_file_buf_org_2;
	_vga_buf_start = _vga_file_buf_org_2;
	_vga_file_buf_org = _vga_file_buf_org_2;
}

void SimonEngine::o_unk_175() {
	_vga_buf_start = _vga_buf_free_start;
}

void SimonEngine::o_unk_176() {
	_vga_buf_free_start = _vga_file_buf_org;
	_vga_buf_start = _vga_file_buf_org;
}

int SimonEngine::o_unk_132_helper(bool *b, char *buf) {
	HitArea *ha;
	*b = true;

	if (!_saveload_flag) {
	strange_jump:;
		_saveload_flag = false;
		savegame_dialog(buf);
	}

start_over:;
	_key_pressed = 0;

	if (_game == GAME_SIMON1CD32)
		goto start_over_3;

start_over_2:;
	_last_hitarea = _last_hitarea_3 = 0;

	do {
		if (_key_pressed != 0) {
			if (_saveload_flag) {
				*b = false;
				return _key_pressed;
			}
			goto start_over;
		}
		delay(100);
	} while (_last_hitarea_3 == 0);

	ha = _last_hitarea;

	if (ha == NULL || ha->id < 205)
		goto start_over_2;

	if (ha->id == 205)
		return ha->id;

	if (ha->id == 206) {
		if (_saveload_row_curpos == 1)
			goto start_over_2;
		if (_saveload_row_curpos < 7)
			_saveload_row_curpos = 1;
		else
			_saveload_row_curpos -= 6;

		goto strange_jump;
	}

	if (ha->id == 207) {
		if (!_savedialog_flag)
			goto start_over_2;
		_saveload_row_curpos += 6;
		if (_saveload_row_curpos >= _num_savegame_rows)
			_saveload_row_curpos = _num_savegame_rows;
		goto strange_jump;
	}

	if (ha->id >= 214)
		goto start_over_2;
	return ha->id - 208;

//FIXME Hack to allow load and save file selection in simon1cd32
//      Uses the follow keys for moving around
//      1 - 6 to select slot to load/save
//      Up Arrow to move up slots
//      Down Arrow to move down slots
//      X to exit
start_over_3:;
	if (_saveload_flag) {
		*b = false;
		delay(1);
		return _key_pressed;
	}

	if (_key_pressed == 17) {
		if (_saveload_row_curpos == 1)
			goto start_over_3;
		if (_saveload_row_curpos < 7)
			_saveload_row_curpos = 1;
		else
			_saveload_row_curpos -= 6;

		goto strange_jump;
	}

	if (_key_pressed == 18) {
		if (!_savedialog_flag)
			goto start_over_3;
		_saveload_row_curpos += 6;
		if (_saveload_row_curpos >= _num_savegame_rows)
			_saveload_row_curpos = _num_savegame_rows;
		goto strange_jump;
	}

	if (_key_pressed == 120)
		return 205;

	if (_key_pressed > 48 && _key_pressed < 55) {
		return _key_pressed - 49;
	}


	delay(1);
	goto start_over_3;

}

void SimonEngine::o_unk_132_helper_3() {
	for (int i = 208; i != 208 + 6; i++)
		set_hitarea_bit_0x40(i);
}

void SimonEngine::o_clear_character(FillOrCopyStruct *fcs, int x, byte b) {
	byte old_text;

	video_putchar(fcs, x, b);
	old_text = fcs->text_color;
	fcs->text_color = fcs->fill_color;

	if (_language == 20) { //Hebrew
		x = 128;
	} else { 
		x += 120;
		if (x != 128)
			x = 129;

	}

	video_putchar(fcs, x);

	fcs->text_color = old_text;
	video_putchar(fcs, 8);
}

void SimonEngine::o_play_music_resource() {
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
	if (_game & GF_SIMON2) {
		int loop = getVarOrByte();

		midi.setLoop (loop != 0);
		if (_last_music_played != music)
			_next_music_to_play = music;
		else 
			midi.startTrack (track);
	} else {
		if (music != _last_music_played) {
			_last_music_played = music;
			loadMusic (music);
			midi.startTrack (track);
		}
	}
}

void SimonEngine::o_unk_120(uint a) {
	uint16 id = TO_BE_16(a);
	if (_game & GF_SIMON2) {
		_lock_word |= 0x8000;
		_vc_ptr = (byte *)&id;
		vc_15_wakeup_id();
		_lock_word &= ~0x8000;
	} else {
		_lock_word |= 0x4000;
		_vc_ptr = (byte *)&id;
		vc_15_wakeup_id();
		_lock_word &= ~0x4000;
	}
}

void SimonEngine::o_play_sound(uint sound_id) {
	if (_game == GAME_SIMON1DOS)
		playSting(sound_id);
	else
		_sound->playEffects(sound_id);
}

void SimonEngine::o_unk_160(uint a) {
	fcs_setTextColor(_fcs_ptr_array_3[_fcs_unk_1], a);
}

void SimonEngine::o_unk_103() {
	lock();
	fcs_unk1(_fcs_unk_1);
	showMessageFormat("\x0C");
	unlock();
}

void SimonEngine::o_kill_sprite_simon1(uint a) {
	uint16 b = TO_BE_16(a);
	_lock_word |= 0x4000;
	_vc_ptr = (byte *)&b;
	vc_60_kill_sprite();
	_lock_word &= ~0x4000;
}

void SimonEngine::o_kill_sprite_simon2(uint a, uint b) {
	uint16 items[2];

	items[0] = TO_BE_16(a);
	items[1] = TO_BE_16(b);

	_lock_word |= 0x8000;
	_vc_ptr = (byte *)&items;
	vc_60_kill_sprite();
	_lock_word &= ~0x8000;
}

/* OK */
void SimonEngine::o_unk26_helper(uint a, uint b, uint c, uint d, uint e, uint f, uint g, uint h) {
	a &= 7;

	if (_fcs_ptr_array_3[a])
		fcs_delete(a);

	_fcs_ptr_array_3[a] = fcs_alloc(b, c, d, e, f, g, h);

	if (a == _fcs_unk_1) {
		_fcs_ptr_1 = _fcs_ptr_array_3[a];
		showmessage_helper_3(_fcs_ptr_1->textLength, _fcs_ptr_1->textMaxLength);
	}
}

} // End of namespace Simon
