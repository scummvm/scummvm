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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "xeen/dialogs/dialogs_difficulty.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

int DifficultyDialog::show(XeenEngine *vm) {
	DifficultyDialog *dlg = new DifficultyDialog(vm);
	int result = dlg->execute();
	delete dlg;

	return result;
}

DifficultyDialog::DifficultyDialog(XeenEngine *vm) : ButtonContainer(vm) {
	loadButtons();
}

int DifficultyDialog::execute() {
	EventsManager &events = *_vm->_events;
	Windows &windows = *_vm->_windows;

	Window &w = windows[6];
	w.open();
	w.writeString(Res.DIFFICULTY_TEXT);
	drawButtons(&w);

	int result = -1;
	while (!_vm->shouldExit()) {
		events.pollEventsAndWait();
		checkEvents(_vm);

		if (_buttonValue == Common::KEYCODE_a)
			result = ADVENTURER;
		else if (_buttonValue == Common::KEYCODE_w)
			result = WARRIOR;
		else if (_buttonValue != Common::KEYCODE_ESCAPE)
			continue;

		break;
	}

	w.close();
	return result;
}

void DifficultyDialog::loadButtons() {
	_sprites.load("choice.icn");
	addButton(Common::Rect(68, 167, 158, 187), Common::KEYCODE_a, &_sprites);
	addButton(Common::Rect(166, 167, 256, 187), Common::KEYCODE_w, &_sprites);
}

} // End of namespace Xeen
