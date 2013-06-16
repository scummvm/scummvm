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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "fullpipe/fullpipe.h"

#include "common/file.h"

#include "fullpipe/utils.h"
#include "fullpipe/objects.h"
#include "fullpipe/motion.h"

namespace Fullpipe {

char *MfcArchive::readPascalString(bool twoByte) {
	char *tmp;
	int len;

	if (twoByte)
		len = readUint16LE();
	else
		len = readByte();

	tmp = (char *)calloc(len + 1, 1);
	read(tmp, len);

	debug(9, "readPascalString: %d <%s>", len, tmp);

	return tmp;
}

int MfcArchive::readCount() {
	int count = readUint16LE();

	if (count == 0xffff)
		count = readUint32LE();

	return count;
}

double MfcArchive::readDouble() {
  // FIXME: This is utterly cruel and unportable

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
	kCInteraction,
	kMessageQueue,
	kExCommand,
	kCObjstateCommand,
	kCGameVar,
	kCMctlCompound,
	kCMovGraph,
	kCMovGraphLink,
	kCMovGraphNode
};

const struct {
	const char *name;
	int id;
} classMap[] = {
	{ "CInteraction",	kCInteraction },
	{ "MessageQueue",	kMessageQueue },
	{ "ExCommand",		kExCommand },
	{ "CObjstateCommand", kCObjstateCommand },
	{ "CGameVar",		kCGameVar },
	{ "CMctlCompound",	kCMctlCompound },
	{ "CMovGraph",		kCMovGraph },
	{ "CMovGraphLink",	kCMovGraphLink },
	{ "CMovGraphNode",	kCMovGraphNode },
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
	case kCInteraction:
		return new CInteraction();
	case kMessageQueue:
		return new MessageQueue();
	case kExCommand:
		return new ExCommand();
	case kCObjstateCommand:
		return new CObjstateCommand();
	case kCGameVar:
		return new CGameVar();
	case kCMctlCompound:
		return new CMctlCompound();
	case kCMovGraph:
		return new CMovGraph();
	case kCMovGraphLink:
		return new CMovGraphLink();
	case kCMovGraphNode:
		return new CMovGraphNode();
	default:
		error("Unknown objectId: %d", objectId);
	}

	return 0;
}

MfcArchive::MfcArchive() {
	for (int i = 0; classMap[i].name; i++) {
		_classMap[classMap[i].name] = classMap[i].id;
	}

	_lastIndex = 1;
	_level = 0;

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

} // End of namespace Fullpipe
