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

// Scripting module data management component header file

#ifndef SAGA_SDATA_H
#define SAGA_SDATA_H

namespace Saga {

#define R_SCRIPT_DATABUF_NUM 5
#define R_SCRIPT_DATABUF_LEN 1024

class SData {
public:	
	SData();
	~SData();
	
	int getWord(int n_buf, int n_word, SDataWord_T *data);
	int putWord(int n_buf, int n_word, SDataWord_T data);
	int setBit(int n_buf, SDataWord_T n_bit, int bitstate);
	int getBit(int n_buf, SDataWord_T n_bit, int *bitstate);
	int readWordS(SDataWord_T word);
	uint16 readWordU(SDataWord_T word);
};

} // End of namespace Saga

#endif
