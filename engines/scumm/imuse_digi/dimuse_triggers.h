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

#if !defined(SCUMM_IMUSE_DIGI_TRIGGERS_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_IMUSE_DIGI_TRIGGERS_H

#include "common/scummsys.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "scumm/imuse_digi/dimuse_defs.h"

namespace Scumm {

class IMuseDigiTriggersHandler {

private:
	IMuseDigital *_engine;
	IMuseDigiTrigger _trigs[DIMUSE_MAX_TRIGGERS] = {};
	IMuseDigiDefer _defers[DIMUSE_MAX_DEFERS] = {};

	int  _defersOn = 0;
	int  _midProcessing = 0;
	char _textBuffer[256] = {};
	char _emptyMarker[1] = {};

public:
	IMuseDigiTriggersHandler(IMuseDigital *engine);
	~IMuseDigiTriggersHandler();

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
	int  deinit();

};

} // End of namespace Scumm
#endif
