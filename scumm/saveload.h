/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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

#define OFFS(type,item) ((int)(&((type*)0)->type::item))
#define SIZE(type,item) sizeof(((type*)0)->type::item)
#define MKLINE(type,item,saveas) {OFFS(type,item),saveas,SIZE(type,item)}
#define MKARRAY(type,item,saveas,num) {OFFS(type,item),128|saveas,SIZE(type,item)}, {num,0,0}
#define MKEND() {0xFFFF,0xFF,0xFF}

#define MKREF(type,item,refid) {OFFS(type,item),refid,0xFF}

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
	uint32 offs;
	uint8 type;
	uint8 size;
};

struct SerializerStream {
#ifdef NONSTANDARD_SAVE
	void *context;

	bool fopen(const char *filename, const char *mode);
	void fclose();
	int fread(void *buf, int size, int cnt);
	int fwrite(void *buf, int size, int cnt);
#else
	FILE *out;

	FILE *fopen(const char *filename, const char *mode) {
		return out = ::fopen(filename, mode);
	}
	void fclose() {
		::fclose(out);
	}
	int fread(void *buf, int size, int cnt) {
		return ::fread(buf, size, cnt, out);
	}
	int fwrite(void *buf, int size, int cnt) {
		return ::fwrite(buf, size, cnt, out);
	}
#endif
};

typedef int SerializerSaveReference(void *me, byte type, void *ref);
typedef void *SerializerLoadReference(void *me, byte type, int ref);

struct Serializer {
	SerializerStream _saveLoadStream;

	union {
		SerializerSaveReference *_save_ref;
		SerializerLoadReference *_load_ref;
		void *_saveload_ref;
	};
	void *_ref_me;

	bool _saveOrLoad;

	void saveBytes(void *b, int len);
	void loadBytes(void *b, int len);
	
	void saveLoadArrayOf(void *b, int len, int datasize, byte filetype);
	void saveLoadEntries(void *d, const SaveLoadEntry *sle);
	void saveLoadArrayOf(void *b, int num, int datasize, const SaveLoadEntry *sle);

	void saveUint32(uint32 d);
	void saveWord(uint16 d);
	void saveByte(byte b);

	byte loadByte();
	uint16 loadWord();
	uint32 loadUint32();

	bool isSaving() { return _saveOrLoad; }

	bool checkEOFLoadStream();

};

#endif
