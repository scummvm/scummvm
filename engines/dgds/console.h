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

#ifndef DGDS_CONSOLE_H
#define DGDS_CONSOLE_H

#include "gui/debugger.h"

namespace Dgds {

class DgdsEngine;

class Console : public GUI::Debugger {
public:
	Console(DgdsEngine *vm);
	~Console() override {}

private:
	bool cmdFileInfo(int argc, const char **argv);
	bool cmdFileSearch(int argc, const char **argv);
	bool cmdFileDump(int argc, const char **argv);
	bool dumpImageFrame(const char *fname, int frame, const char *outpath);
	bool cmdImageDumpAll(int argc, const char **argv);
	bool cmdImageDump(int argc, const char **argv);
	bool cmdGlobal(int argc, const char **argv);
	bool cmdScene(int argc, const char **argv);
	void printOp(int indent, const char *text);
	bool cmdScriptDump(int argc, const char **argv);
	bool cmdTrigger(int argc, const char **argv);
	bool cmdSetHotAreaDebug(int argc, const char **argv);
	DgdsEngine *_vm;
};

} // End of namespace Dgds

#endif // DGDS_DGDS_H
