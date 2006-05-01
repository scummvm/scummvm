/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */
#ifndef GOB_INIT_H
#define GOB_INIT_H

#include "gob/video.h"

namespace Gob {

class Init {
public:
	void findBestCfg(void);
	void initGame(char *totFile);

	virtual void soundVideo(int32 smallHeapSize, int16 flag) = 0;

	Init(GobEngine *vm);
	virtual ~Init() {};

protected:
	Video::PalDesc *_palDesc;
	static const char *_fontNames[4];
	GobEngine *_vm;

	void cleanup(void);
};

class Init_v1 : public Init {
public:
	virtual void soundVideo(int32 smallHeapSize, int16 flag);

	Init_v1(GobEngine *vm);
	virtual ~Init_v1() {};
};

class Init_v2 : public Init_v1 {
public:
	virtual void soundVideo(int32 smallHeapSize, int16 flag);

	Init_v2(GobEngine *vm);
	virtual ~Init_v2() {};
};

}				// End of namespace Gob

#endif
