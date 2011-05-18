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

#ifndef M4_CONSOLE_H
#define M4_CONSOLE_H

#include "gui/debugger.h"

namespace M4 {

class MadsM4Engine;
class MadsEngine;

class Console : public GUI::Debugger {
private:
	MadsM4Engine *_vm;

	bool cmdLoadScene(int argc, const char **argv);
	bool cmdStartingScene(int argc, const char **argv);
	bool cmdShowHotSpots(int argc, const char **argv);
	bool cmdListHotSpots(int argc, const char **argv);
	bool cmdPlaySound(int argc, const char **argv);
	bool cmdPlayDSRSound(int argc, const char **argv);
	bool cmdShowResources(int argc, const char **argv);
	bool cmdShowCodes(int argc, const char **argv);
	bool cmdDumpFile(int argc, const char **argv);
	bool cmdShowSprite(int argc, const char **argv);
	bool cmdStartConversation(int argc, const char **argv);
	bool cmdShowTextview(int argc, const char **argv);
	bool cmdShowAnimview(int argc, const char **argv);

public:
	Console(MadsM4Engine *vm);
	virtual ~Console();
};

class MadsConsole : public Console {
private:
	MadsEngine *_vm;

	bool cmdObject(int argc, const char **argv);
	bool cmdMessage(int argc, const char **argv);
	bool cmdSceneInfo(int argc, const char **argv);
	bool cmdPlayAnimation(int argc, const char **argv);

public:
	MadsConsole(MadsEngine *vm);
	virtual ~MadsConsole() {}
};

class M4Console : public Console {
private:
	M4Engine *_vm;

	bool cmdSceneInfo(int argc, const char **argv);
public:
	M4Console(M4Engine *vm);
	virtual ~M4Console() {}
};

} // End of namespace M4


#endif
