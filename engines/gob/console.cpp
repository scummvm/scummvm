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

#include "gob/console.h"
#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/dataio.h"
#include "gob/cheater.h"

namespace Gob {

GobConsole::GobConsole(GobEngine *vm) : GUI::Debugger(), _vm(vm), _cheater(0) {
	registerCmd("varSize",      WRAP_METHOD(GobConsole, cmd_varSize));
	registerCmd("dumpVars",     WRAP_METHOD(GobConsole, cmd_dumpVars));
	registerCmd("var8",         WRAP_METHOD(GobConsole, cmd_var8));
	registerCmd("var16",        WRAP_METHOD(GobConsole, cmd_var16));
	registerCmd("var32",        WRAP_METHOD(GobConsole, cmd_var32));
	registerCmd("varString",    WRAP_METHOD(GobConsole, cmd_varString));
	registerCmd("cheat",        WRAP_METHOD(GobConsole, cmd_cheat));
	registerCmd("listArchives", WRAP_METHOD(GobConsole, cmd_listArchives));
}

GobConsole::~GobConsole() {
}

void GobConsole::registerCheater(Cheater *cheater) {
	_cheater = cheater;
}

void GobConsole::unregisterCheater() {
	_cheater = 0;
}

bool GobConsole::cmd_varSize(int argc, const char **argv) {
	debugPrintf("Size of the variable space: %d bytes\n", _vm->_inter->_variables->getSize());
	return true;
}

bool GobConsole::cmd_dumpVars(int argc, const char **argv) {
	if (!_vm->_inter->_variables)
		return true;

	Common::DumpFile file;

	if (!file.open("variables.dmp"))
		return true;

	file.write(_vm->_inter->_variables->getAddressOff8(0), _vm->_inter->_variables->getSize());

	file.flush();
	file.close();

	return true;
}

bool GobConsole::cmd_var8(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: var8 <var offset> (<value>)\n");
		return true;
	}

	uint32 varNum = atoi(argv[1]);

	if (varNum >= _vm->_inter->_variables->getSize()) {
		debugPrintf("Variable offset out of range\n");
		return true;
	}

	if (argc > 2) {
		uint32 varVal = atoi(argv[2]);
		_vm->_inter->_variables->writeOff8(varNum, varVal);
	}

	debugPrintf("var8_%d = %d\n", varNum, _vm->_inter->_variables->readOff8(varNum));

	return true;
}

bool GobConsole::cmd_var16(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: var16 <var offset> (<value>)\n");
		return true;
	}

	uint32 varNum = atoi(argv[1]);

	if ((varNum + 1) >= _vm->_inter->_variables->getSize()) {
		debugPrintf("Variable offset out of range\n");
		return true;
	}

	if (argc > 2) {
		uint32 varVal = atoi(argv[2]);
		_vm->_inter->_variables->writeOff16(varNum, varVal);
	}

	debugPrintf("var16_%d = %d\n", varNum, _vm->_inter->_variables->readOff16(varNum));

	return true;
}

bool GobConsole::cmd_var32(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: var32 <var offset> (<value>)\n");
		return true;
	}

	uint32 varNum = atoi(argv[1]);

	if ((varNum + 3) >= _vm->_inter->_variables->getSize()) {
		debugPrintf("Variable offset out of range\n");
		return true;
	}

	if (argc > 2) {
		uint32 varVal = atoi(argv[2]);
		_vm->_inter->_variables->writeOff32(varNum, varVal);
	}

	debugPrintf("var8_%d = %d\n", varNum, _vm->_inter->_variables->readOff32(varNum));

	return true;
}

bool GobConsole::cmd_varString(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: varString <var offset> (<value>)\n");
		return true;
	}

	uint32 varNum = atoi(argv[1]);

	if (varNum >= _vm->_inter->_variables->getSize()) {
		debugPrintf("Variable offset out of range\n");
		return true;
	}

	if (argc > 2) {
		uint32 maxLength = _vm->_inter->_variables->getSize() - varNum;

		Common::strlcpy(_vm->_inter->_variables->getAddressOffString(varNum), argv[2], maxLength);
	}

	debugPrintf("varString_%d = \"%s\"\n", varNum, _vm->_inter->_variables->getAddressOffString(varNum));

	return true;
}

bool GobConsole::cmd_cheat(int argc, const char **argv) {
	if (_cheater)
		return _cheater->cheat(*this);

	return true;
}

bool GobConsole::cmd_listArchives(int argc, const char **argv) {
	Common::Array<ArchiveInfo> info;

	_vm->_dataIO->getArchiveInfo(info);

	debugPrintf("   Archive    | Base | FileCount\n");
	debugPrintf("--------------------------------\n");
	for (Common::Array<ArchiveInfo>::const_iterator it = info.begin(); it != info.end(); ++it)
		if (!it->name.empty())
		debugPrintf("%13s |   %d  | %d\n", it->name.c_str(), it->base, it->fileCount);

	return true;
}

} // End of namespace Gob
