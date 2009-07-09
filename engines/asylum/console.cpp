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

// Console module

#include "asylum/asylum.h"
#include "asylum/console.h"
#include "asylum/scene.h"

namespace Asylum {

extern int g_debugPolygons;
extern int g_debugBarriers;

Console::Console(AsylumEngine *vm) : GUI::Debugger() {
	_vm = vm;

	DCmd_Register("video",			WRAP_METHOD(Console, cmdPlayVideo));
	DCmd_Register("script",			WRAP_METHOD(Console, cmdRunScript));
	DCmd_Register("scene",			WRAP_METHOD(Console, cmdChangeScene));

	DVar_Register("showpolygons",  &g_debugPolygons, DVAR_INT, 0);
	DVar_Register("showbarriers",  &g_debugBarriers, DVAR_INT, 0);
}

Console::~Console() {
}

bool Console::cmdPlayVideo(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage %s <video number>\n", argv[0]);
		return true;
	}
	
	ScriptMan.setDelayedVideoIndex(atoi(argv[1]));

	return false;
}

bool Console::cmdRunScript(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage %s <script number>\n", argv[0]);
		return true;
	}
	
	/*
	_vm->_interpreter->_currentScriptIndex = atoi(argv[1]);
	_vm->_interpreter->_currentLine = 0;
	*/

	return false;
}

bool Console::cmdChangeScene(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage %s <scene number>\n", argv[0]);
		return true;
	}
	
	ScriptMan.setDelayedSceneIndex(atoi(argv[1]));

	return false;
}


} // End of namespace Asulym
