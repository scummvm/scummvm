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

// Palette animation module
#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/events.h"

#include "saga/palanim.h"
#include "saga/interface.h"

namespace Saga {

PalAnim::PalAnim(SagaEngine *vm) : _vm(vm) {
	_loaded = false;
	_entryCount = 0;
	_entries = NULL;
}

PalAnim::~PalAnim() {
}

int PalAnim::loadPalAnim(const byte *resdata, size_t resdata_len) {
	void *test_p;

	uint16 i;

	if (_loaded) {
		freePalAnim();
	}

	if (resdata == NULL) {
		return FAILURE;
	}

	MemoryReadStreamEndian readS(resdata, resdata_len, _vm->isBigEndian());

	if (_vm->getGameId() == GID_IHNM) {
		return SUCCESS;
	}

	_entryCount = readS.readUint16();

	debug(3, "PalAnim::loadPalAnim(): Loading %d PALANIM entries.", _entryCount);

	test_p = malloc(_entryCount * sizeof(PalanimEntry));
	_entries = (PalanimEntry *)test_p;

	for (i = 0; i < _entryCount; i++) {
		int color_count;
		int pal_count;
		int p, c;

		_entries[i].cycle = 0;

		color_count = readS.readUint16();
		pal_count = readS.readUint16();

		_entries[i].pal_count = pal_count;
		_entries[i].color_count = color_count;

		debug(2, "PalAnim::loadPalAnim(): Entry %d: Loading %d palette indices.\n", i, pal_count);

		test_p = malloc(sizeof(char) * pal_count);
		_entries[i].pal_index = (byte *)test_p;

		debug(2, "PalAnim::loadPalAnim(): Entry %d: Loading %d SAGA_COLOR structures.", i, color_count);

		test_p = malloc(sizeof(Color) * color_count);
		_entries[i].colors = (Color *)test_p;

		for (p = 0; p < pal_count; p++) {
			_entries[i].pal_index[p] = readS.readByte();
		}

		for (c = 0; c < color_count; c++) {
			_entries[i].colors[c].red = readS.readByte();
			_entries[i].colors[c].green = readS.readByte();
			_entries[i].colors[c].blue = readS.readByte();
		}
	}

	_loaded = true;
	return SUCCESS;
}

int PalAnim::cycleStart() {
	Event event;

	if (!_loaded) {
		return FAILURE;
	}

	event.type = kEvTOneshot;
	event.code = kPalAnimEvent;
	event.op = kEventCycleStep;
	event.time = PALANIM_CYCLETIME;
	_vm->_events->queue(&event);

	return SUCCESS;
}

int PalAnim::cycleStep(int vectortime) {
	static PalEntry pal[256];
	uint16 pal_index;
	uint16 col_index;

	uint16 i, j;
	uint16 cycle;
	uint16 cycle_limit;

	Event event;

	if (!_loaded) {
		return FAILURE;
	}

	_vm->_gfx->getCurrentPal(pal);

	for (i = 0; i < _entryCount; i++) {
		cycle = _entries[i].cycle;
		cycle_limit = _entries[i].color_count;
		for (j = 0; j < _entries[i].pal_count; j++) {
			pal_index = (unsigned char)_entries[i].pal_index[j];
			col_index = (cycle + j) % cycle_limit;
			pal[pal_index].red = (byte) _entries[i].colors[col_index].red;
			pal[pal_index].green = (byte) _entries[i].colors[col_index].green;
			pal[pal_index].blue = (byte) _entries[i].colors[col_index].blue;
		}

		_entries[i].cycle++;

		if (_entries[i].cycle == cycle_limit) {
			_entries[i].cycle = 0;
		}
	}

	// Don't cycle the palette when the map is open
	// Fixes bug #1900258 - "ITE: Glitch when looking at the map while at the docks"
	if (_vm->_interface->getMode() != kPanelMap)
		_vm->_gfx->setPalette(pal);

	event.type = kEvTOneshot;
	event.code = kPalAnimEvent;
	event.op = kEventCycleStep;
	event.time = vectortime + PALANIM_CYCLETIME;
	_vm->_events->queue(&event);

	return SUCCESS;
}

int PalAnim::freePalAnim() {
	uint16 i;

	if (!_loaded) {
		return FAILURE;
	}

	for (i = 0; i < _entryCount; i++) {
		debug(2, "PalAnim::freePalAnim(): Entry %d: Freeing colors.", i);
		free(_entries[i].colors);
		debug(2, "PalAnim::freePalAnim(): Entry %d: Freeing indices.", i);
		free(_entries[i].pal_index);
	}

	debug(3, "PalAnim::freePalAnim(): Freeing entries.");

	free(_entries);

	_loaded = false;

	return SUCCESS;
}

} // End of namespace Saga
