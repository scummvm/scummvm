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
 * $URL$
 * $Id$
 *
 */

// Console module header file

#ifndef ASYLUM_CONSOLE_H
#define ASYLUM_CONSOLE_H

#include "gui/debugger.h"

namespace Asylum {

enum kDebugLevels {
	kDebugLevelMain      = 1 << 0,
	kDebugLevelResources = 1 << 1,
	kDebugLevelSprites   = 1 << 2,
	kDebugLevelInput     = 1 << 3,
	kDebugLevelMenu      = 1 << 4,
	kDebugLevelScripts   = 1 << 5,
	kDebugLevelSound     = 1 << 6,
	kDebugLevelSavegame  = 1 << 7,
	kDebugLevelScene     = 1 << 8,
	kDebugLevelObjects   = 1 << 9,
	kDebugLevelActor     = 1 << 10
};

class AsylumEngine;

struct ActionArea;

class Console : public GUI::Debugger {
public:
	Console(AsylumEngine *vm);
	virtual ~Console(void);

private:
	AsylumEngine *_vm;

	bool cmdHelp(int argc, const char **argv);

	bool cmdListFiles(int argc, const char **argv);

	bool cmdListActions(int32 argc, const char **argv);
	bool cmdListActors(int32 argc, const char **argv);
	bool cmdListFlags(int32 argc, const char **argv);
	bool cmdShowObject(int32 argc, const char **argv);
	bool cmdListObjects(int32 argc, const char **argv);
	bool cmdShowWorldStats(int32 argc, const char **argv);

	bool cmdPlayVideo(int32 argc, const char **argv);
	bool cmdRunScript(int32 argc, const char **argv);
	bool cmdChangeScene(int32 argc, const char **argv);
	bool cmdRunEncounter(int32 argc, const char **argv);

	bool cmdToggleFlag(int32 argc, const char **argv);
};

} // End of namespace Asylum

#endif
