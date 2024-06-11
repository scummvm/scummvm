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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Console module header file

#ifndef ASYLUM_CONSOLE_H
#define ASYLUM_CONSOLE_H

#include "gui/debugger.h"

#include "views/insertdisc.h"
#include "views/resviewer.h"

namespace Asylum {

class AsylumEngine;

struct ActionArea;

class Console : public GUI::Debugger {
public:
	Console(AsylumEngine *vm);
	virtual ~Console(void);

private:
	AsylumEngine *_vm;
	InsertDisc _insertDisc;
	ResourceViewer _resViewer;

	bool cmdHelp(int argc, const char **argv);

	bool cmdListFiles(int argc, const char **argv);

	bool cmdListActions(int argc, const char **argv);
	bool cmdListActors(int argc, const char **argv);
	bool cmdListFlags(int argc, const char **argv);
	bool cmdShowObject(int argc, const char **argv);
	bool cmdShowAction(int argc, const char **argv);
	bool cmdListObjects(int argc, const char **argv);
	bool cmdShowWorldStats(int argc, const char **argv);

	bool cmdPlayVideo(int argc, const char **argv);
	bool cmdShowScript(int argc, const char **argv);
	bool cmdRunScript(int argc, const char **argv);
	bool cmdKillScript(int argc, const char **argv);
	bool cmdInsertDisc(int argc, const char **argv);
	bool cmdChangeScene(int argc, const char **argv);
	bool cmdRunPuzzle(int argc, const char **argv);

	bool cmdGetStatus(int argc, const char **argv);
	bool cmdSetStatus(int argc, const char **argv);

	bool cmdRunEncounter(int argc, const char **argv);
	bool cmdShowEncounter(int argc, const char **argv);

	bool cmdListItems(int argc, const char **argv);
	bool cmdAddToInventory(int argc, const char **argv);
	bool cmdRemoveFromInventory(int argc, const char **argv);

	bool cmdSetPalette(int argc, const char **argv);
	bool cmdViewResource(int argc, const char **argv);
	bool cmdDrawActionArea(int argc, const char **argv);

	bool cmdToggleFlag(int argc, const char **argv);
};

} // End of namespace Asylum

#endif // ASYLUM_CONSOLE_H
