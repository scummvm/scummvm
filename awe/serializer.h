/* AWE - Another World Engine
 * Copyright (C) 2004 Gregory Montoir
 * Copyright (C) 2004 The ScummVM project
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
 */

#ifndef __SERIALIZER_H__
#define __SERIALIZER_H__

#include "stdafx.h"

#include "intern.h"

namespace Awe {

#define VER(x) x

#define SE_INT(i,sz,ver)     { Serializer::SET_INT, sz, 1, i, ver, Serializer::CUR_VER }
#define SE_ARRAY(a,n,sz,ver) { Serializer::SET_ARRAY, sz, n, a, ver, Serializer::CUR_VER }
#define SE_PTR(p,ver)        { Serializer::SET_PTR, 0, 0, p, ver, Serializer::CUR_VER }
#define SE_END()             { Serializer::SET_END, 0, 0, 0, 0, 0 }

struct File;

struct Serializer {
	enum {
		CUR_VER = 1
	};

	enum EntryType {
		SET_INT,
		SET_ARRAY,
		SET_PTR,
		SET_END
	};

	enum {
		SES_INT8  = 1,
		SES_INT16 = 2,
		SES_INT32 = 4
	};

	enum Mode {
		SM_SAVE,
		SM_LOAD
	};

	struct Entry {
		EntryType type;
		uint8 size;
		uint16 n;
		void *data;
		uint16 minVer;
		uint16 maxVer;
	};

	File *_stream;
	Mode _mode;
	uint8 *_ptrBlock;
	uint16 _saveVer;
	
	Serializer(File *stream, Mode mode, uint8 *ptrBlock, uint16 saveVer = CUR_VER);

	void saveOrLoadEntries(Entry *entry);

	void saveEntries(Entry *entry);
	void loadEntries(Entry *entry);

	void saveInt(uint8 es, void *p);
	void loadInt(uint8 es, void *p);
};

}

#endif
