/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

// Palette animation module header file

#ifndef SAGA_PALANIM_H
#define SAGA_PALANIM_H

namespace Saga {

#define PALANIM_CYCLETIME 100

struct PalanimEntry {
	uint16 pal_count;
	uint16 color_count;
	uint16 cycle;
	byte *pal_index;
	Color *colors;
};

class PalAnim {
 public:
	PalAnim(SagaEngine *vm);
	~PalAnim(void);

	int loadPalAnim(const byte *, size_t);
	int cycleStart();
	int cycleStep(int vectortime);
	int freePalAnim();

 private:
	SagaEngine *_vm;

	bool _loaded;
	uint16 _entryCount;
	PalanimEntry *_entries;
};

} // End of namespace Saga

#endif

