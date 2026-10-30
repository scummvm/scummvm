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

#ifndef GRIM_IMUSE_TRIGGERS_H
#define GRIM_IMUSE_TRIGGERS_H

#include "common/scummsys.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "engines/grim/imuse/imuse_defs.h"

namespace Grim {

class IMuseTriggersHandler {

private:
	IMuse *_engine;
	Common::Mutex *_mutex;
	IMuseTrigger _trigs[IMUSE_MAX_TRIGGERS] = {};
	IMuseDefer _defers[IMUSE_MAX_DEFERS] = {};

	int  _defersOn = 0;
	int  _midProcessing = 0;
	char _textBuffer[256] = {};
	char _emptyMarker[1] = {};

public:
	IMuseTriggersHandler(IMuse *engine, Common::Mutex *mutex);
	~IMuseTriggersHandler();

	int  init();
	int  clearAllTriggers();
	void saveLoad(Common::Serializer &ser);
	int  setTrigger(int soundId, char *marker, int opcode, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n);
	int  checkTrigger(int soundId, char *marker, int opcode);
	int  clearTrigger(int soundId, char *marker, int opcode);
	void processTriggers(int soundId, char *marker);
	int  deferCommand(int count, int opcode, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n);
	void loop();
	int  countPendingSounds(int soundId);

};

} // End of namespace Grim
#endif
