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
	R_CLICKAREA *clickarea;
	Point *point;

	assert(exmap_res != NULL);

	MemoryReadStream readS(exmap_res, exmap_res_len);

	// Load exits
	_nExits = readS.readSint16LE();
	if (_nExits < 0) {
		return;
	}

	_exitsTbl = (R_ACTIONMAP_ENTRY *)malloc(_nExits * sizeof *_exitsTbl);
	if (_exitsTbl == NULL) {
		warning("Memory allocation failure");
		return;
	}

	for (int i = 0; i < _nExits; i++) {
		exmap_entry = &_exitsTbl[i];
		exmap_entry->flags = readS.readByte();
		exmap_entry->nClickareas = readS.readByte();
		exmap_entry->defaultVerb = readS.readByte();
		readS.readByte();
		exmap_entry->exitScene = readS.readUint16LE();
		exmap_entry->entranceNum = readS.readUint16LE();

		exmap_entry->clickareas = (R_CLICKAREA *)malloc(exmap_entry->nClickareas * sizeof *(exmap_entry->clickareas));

		if (exmap_entry->clickareas == NULL) {
			warning("Error: Memory allocation failed");
			return;
		}

		// Load all clickareas for this object
		for (int k = 0; k < exmap_entry->nClickareas; k++) {
			clickarea = &exmap_entry->clickareas[k];
			clickarea->n_points = readS.readUint16LE();
			assert(clickarea->n_points != 0);

			clickarea->points = (Point *)malloc(clickarea->n_points * sizeof *(clickarea->points));
			if (clickarea->points == NULL) {
				warning("Error: Memory allocation failed");
				return;
			}

			// Load all points for this clickarea
			for (int m = 0; m < clickarea->n_points; m++) {
				point = &clickarea->points[m];
				point->x = readS.readSint16LE();
				point->y = readS.readSint16LE();
			}
		}
	}
}

ActionMap::~ActionMap(void) {
	// Frees the currently loaded exit map data
	R_ACTIONMAP_ENTRY *exmap_entry;
	R_CLICKAREA *clickarea;
	int i;

	if (_exitsTbl) {
		for (i = 0; i < _nExits; i++) {
			exmap_entry = &_exitsTbl[i];

			for (int k = 0; k < exmap_entry->nClickareas; k++) {
				clickarea = &exmap_entry->clickareas[k];
				free(clickarea->points);
			}
			free(exmap_entry->clickareas);
		}

		free(_exitsTbl);
	}
}

const int ActionMap::getExitScene(int exitNum) {
	assert(exitNum < _nExits);

	return _exitsTbl[exitNum].exitScene;
}


int ActionMap::hitTest(const Point& imouse) {
	R_ACTIONMAP_ENTRY *exmap_entry;
	R_CLICKAREA *clickarea;
	Point *points;
	int n_points;

	int i, k;

	// Loop through all scene objects
	for (i = 0; i < _nExits; i++) {
		exmap_entry = &_exitsTbl[i];

		// Hit-test all clickareas for this object
		for (k = 0; k < exmap_entry->nClickareas; k++) {
			clickarea = &exmap_entry->clickareas[k];
			n_points = clickarea->n_points;
			points = clickarea->points;

			if (n_points == 2) {
				// Hit-test a box region
				if ((imouse.x > points[0].x) && (imouse.x <= points[1].x) &&
					(imouse.y > points[0].y) &&
					(imouse.y <= points[1].y)) {
						return i;
				}
			} else if (n_points > 2) {
				// Hit-test a polygon
				if (_vm->_gfx->hitTestPoly(points, n_points, imouse)) {
					return i;
				}
			}
		}
	}

	return -1;
}

int ActionMap::draw(R_SURFACE *ds, int color) {
	R_ACTIONMAP_ENTRY *exmap_entry;
	R_CLICKAREA *clickarea;

	int i, k;

	for (i = 0; i < _nExits; i++) {
		exmap_entry = &_exitsTbl[i];

		for (k = 0; k < exmap_entry->nClickareas; k++) {
			clickarea = &exmap_entry->clickareas[k];
			if (clickarea->n_points == 2) {
				// 2 points represent a box
				_vm->_gfx->drawFrame(ds, &clickarea->points[0], &clickarea->points[1], color);
			} else if (clickarea->n_points > 2) {
				// Otherwise draw a polyline
				_vm->_gfx->drawPolyLine(ds, clickarea->points, clickarea->n_points, color);
			}
		}
	}

	return R_SUCCESS;
}

void ActionMap::info(void) {
	_vm->_console->print("%d exits loaded.\n", _nExits);

	for (int i = 0; i < _nExits; i++) {
		_vm->_console->print ("Action %d: Exit to: %d", i, _exitsTbl[i].exitScene);
	}
}

} // End of namespace Saga
