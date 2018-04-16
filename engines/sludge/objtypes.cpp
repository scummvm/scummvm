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

#include "sludge/allfiles.h"
#include "sludge/fileset.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/sludge.h"
#include "sludge/variable.h"
#include "sludge/version.h"

namespace Sludge {

ObjectManager::~ObjectManager() {
	kill();
}

bool ObjectManager::init() {
	_allObjectTypes.clear();
	return true;
}

void ObjectManager::kill() {
	ObjectTypeList::iterator it;
	for (it = _allObjectTypes.begin(); it != _allObjectTypes.end(); ++it) {
		delete [](*it)->allCombis;
		delete (*it);
		(*it) = nullptr;
	}
	_allObjectTypes.clear();
}

ObjectType *ObjectManager::findObjectType(int i) {
	ObjectTypeList::iterator it;
	for (it = _allObjectTypes.begin(); it != _allObjectTypes.end(); ++it) {
		if ((*it)->objectNum == i) {
			return (*it);
		}
	}
	return loadObjectType(i);
}

ObjectType *ObjectManager::loadObjectType(int i) {
	int a, nameNum;
	ObjectType *newType = new ObjectType;
	ResourceManager *rm = _vm->_resMan;

	if (checkNew(newType)) {
		if (rm->openObjectSlice(i)) {
			Common::SeekableReadStream *readStream = rm->getData();
			nameNum = readStream->readUint16BE();
			newType->r = (byte)readStream->readByte();
			newType->g = (byte)readStream->readByte();
			newType->b = (byte)readStream->readByte();
			newType->speechGap = readStream->readByte();
			newType->walkSpeed = readStream->readByte();
			newType->wrapSpeech = readStream->readUint32LE();
			newType->spinSpeed = readStream->readUint16BE();

			if (gameVersion >= VERSION(1, 6)) {
				// aaLoad
				readStream->readByte();
				readStream->readFloatLE();
				readStream->readFloatLE();
			}

			if (gameVersion >= VERSION(1, 4)) {
				newType->flags = readStream->readUint16BE();
			} else {
				newType->flags = 0;
			}

			newType->numCom = readStream->readUint16BE();
			newType->allCombis = (newType->numCom) ? new Combination[newType->numCom] : nullptr;


			for (a = 0; a < newType->numCom; a++) {
				newType->allCombis[a].withObj = readStream->readUint16BE();
				newType->allCombis[a].funcNum = readStream->readUint16BE();
			}

			rm->finishAccess();
			newType->screenName = rm->getNumberedString(nameNum);
			newType->objectNum = i;
			_allObjectTypes.push_back(newType);
			return newType;
		}
	}

	return nullptr;
}

ObjectType *ObjectManager::loadObjectRef(Common::SeekableReadStream *stream) {
	ObjectType *r = loadObjectType(stream->readUint16BE());
	r->screenName.clear();
	r->screenName = readString(stream);
	return r;
}

void ObjectManager::saveObjectRef(ObjectType *r, Common::WriteStream *stream) {
	stream->writeUint16BE(r->objectNum);
	writeString(r->screenName, stream);
}

int ObjectManager::getCombinationFunction(int withThis, int thisObject) {
	int i, num = 0;
	ObjectType *obj = findObjectType(thisObject);

	for (i = 0; i < obj->numCom; i++) {
		if (obj->allCombis[i].withObj == withThis) {
			num = obj->allCombis[i].funcNum;
			break;
		}
	}

	return num;
}

void ObjectManager::removeObjectType(ObjectType *oT) {
	_allObjectTypes.remove(oT);
	delete []oT->allCombis;
	delete oT;
	oT = nullptr;
}

} // End of namespace Sludge
