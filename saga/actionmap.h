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

struct R_ACTIONMAP_ENTRY {
	int unknown00;
	int unknown02;
	int exit_scene;
	int unknown06;

	int pt_count;
	R_POINT *pt_tbl;
};

class ActionMap {
 public:
	int reg(void);
	ActionMap(SagaEngine *vm);
	~ActionMap(void);

	int load(const byte *exmap_res, size_t exmap_res_len);
	int draw(R_SURFACE *ds, int color);

	int freeMap(void);
	int shutdown(void);

	void actionInfo(int argc, char *argv[]);

private:
	SagaEngine *_vm;

	bool _initialized;
	int _exits_loaded;
	int _n_exits;
	R_ACTIONMAP_ENTRY *_exits_tbl;
	const byte *_exmap_res;
	size_t _exmap_res_len;
};

} // End of namespace Saga

#endif
