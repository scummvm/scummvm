/* AWE - Another World Engine
 * Copyright (C) 2004 Gregory Montoir
 * Copyright (C) 2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "stdafx.h"

#include "intern.h"
#include "logic.h"
#include "resource.h"
#include "video.h"

namespace Awe {

struct SystemStub;

struct Engine {
	enum {
		MAX_SAVE_SLOTS = 100
	};

	SystemStub *_stub;
	Logic _log;
	Resource _res;
	Video _vid;
	const char *_dataDir, *_saveDir;
	uint8 _stateSlot;

	Engine(SystemStub *stub, const char *dataDir, const char *saveDir);

	void run();
	void setup();
	void finish();
	void processInput();
	
	void makeGameStateName(uint8 slot, char *buf);
	void saveGameState(uint8 slot, const char *desc);
	void loadGameState(uint8 slot);
};

}

#endif
