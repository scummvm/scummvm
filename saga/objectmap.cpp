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

// Object map / Object click-area module 

// Polygon Hit Test code ( HitTestPoly() ) adapted from code (C) Eric Haines
// appearing in Graphics Gems IV, "Point in Polygon Strategies."
// p. 24-46, code: p. 34-45
#include "saga.h"
#include "reinherit.h"

#include "cvar_mod.h"
#include "console_mod.h"
#include "gfx_mod.h"
#include "font_mod.h"

#include "objectmap_mod.h"
#include "objectmap.h"

namespace Saga {

static R_OBJECTMAP_INFO OMInfo;

int OBJECTMAP_Register() {
	CVAR_RegisterFunc(CF_object_info, "object_info", NULL, R_CVAR_NONE, 0, 0);

	return R_SUCCESS;
}

// Initializes the object map module, creates module allocation context
int OBJECTMAP_Init() {
	debug(0, "OBJECTMAP Module: Initializing...");

	OMInfo.initialized = 1;
	return R_SUCCESS;
}

// Shuts down the object map module, destroys module allocation context
int OBJECTMAP_Shutdown() {
	if (!OMInfo.initialized) {
		return R_FAILURE;
	}

	debug(0, "OBJECTMAP Module: Shutting down...");

	OBJECTMAP_Free();
	OBJECTMAP_FreeNames();

	debug(0, "OBJECTMAP Module: Shutdown AOK.");

	OMInfo.initialized = 0;
	return R_SUCCESS;
}

// Loads an object map resource ( objects ( clickareas ( points ) ) ) 
int OBJECTMAP_Load(const byte *om_res, size_t om_res_len) {
	R_OBJECTMAP_ENTRY *object_map;
	R_CLICKAREA *clickarea;
	R_POINT *point;

	int i, k, m;

	MemoryReadStream *readS = new MemoryReadStream(om_res, om_res_len);

	if (!OMInfo.initialized) {
		warning("Error: Object map module not initialized");
		return R_FAILURE;
	}

	if (OMInfo.objects_loaded) {
		OBJECTMAP_Free();
	}

	// Obtain object count N and allocate space for N objects
	OMInfo.n_objects = readS->readUint16LE();

	OMInfo.object_maps = (R_OBJECTMAP_ENTRY *)malloc(OMInfo.n_objects * sizeof *OMInfo.object_maps);

	if (OMInfo.object_maps == NULL) {
		warning("Error: Memory allocation failed");
		return R_MEM;
	}

	// Load all N objects
	for (i = 0; i < OMInfo.n_objects; i++) {
		object_map = &OMInfo.object_maps[i];
		object_map->unknown0 = readS->readByte();
		object_map->n_clickareas = readS->readByte();
		object_map->flags = readS->readUint16LE();
		object_map->object_num = readS->readUint16LE();
		object_map->script_num = readS->readUint16LE();
		object_map->clickareas = (R_CLICKAREA *)malloc(object_map->n_clickareas * sizeof *(object_map->clickareas));

		if (object_map->clickareas == NULL) {
			warning("Error: Memory allocation failed");
			return R_MEM;
		}

		// Load all clickareas for this object
		for (k = 0; k < object_map->n_clickareas; k++) {
			clickarea = &object_map->clickareas[k];
			clickarea->n_points = readS->readUint16LE();
			assert(clickarea->n_points != 0);

			clickarea->points = (R_POINT *)malloc(clickarea->n_points * sizeof *(clickarea->points));
			if (clickarea->points == NULL) {
				warning("Error: Memory allocation failed");
				return R_MEM;
			}

			// Load all points for this clickarea
			for (m = 0; m < clickarea->n_points; m++) {
				point = &clickarea->points[m];
				point->x = readS->readSint16LE();
				point->y = readS->readSint16LE();
			}
			debug(2, "OBJECTMAP_Load(): Read %d points for clickarea %d in object %d.",
					clickarea->n_points, k, object_map->object_num);
		}
	}

	OMInfo.objects_loaded = 1;

	return R_SUCCESS;
}

// Frees all storage allocated for the current object map data
int OBJECTMAP_Free() {
	R_OBJECTMAP_ENTRY *object_map;
	R_CLICKAREA *clickarea;

	int i, k;

	if (!OMInfo.objects_loaded) {
		return R_FAILURE;
	}

	for (i = 0; i < OMInfo.n_objects; i++) {
		object_map = &OMInfo.object_maps[i];
		for (k = 0; k < object_map->n_clickareas; k++) {
			clickarea = &object_map->clickareas[k];
			free(clickarea->points);
		}
		free(object_map->clickareas);
	}

	if (OMInfo.n_objects) {
		free(OMInfo.object_maps);
	}

	OMInfo.objects_loaded = 0;

	return R_SUCCESS;
}

// Loads an object name list resource
int OBJECTMAP_LoadNames(const unsigned char *onl_res, size_t onl_res_len) {
	int table_len;
	int n_names;
	size_t name_offset;

	int i;

	MemoryReadStream *readS = new MemoryReadStream(onl_res, onl_res_len);

	if (OMInfo.names_loaded) {
		OBJECTMAP_FreeNames();
	}

	table_len = readS->readUint16LE();

	n_names = table_len / 2 - 2;
	OMInfo.n_names = n_names;

	debug(2, "OBJECTMAP_LoadNames: Loading %d object names.", n_names);
	OMInfo.names = (const char **)malloc(n_names * sizeof *OMInfo.names);

	if (OMInfo.names == NULL) {
		warning("Error: Memory allocation failed");
		return R_MEM;
	}

	for (i = 0; i < n_names; i++) {
		name_offset = readS->readUint16LE();
		OMInfo.names[i] = (const char *)(onl_res + name_offset);

		debug(3, "Loaded object name string: %s", OMInfo.names[i]);
	}

	OMInfo.names_loaded = 1;

	return R_SUCCESS;
}

// Frees all storage allocated for the current object name list data
int OBJECTMAP_FreeNames() {
	if (!OMInfo.names_loaded) {
		return R_FAILURE;
	}

	if (OMInfo.n_names) {
		free(OMInfo.names);
	}

	OMInfo.names_loaded = 0;
	return R_SUCCESS;
}

// If 'object' is a valid object number in the currently loaded object 
// name list resource, the funciton sets '*name' to the descriptive string
// corresponding to 'object' and returns R_SUCCESS. Otherwise it returns
// R_FAILURE.
int OBJECTMAP_GetName(int object, const char **name) {
	if (!OMInfo.names_loaded) {
		return R_FAILURE;
	}

	if ((object <= 0) || (object > OMInfo.n_names)) {
		return R_FAILURE;
	}

	*name = OMInfo.names[object - 1];

	return R_SUCCESS;
}

int OBJECTMAP_GetFlags(int object, uint16 *flags) {
	int i;

	if (!OMInfo.names_loaded) {
		return R_FAILURE;
	}

	if ((object <= 0) || (object > OMInfo.n_names)) {
		return R_FAILURE;
	}

	for (i = 0; i < OMInfo.n_objects; i++) {
		if (OMInfo.object_maps[i].object_num == object) {
			*flags = OMInfo.object_maps[i].flags;
			return R_SUCCESS;
		}
	}

	return R_FAILURE;
}

// If 'object' is a valid object number in the currently loaded object 
// name list resource, the funciton sets '*ep_num' to the entrypoint number
// corresponding to 'object' and returns R_SUCCESS. Otherwise, it returns
// R_FAILURE.
int OBJECTMAP_GetEPNum(int object, int *ep_num) {
	int i;

	if (!OMInfo.names_loaded) {
		return R_FAILURE;
	}

	if ((object < 0) || (object > (OMInfo.n_objects + 1))) {
		return R_FAILURE;
	}

	for (i = 0; i < OMInfo.n_objects; i++) {

		if (OMInfo.object_maps[i].object_num == object) {

			*ep_num = OMInfo.object_maps[i].script_num;
			return R_SUCCESS;
		}
	}

	return R_FAILURE;
}

// Uses GFX_DrawLine to display all clickareas for each object in the 
// currently loaded object map resource.
int OBJECTMAP_Draw(R_SURFACE *ds, R_POINT *imouse_pt, int color, int color2) {
	R_OBJECTMAP_ENTRY *object_map;
	R_CLICKAREA *clickarea;

	char txt_buf[32];

	int draw_color = color;
	int draw_txt = 0;

	int hit_object = 0;
	int object_num = 0;

	int pointcount = 0;
	int i, k;

	assert(OMInfo.initialized);

	if (!OMInfo.objects_loaded) {
		return R_FAILURE;
	}

	if (imouse_pt != NULL) {
		if (OBJECTMAP_HitTest(imouse_pt, &object_num) == R_SUCCESS) {
			hit_object = 1;
		}
	}

	for (i = 0; i < OMInfo.n_objects; i++) {
		draw_color = color;
		if (hit_object && (object_num == OMInfo.object_maps[i].object_num)) {
			snprintf(txt_buf, sizeof txt_buf, "obj %d: ? %d, f %X",
					OMInfo.object_maps[i].object_num,
					OMInfo.object_maps[i].unknown0,
					OMInfo.object_maps[i].flags);
			draw_txt = 1;
			draw_color = color2;
		}

		object_map = &OMInfo.object_maps[i];

		for (k = 0; k < object_map->n_clickareas; k++) {
			clickarea = &object_map->clickareas[k];
			pointcount = 0;
			if (clickarea->n_points == 2) {
				// 2 points represent a box
				GFX_DrawFrame(ds, &clickarea->points[0], &clickarea->points[1], draw_color);
			} else if (clickarea->n_points > 2) {
				// Otherwise draw a polyline
				GFX_DrawPolyLine(ds, clickarea->points, clickarea->n_points, draw_color);
			}
		}
	}

	if (draw_txt) {
		FONT_Draw(SMALL_FONT_ID, ds, txt_buf, 0, 2, 2, GFX_GetWhite(), GFX_GetBlack(), FONT_OUTLINE);
	}

	return R_SUCCESS;
}

static bool MATH_HitTestPoly(R_POINT *points, unsigned int npoints, R_POINT test_point) {
	int yflag0;
	int yflag1;
	bool inside_flag = false;
	unsigned int pt;

	R_POINT *vtx0 = &points[npoints - 1];
	R_POINT *vtx1 = &points[0];

	yflag0 = (vtx0->y >= test_point.y);
	for (pt = 0; pt < npoints; pt++, vtx1++) {
		yflag1 = (vtx1->y >= test_point.y);
		if (yflag0 != yflag1) {
			if (((vtx1->y - test_point.y) * (vtx0->x - vtx1->x) >=
				(vtx1->x - test_point.x) * (vtx0->y - vtx1->y)) == yflag1) {
				inside_flag = !inside_flag;
			}
		}
		yflag0 = yflag1;
		vtx0 = vtx1;
	}

	return inside_flag;
}

int OBJECTMAP_HitTest(R_POINT * imouse_pt, int *object_num) {
	R_POINT imouse;
	R_OBJECTMAP_ENTRY *object_map;
	R_CLICKAREA *clickarea;
	R_POINT *points;
	int n_points;

	int i, k;

	assert((imouse_pt != NULL) && (object_num != NULL));

	imouse.x = imouse_pt->x;
	imouse.y = imouse_pt->y;

	// Loop through all scene objects
	for (i = 0; i < OMInfo.n_objects; i++) {
		object_map = &OMInfo.object_maps[i];

		// Hit-test all clickareas for this object
		for (k = 0; k < object_map->n_clickareas; k++) {
			clickarea = &object_map->clickareas[k];
			n_points = clickarea->n_points;
			points = clickarea->points;

			if (n_points == 2) {
				// Hit-test a box region
				if ((imouse.x > points[0].x) && (imouse.x <= points[1].x) &&
					(imouse.y > points[0].y) &&
					(imouse.y <= points[1].y)) {
						*object_num = object_map->object_num;
						return R_SUCCESS;
				}
			} else if (n_points > 2) {
				// Hit-test a polygon
				if (MATH_HitTestPoly(points, n_points, imouse)) {
					*object_num = object_map->object_num;
					return R_SUCCESS;
				}
			}
		}
	}

	*object_num = 0;

	return R_FAILURE;
}

static void CF_object_info(int argc, char *argv[]) {
	int i;

	(void)(argc);
	(void)(argv);

	if (!OMInfo.initialized) {
		return;
	}

	CON_Print("%d objects loaded.", OMInfo.n_objects);

	for (i = 0; i < OMInfo.n_objects; i++) {
		CON_Print("%s:", OMInfo.names[i]);
		CON_Print("%d. Unk1: %d, flags: %X, name_i: %d, scr_n: %d, ca_ct: %d", i, OMInfo.object_maps[i].unknown0,
					OMInfo.object_maps[i].flags,
					OMInfo.object_maps[i].object_num,
					OMInfo.object_maps[i].script_num,
					OMInfo.object_maps[i].n_clickareas);
	}

	return;
}

} // End of namespace Saga
