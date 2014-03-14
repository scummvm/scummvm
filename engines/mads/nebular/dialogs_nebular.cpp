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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "mads/mads.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/nebular/dialogs_nebular.h"

namespace MADS {

namespace Nebular {

CopyProtectionDialog::CopyProtectionDialog(MADSEngine *vm, bool priorAnswerWrong):
		TextDialog(vm, FONT_INTERFACE, Common::Point(-1, -1), 32) {
	getHogAnusEntry(_hogEntry);

	if (priorAnswerWrong) {
		addLine("ANSWER INCORRECT!", true);
		wordWrap("\n");
		addLine("(But we'll give you another chance!)");
	} else {
		addLine("REX NEBULAR version 8.43", true);
		wordWrap("\n");
		addLine("(Copy Protection, for your convenience)");
	}
	wordWrap("\n");

	wordWrap("Now comes the part that everybody hates.  But if we don't");
	wordWrap("do this, nasty rodent-like people will pirate this game");
	wordWrap("and a whole generation of talented designers, programmers,");
	wordWrap("artists, and playtesters will go hungry, and will wander");
	wordWrap("aimlessly through the land at night searching for peace.");
	wordWrap("So let's grit our teeth and get it over with.  Just get");
	
	Common::String line = "out your copy of ";
	line += _hogEntry._bookId == 103 ? "the GAME MANUAL" : "REX'S LOGBOOK";
	line += ".  See!  That was easy.  ";
	wordWrap(line);

	line = Common::String::format("Next, just turn to page %d. On line %d, find word number %d, ",
		_hogEntry._pageNum, _hogEntry._lineNum, _hogEntry._wordNum);
	wordWrap(line);

	wordWrap("and type it on the line below (we',27h,'ve even given you");
	wordWrap("first letter as a hint).  As soon as you do that, we can get");
	wordWrap("right into this really COOL adventure game!\n");
	wordWrap("\n");
	wordWrap("                    ");
	addInput();
	wordWrap("\n");
}

bool CopyProtectionDialog::show() {
	draw();
	_vm->_events->showCursor();

	// TODO: Replace with text input
	while (!_vm->shouldQuit() && !_vm->_events->_keyPressed &&
			!_vm->_events->_mouseClicked) {
		_vm->_events->delay(1);
	}

	return true;
}

bool CopyProtectionDialog::getHogAnusEntry(HOGANUS &entry) {
	File f;
	f.open("*HOGANUS.DAT");

	// Read in the total number of entries, and randomly pick an entry to use
	int numEntries = f.readUint16LE();
	int entryIndex = _vm->getRandomNumber(numEntries - 2) + 1;

	// Read in the encrypted entry
	f.seek(28 * entryIndex + 2);
	byte entryData[28];
	f.read(entryData, 28);

	// Decrypt it
	for (int i = 0; i < 28; ++i)
		entryData[i] = ~entryData[i];

	// Fill out the fields
	entry._bookId = entryData[0];
	entry._pageNum = READ_LE_UINT16(&entryData[2]);
	entry._lineNum = READ_LE_UINT16(&entryData[4]);
	entry._wordNum = READ_LE_UINT16(&entryData[6]);
	entry._word = Common::String((char *)&entryData[8]);

	f.close();
	return true;
}


} // End of namespace Nebular

} // End of namespace MADS
