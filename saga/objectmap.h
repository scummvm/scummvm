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
/*

 Description:   
 
    Object map / Object click-area module header file

 Notes: 
*/

#ifndef SAGA_OBJECTMAP_H_
#define SAGA_OBJECTMAP_H_

namespace Saga {

typedef struct R_CLICKAREA_tag {

	int n_points;
	R_POINT *points;

} R_CLICKAREA;

typedef struct R_OBJECTMAP_ENTRY_tag {

	int unknown0;
	uint flags;

	int object_num;
	int script_num;

	int n_clickareas;
	R_CLICKAREA *clickareas;

} R_OBJECTMAP_ENTRY;

typedef struct R_OBJECTMAP_INFO_tag {

	int initialized;

	int objects_loaded;
	int n_objects;
	R_OBJECTMAP_ENTRY *object_maps;

	int names_loaded;
	int n_names;
	char **names;

} R_OBJECTMAP_INFO;

static void CF_object_info(int argc, char *argv[]);

} // End of namespace Saga

#endif				/* SAGA_OBJECTMAP_H_ */
