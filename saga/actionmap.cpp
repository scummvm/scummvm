/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

/* Action map module */
#include "saga/saga.h"

#include "saga/cvar_mod.h"
#include "saga/gfx.h"
#include "saga/console.h"

#include "saga/actionmap.h"

namespace Saga {

ActionMap::ActionMap(SagaEngine *vm, const byte * exmap_res, size_t exmap_res_len) : _vm(vm) {
	// Loads exit map data from specified exit map resource
	R_ACTIONMAP_ENTRY *exmap_entry;
	Point *exmap_pt_tbl;

	int exit_ct;
	int i, pt;

	assert(exmap_res != NULL);

	MemoryReadStream readS(exmap_res, exmap_res_len);

	// Load exits
	exit_ct = readS.readSint16LE();
	if (exit_ct < 0) {
		return;
	}

	exmap_entry = (R_ACTIONMAP_ENTRY *)malloc(exit_ct * sizeof *exmap_entry);
	if (exmap_entry == NULL) {
		warning("Memory allocation failure");
		return;
	}

	for (i = 0; i < exit_ct; i++) {
		exmap_entry[i].unknown00 = readS.readSint16LE();
		exmap_entry[i].unknown02 = readS.readSint16LE();
		exmap_entry[i].exitScene = readS.readSint16LE();
		exmap_entry[i].unknown06 = readS.readSint16LE();

		exmap_entry[i].pt_count = readS.readSint16LE();
		if (exmap_entry[i].pt_count < 0) {
			free(exmap_entry);
			return;
		}

		exmap_pt_tbl = (Point *)malloc(exmap_entry[i].pt_count * sizeof *exmap_pt_tbl);
		if (exmap_pt_tbl == NULL) {
			warning("Memory allocation failure");
			return;
		}

		for (pt = 0; pt < exmap_entry[i].pt_count; pt++) {
			exmap_pt_tbl[pt].x = readS.readSint16LE();
			exmap_pt_tbl[pt].y = readS.readSint16LE();
		}

		exmap_entry[i].pt_tbl = exmap_pt_tbl;
	}

	_nExits = exit_ct;
	_exitsTbl = exmap_entry;
}

ActionMap::~ActionMap(void) {
	// Frees the currently loaded exit map data
	R_ACTIONMAP_ENTRY *exmap_entry;
	int i;

	if (_exitsTbl) {
		for (i = 0; i < _nExits; i++) {
			exmap_entry = &_exitsTbl[i];

			if (exmap_entry != NULL)
				free(exmap_entry->pt_tbl);
		}

		free(_exitsTbl);
	}
}

int ActionMap::draw(R_SURFACE *ds, int color) {
	int i;

	for (i = 0; i < _nExits; i++) {
		if (_exitsTbl[i].pt_count == 2) {
			_vm->_gfx->drawFrame(ds,
				&_exitsTbl[i].pt_tbl[0],
				&_exitsTbl[i].pt_tbl[1], color);
		} else if (_exitsTbl[i].pt_count > 2) {
			_vm->_gfx->drawPolyLine(ds, _exitsTbl[i].pt_tbl,
							 _exitsTbl[i].pt_count, color);
		}
	}

	return R_SUCCESS;
}

void ActionMap::info(void) {
	Point *pt;

	int i;
	int pt_i;

	_vm->_console->print("%d exits loaded.\n", _nExits);

	for (i = 0; i < _nExits; i++) {
		_vm->_console->print ("Action %d: Exit to: %d; Pts: %d; Unk0: %d Unk2: %d Scr_N: %d",
				   i, _exitsTbl[i].exitScene,
				   _exitsTbl[i].pt_count,
				   _exitsTbl[i].unknown00,
				   _exitsTbl[i].unknown02,
				   _exitsTbl[i].unknown06);

		for (pt_i = 0; pt_i < _exitsTbl[i].pt_count; pt_i++) {
			pt = &_exitsTbl[i].pt_tbl[pt_i];

			_vm->_console->print("   pt: %d (%d, %d)", pt_i, pt->x, pt->y);
		}
	}
}

} // End of namespace Saga
