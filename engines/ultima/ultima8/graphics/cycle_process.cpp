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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/cycle_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/graphics/palette.h"

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
	uint8 *paldata = pal->_palette;

	// Step 1: Rotate 7 colors (1~7)
	uint8 tmpcol[3];
	// tmp copy of color 1
	copyColor(tmpcol, paldata + 3);
	for (int i = 1; i < 7; i++) {
		uint8 *dstptr = paldata + i * 3;
		const uint8 *srcptr = paldata + (i + 1) * 3;
		copyColor(dstptr, srcptr);
	}
	// move color 1 -> color 7
	copyColor(paldata + 3 * 7, tmpcol);

	// Step 2: Cycle 7 other colors 8~14 by increasing their value
	// until they hit max, then reset.
	for (int i = 0; i < 7; i++) {
		bool wrapped = cycleColor(_cycleColData[i], CYCLE_COL_FLAGS[i]);
		if (CYCLE_RANDOMIZE[i] && wrapped) {
			_cycleColData[i][0] += (getRandom() % 10);
			_cycleColData[i][1] += (getRandom() % 10);
			_cycleColData[i][2] += (getRandom() % 10);
		}
		uint8 *dstptr = paldata + (i + 8) * 3;
		copyColor(dstptr, _cycleColData[i]);
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
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
