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

// Action map module - private header

#ifndef SAGA_ACTIONMAP_H_
#define SAGA_ACTIONMAP_H_

namespace Saga {

enum ACTION_FLAGS {
	ACTION_ENABLED = (1<<0),   // Zone is enabled
	ACTION_EXIT = (1<<1),      // Causes char to exit

	//	The following flag causes the zone to act differently.
	//	When the actor hits the zone, it will immediately begin walking
	//	in the specified direction, and the actual specified effect of
	//	the zone will be delayed until the actor leaves the zone.
	ACTION_AUTOWALK = (1<<2),

	//	zone activates only when character stops walking
	ACTION_TERMINUS = (1<<3)
};

struct R_ACTIONMAP_ENTRY {
	int flags;
	int nClickareas;
	int defaultVerb;
	int exitScene;
	int entranceNum;

	R_CLICKAREA *clickareas;
};

class ActionMap {
 public:
	int reg(void);
	ActionMap(SagaEngine *vm, const byte *exmap_res, size_t exmap_res_len);
	~ActionMap(void);

	const int getExitScene(int exitNum);
	int hitTest(const Point& imousePt);
	int draw(R_SURFACE *ds, int color);

	void info(void);

private:
	SagaEngine *_vm;

	int _nExits;
	R_ACTIONMAP_ENTRY *_exitsTbl;
};

} // End of namespace Saga

#endif
