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
#include "sludge/objtypes.h"
#include "sludge/variable.h"
#include "sludge/newfatal.h"
#include "sludge/moreio.h"
#include "sludge/sludge.h"
#include "sludge/fileset.h"
#include "sludge/version.h"

namespace Sludge {

objectType *allObjectTypes = NULL;

bool initObjectTypes() {
	return true;
}

objectType *findObjectType(int i) {
	objectType *huntType = allObjectTypes;

	while (huntType) {
		if (huntType->objectNum == i)
			return huntType;
		huntType = huntType->next;
	}

	return loadObjectType(i);
}

objectType *loadObjectType(int i) {
	int a, nameNum;
	objectType *newType = new objectType;

	if (checkNew(newType)) {
		if (g_sludge->_resMan->openObjectSlice(i)) {
			Common::SeekableReadStream *readStream = g_sludge->_resMan->getData();
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
			newType->allCombis = (newType->numCom) ? new combination[newType->numCom] : NULL;


			for (a = 0; a < newType->numCom; a++) {
				newType->allCombis[a].withObj = readStream->readUint16BE();
				newType->allCombis[a].funcNum = readStream->readUint16BE();
			}

			g_sludge->_resMan->finishAccess();
			newType->screenName = g_sludge->_resMan->getNumberedString(nameNum);
			newType->objectNum = i;
			newType->next = allObjectTypes;
			allObjectTypes = newType;
			return newType;
		}
	}

	return NULL;
}

objectType *loadObjectRef(Common::SeekableReadStream *stream) {
	objectType *r = loadObjectType(stream->readUint16BE());
	r->screenName.clear();
	r->screenName = readString(stream);
	return r;
}

void saveObjectRef(objectType *r, Common::WriteStream *stream) {
	stream->writeUint16BE(r->objectNum);
	writeString(r->screenName, stream);
}

int getCombinationFunction(int withThis, int thisObject) {
	int i, num = 0;
	objectType *obj = findObjectType(thisObject);

	for (i = 0; i < obj->numCom; i++) {
		if (obj->allCombis[i].withObj == withThis) {
			num = obj->allCombis[i].funcNum;
			break;
		}
	}

	return num;
}

void removeObjectType(objectType *oT) {
	objectType **huntRegion = &allObjectTypes;

	while (*huntRegion) {
		if ((*huntRegion) == oT) {
			*huntRegion = oT->next;
			delete []oT->allCombis;
			delete oT;
			return;
		} else {
			huntRegion = &((*huntRegion)->next);
		}
	}
	fatal("Can't delete object type: bad pointer");
}

} // End of namespace Sludge
