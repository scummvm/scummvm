/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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

#ifndef BSCOLLISION_H
#define BSCOLLISION_H

/*#include "objectman.h"

class SwordLogic;

class SwordCollision {
public:
	SwordCollision(ObjectMan *pObjMan, SwordLogic *pLogic);
	~SwordCollision(void);
	void checkCollisions(void);
	void fnBumpOff(void);
	void fnBumpOn(void);
private:
	int32 getIntersect(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3);
	int noCol;
	ObjectMan *_objMan;
	SwordLogic *_logic; // for CFN_preset_script
};*/
// maybe it's better to make this part of SwordRouter

#endif // BSCOLLISION_H
