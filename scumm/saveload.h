/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
 * $Header$
 *
 */

#ifndef SAVELOAD_H
#define SAVELOAD_H

#include "common/scummsys.h"

class SaveFile;

namespace Scumm {

// Support for "old" savegames (made with 2501 CVS build)
// Can be useful for other ports too :)

#define VER(x) x
#define CURRENT_VER 30

// To work around a warning in GCC 3.2 (and 3.1 ?) regarding non-POD types,
// we use a small trick: instead of 0 we use 42. Why? Well, it seems newer GCC
// versions hae a heuristic built in to detect "offset-of" patterns - which is exactly
// what our OFFS macro does. Now, for non-POD types this is not really legal, because
// member need not be at a fixed offset relative to the variable, even if they are in
// current reality (many of our complex structs are non-POD; for an explanation of 
// what POD means refer to http://www-cpd.fnal.gov/personal/wb/boost/ISOcxx/doc/POD.html)

#define OFFS(type,item) (((long)(&((type*)42)->type::item))-42)
#define SIZE(type,item) sizeof(((type*)42)->type::item)

// Any item that is still in use automatically gets a maxVersion equal to CURRENT_VER
#define MKLINE(type,item,saveas,minVer) {OFFS(type,item),saveas,SIZE(type,item),minVer,CURRENT_VER}
#define MKARRAY(type,item,saveas,dim,minVer) {OFFS(type,item),128|saveas,SIZE(type,item),minVer,CURRENT_VER}, {dim,1,0,0,0}
#define MKARRAY2(type,item,saveas,dim,dim2,rowlen,minVer) {OFFS(type,item),128|saveas,SIZE(type,item),minVer,CURRENT_VER}, {dim,dim2,rowlen,0,0}

// Use this if you have an entry that used to be smaller:
#define MKLINE_OLD(type,item,saveas,minVer,maxVer) {OFFS(type,item),saveas,SIZE(type,item),minVer,maxVer}
#define MKARRAY_OLD(type,item,saveas,dim,minVer,maxVer) {OFFS(type,item),128|saveas,SIZE(type,item),minVer,maxVer}, {dim,1,0,0,0}
#define MKARRAY2_OLD(type,item,saveas,dim,dim2,rowlen,minVer,maxVer) {OFFS(type,item),128|saveas,SIZE(type,item),minVer,maxVer}, {dim,dim2,rowlen,0,0}

// An obsolete item/array, to be ignored upon load. We retain the type/item params to make it easier to debug.
// Obsolete items have size == 0.
#define MK_OBSOLETE(type,item,saveas,minVer,maxVer) {0,saveas,0,minVer,maxVer}
#define MK_OBSOLETE_ARRAY(type,item,saveas,dim,minVer,maxVer) {0,128|saveas,0,minVer,maxVer}, {dim,1,0,0,0}
#define MK_OBSOLETE_ARRAY2(type,item,saveas,dim,dim2,rowlen,minVer,maxVer) {0,128|saveas,0,minVer,maxVer}, {dim,dim2,rowlen,0,0}

// End marker
#define MKEND() {0xFFFF,0xFF,0xFF,0,0}

// A reference
#define MKREF(type,item,refid,minVer) {OFFS(type,item),refid,0xFF,minVer,CURRENT_VER}

// An obsolete reference.
#define MK_OBSOLETE_REF(type,item,refid,minVer,maxVer) {0,sleUint16,0,minVer,maxVer}


enum {
	sleByte = 1,
	sleUint8 = 1,
	sleInt8 = 1,
	sleInt16 = 2,
	sleUint16 = 3,
	sleInt32 = 4,
	sleUint32 = 5
};

struct SaveLoadEntry {
	uint32 offs;	// or: array dimension
	uint16 type;	// or: array dimension 2
	uint16 size;	// or: array row length
	uint8 minVersion;
	uint8 maxVersion;
};

typedef int SerializerSaveReference(void *me, byte type, void *ref);
typedef void *SerializerLoadReference(void *me, byte type, int ref);

class Serializer {
public:
	Serializer(SaveFile *stream, bool saveOrLoad, uint32 savegameVersion)
		: _save_ref(0), _load_ref(0), _ref_me(0),
		  _saveLoadStream(stream), _saveOrLoad(saveOrLoad),
		  _savegameVersion(savegameVersion)
	{ }

	SerializerSaveReference *_save_ref;
	SerializerLoadReference *_load_ref;
	void *_ref_me;

	void saveLoadArrayOf(void *b, int len, int datasize, byte filetype);
	void saveLoadArrayOf(void *b, int num, int datasize, const SaveLoadEntry *sle);
	void saveLoadEntries(void *d, const SaveLoadEntry *sle);

	bool isSaving() { return _saveOrLoad; }
	bool isLoading() { return !_saveOrLoad; }
	uint32 getVersion() { return _savegameVersion; }

	void saveUint32(uint32 d);
	void saveUint16(uint16 d);
	void saveByte(byte b);

	byte loadByte();
	uint16 loadUint16();
	uint32 loadUint32();

	void saveBytes(void *b, int len);
	void loadBytes(void *b, int len);
	
protected:
	SaveFile *_saveLoadStream;
	bool _saveOrLoad;
	uint32 _savegameVersion;

	void saveArrayOf(void *b, int len, int datasize, byte filetype);
	void loadArrayOf(void *b, int len, int datasize, byte filetype);

	void saveEntries(void *d, const SaveLoadEntry *sle);
	void loadEntries(void *d, const SaveLoadEntry *sle);
};

} // End of namespace Scumm

#endif
