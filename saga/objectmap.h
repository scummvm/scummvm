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

// Object map / Object click-area module header file

#ifndef SAGA_OBJECTMAP_H_
#define SAGA_OBJECTMAP_H_

namespace Saga {

enum R_OBJECT_FLAGS {
	R_OBJECT_NORMAL = 0x02
};

struct R_CLICKAREA {
	int n_points;
	R_POINT *points;
};

struct R_OBJECTMAP_ENTRY {
	int unknown0;
	uint16 flags;

	int object_num;
	int script_num;

	int n_clickareas;
	R_CLICKAREA *clickareas;
};

class Gfx;

class ObjectMap{
public:
	int reg(void);
	ObjectMap(Gfx *gfx);
	~ObjectMap(void);
	int load(const byte *om_res, size_t om_res_len);
	int freeMem(void);
	int loadNames(const byte *onl_res, size_t onl_res_len);
	int freeNames();
	int getName(int object, const char **name);
	int getFlags(int object, uint16 *flags);
	int getEPNum(int object, int *ep_num);
	int draw(R_SURFACE *draw_surface, R_POINT *imouse_pt, int color, int color2);
	int hitTest(R_POINT *imouse_pt, int *object_num);
	void objectInfo(int argc, char *argv[]);
private:
	int _initialized;

	int _objects_loaded;
	int _n_objects;
	R_OBJECTMAP_ENTRY *_object_maps;

	int _names_loaded;
	int _n_names;
	const char **_names;
	Gfx *_gfx;
};

} // End of namespace Saga

#endif
