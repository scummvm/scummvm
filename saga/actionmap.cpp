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
#include "reinherit.h"

#include "yslib.h"

#include "cvar_mod.h"
#include "console_mod.h"
#include "gfx_mod.h"

#include "actionmap_mod.h"
#include "actionmap.h"

namespace Saga {
static R_ACTIONMAP_INFO ActmapModule;

int ACTIONMAP_Register(void) {
	CVAR_RegisterFunc(CF_action_info,
					  "action_info", NULL, R_CVAR_NONE, 0, 0);
	return R_SUCCESS;
}

int ACTIONMAP_Init(void) {
	R_printf(R_STDOUT, "ACTIONMAP Module: Initializing...\n");
	ActmapModule.init = 1;
	return R_SUCCESS;
}

int ACTIONMAP_Load(const byte * exmap_res, size_t exmap_res_len) {
	// Loads exit map data from specified exit map resource
	R_ACTIONMAP_ENTRY *exmap_entry;
	R_POINT *exmap_pt_tbl;

	int exit_ct;
	int i, pt;

	assert(ActmapModule.init);
	assert(exmap_res != NULL);

	MemoryReadStream *exmapStream = new MemoryReadStream(exmap_res, exmap_res_len);

	// Load exits
	exit_ct = exmapStream->readSint16LE();
	if (exit_ct < 0) {
		return R_FAILURE;
	}

	exmap_entry = (R_ACTIONMAP_ENTRY *)malloc(exit_ct * sizeof *exmap_entry);
	if (exmap_entry == NULL) {
		R_printf(R_STDERR, "Memory allocation failure.\n");
		return R_MEM;
	}

	for (i = 0; i < exit_ct; i++) {
		exmap_entry[i].unknown00 = exmapStream->readSint16LE();
		exmap_entry[i].unknown02 = exmapStream->readSint16LE();
		exmap_entry[i].exit_scene = exmapStream->readSint16LE();
		exmap_entry[i].unknown06 = exmapStream->readSint16LE();

		exmap_entry[i].pt_count = exmapStream->readSint16LE();
		if (exmap_entry[i].pt_count < 0) {
			free(exmap_entry);
			return R_FAILURE;
		}

		exmap_pt_tbl = (R_POINT *)malloc(exmap_entry[i].pt_count * sizeof *exmap_pt_tbl);
		if (exmap_pt_tbl == NULL) {
			R_printf(R_STDERR, "Memory allocation failure.\n");
			return R_MEM;
		}

		for (pt = 0; pt < exmap_entry[i].pt_count; pt++) {
			exmap_pt_tbl[pt].x = exmapStream->readSint16LE();
			exmap_pt_tbl[pt].y = exmapStream->readSint16LE();
		}

		exmap_entry[i].pt_tbl = exmap_pt_tbl;
	}

	ActmapModule.exits_loaded = 1;
	ActmapModule.n_exits = exit_ct;
	ActmapModule.exits_tbl = exmap_entry;

	ActmapModule.exmap_res = exmap_res;
	ActmapModule.exmap_res_len = exmap_res_len;

	return R_SUCCESS;
}

int ACTIONMAP_Free(void) {
	// Frees the currently loaded exit map data
	R_ACTIONMAP_ENTRY *exmap_entry;
	int i;

	if (!ActmapModule.exits_loaded) {
		return R_SUCCESS;
	}

	for (i = 0; i < ActmapModule.n_exits; i++) {
		exmap_entry = &ActmapModule.exits_tbl[i];

		free(exmap_entry->pt_tbl);
	}

	free(ActmapModule.exits_tbl);

	ActmapModule.exits_loaded = 0;
	ActmapModule.exits_tbl = NULL;
	ActmapModule.n_exits = 0;

	return R_SUCCESS;
}

int ACTIONMAP_Shutdown(void) {
	return R_SUCCESS;
}

int ACTIONMAP_Draw(R_SURFACE * ds, int color) {
	int i;

	assert(ActmapModule.init);

	if (!ActmapModule.exits_loaded) {
		return R_FAILURE;
	}

	for (i = 0; i < ActmapModule.n_exits; i++) {
		if (ActmapModule.exits_tbl[i].pt_count == 2) {
			GFX_DrawFrame(ds,
				&ActmapModule.exits_tbl[i].pt_tbl[0],
				&ActmapModule.exits_tbl[i].pt_tbl[1], color);
		} else if (ActmapModule.exits_tbl[i].pt_count > 2) {
			GFX_DrawPolyLine(ds, ActmapModule.exits_tbl[i].pt_tbl,
							 ActmapModule.exits_tbl[i].pt_count, color);
		}
	}

	return R_SUCCESS;
}

void CF_action_info(int argc, char *argv[]) {
	R_POINT *pt;

	int i;
	int pt_i;

	YS_IGNORE_PARAM(argc);
	YS_IGNORE_PARAM(argv);

	if (!ActmapModule.exits_loaded) {
		return;
	}

	CON_Print("%d exits loaded.\n", ActmapModule.n_exits);

	for (i = 0; i < ActmapModule.n_exits; i++) {
		CON_Print ("Action %d: Exit to: %d; Pts: %d; Unk0: %d Unk2: %d Scr_N: %d",
				   i, ActmapModule.exits_tbl[i].exit_scene,
				   ActmapModule.exits_tbl[i].pt_count,
				   ActmapModule.exits_tbl[i].unknown00,
				   ActmapModule.exits_tbl[i].unknown02,
				   ActmapModule.exits_tbl[i].unknown06);

		for (pt_i = 0; pt_i < ActmapModule.exits_tbl[i].pt_count; pt_i++) {
			pt = &ActmapModule.exits_tbl[i].pt_tbl[pt_i];

			CON_Print("   pt: %d (%d, %d)", pt_i, pt->x, pt->y);
		}
	}

	return;
}

} // End of namespace Saga
