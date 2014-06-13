/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "fullpipe/fullpipe.h"

#include "common/file.h"
#include "common/memstream.h"

#include "fullpipe/objects.h"
#include "fullpipe/motion.h"
#include "fullpipe/ngiarchive.h"
#include "fullpipe/messages.h"
#include "fullpipe/interaction.h"

namespace Fullpipe {

bool CObject::loadFile(const char *fname) {
	Common::File file;

	if (!file.open(fname))
		return false;

	MfcArchive archive(&file);

	return load(archive);
}

bool ObList::load(MfcArchive &file) {
	debug(5, "ObList::load()");
	int count = file.readCount();

	debug(9, "ObList::count: %d:", count);

	for (int i = 0; i < count; i++) {
		debug(9, "ObList::[%d]", i);
		CObject *t = file.readClass();

		push_back(t);
	}

	return true;
}

bool ObArray::load(MfcArchive &file) {
	debug(5, "ObArray::load()");
	int count = file.readCount();

	resize(count);

	for (int i = 0; i < count; i++) {
		CObject *t = file.readClass();

		push_back(*t);
	}

	return true;
}

bool DWordArray::load(MfcArchive &file) {
	debug(5, "DWordArray::load()");
	int count = file.readCount();

	debug(9, "DWordArray::count: %d", count);

	resize(count);

	for (int i = 0; i < count; i++) {
		int32 t = file.readUint32LE();

		push_back(t);
	}

	return true;
}

char *MfcArchive::readPascalString(bool twoByte) {
	char *tmp;
	int len;

	if (twoByte)
		len = readUint16LE();
	else
		len = readByte();

	tmp = (char *)calloc(len + 1, 1);
	read(tmp, len);

	debug(9, "readPascalString: %d <%s>", len, transCyrillic((byte *)tmp));

	return tmp;
}

MemoryObject::MemoryObject() {
	_memfilename = 0;
	_mfield_8 = 0;
	_mfield_C = 0;
	_mfield_10 = -1;
	_mfield_14 = 1;
	_dataSize = 0;
	_mflags = 0;
	_libHandle = 0;
	_data = 0;
}

MemoryObject::~MemoryObject() {
	freeData();
	if (_memfilename)
		free(_memfilename);
}

bool MemoryObject::load(MfcArchive &file) {
	debug(5, "MemoryObject::load()");
	_memfilename = file.readPascalString();

	if (char *p = strchr(_memfilename, '\\')) {
		for (char *d = _memfilename; *p;) {
			p++;
			*d++ = *p;
		}
	}

	if (g_fp->_currArchive) {
		_mfield_14 = 0;
		_libHandle = g_fp->_currArchive;
	}

	return true;
}

void MemoryObject::loadFile(char *filename) {
	debug(5, "MemoryObject::loadFile(<%s>)", filename);

	if (!*filename)
		return;

	if (!_data) {
		NGIArchive *arr = g_fp->_currArchive;

		if (g_fp->_currArchive != _libHandle && _libHandle)
			g_fp->_currArchive = _libHandle;

		Common::SeekableReadStream *s = g_fp->_currArchive->createReadStreamForMember(filename);

		if (s) {
			assert(s->size() > 0);

			_dataSize = s->size();

			debug(5, "Loading %s (%d bytes)", filename, _dataSize);
			_data = (byte *)calloc(_dataSize, 1);
			s->read(_data, _dataSize);

			delete s;
		} else {
			warning("MemoryObject::loadFile(): reading failure");
		}

		g_fp->_currArchive = arr;
	}
}

byte *MemoryObject::getData() {
	load();

	if (_mfield_14 || _mflags & 1) {
		return _data;
	} else {
		error("Unhandled packed data");
	}
}

byte *MemoryObject::loadData() {
	load();
	return _data;
}

void MemoryObject::freeData() {
	debug(8, "MemoryObject::freeData(): file: %s", _memfilename);

	if (_data)
		free(_data);

	_data = 0;
}

bool MemoryObject::testFlags() {
	if (_mfield_8)
		return false;

	if (_mflags & 1)
		return true;

	return false;
}

MemoryObject2::MemoryObject2() {
	_rows = 0;
}

MemoryObject2::~MemoryObject2() {
	if (_rows)
		free(_rows);
}

bool MemoryObject2::load(MfcArchive &file) {
	debug(5, "MemoryObject2::load()");
	MemoryObject::load(file);

	_mflags |= 1;

	debug(5, "MemoryObject2::load: <%s>", _memfilename);

	if (_memfilename && *_memfilename) {
		MemoryObject::loadFile(_memfilename);
	}

	return true;
}

void MemoryObject2::copyData(byte *src, int dataSize) {
	if (_data)
		freeData();

	_dataSize = dataSize;
	_data = (byte *)malloc(dataSize);

	memcpy(_data, src, _dataSize);
}

int MfcArchive::readCount() {
	int count = readUint16LE();

	if (count == 0xffff)
		count = readUint32LE();

	return count;
}

double MfcArchive::readDouble() {
	// FIXME: This is utterly cruel and unportable
	// Some articles on the matter:
	// http://randomascii.wordpress.com/2013/02/07/float-precision-revisited-nine-digit-float-portability/
	// http://randomascii.wordpress.com/2012/01/11/tricks-with-the-floating-point-format/

	union {
		struct {
			int32 a;
			int32 b;
		} i;
		double d;
	} tmp;

	tmp.i.a = readUint32LE();
	tmp.i.b = readUint32LE();

	return tmp.d;
}

enum {
	kNullObject,
	kInteraction,
	kMessageQueue,
	kExCommand,
	kObjstateCommand,
	kGameVar,
	kMctlCompound,
	kMovGraph,
	kMovGraphLink,
	kMovGraphNode,
	kReactParallel,
	kReactPolygonal
};

const struct {
	const char *name;
	int id;
} classMap[] = {
	{ "CInteraction",	kInteraction },
	{ "MessageQueue",	kMessageQueue },
	{ "ExCommand",		kExCommand },
	{ "CObjstateCommand", kObjstateCommand },
	{ "CGameVar",		kGameVar },
	{ "CMctlCompound",	kMctlCompound },
	{ "CMovGraph",		kMovGraph },
	{ "CMovGraphLink",	kMovGraphLink },
	{ "CMovGraphNode",	kMovGraphNode },
	{ "CReactParallel",	kReactParallel },
	{ "CReactPolygonal", kReactPolygonal },
	{ 0, 0 }
};

static const char *lookupObjectId(int id) {
	for (int i = 0; classMap[i].name; i++) {
		if (classMap[i].id == id)
			return classMap[i].name;
	}

	return "";
}

static CObject *createObject(int objectId) {
	switch (objectId) {
	case kNullObject:
		return 0;
	case kInteraction:
		return new Interaction();
	case kMessageQueue:
		return new MessageQueue();
	case kExCommand:
		return new ExCommand();
	case kObjstateCommand:
		return new ObjstateCommand();
	case kGameVar:
		return new GameVar();
	case kMctlCompound:
		return new MctlCompound();
	case kMovGraph:
		return new MovGraph();
	case kMovGraphLink:
		return new MovGraphLink();
	case kMovGraphNode:
		return new MovGraphNode();
	case kReactParallel:
		return new ReactParallel();
	case kReactPolygonal:
		return new ReactPolygonal();
	default:
		error("Unknown objectId: %d", objectId);
	}

	return 0;
}

MfcArchive::MfcArchive(Common::SeekableReadStream *stream) {
	for (int i = 0; classMap[i].name; i++) {
		_classMap[classMap[i].name] = classMap[i].id;
	}

	_lastIndex = 1;
	_level = 0;

	_stream = stream;

	_objectMap.push_back(0);
	_objectIdMap.push_back(kNullObject);
}

CObject *MfcArchive::readClass() {
	bool isCopyReturned;
	CObject *res = parseClass(&isCopyReturned);

	if (res && !isCopyReturned)
		res->load(*this);

	return res;
}

CObject *MfcArchive::parseClass(bool *isCopyReturned) {
	char *name;
	int objectId = 0;
	CObject *res = 0;

	uint obTag = readUint16LE();

	debug(7, "parseClass::obTag = %d (%04x)  at 0x%08x", obTag, obTag, pos() - 2);

	if (obTag == 0xffff) {
		int schema = readUint16LE();

		debug(7, "parseClass::schema = %d", schema);

		name = readPascalString(true);
		debug(7, "parseClass::class <%s>", name);

		if (!_classMap.contains(name)) {
			error("Unknown class in MfcArchive: <%s>", name);
		}

		objectId = _classMap[name];

		debug(7, "tag: %d 0x%x (%x)", _objectMap.size() - 1, _objectMap.size() - 1, objectId);

		res = createObject(objectId);
		_objectMap.push_back(res);
		_objectIdMap.push_back(objectId);

		_objectMap.push_back(res); // Basically a hack, but behavior is all correct
		_objectIdMap.push_back(objectId);

		*isCopyReturned = false;
	} else if ((obTag & 0x8000) == 0) {
		if (_objectMap.size() < obTag) {
			error("Object index too big: %d  at 0x%08x", obTag, pos() - 2);
		}
		debug(7, "parseClass::obTag <%s>", lookupObjectId(_objectIdMap[obTag]));

		res = _objectMap[obTag];

		*isCopyReturned = true;
	} else {

		obTag &= ~0x8000;

		if (_objectMap.size() < obTag) {
			error("Object index too big: %d  at 0x%08x", obTag, pos() - 2);
		}

		debug(7, "parseClass::obTag <%s>", lookupObjectId(_objectIdMap[obTag]));

		objectId = _objectIdMap[obTag];

		res = createObject(objectId);
		_objectMap.push_back(res);
		_objectIdMap.push_back(objectId);

		*isCopyReturned = false;
	}

	return res;
}

char *genFileName(int superId, int sceneId, const char *ext) {
	char *s = (char *)calloc(256, 1);

	if (superId) {
		snprintf(s, 255, "%04d%04d.%s", superId, sceneId, ext);
	} else {
		snprintf(s, 255, "%04d.%s", sceneId, ext);
	}

	debug(7, "genFileName: %s", s);

	return s;
}

// Translates cp-1251..utf-8
byte *transCyrillic(byte *s) {
	static byte tmp[1024];

	static int trans[] = { 0xa8, 0xd081, 0xb8, 0xd191, 0xc0, 0xd090,
		0xc1, 0xd091, 0xc2, 0xd092, 0xc3, 0xd093, 0xc4, 0xd094,
		0xc5, 0xd095, 0xc6, 0xd096, 0xc7, 0xd097, 0xc8, 0xd098,
		0xc9, 0xd099, 0xca, 0xd09a, 0xcb, 0xd09b, 0xcc, 0xd09c,
		0xcd, 0xd09d, 0xce, 0xd09e, 0xcf, 0xd09f, 0xd0, 0xd0a0,
		0xd1, 0xd0a1, 0xd2, 0xd0a2, 0xd3, 0xd0a3, 0xd4, 0xd0a4,
		0xd5, 0xd0a5, 0xd6, 0xd0a6, 0xd7, 0xd0a7, 0xd8, 0xd0a8,
		0xd9, 0xd0a9, 0xda, 0xd0aa, 0xdb, 0xd0ab, 0xdc, 0xd0ac,
		0xdd, 0xd0ad, 0xde, 0xd0ae, 0xdf, 0xd0af, 0xe0, 0xd0b0,
		0xe1, 0xd0b1, 0xe2, 0xd0b2, 0xe3, 0xd0b3, 0xe4, 0xd0b4,
		0xe5, 0xd0b5, 0xe6, 0xd0b6, 0xe7, 0xd0b7, 0xe8, 0xd0b8,
		0xe9, 0xd0b9, 0xea, 0xd0ba, 0xeb, 0xd0bb, 0xec, 0xd0bc,
		0xed, 0xd0bd, 0xee, 0xd0be, 0xef, 0xd0bf, 0xf0, 0xd180,
		0xf1, 0xd181, 0xf2, 0xd182, 0xf3, 0xd183, 0xf4, 0xd184,
		0xf5, 0xd185, 0xf6, 0xd186, 0xf7, 0xd187, 0xf8, 0xd188,
		0xf9, 0xd189, 0xfa, 0xd18a, 0xfb, 0xd18b, 0xfc, 0xd18c,
		0xfd, 0xd18d, 0xfe, 0xd18e, 0xff, 0xd18f };

	int i = 0;

	for (byte *p = s; *p; p++) {
		if (*p < 128) {
			tmp[i++] = *p;
		} else {
			int j;
			for (j = 0; trans[j]; j += 2) {
				if (trans[j] == *p) {
					tmp[i++] = (trans[j + 1] >> 8) & 0xff;
					tmp[i++] = trans[j + 1] & 0xff;
					break;
				}
			}

			assert(trans[j]);
		}
	}

	tmp[i] = 0;

	return tmp;
}

} // End of namespace Fullpipe
