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

enum OBJECT_FLAGS {
	OBJECT_ENABLED = (1<<0),   // Object is enabled
	OBJECT_EXIT = (1<<1),      // Causes char to exit

	// Causes the character not to walk to the object (but they will 
	// look at it).
	OBJECT_NOWALK = (1<<2),

	//	When the object is clicked on it projects the
	//	click point downwards from the middle of the object until it
	//	reaches the lowest point in the zone.
	OBJECT_PROJECT = (1<<3)
};

struct R_OBJECTMAP_ENTRY {
	byte flags;
	byte defaultVerb;

	int objectNum;
	int scriptNum;

	int nClickareas;
	R_CLICKAREA *clickareas;
};

class ObjectMap{
public:
	int reg(void);
	ObjectMap(SagaEngine *vm);
	~ObjectMap(void);
	int load(const byte *om_res, size_t om_res_len);
	int freeMem(void);
	int loadNames(const byte *onl_res, size_t onl_res_len);
	int freeNames();
	const char *getName(int object);
	const uint16 getFlags(int object);
	const int getEPNum(int object);
	int draw(R_SURFACE *draw_surface, Point imousePt, int color, int color2);
	int hitTest(Point imousePt);
	void info(void);

private:

	bool _objectsLoaded;
	int _nObjects;
	R_OBJECTMAP_ENTRY *_objectMaps;

	bool _namesLoaded;
	int _nNames;
	const char **_names;
	SagaEngine *_vm;
};

} // End of namespace Saga

#endif
