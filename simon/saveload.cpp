/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "gui/about.h"
#include "gui/message.h"

#include "simon/simon.h"
#include "simon/intern.h"

#include "common/savefile.h"
#include "common/system.h"

namespace Simon {

void SimonEngine::o_save_game() {
	save_or_load_dialog(false);
}

void SimonEngine::o_load_game() {
	save_or_load_dialog(true);
}

int SimonEngine::count_savegames() {
	Common::InSaveFile *f;
	uint i = 1;
	bool marks[256];

	char *prefix = gen_savename(999);
	prefix[strlen(prefix)-3] = '\0';
	_saveFileMan->listSavefiles(prefix, marks, 256);

	while (i < 256) {
		if (marks[i] &&
		    (f = _saveFileMan->openForLoading(gen_savename(i)))) {
			i++;
			delete f;
		} else
			break;
	}
	return i;
}

int SimonEngine::display_savegame_list(int curpos, bool load, char *dst) {
	int slot, last_slot;
	Common::InSaveFile *in;

	showMessageFormat("\xC");

	memset(dst, 0, 18 * 6);

	slot = curpos;

	while (curpos + 6 > slot) {
		if (!(in = _saveFileMan->openForLoading(gen_savename(slot))))
			break;

		in->read(dst, 18);
		delete in;
		last_slot = slot;
		if (slot < 10)
			showMessageFormat(" ");
		showMessageFormat("%d", slot);
		showMessageFormat(".%s\n", dst);
		dst += 18;
		slot++;
	}
	// while_break
	if (!load) {
		if (curpos + 6 == slot)
			slot++;
		else {
			if (slot < 10)
				showMessageFormat(" ");
			showMessageFormat("%d.\n", slot);
		}
	} else {
		if (curpos + 6 == slot) {
			if ((in = _saveFileMan->openForLoading(gen_savename(slot)))) {
				slot++;
				delete in;
			}
		}
	}

	return slot - curpos;
}

void SimonEngine::quick_load_or_save() {
	// simon1demo subroutines are missing too many segments
	// original demo didn't allow load or save either.
	if (getGameId() == GID_SIMON1DEMO)
		return;

	bool success;
	char buf[50];

	char *filename = gen_savename(_saveLoadSlot);
	if (_saveLoadType == 2) {
		Subroutine *sub;
		success = load_game(_saveLoadSlot);
		if (!success) {
			sprintf(buf, "Failed to load game state to file:\n\n%s", filename);
		} else {
			// Redraw Inventory
			mouseOff();
			drawIconArray(2, getItem1Ptr(), 0, 0);
			mouseOn();
			// Reset engine?
			vc_set_bit_to(97, true);
			sub = getSubroutineByID(100);
			startSubroutine(sub);
		}
	} else {
		success = save_game(_saveLoadSlot, _saveLoadName);
		if (!success)
			sprintf(buf, "Failed to save game state to file:\n\n%s", filename);
	}

	if (!success) {
		GUI::MessageDialog dialog(buf, "OK");
		dialog.runModal();

	} else if (_saveLoadType == 1) {
		sprintf(buf, "Successfully saved game state in file:\n\n%s", filename);
		GUI::TimedMessageDialog dialog(buf, 1500);
		dialog.runModal();

	}

	_saveLoadType = 0;
}

void SimonEngine::savegame_dialog(char *buf) {
	int i;

	o_unk_132_helper_3();

	i = display_savegame_list(_saveLoadRowCurPos, _saveOrLoad, buf);

	_saveDialogFlag = true;

	if (i != 7) {
		i++;
		if (!_saveOrLoad)
			i++;
		_saveDialogFlag = false;
	}

	if (!--i)
		return;

	do {
		clear_hitarea_bit_0x40(0xd0 + i - 1);
	} while (--i);
}

void SimonEngine::save_or_load_dialog(bool load) {
	time_t save_time;
	int number_of_savegames;
	int i;
	int unk132_result;
	FillOrCopyStruct *fcs;
	char *name;
	int name_len;
	bool b;
	char buf[108];

	_saveOrLoad = load;

	save_time = time(NULL);

	_copyPartialMode = 1;

	number_of_savegames = count_savegames();
	if (!load)
		number_of_savegames++;
	number_of_savegames -= 6;
	if (number_of_savegames < 0)
		number_of_savegames = 0;
	number_of_savegames++;
	_numSaveGameRows = number_of_savegames;

	_saveLoadRowCurPos = 1;
	if (!load)
		_saveLoadRowCurPos = number_of_savegames;

	_saveLoadFlag = false;

restart:;
	do {
		i = o_unk_132_helper(&b, buf);
	} while (!b);

	if (i == 205)
		goto get_out;
	if (!load) {
		// if_1
	if_1:;
		unk132_result = i;

		set_hitarea_bit_0x40(0xd0 + i);
		leaveHitAreaById(0xd0 + i);

		// some code here

		fcs = _windowArray[5];

		fcs->textRow = unk132_result;

		if (_language == Common::HB_ISR) { //Hebrew
			// init x offset with a 2 character savegame number + a period (18 pix)
			fcs->textColumn = 3;
			fcs->textColumnOffset = 6;
			fcs->textLength = 3;
		} else {
			// init x offset with a 2 character savegame number + a period (18 pix)
			fcs->textColumn = 2;
			fcs->textColumnOffset = 2;
			fcs->textLength = 3;
		}

		name = buf + i * 18;

		// now process entire savegame name to get correct x offset for cursor
		name_len = 0;
		while (name[name_len]) {
			if (_language == Common::HB_ISR) { //Hebrew
				byte width = 6;
				if (name[name_len] >= 64 && name[name_len] < 91)
					width = _hebrew_char_widths [name[name_len] - 64];
				fcs->textLength++;
				fcs->textColumnOffset -= width;
				if (fcs->textColumnOffset < width) {
					fcs->textColumnOffset += 8;
					fcs->textColumn++;
				}
			} else {
				fcs->textLength++;
				fcs->textColumnOffset += 6;
				if (name[name_len] == 'i' || name[name_len] == 'l')
					fcs->textColumnOffset -= 2;
				if (fcs->textColumnOffset >= 8) {
					fcs->textColumnOffset -= 8;
					fcs->textColumn++;
				}
			}
			name_len++;
		}
		// while_1_end

		// do_3_start
		for (;;) {
			video_putchar(fcs, 0x7f);

			_saveLoadFlag = true;

			// do_2
			do {
				i = o_unk_132_helper(&b, buf);

				if (b) {
					if (i == 205)
						goto get_out;
					clear_hitarea_bit_0x40(0xd0 + unk132_result);
					if (_saveLoadFlag) {
						o_clear_character(_windowArray[5], 8);
						// move code
					}
					goto if_1;
				}

				// is_not_b
				if (!_saveLoadFlag) {
					clear_hitarea_bit_0x40(0xd0 + unk132_result);
					goto restart;
				}
			} while (i >= 0x80 || i == 0);

			// after_do_2
			o_clear_character(_windowArray[5], 8);
			if (i == 10 || i == 13)
				break;
			if (i == 8) {
				// do_backspace
				if (name_len != 0) {
					int x;
					byte m;

					name_len--;
					m = name[name_len];

					if (_language == Common::HB_ISR) //Hebrew
						x = 8;
					else
						x = (name[name_len] == 'i' || name[name_len] == 'l') ? 1 : 8;

					name[name_len] = 0;

					o_clear_character(_windowArray[5], x, m);
				}
			} else if (i >= 32 && name_len != 17) {
				name[name_len++] = i;

				video_putchar(_windowArray[5], i);
			}
		}

		// do_save
		if (!save_game(_saveLoadRowCurPos + unk132_result, buf + unk132_result * 18))
			o_file_error(_windowArray[5], true);
	} else {
		if (!load_game(_saveLoadRowCurPos + i))
			o_file_error(_windowArray[5], false);
	}

get_out:;
	o_unk_132_helper_3();

	_base_time = time(NULL) - save_time + _base_time;
	_copyPartialMode = 0;

	dx_copy_rgn_from_3_to_2(94, 208, 46, 80);

	i = _timer4;
	do {
		delay(10);
	} while (i == _timer4);

	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void SimonEngine::o_file_error(FillOrCopyStruct *fcs, bool save_error) {
	HitArea *ha;
	const char *string, *string2;

	if (save_error) {
		string = "\r       Save failed.";
		string2 = "\r       Disk error.";
	} else {
		string = "\r       Load failed.";
		string2 = "\r     File not found.";
	}

	video_putchar(fcs, 0xC);
	for (; *string; string++)
		video_putchar(fcs, *string);
	for (; *string2; string2++)
		video_putchar(fcs, *string2);

	fcs->textColumn = (fcs->width >> 1) - 3;
	fcs->textRow = fcs->height - 1;
	fcs->textLength = 0;

	string = "[ OK ]";
	for (; *string; string++)
		video_putchar(fcs, *string);

	ha = findEmptyHitArea();
	ha->x = ((fcs->width >> 1) + (fcs->x - 3)) << 3;
	ha->y = (fcs->height << 3) + fcs->y - 8;
	ha->width = 48;
	ha->height = 8;
	ha->flags = 0x20;
	ha->id = 0x7FFF;
	ha->layer = 0x3EF;

loop:;
	_lastHitArea = _lastHitArea3 = 0;

	do {
		delay(1);
	} while (_lastHitArea3 == 0);

	ha = _lastHitArea;
	if (ha == NULL || ha->id != 0x7FFF)
		goto loop;

	// Return
	delete_hitarea(0x7FFF);
}

bool SimonEngine::save_game(uint slot, char *caption) {
	Common::OutSaveFile *f;
	uint item_index, num_item, i, j;
	TimeEvent *te;

	_lockWord |= 0x100;

	f = _saveFileMan->openForSaving(gen_savename(slot));
	if (f == NULL) {
		_lockWord &= ~0x100;
		return false;
	}

	if (getGameType() == GType_FF) {
		f->write(caption, 100);
	} else {
		f->write(caption, 18);
	}

	f->writeUint32BE(_itemArrayInited - 1);
	f->writeUint32BE(0xFFFFFFFF);
	f->writeUint32BE(0);
	f->writeUint32BE(0);

	i = 0;
	for (te = _firstTimeStruct; te; te = te->next)
		i++;
	f->writeUint32BE(i);

	for (te = _firstTimeStruct; te; te = te->next) {
		f->writeUint32BE(te->time + _base_time);
		f->writeUint16BE(te->subroutine_id);
	}

	item_index = 1;
	for (num_item = _itemArrayInited - 1; num_item; num_item--) {
		Item *item = _itemArrayPtr[item_index++];

		f->writeUint16BE(item->parent);
		f->writeUint16BE(item->sibling);
		f->writeUint16BE(item->state);
		f->writeUint16BE(item->classFlags);

		Child1 *child1 = (Child1 *)findChildOfType(item, 1);
		if (child1) {
			f->writeUint16BE(child1->fr2);
		}

		Child2 *child2 = (Child2 *)findChildOfType(item, 2);
		if (child2) {
			f->writeUint32BE(child2->avail_props);
			i = child2->avail_props & 1;

			for (j = 1; j < 16; j++) {
				if ((1 << j) & child2->avail_props) {
					f->writeUint16BE(child2->array[i++]);
				}
			}
		}

		Child9 *child9 = (Child9 *)findChildOfType(item, 9);
		if (child9) {
			for (i = 0; i != 4; i++) {
				f->writeUint16BE(child9->array[i]);
			}
		}
	}

	// write the 255 variables
	for (i = 0; i != 255; i++) {
		f->writeUint16BE(readVariable(i));
	}

	// write the items in array 6
	for (i = 0; i != 10; i++) {
		f->writeUint16BE(itemPtrToID(_itemArray6[i]));
	}

	// Write the bits in array 1 & 2
	for (i = 0; i != 32; i++)
		f->writeUint16BE(_bitArray[i]);

	// Write the bits in array 3
	if (getGameType() == GType_FF) {
		for (i = 33; i != 48; i++)
			f->writeUint16BE(_bitArray[i]);
	}

	f->flush();
	bool result = !f->ioFailed();

	delete f;
	_lockWord &= ~0x100;

	return result;
}

char *SimonEngine::gen_savename(int slot) {
	static char buf[15];

	if (getGameType() == GType_FF) {
		if (slot == 999)
			sprintf(buf, "save.%.3d", slot);
		else
			sprintf(buf, "feeble.%.3d", slot);
	} else if (getGameType() == GType_SIMON2) {
		sprintf(buf, "simon2.%.3d", slot);
	} else {
		sprintf(buf, "simon1.%.3d", slot);
	}
	return buf;
}

bool SimonEngine::load_game(uint slot) {
	char ident[100];
	Common::InSaveFile *f;
	uint num, item_index, i, j;

	_lockWord |= 0x100;

	f = _saveFileMan->openForLoading(gen_savename(slot));
	if (f == NULL) {
		_lockWord &= ~0x100;
		return false;
	}

	if (getGameType() == GType_FF) {
		f->read(ident, 100);
	} else {
		f->read(ident, 18);
	}

	num = f->readUint32BE();

	if (f->readUint32BE() != 0xFFFFFFFF || num != _itemArrayInited - 1) {
		delete f;
		_lockWord &= ~0x100;
		return false;
	}

	f->readUint32BE();
	f->readUint32BE();
	_noParentNotify = true;


	// add all timers
	killAllTimers();
	for (num = f->readUint32BE(); num; num--) {
		uint32 timeout = f->readUint32BE();
		uint16 func_to_call = f->readUint16BE();
		addTimeEvent(timeout, func_to_call);
	}

	item_index = 1;
	for (num = _itemArrayInited - 1; num; num--) {
		Item *item = _itemArrayPtr[item_index++], *parent_item;

		uint parent = f->readUint16BE();
		uint sibling = f->readUint16BE();

		parent_item = derefItem(parent);

		setItemParent(item, parent_item);

		if (parent_item == NULL) {
			item->parent = parent;
			item->sibling = sibling;
		}

		item->state = f->readUint16BE();
		item->classFlags = f->readUint16BE();

		Child1 *child1 = (Child1 *)findChildOfType(item, 1);
		if (child1 != NULL) {
			child1->fr2 = f->readUint16BE();
		}

		Child2 *child2 = (Child2 *)findChildOfType(item, 2);
		if (child2 != NULL) {
			child2->avail_props = f->readUint32BE();
			i = child2->avail_props & 1;

			for (j = 1; j < 16; j++) {
				if ((1 << j) & child2->avail_props) {
					child2->array[i++] = f->readUint16BE();
				}
			}
		}

		Child9 *child9 = (Child9 *) findChildOfType(item, 9);
		if (child9) {
			for (i = 0; i != 4; i++) {
				child9->array[i] = f->readUint16BE();
			}
		}
	}


	// read the 255 variables
	for (i = 0; i != 255; i++) {
		writeVariable(i, f->readUint16BE());
	}

	// read the items in array 6
	for (i = 0; i != 10; i++) {
		_itemArray6[i] = derefItem(f->readUint16BE());
	}

	// Read the bits in array 1 & 2
	for (i = 0; i != 32; i++)
		_bitArray[i] = f->readUint16BE();

	// Read the bits in array 3
	if (getGameType() == GType_FF) {
		for (i = 33; i != 48; i++)
			_bitArray[i] = f->readUint16BE();
	}

	if (f->ioFailed()) {
		error("load failed");
	}

	delete f;

	_noParentNotify = false;

	_lockWord &= ~0x100;

	return true;
}

} // End of namespace Simon
