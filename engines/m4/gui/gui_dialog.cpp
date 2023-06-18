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

#include "m4/gui/gui_dialog.h"
#include "m4/mem/memman.h"
#include "m4/globals.h"

namespace M4 {

bool gui_dialog_init() {
	_G(listboxSearchStr)[0] = '\0';
	return true;
}

void gui_dialog_shutdown() {
}

void vmng_TextScrn_Destroy(TextScrn *myTextScrn) {
	TextItem *myTextItems;
	TextItem *tempTextItem;
	tempTextItem = myTextItems = myTextScrn->myTextItems;

	while (tempTextItem) {
		myTextItems = myTextItems->next;
		mem_free(tempTextItem->prompt);
		mem_free((void *)tempTextItem);
		tempTextItem = myTextItems;
	}

	delete myTextScrn->textScrnBuffer;
	mem_free((void *)myTextScrn);
}

void vmng_Dialog_Destroy(Dialog *d) {
	error("TODO: vmng_Dialog_Destroy");
}

} // namespace M4
