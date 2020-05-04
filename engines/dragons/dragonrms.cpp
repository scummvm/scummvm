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
#include "common/memstream.h"
#include "dragons/dragonrms.h"
#include "dragons/dragonobd.h"
#include "dragons/bigfile.h"

namespace Dragons {

#define DRAGON_RMS_STRUCT_SIZE 0x1c

DragonRMS::DragonRMS(BigfileArchive *bigfileArchive, DragonOBD *dragonOBD) : _dragonOBD(dragonOBD) {
	uint32 fileSize;
	byte *data = bigfileArchive->load("dragon.rms", fileSize);
	Common::SeekableReadStream *readStream = new Common::MemoryReadStream(data, fileSize, DisposeAfterUse::YES);

	_count = fileSize / DRAGON_RMS_STRUCT_SIZE;

	_rmsObjects = new RMS[_count];

	for (int i = 0; i < _count; i++) {
		_rmsObjects[i]._field0 = readStream->readSint32LE();
		readStream->read(_rmsObjects[i]._sceneName, 4);
		_rmsObjects[i]._afterDataLoadScript = readStream->readSint32LE();
		_rmsObjects[i]._afterSceneLoadScript = readStream->readSint32LE();
		_rmsObjects[i]._beforeLoadScript = readStream->readSint32LE();
		_rmsObjects[i]._inventoryBagPosition = readStream->readSint16LE();
		_rmsObjects[i]._field16 = readStream->readSint32LE();
		_rmsObjects[i]._field1a = readStream->readSint16LE();
	}
	delete readStream;
}

DragonRMS::~DragonRMS() {
	delete[] _rmsObjects;
}

char *DragonRMS::getSceneName(uint32 sceneId) {
	return getRMS(sceneId)->_sceneName;
}

byte *DragonRMS::getBeforeSceneDataLoadedScript(uint32 sceneId) {
	return _dragonOBD->getObdAtOffset(getRMS(sceneId)->_beforeLoadScript);
}

byte *DragonRMS::getAfterSceneDataLoadedScript(uint32 sceneId) {
	return _dragonOBD->getObdAtOffset(getRMS(sceneId)->_afterDataLoadScript);
}

byte *DragonRMS::getAfterSceneLoadedScript(uint32 sceneId) {
	return _dragonOBD->getObdAtOffset(getRMS(sceneId)->_afterSceneLoadScript);
}

int16 DragonRMS::getInventoryPosition(uint32 sceneId) {
	return getRMS(sceneId)->_inventoryBagPosition;
}

RMS *DragonRMS::getRMS(uint32 sceneId) {
	sceneId &= 0x7fff;
	assert(sceneId > 1);
	assert(sceneId - 2 < _count);
	return &_rmsObjects[sceneId - 2];
}

} // End of namespace Dragons
