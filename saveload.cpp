/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Change Log:
 * $Log$
 * Revision 1.1  2001/10/09 14:30:13  strigeus
 * Initial revision
 *
 *
 */

#include "stdafx.h"
#include "scumm.h"


bool Scumm::saveState(const char *filename) {
	FILE *out = fopen(filename,"wb");
	if (out==NULL)
		return false;
	saveOrLoad(out,true);
	fclose(out);
	return true;
}

bool Scumm::loadState(const char *filename) {
	FILE *out = fopen(filename,"rb");
	if (out==NULL)
		return false;
	saveOrLoad(out,false);
	fclose(out);
	return true;
}

void Scumm::saveOrLoad(FILE *inout, bool mode) {
	_saveLoadStream = inout;
	_saveOrLoad = mode;
}


void Scumm::saveLoadBytes(void *b, int len) {
	if (_saveOrLoad)
		fwrite(b, 1, len, _saveLoadStream);
	else
		fread(b, 1, len, _saveLoadStream);
}
