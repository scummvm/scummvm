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

#include "common/file.h"
#include "gui/debugger.h"

#include "trecision/console.h"
#include "trecision/dialog.h"
#include "trecision/scheduler.h"
#include "trecision/text.h"
#include "trecision/trecision.h"

namespace Trecision {

Console::Console(TrecisionEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("room",			WRAP_METHOD(Console, Cmd_Room));
	registerCmd("filedump",		WRAP_METHOD(Console, Cmd_DumpFile));
	registerCmd("dialog",		WRAP_METHOD(Console, Cmd_Dialog));
	registerCmd("item",			WRAP_METHOD(Console, Cmd_Item));
	registerCmd("say",			WRAP_METHOD(Console, Cmd_Say));
}

Console::~Console() {
}

bool Console::Cmd_Room(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Current room: %d\n", _vm->_curRoom);
		debugPrintf("Use %s <roomId> to teleport\n", argv[0]);
		return true;
	}

	const int newRoom = atoi(argv[1]);
	_vm->changeRoom(newRoom);

	return false;
}

bool Console::Cmd_DumpFile(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <file name>\n", argv[0]);
		return true;
	}

	Common::String fileName = argv[1];

	Common::SeekableReadStream *dataFile = _vm->_dataFile.createReadStreamForCompressedMember(fileName);

	Common::DumpFile *outFile = new Common::DumpFile();
	Common::String outName = fileName + ".dump";
	outFile->open(outName);
	outFile->writeStream(dataFile, dataFile->size());
	outFile->finalize();
	outFile->close();

	return true;
}

bool Console::Cmd_Dialog(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Use %s <dialogId> to start a dialog\n", argv[0]);
		return true;
	}

	const int dialogId = atoi(argv[1]);
	_vm->_dialogMgr->playDialog(dialogId);

	return false;
}

bool Console::Cmd_Item(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Use %s <itemId> to add an item to the inventory\n", argv[0]);
		debugPrintf("Use %s <itemId> remove to remove an item from the inventory\n", argv[0]);
		return true;
	}

	const int itemId = atoi(argv[1]);
	if (argc >= 3 && !scumm_stricmp(argv[2], "remove")) {
		_vm->removeIcon(itemId);
	} else {
		_vm->addIcon(itemId);
	}

	return false;
}

bool Console::Cmd_Say(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Use %s <sentenceId> to hear a sentence from Joshua\n", argv[0]);
		return true;
	}

	const uint16 sentenceId = (uint16)atoi(argv[1]);
	_vm->_textMgr->characterSay(sentenceId);

	return false;
}

} // End of namespace Trecision
