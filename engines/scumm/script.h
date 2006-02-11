/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef SCRIPT_H
#define SCRIPT_H

#include "base/engine.h"

namespace Scumm {

/**
 * The number of script slots, which determines the maximal number
 * of concurrently running scripts.
 * WARNING: Do NOT changes this value unless you really have to, as
 * this will break savegame compatibility if done carelessly. If you
 * have to change it, make sure you update saveload.cpp accordingly!
 */
enum {
	NUM_SCRIPT_SLOT = 80
};

/* Script status type (slot.status) */
enum {
	ssDead = 0,
	ssPaused = 1,
	ssRunning = 2
};

struct ScriptSlot {
	uint32 offs;
	int32 delay;
	uint16 number;
	uint16 delayFrameCount;
	bool freezeResistant, recursive;
	bool didexec;
	byte status;
	byte where;
	byte freezeCount;
	byte cutsceneOverride;
	byte cycle;
};

struct NestedScript {
	uint16 number;
	uint8 where;
	uint8 slot;
};

struct VirtualMachineState {
	uint32 cutScenePtr[5];
	byte cutSceneScript[5];
	int16 cutSceneData[5];
	int16 cutSceneScriptIndex;
	byte cutSceneStackPointer;
	ScriptSlot slot[NUM_SCRIPT_SLOT];
	int32 localvar[NUM_SCRIPT_SLOT][26];

	NestedScript nest[15];
	byte numNestedScripts;
};

} // End of namespace Scumm

#endif
