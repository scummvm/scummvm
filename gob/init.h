/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#ifndef GOB_INIT_H
#define GOB_INIT_H

namespace Gob {

class Init {
public:
	void findBestCfg(void);
	void soundVideo(int32 smallHeapSize, int16 flag);

	void initGame(char *totFile);

	Init(GobEngine *vm);

protected:
	Video::PalDesc *_palDesc;
	static const char *_fontNames[4];
	GobEngine *_vm;

	void cleanup(void);
};

}				// End of namespace Gob

#endif
