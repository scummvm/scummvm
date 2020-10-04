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

#include "ngi/ngi.h"

#include "common/file.h"
#include "common/memstream.h"
#include "common/ptr.h"

#include "ngi/objects.h"
#include "ngi/motion.h"
#include "ngi/ngiarchive.h"
#include "ngi/messages.h"
#include "ngi/interaction.h"

namespace NGI {

bool CObject::loadFile(const Common::String &fname) {
	Common::File file;

	if (!file.open(fname))
		return false;

	MfcArchive archive(&file);

	return load(archive);
}

bool ObArray::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "ObArray::load()");
	int count = file.readCount();

	reserve(count);

	for (int i = 0; i < count; i++) {
		CObject *t = file.readClass<CObject>();

		push_back(*t);
	}

	return true;
}

bool DWordArray::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "DWordArray::load()");
	int count = file.readCount();

	debugC(9, kDebugLoading, "DWordArray::count: %d", count);

	reserve(count);

	for (int i = 0; i < count; i++) {
		int32 t = file.readSint32LE();

		push_back(t);
	}

	return true;
}

Common::String MfcArchive::readPascalString(bool twoByte) {
	char *tmp;
	int len;
	Common::String result;

	if (twoByte)
		len = readUint16LE();
	else
		len = readByte();

	tmp = (char *)calloc(len + 1, 1);
	read(tmp, len);
	result = tmp;
	free(tmp);

	debugC(9, kDebugLoading, "readPascalString: %d <%s>", len, transCyrillic(result));

	return result;
}

void MfcArchive::writePascalString(const Common::String &str, bool twoByte) {
	int len = str.size();

	if (twoByte)
		writeUint16LE(len);
	else
		writeByte(len);

	write(str.c_str(), len);
}

MemoryObject::MemoryObject() {
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
}

bool MemoryObject::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "MemoryObject::load()");
	_memfilename = file.readPascalString();

	while (_memfilename.contains('\\')) {
		_memfilename.deleteChar(0);
	}

	if (g_nmi->_currArchive) {
		_mfield_14 = 0;
		_libHandle = g_nmi->_currArchive;
	}

	return true;
}

void MemoryObject::loadFile(const Common::String &filename) {
	debugC(5, kDebugLoading, "MemoryObject::loadFile(<%s>)", filename.c_str());

	if (filename.empty())
		return;

	if (!_data) {
		NGIArchive *arr = g_nmi->_currArchive;

		if (g_nmi->_currArchive != _libHandle && _libHandle)
			g_nmi->_currArchive = _libHandle;

		Common::SeekableReadStream *s = g_nmi->_currArchive->createReadStreamForMember(filename);

		if (s) {
			assert(s->size() > 0);

			_dataSize = s->size();

			debugC(5, kDebugLoading, "Loading %s (%d bytes)", filename.c_str(), _dataSize);
			_data = (byte *)calloc(_dataSize, 1);
			s->read(_data, _dataSize);
		} else {
			// We have no object to read. This is fine
		}

		g_nmi->_currArchive = arr;
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
	debugC(8, kDebugMemory, "MemoryObject::freeData(): file: %s", _memfilename.c_str());

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
	debugC(5, kDebugLoading, "MemoryObject2::load()");
	MemoryObject::load(file);

	_mflags |= 1;

	debugC(5, kDebugLoading, "MemoryObject2::load: <%s>", _memfilename.c_str());

	if (!_memfilename.empty()) {
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
	_stream = stream;
	_wstream = 0;

	init();
}

MfcArchive::MfcArchive(Common::WriteStream *stream) {
	_wstream = stream;
	_stream = 0;

	init();
}

void MfcArchive::init() {
	for (int i = 0; classMap[i].name; i++) {
		_classMap[classMap[i].name] = classMap[i].id;
	}

	_lastIndex = 1;
	_level = 0;

	_objectMap.push_back(0);
	_objectIdMap.push_back(kNullObject);
}

CObject *MfcArchive::readBaseClass() {
	bool isCopyReturned;
	CObject *res = parseClass(&isCopyReturned);

	if (res && !isCopyReturned)
		res->load(*this);

	return res;
}

CObject *MfcArchive::parseClass(bool *isCopyReturned) {
	Common::String name;
	int objectId = 0;
	CObject *res = 0;

	uint obTag = readUint16LE();

	debugC(7, kDebugLoading, "parseClass::obTag = %d (%04x)  at 0x%08x", obTag, obTag, pos() - 2);

	if (obTag == 0x0000) {
		return NULL;
	} else if (obTag == 0xffff) {
		int schema = readUint16LE();

		debugC(7, kDebugLoading, "parseClass::schema = %d", schema);

		name = readPascalString(true);
		debugC(7, kDebugLoading, "parseClass::class <%s>", name.c_str());

		if (!_classMap.contains(name.c_str())) {
			error("Unknown class in MfcArchive: <%s>", name.c_str());
		}

		objectId = _classMap[name.c_str()];

		debugC(7, kDebugLoading, "tag: %d 0x%x (%x)", _objectMap.size() - 1, _objectMap.size() - 1, objectId);

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
		debugC(7, kDebugLoading, "parseClass::obTag <%s>", lookupObjectId(_objectIdMap[obTag]));

		res = _objectMap[obTag];

		*isCopyReturned = true;
	} else {

		obTag &= ~0x8000;

		if (_objectMap.size() < obTag) {
			error("Object index too big: %d  at 0x%08x", obTag, pos() - 2);
		}

		debugC(7, kDebugLoading, "parseClass::obTag <%s>", lookupObjectId(_objectIdMap[obTag]));

		objectId = _objectIdMap[obTag];

		res = createObject(objectId);
		_objectMap.push_back(res);
		_objectIdMap.push_back(objectId);

		*isCopyReturned = false;
	}

	return res;
}

void MfcArchive::writeObject(CObject *obj) {
	if (obj == NULL) {
		writeUint16LE(0);
	} else if (_objectHash.contains(obj)) {
		int32 idx = _objectHash[obj];

		if (idx < 0x7fff) {
			writeUint16LE(idx);
		} else {
			writeUint16LE(0x7fff);
			writeUint32LE(idx);
		}
	} else {
		writeUint16LE(0xffff); // New class
		_objectHash[obj] = _lastIndex++;

		writeUint16LE(1); // schema

		switch (obj->_objtype) {
		case kObjTypeGameVar:
			writePascalString(lookupObjectId(kGameVar), true); // Two byte counter
			break;
		default:
			error("Unhandled save for object type: %d", obj->_objtype);
		}

		obj->save(*this);
	}
}

Common::String genFileName(int superId, int sceneId, const char *ext) {
	Common::String s;

	if (superId) {
		s = Common::String::format("%04d%04d.%s", superId, sceneId, ext);
	} else {
		s = Common::String::format("%04d.%s", sceneId, ext);
	}

	debugC(7, kDebugLoading, "genFileName: %s", s.c_str());

	return s;
}

// Translates cp-1251..utf-8
byte *transCyrillic(const Common::String &str) {
	const byte *s = (const byte *)str.c_str();
	static byte tmp[1024];

#ifndef WIN32
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
#endif

	int i = 0;

	for (const byte *p = s; *p; p++) {
#ifdef WIN32
		// translate from cp1251 to cp866
		byte c = *p;
		if (c >= 0xC0 && c <= 0xEF)
			c = c - 0xC0 + 0x80;
		else if (c >= 0xF0)
			c = c - 0xF0 + 0xE0;
		else if (c == 0xA8)
			c = 0xF0;
		else if (c == 0xB8)
			c = 0xF1;
		tmp[i++] = c;
#else
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
#endif
	}

	tmp[i] = 0;

	return tmp;
}

void NGIEngine::loadGameObjH() {
	Common::File file;

	if (!file.open("gameobj.h"))
		return;

	while(true) {
		Common::String s = file.readLine();

		if (file.eos())
			break;

		if (!s.hasPrefix("#define ")) {
			warning("Bad read: <%s>", s.c_str());
			continue;
		}

		int cnt = 0;
		const char *ptr = &s.c_str()[8]; // Skip '#define ''

		while (*ptr && *ptr != ' ') {
			cnt++;
			ptr++;
		}

		Common::String val(&s.c_str()[8], cnt);
		int key = strtol(ptr, NULL, 10);

		_gameObjH[(uint16)key] = val;
	}
}

Common::String NGIEngine::gameIdToStr(uint16 id) {
	if (_gameObjH.contains(id))
		return _gameObjH[id];

	return Common::String::format("%d", id);
}

} // End of namespace NGI
