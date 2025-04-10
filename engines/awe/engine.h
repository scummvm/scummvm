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

#ifndef AWE_ENGINE_H
#define AWE_ENGINE_H

#include "awe/intern.h"
#include "awe/logic.h"
#include "awe/resource.h"
#include "awe/video.h"

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

} // namespace Awe

#endif
