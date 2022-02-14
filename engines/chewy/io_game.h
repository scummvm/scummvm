/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CHEWY_IOGame2_H
#define CHEWY_IOGame2_H

#include "chewy/mcga.h"
#include "chewy/cursor.h"
#include "chewy/ngstypes.h"

namespace Chewy {

#define IOG_END 1
#define IOG_LOAD 2
#define IOG_SAVE 3
#define IOG_BACK 4
#define USER_NAME 36

#define DOPPEL_KLICK 0.5

class IOGame {
private:
	Cursor *_cur;
	IogInit *_io;
	InputMgr *_in;
	McgaGraphics *_out;
	char _fileFind[20][USER_NAME + 4];

private:
	int16 get_savegame_files();

public:
	IOGame(McgaGraphics *out, InputMgr *in, Cursor *cur);

	char *io_init(IogInit *iostruc);
	void load(int16 nr, char *fname);
	void save_entry(int16 nr, char *fname);
};

} // namespace Chewy

#endif
