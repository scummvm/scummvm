/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#ifndef SAGA_RESFILE_H
#define SAGA_RESFILE_H

/* The 'RSC' resource file format used by SAGA is quite simple. 
 * At the end of the resource file is an 8 byte structure. The first
 * 32 bit value specifies the offset of the resource table, the 
 * second specifies the number of entries in the resource table.
 * Each entry in the resource table is itself 32 bytes, the first 
 * 32 bit value of which specifies the offset of the resource, the 
 * second specifies the length of the resource.
 */
 
#include "common/file.h"

class ResourceFile : public File {

public:

	struct Resource {
		int32 res_offset;
		int32 res_len;
	};

private:

	long _file_len;

protected:

	enum ResourceConstants {
		RSC_TABLEINFO_SIZE  = 8,
		RSC_TABLEENTRY_SIZE = 8
	};

	const char *_resDirectory;
	
	int32 _resTblOffset;
	int32 _resTblCt;

	bool _resTblLoaded;
	Resource *_resTbl;

public:

	ResourceFile();
	virtual ~ResourceFile();

	bool open(const char *filename, const char *directory);
	void close();

	inline int16 readSint16LE() {
		return readUint16LE();
	}

	inline int32 readSint32LE() {
		return readUint32LE();
	}

	inline int16 readSint16BE() {
		return readUint16BE();
	}

	inline int32 readSint32BE() {
		return readUint32BE();
	}

	int32 getResourceCt();
	int32 getResourceOffset( int32 rn );
	int32 getResourceLen( int32 rn );

	bool loadResource( int32 rn, byte **res, int32 *res_len );
	void freeResource( byte *res );

};



#endif





