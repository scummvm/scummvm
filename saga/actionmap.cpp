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
#include "saga.h"

#include "cvar_mod.h"
#include "gfx_mod.h"
#include "console_mod.h"

#include "actionmap.h"

namespace Saga {

static void CF_action_info(int argc, char *argv[], void *refCon);

int ActionMap::reg(void) {
	CVAR_RegisterFunc(CF_action_info,
					  "action_info", NULL, R_CVAR_NONE, 0, 0, this);
	return R_SUCCESS;
}

ActionMap::ActionMap(void) {
	debug(0, "ACTIONMAP Module: Initializing...");
	_initialized = true;
}

ActionMap::~ActionMap(void) {
	freeMap();
}


int ActionMap::load(const byte * exmap_res, size_t exmap_res_len) {
	// Loads exit map data from specified exit map resource
	R_ACTIONMAP_ENTRY *exmap_entry;
	R_POINT *exmap_pt_tbl;

	int exit_ct;
	int i, pt;

	assert(_initialized);
	assert(exmap_res != NULL);

	MemoryReadStream *readS = new MemoryReadStream(exmap_res, exmap_res_len);

	// Load exits
	exit_ct = readS->readSint16LE();
	if (exit_ct < 0) {
		return R_FAILURE;
	}

	exmap_entry = (R_ACTIONMAP_ENTRY *)malloc(exit_ct * sizeof *exmap_entry);
	if (exmap_entry == NULL) {
		warning("Memory allocation failure");
		return R_MEM;
	}

	for (i = 0; i < exit_ct; i++) {
		exmap_entry[i].unknown00 = readS->readSint16LE();
		exmap_entry[i].unknown02 = readS->readSint16LE();
		exmap_entry[i].exit_scene = readS->readSint16LE();
		exmap_entry[i].unknown06 = readS->readSint16LE();

		exmap_entry[i].pt_count = readS->readSint16LE();
		if (exmap_entry[i].pt_count < 0) {
			free(exmap_entry);
			return R_FAILURE;
		}

		exmap_pt_tbl = (R_POINT *)malloc(exmap_entry[i].pt_count * sizeof *exmap_pt_tbl);
		if (exmap_pt_tbl == NULL) {
			warning("Memory allocation failure");
			return R_MEM;
		}

		for (pt = 0; pt < exmap_entry[i].pt_count; pt++) {
			exmap_pt_tbl[pt].x = readS->readSint16LE();
			exmap_pt_tbl[pt].y = readS->readSint16LE();
		}

		exmap_entry[i].pt_tbl = exmap_pt_tbl;
	}

	_exits_loaded = 1;
	_n_exits = exit_ct;
	_exits_tbl = exmap_entry;

	_exmap_res = exmap_res;
	_exmap_res_len = exmap_res_len;

	return R_SUCCESS;
}

int ActionMap::freeMap(void) {
	// Frees the currently loaded exit map data
	R_ACTIONMAP_ENTRY *exmap_entry;
	int i;

	if (!_exits_loaded) {
		return R_SUCCESS;
	}

	for (i = 0; i < _n_exits; i++) {
		exmap_entry = &_exits_tbl[i];

		if (exmap_entry->pt_tbl)
			free(exmap_entry->pt_tbl);
	}

	if (_exits_tbl)
		free(_exits_tbl);

	_exits_loaded = 0;
	_exits_tbl = NULL;
	_n_exits = 0;

	return R_SUCCESS;
}

int ActionMap::shutdown(void) {
	return R_SUCCESS;
}

int ActionMap::draw(R_SURFACE * ds, int color) {
	int i;

	assert(_initialized);

	if (!_exits_loaded) {
		return R_FAILURE;
	}

	for (i = 0; i < _n_exits; i++) {
		if (_exits_tbl[i].pt_count == 2) {
			GFX_DrawFrame(ds,
				&_exits_tbl[i].pt_tbl[0],
				&_exits_tbl[i].pt_tbl[1], color);
		} else if (_exits_tbl[i].pt_count > 2) {
			GFX_DrawPolyLine(ds, _exits_tbl[i].pt_tbl,
							 _exits_tbl[i].pt_count, color);
		}
	}

	return R_SUCCESS;
}

void ActionMap::actionInfo(int argc, char *argv[]) {
	R_POINT *pt;

	int i;
	int pt_i;

	(void)(argc);
	(void)(argv);

	if (!_exits_loaded) {
		return;
	}

	CON_Print("%d exits loaded.\n", _n_exits);

	for (i = 0; i < _n_exits; i++) {
		CON_Print ("Action %d: Exit to: %d; Pts: %d; Unk0: %d Unk2: %d Scr_N: %d",
				   i, _exits_tbl[i].exit_scene,
				   _exits_tbl[i].pt_count,
				   _exits_tbl[i].unknown00,
				   _exits_tbl[i].unknown02,
				   _exits_tbl[i].unknown06);

		for (pt_i = 0; pt_i < _exits_tbl[i].pt_count; pt_i++) {
			pt = &_exits_tbl[i].pt_tbl[pt_i];

			CON_Print("   pt: %d (%d, %d)", pt_i, pt->x, pt->y);
		}
	}
}

static void CF_action_info(int argc, char *argv[], void *refCon) {
	((ActionMap *)refCon)->actionInfo(argc, argv);
}

} // End of namespace Saga
