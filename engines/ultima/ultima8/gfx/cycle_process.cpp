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

#include "ultima/ultima8/gfx/cycle_process.h"
#include "ultima/ultima8/gfx/palette.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

CycleProcess *CycleProcess::_instance = nullptr;

// Which color to cycle in each of the palette colors 8~14
static const bool CYCLE_COL_FLAGS[7][3] = {
	{ 1, 0, 0 },
	{ 0, 0, 1 },
	{ 1, 0, 0 },
	{ 0, 0, 1 },
	{ 1, 1, 0 },
	{ 1, 1, 1 },
	{ 0, 1, 0 }
};

static const uint8 CYCLE_INIT_COLS[7][3] = {{ 0, 0, 0 }, { 0, 0, 0 }, { 124, 0, 0 }, { 0, 0, 124 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }};

static const bool CYCLE_RANDOMIZE[7] = {
	false, false, false, false, false, false, true
};

// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(CycleProcess)

static inline void copyColor(uint8 *dst, const uint8 *src) {
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

CycleProcess::CycleProcess() : Process(), _running(1) {
	_instance = this;
	_ticksPerRun = 2;
	_type = 1; // persistent
	for (int i = 0; i < 7; i++) {
		copyColor(_cycleColData[i], CYCLE_INIT_COLS[i]);
	}
}

CycleProcess::~CycleProcess(void) {
	if (_instance == this)
		_instance = nullptr;
}

CycleProcess *CycleProcess::get_instance() {
	return _instance;
}

static bool cycleColor(uint8 *col, const bool flags[3]) {
	bool wrapped = false;

	for (int i = 0; i < 3; i++) {
	  if (flags[i]) {
		col[i] += 8;
	  }
	  if (col[i] > 252) {
		col[i] = 0;
		wrapped = true;
	  }
	}
	return wrapped;
}

void CycleProcess::run() {
	if (!_running)
		return;

	PaletteManager  *pm = PaletteManager::get_instance();
	Palette *pal = pm->getPalette(PaletteManager::Pal_Game);

	// Step 1: Rotate 7 colors (1~7)
	byte r1, g1, b1;
	// tmp copy of color 1
	pal->get(1, r1, g1, b1);
	for (int i = 1; i < 7; i++) {
		byte r, g, b;
		pal->get(i + 1, r, g, b);
		pal->set(i, r, g, b);
	}
	// move color 1 -> color 7
	pal->set(7, r1, g1, b1);

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();

	// Step 2: Cycle 7 other colors 8~14 by increasing their value
	// until they hit max, then reset.
	for (int i = 0; i < 7; i++) {
		bool wrapped = cycleColor(_cycleColData[i], CYCLE_COL_FLAGS[i]);
		if (CYCLE_RANDOMIZE[i] && wrapped) {
			_cycleColData[i][0] += rs.getRandomNumber(9);
			_cycleColData[i][1] += rs.getRandomNumber(9);
			_cycleColData[i][2] += rs.getRandomNumber(9);
		}
		pal->set(i + 8, _cycleColData[i][0], _cycleColData[i][1], _cycleColData[i][2]);
	}

	// Update the cached palette.
	pm->updatedPalette(PaletteManager::Pal_Game, 16);
}

void CycleProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeByte(_running);
}

bool CycleProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_running = rs->readByte();
	_instance = this; //static

	_type = 1; // should be persistent but older savegames may not know that.
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
