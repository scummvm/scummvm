/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
#include "stdafx.h"
#include "simon/simon.h"
#include "simon/intern.h"

#ifdef _WIN32_WCE

extern bool toolbar_drawn;
extern bool draw_keyboard;

#endif

int SimonState::runScript()
{
	byte opcode;
	bool flag, condition;

	do {
		if (_mainscript_toggle)
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
				uint rand_value;

				for (;;) {
					uint value_2 = value;
					rand_value = rand() & 0x7FFF;

					if (value == 0)
						error("Invalid random range");

					value = 0x8000 / value;

					if (value == 0)
						error("Invalid random range");

					if (rand_value / value != value_2)
						break;

					value = value_2;
				}

				writeVariable(var, rand_value / value);
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

		case 66:{									/* set array 2 */
				uint var = getVarOrByte();
				uint string_id = getNextStringID();
				if (var < 20)
					_stringid_array_2[var] = string_id;
			}
			break;

		case 67:{									/* set array 3 and 4 */
				if (_game & GAME_TALKIE) {
					uint var = getVarOrByte();
					uint string_id = getNextStringID();
					uint value = getNextWord();
					if (var < 20) {
						_stringid_array_3[var] = string_id;
						_array_4[var] = value;
					}
				} else {
					uint var = getVarOrByte();
					uint string_id = getNextStringID();
					if (var < 20) {
						_stringid_array_3[var] = string_id;
					}
				}
			}
			break;

		case 68:{									/* exit interpreter */
				_system->quit();
			}
			break;

		case 69:{									/* return 1 */
				return 1;
			}

		case 70:{									/* show string from array */
				const char *str = (const char *)getStringPtrByID(_stringid_array_3[getVarOrByte()]);

				if (_game & GAME_SIMON2) {
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

		case 76:{									/* add event */
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

		case 82:{									/* dummy opcode? */
				getVarOrByte();
			}
			break;

		case 83:{									/* restart subroutine */
				return -10;
			}

		case 87:{									/* dummy opcode? */
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

		case 98:{
				if (!(_game & GAME_SIMON2)) {
					uint a = getVarOrWord();
					uint b = getVarOrByte();
					uint c = getVarOrWord();
					uint d = getVarOrWord();
					uint f = getVarOrWord();
					start_vga_code(b, a / 100, a, c, d, f);
				} else {
					uint a = getVarOrWord();
					uint b = getVarOrWord();
					uint c = getVarOrByte();
					uint d = getVarOrWord();
					uint e = getVarOrWord();
					uint f = getVarOrWord();
					start_vga_code(c, a, b, d, e, f);
				}
			}
			break;

		case 99:{
				if (!(_game & GAME_SIMON2)) {
					o_unk_99_simon1(getVarOrWord());
				} else {
					uint a = getVarOrWord();
					uint b = getVarOrWord();
					o_unk_99_simon2(a, b);
				}
			}
			break;

		case 100:{
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

		case 107:{									/* ADD_ITEM_HITAREA(id,x,y,w,h,item,unk3) */
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

		case 119:{									/* WAIT_VGA */
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

		case 121:{									/* SET_VGA_ITEM */
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
				o_unk_127();
			}
			break;

		case 128:{									/* dummy instruction? */
				getVarOrWord();
			}
			break;

		case 129:{									/* dummy instruction? */
				getVarOrWord();
				condition = true;
			}
			break;

		case 130:{									/* set script cond */
				uint a = getVarOrByte();
				if (a == 1) {
					getNextWord();
					_script_cond_b = getNextWord();
				} else {
					getNextWord();
					_script_cond_c = getNextWord();
				}
			}
			break;

		case 132:{
#ifdef _WIN32_WCE

				if (!draw_keyboard) {
					draw_keyboard = true;
					toolbar_drawn = false;
				}
#endif
				o_save_game();
			}
			break;

		case 133:{
				o_load_game();
			}
			break;

		case 134:{
				warning("stopMidiMusic: not implemented");
				/* dummy proc */
			}
			break;

		case 135:{
				o_quit_if_user_presses_y();
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

		case 138:{
				o_unk_138();
			}
			break;

		case 139:{									/* SET_PARENT_SPECIAL */
				Item *item = getNextItemPtr();
				_no_parent_notify = true;
				setItemParent(item, getNextItemPtr());
				_no_parent_notify = false;
			}
			break;

		case 140:{
				killAllTimers();
				addTimeEvent(3, 0xA0);
			}
			break;

		case 141:{
				uint which = getVarOrByte();
				Item *item = getNextItemPtr();
				if (which == 1) {
					_subject_item = item;
				} else {
					_object_item = item;
				}
			}
			break;

		case 142:{
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
				uint value = getVarOrByte();
				ThreeValues *tv;

				switch (value) {
				case 1:
					tv = &_threevalues_1;
					break;
				case 2:
					tv = &_threevalues_2;
					break;
				case 101:
					tv = &_threevalues_3;
					break;
				case 102:
					tv = &_threevalues_4;
					break;
				default:
					error("setup text, invalid value %d", value);
				}

				tv->a = getVarOrWord();
				tv->b = getVarOrByte();
				tv->c = getVarOrWord();
			}
			break;

		case 162:{
				o_print_str();
			}
			break;

		case 163:{
				o_unk_163(getVarOrWord());
			}
			break;

		case 164:{
				_show_preposition = true;
				o_setup_cond_c();
				_show_preposition = false;
			}
			break;

		case 165:{
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

		case 168:{									/* is bit clear? */
				uint bit = getVarOrByte();
				condition = (_bit_array[(bit >> 4) + 16] & (1 << (bit & 15))) == 0;
			}
			break;

		case 169:{									/* is bit set? */
				uint bit = getVarOrByte();
				condition = (_bit_array[(bit >> 4) + 16] & (1 << (bit & 15))) != 0;
			}
			break;

		case 175:{
				o_unk_175();
			}
			break;

		case 176:{
				o_unk_176();
			}
			break;

		case 177:{
				o_177();
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

		case 179:{
				if (_game == GAME_SIMON1TALKIE || _game == GAME_SIMON1WIN) {
					uint b = getVarOrByte();
					/*uint c = */ getVarOrByte();
					uint a = getVarOrByte();
					uint d = _array_4[a];
					if (d != 0)
						talk_with_speech(d, b);
				} else if (_game == GAME_SIMON1DEMO || _game == GAME_SIMON1DOS) {
					uint b = getVarOrByte();
					uint c = getVarOrByte();
					uint a = getVarOrByte();
					const char *s = (const char *)getStringPtrByID(_stringid_array_3[a]);
					ThreeValues *tv;

					switch (b) {
					case 1:
						tv = &_threevalues_1;
						break;
					case 2:
						tv = &_threevalues_2;
						break;
					case 101:
						tv = &_threevalues_3;
						break;
					case 102:
						tv = &_threevalues_4;
						break;
					default:
						error("setup text, invalid value %d", b);
					}

					talk_with_text(b, c, s, tv->a, tv->b, tv->c);
				} else if (_game == GAME_SIMON2TALKIE || _game == GAME_SIMON2WIN) {
					uint b = getVarOrByte();
					uint c = getVarOrByte();
					uint a = getVarOrByte();
					uint d;
					const char *s = (const char *)getStringPtrByID(_stringid_array_3[a]);
					ThreeValues *tv;

					switch (b) {
					case 1:
						tv = &_threevalues_1;
						break;
					case 2:
						tv = &_threevalues_2;
						break;
					case 101:
						tv = &_threevalues_3;
						break;
					case 102:
						tv = &_threevalues_4;
						break;
					default:
						error("setup text, invalid value %d", b);
					}

					d = _array_4[a];
					if (d != 0 && !_vk_t_toggle)
						talk_with_speech(d, b);

					if (s != NULL && _vk_t_toggle)
						talk_with_text(b, c, s, tv->a, tv->b, tv->c);
				} else if (_game == GAME_SIMON2DOS) {
					uint b = getVarOrByte();
					uint c = getVarOrByte();
					uint a = getVarOrByte();
					const char *s = (const char *)getStringPtrByID(_stringid_array_3[a]);
					ThreeValues *tv;

					switch (b) {
					case 1:
						tv = &_threevalues_1;
						break;
					case 2:
						tv = &_threevalues_2;
						break;
					case 101:
						tv = &_threevalues_3;
						break;
					case 102:
						tv = &_threevalues_4;
						break;
					default:
						error("setup text, invalid value %d", b);
					}

					if (s != NULL)
						talk_with_text(b, c, s, tv->a, tv->b, tv->c);
				}
			}
			break;

		case 180:{
				o_force_unlock();
			}
			break;

		case 181:{
				o_force_lock();
				if (_game & GAME_SIMON2) {
					fcs_unk_2(1);
					showMessageFormat("\xC");
				}
			}
			break;

		case 182:{
				if (_game & GAME_SIMON2)
					goto invalid_opcode;
				o_read_vgares_328();
			}
			break;

		case 183:{
				if (_game & GAME_SIMON2)
					goto invalid_opcode;
				o_read_vgares_23();
			}
			break;

		case 184:{
				o_clear_vgapointer_entry(getVarOrWord());
			}
			break;

		case 185:{
				if (_game & GAME_SIMON2)
					goto invalid_opcode;
				getVarOrWord();
			}
			break;

		case 186:{
				o_unk_186();
			}
			break;

		case 187:{
				if (_game & GAME_SIMON2)
					goto invalid_opcode;
				o_fade_to_black();
			}
			break;

		case 188:
			if (!(_game & GAME_SIMON2))
				goto invalid_opcode;
			{
				uint i = getVarOrByte();
				uint str = getNextStringID();
				condition = (str < 20 && _stringid_array_2[i] == str);
			}
			break;

		case 189:{
				if (!(_game & GAME_SIMON2))
					goto invalid_opcode;
				_op_189_flags = 0;
			}
			break;

		case 190:{
				uint i;
				if (!(_game & GAME_SIMON2))
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

	} while (condition != flag);

	return 0;
}

int SimonState::startSubroutine(Subroutine *sub)
{
	int result = -1;
	SubroutineLine *sl;
	byte *old_code_ptr;

	if (_mainscript_toggle)
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

			if (_mainscript_toggle)
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

int SimonState::startSubroutineEx(Subroutine *sub)
{
	_item_1_ptr = _item_1;
	return startSubroutine(sub);
}

bool SimonState::checkIfToRunSubroutineLine(SubroutineLine *sl, Subroutine *sub)
{
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

void SimonState::o_190_helper(uint i)
{
	warning("o_190_helper not implemented");
}


bool SimonState::o_unk_23(uint a)
{
	if (a == 0)
		return 0;

	if (a == 100)
		return 1;

	a += _script_unk_1;
	if (a <= 0) {
		_script_unk_1 = 0;
		return 0;
	}

	if (((uint) (rand() >> 5)) % 100 < a) {
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

void SimonState::o_177()
{
	if (_game == GAME_SIMON1TALKIE || _game == GAME_SIMON1WIN) {
		uint a = getVarOrByte();
		/*uint b = */ getVarOrByte();
		uint offs;
		Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
		if (child != NULL && child->avail_props & 0x200) {
			offs = getOffsetOfChild2Param(child, 0x200);
			talk_with_speech(child->array[offs], a);
		} else if (child != NULL && child->avail_props & 0x100) {
			offs = getOffsetOfChild2Param(child, 0x100);
			talk_with_speech(child->array[offs] + 3550, a);
		}
	} else if ((_game == GAME_SIMON1DEMO) || (_game == GAME_SIMON1DOS)) {
		uint a = getVarOrByte();
		uint b = getVarOrByte();
		Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
		if (child != NULL && child->avail_props & 1) {
			const char *s = (const char *)getStringPtrByID(child->array[0]);
			ThreeValues *tv;
			char buf[256];
			switch (a) {
			case 1:
				tv = &_threevalues_1;
				break;
			case 2:
				tv = &_threevalues_2;
				break;
			case 101:
				tv = &_threevalues_3;
				break;
			case 102:
				tv = &_threevalues_4;
				break;
			default:
				error("setup text, invalid value %d", a);
			}

			if (child->avail_props & 0x100) {
				uint x = getOffsetOfChild2Param(child, 0x100);
				sprintf(buf, "%d%s", child->array[x], s);
				s = buf;
			}

			talk_with_text(a, b, s, tv->a, tv->b, tv->c);
		}
	} else if (_game == GAME_SIMON2TALKIE || _game == GAME_SIMON2WIN) {
		uint a = getVarOrByte();
		uint b = getVarOrByte();
		Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
		const char *s = NULL;
		ThreeValues *tv = NULL;
		char buf[256];

		if (child != NULL && child->avail_props & 1) {
			s = (const char *)getStringPtrByID(child->array[0]);
			switch (a) {
			case 1:
				tv = &_threevalues_1;
				break;
			case 2:
				tv = &_threevalues_2;
				break;
			case 101:
				tv = &_threevalues_3;
				break;
			case 102:
				tv = &_threevalues_4;
				break;
			default:
				error("setup text, invalid value %d", a);
			}
		}

		if (child != NULL && child->avail_props & 0x200) {
			uint var200 = child->array[getOffsetOfChild2Param(child, 0x200)];

			if (child->avail_props & 0x100) {
				uint var100 = child->array[getOffsetOfChild2Param(child, 0x100)];

				if (var200 == 116)
					var200 = var100 + 115;
				if (var200 == 92)
					var200 = var100 + 98;
				if (var200 == 99)
					var200 = 9;
				if (var200 == 97) {
					switch (var100) {
					case 12:
						var200 = 109;
						break;
					case 14:
						var200 = 108;
						break;
					case 18:
						var200 = 107;
						break;
					case 20:
						var200 = 106;
						break;
					case 22:
						var200 = 105;
						break;
					case 28:
						var200 = 104;
						break;
					case 90:
						var200 = 103;
						break;
					case 92:
						var200 = 102;
						break;
					case 100:
						var200 = 51;
						break;
					default:
						error("o_177: invalid case %d", var100);
					}
				}
			}

			if (!_vk_t_toggle)
				talk_with_speech(var200, a);
		}

		if (!_vk_t_toggle)
			return;

		if (child == NULL || !(child->avail_props & 1))
			return;

		if (child->avail_props & 0x100) {
			sprintf(buf, "%d%s", child->array[getOffsetOfChild2Param(child, 0x100)], s);
			s = buf;
		}

		talk_with_text(a, b, s, tv->a, tv->b, tv->c);
	} else if (_game == GAME_SIMON2DOS) {
		uint a = getVarOrByte();
		uint b = getVarOrByte();
		Child2 *child = (Child2 *)findChildOfType(getNextItemPtr(), 2);
		const char *s = NULL;
		ThreeValues *tv = NULL;
		char buf[256];

		if (child != NULL && child->avail_props & 1) {
			s = (const char *)getStringPtrByID(child->array[0]);
			switch (a) {
			case 1:
				tv = &_threevalues_1;
				break;
			case 2:
				tv = &_threevalues_2;
				break;
			case 101:
				tv = &_threevalues_3;
				break;
			case 102:
				tv = &_threevalues_4;
				break;
			default:
				error("setup text, invalid value %d", a);
			}
		}

		if (child == NULL || !(child->avail_props & 1))
			return;

		if (child->avail_props & 0x100) {
			sprintf(buf, "%d%s", child->array[getOffsetOfChild2Param(child, 0x100)], s);
			s = buf;
		}

		talk_with_text(a, b, s, tv->a, tv->b, tv->c);
	}
}

void SimonState::o_quit_if_user_presses_y()
{
	OSystem::Event event;

	for (;;) {
		_system->poll_event(&event);
		if (event.event_code == OSystem::EVENT_KEYDOWN)
			if (event.kbd.keycode == 'y')
				_system->quit();
			else if (event.kbd.keycode == 'n')
				break;
		delay(10);
	}
}

void SimonState::o_unk_137(uint fcs_index)
{
	FillOrCopyStruct *fcs;

	fcs = _fcs_ptr_array_3[fcs_index & 7];
	if (fcs->fcs_data == NULL)
		return;
	fcs_unk_proc_1(fcs_index, fcs->fcs_data->item_ptr, fcs->fcs_data->unk1, fcs->fcs_data->unk2);
}

void SimonState::o_unk_138()
{
	_vga_buf_start = _vga_buf_free_start;
	_vga_file_buf_org = _vga_buf_free_start;
}

void SimonState::o_unk_186()
{
	_vga_buf_free_start = _vga_file_buf_org_2;
	_vga_buf_start = _vga_file_buf_org_2;
	_vga_file_buf_org = _vga_file_buf_org_2;
}

void SimonState::o_unk_175()
{
	_vga_buf_start = _vga_buf_free_start;
}

void SimonState::o_unk_176()
{
	_vga_buf_free_start = _vga_file_buf_org;
	_vga_buf_start = _vga_file_buf_org;
}

int SimonState::o_unk_132_helper(bool *b, char *buf)
{
	HitArea *ha;

	*b = true;


	if (!_saveload_flag) {
	strange_jump:;
		_saveload_flag = false;
		savegame_dialog(buf);
	}

start_over:;
	_key_pressed = 0;

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
//    if (_saveload_row_curpos >= _num_savegame_rows)
//      _saveload_row_curpos = _num_savegame_rows;
		goto strange_jump;
	}

	if (ha->id >= 214)
		goto start_over_2;
	return ha->id - 208;
}

void SimonState::o_unk_132_helper_3()
{
	for (int i = 208; i != 208 + 6; i++)
		set_hitarea_bit_0x40(i);
}

void SimonState::o_unk_132_helper_2(FillOrCopyStruct *fcs, int x)
{
	byte old_text;

	video_putchar(fcs, x);
	old_text = fcs->text_color;
	fcs->text_color = fcs->fill_color;

	x += 120;
	if (x != 128)
		x = 129;
	video_putchar(fcs, x);

	fcs->text_color = old_text;
	video_putchar(fcs, 8);
}

void SimonState::o_unk_127()
{
	if (_game & GAME_SIMON2) {
		uint a = getVarOrWord();
		uint b = getVarOrWord();
		uint c = getVarOrByte();

		warning("o_unk_127(%d,%d,%d) not implemented properly", a, b, c);

		//FIXME simon 2 attempts to use music track 93 in ending sequences 
		// which doesn't exist so prevent that happening
		if ((a != _last_music_played) && (a < 93)) {
			_last_music_played = a;
			playMusic(a);
		}
	} else {
		uint a = getVarOrWord();
		/*uint b = */ getVarOrWord();

		//FIXME music track 12 uses a different and unknown format
		// Using music in next area as temporary work around
		if (a == 12)
			a=0;

		//FIXME simon 1 attempts to use music track 35 in ending sequences 
		// which doesn't exist so prevent that happening
		if ((a != _last_music_played)  && (a < 35)) {
			_last_music_played = a;
			playMusic(a);
		}
	}
}

void SimonState::o_unk_120(uint a)
{
	uint16 id = TO_BE_16(a);
	_lock_word |= 0x4000;
	_vc_ptr = (byte *)&id;
	vc_15_start_funkystruct_by_id();
	_lock_word &= ~0x4000;
}

void SimonState::o_unk_163(uint a)
{
	playEffects(a);
}

void SimonState::o_unk_160(uint a)
{
	fcs_setTextColor(_fcs_ptr_array_3[_fcs_unk_1], a);
}

void SimonState::o_unk_103()
{
	lock();
	fcs_unk1(_fcs_unk_1);
	showMessageFormat("\x0C");
	unlock();
}

void SimonState::o_unk_99_simon1(uint a)
{
	uint16 b = TO_BE_16(a);
	_lock_word |= 0x4000;
	_vc_ptr = (byte *)&b;
	vc_60();
	_lock_word &= ~0x4000;
}

void SimonState::o_unk_99_simon2(uint a, uint b)
{
	uint16 items[2];

	items[0] = TO_BE_16(a);
	items[1] = TO_BE_16(b);

	_lock_word |= 0x4000;
	_vc_ptr = (byte *)&items;
	vc_60();
	_lock_word &= ~0x4000;
}

/* OK */
void SimonState::o_unk26_helper(uint a, uint b, uint c, uint d, uint e, uint f, uint g, uint h)
{
	a &= 7;

	if (_fcs_ptr_array_3[a])
		fcs_delete(a);

	_fcs_ptr_array_3[a] = fcs_alloc(b, c, d, e, f, g, h);

	if (a == _fcs_unk_1) {
		_fcs_ptr_1 = _fcs_ptr_array_3[a];
		showmessage_helper_3(_fcs_ptr_1->textLength,
                             _fcs_ptr_1->textMaxLength);
	}
}
