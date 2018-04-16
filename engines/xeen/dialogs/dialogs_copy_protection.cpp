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

#include "xeen/dialogs/dialogs_copy_protection.h"
#include "xeen/dialogs/dialogs_input.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

bool CopyProtection::show(XeenEngine *vm) {
	CopyProtection *dlg = new CopyProtection(vm);
	int result = dlg->execute();
	delete dlg;

	return result;
}

CopyProtection::CopyProtection(XeenEngine *vm) : Input(vm, &(*vm->_windows)[11]) {
	loadEntries();
}

bool CopyProtection::execute() {
	EventsManager &events = *_vm->_events;
	Sound &sound = *_vm->_sound;
	Window &w = *_window;
	bool result = false;
	Common::String line;

	// Choose a random entry
	ProtectionEntry &protEntry = _entries[_vm->getRandomNumber(_entries.size() - 1)];
	Common::String msg = Common::String::format(Res.WHATS_THE_PASSWORD,
		protEntry._pageNum, protEntry._lineNum, protEntry._wordNum);

	w.open();
	w.writeString(msg);
	w.update();

	for (int tryNum = 0; tryNum < 3 && !_vm->shouldExit(); ++tryNum) {
		line.clear();
		if (getString(line, 20, 200, false) && !line.compareToIgnoreCase(protEntry._text)) {
			sound.playFX(20);
			result = true;
			break;
		}

		sound.playFX(21);
		w.writeString("\x3l\v040\n\x4""200");
		w.writeString(Res.PASSWORD_INCORRECT);
		w.update();

		events.updateGameCounter();
		events.wait(50, false);
	}

	w.close();
	return result;
}

void CopyProtection::loadEntries() {
	FileManager &files = *g_vm->_files;
	File f(files._ccNum ? "timer.drv" : "cpstruct");
	ProtectionEntry pe;
	byte seed = 0;
	char text[13];

	while (f.pos() < f.size()) {
		pe._pageNum = f.readByte() ^ seed++;
		pe._lineNum = f.readByte() ^ seed++;
		pe._wordNum = f.readByte() ^ seed++;

		for (int idx = 0; idx < 13; ++idx)
			text[idx] = f.readByte() ^ seed++;
		text[12] = '\0';
		pe._text = Common::String(text);

		_entries.push_back(pe);
	}
}

} // End of namespace Xeen
