/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

void TotEngine::readBitmap(int32 bitmapPosition, byte *buf, uint bitmapSize, uint error) {
	Common::File bitmapFile;
	if (!bitmapFile.open("BITMAPS.DAT")) {
		showError(error);
	}
	bitmapFile.seek(bitmapPosition);
	bitmapFile.read(buf, bitmapSize);

	bitmapFile.close();
}

void TotEngine::loadCharAnimation() {
	Common::File characterFile;
	_cpCounter = _cpCounter2;
	if (!characterFile.open("PERSONAJ.SPT"))
		showError(265);

	_mainCharFrameSize = characterFile.readUint16LE();

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < kWalkFrameCount; j++) {
			if (g_engine->_mainCharAnimation.bitmap[i][j])
				free(g_engine->_mainCharAnimation.bitmap[i][j]);
			_mainCharAnimation.bitmap[i][j] = (byte *)malloc(_mainCharFrameSize);
			characterFile.read(_mainCharAnimation.bitmap[i][j], _mainCharFrameSize);
		}
	for (int i = 0; i < 4; i++)
		for (int j = kWalkFrameCount; j < (kWalkFrameCount + 10 * 3); j++) {
			if (g_engine->_mainCharAnimation.bitmap[i][j])
				free(g_engine->_mainCharAnimation.bitmap[i][j]);
			_mainCharAnimation.bitmap[i][j] = (byte *)malloc(_mainCharFrameSize);
			characterFile.read(_mainCharAnimation.bitmap[i][j], _mainCharFrameSize);
		}
	characterFile.close();
}

void TotEngine::readTextFile() {
	if (!_verbFile.open("CONVERSA.TXT")) {
		showError(313);
	}
}

TextEntry TotEngine::readTextRegister(uint numRegister) {
	_verbFile.seek(numRegister * kVerbRegSize);
	return readTextRegister();
}

TextEntry TotEngine::readTextRegister() {
	TextEntry regmht;
	// Since the text is encrypted it's safer to save the size as reported by
	// the pascal string, because the encrypter character might be the termination
    // character
	byte size = _verbFile.readByte();
	_verbFile.seek(-1, SEEK_CUR);
	regmht.text = _verbFile.readPascalString(false);
	_verbFile.skip(255 - size);
	regmht.continued = _verbFile.readByte();
	regmht.response = _verbFile.readUint16LE();
	regmht.pointer = _verbFile.readSint32LE();
	return regmht;
}

void TotEngine::readConversationFile() {
	Common::File conversationFile;
	if (!conversationFile.open(Common::Path(Common::String("CONVERSA.TRE")))) {
		showError(314);
	}
	int64 fileSize = conversationFile.size();
	byte *buf = (byte *)malloc(fileSize);
	conversationFile.read(buf, fileSize);

	_conversationData = new Common::MemorySeekableReadWriteStream(buf, fileSize, DisposeAfterUse::YES);
	conversationFile.close();
}

void saveDoorMetadata(DoorRegistry doors, Common::SeekableWriteStream *out) {
	out->writeUint16LE(doors.nextScene);
	out->writeUint16LE(doors.exitPosX);
	out->writeUint16LE(doors.exitPosY);
	out->writeByte(doors.openclosed);
	out->writeByte(doors.doorcode);
}

void savePoint(Common::Point point, Common::SeekableWriteStream *out) {
	out->writeUint16LE(point.x);
	out->writeUint16LE(point.y);
}

void saveBitmapRegister(RoomBitmapRegister bitmap, Common::SeekableWriteStream *out) {
	out->writeSint32LE(bitmap.bitmapPointer);
	out->writeUint16LE(bitmap.bitmapSize);
	out->writeUint16LE(bitmap.coordx);
	out->writeUint16LE(bitmap.coordy);
	out->writeUint16LE(bitmap.depth);
}

void saveRoomObjectList(RoomObjectListEntry objectList, Common::SeekableWriteStream *out) {

	out->writeUint16LE(objectList.fileIndex);
	out->writeByte(objectList.objectName.size());
	int paddingSize = 20 - objectList.objectName.size();
	if (paddingSize < 20) {
		out->writeString(objectList.objectName);
	}
	if (paddingSize > 0) {
		char *padding = (char *)calloc(paddingSize, 1);
		out->write(padding, paddingSize);

		free(padding);
	}
}

void saveRoom(RoomFileRegister *room, Common::SeekableWriteStream *out) {
	out->writeUint16LE(room->code);
	out->writeUint32LE(room->roomImagePointer);
	out->writeUint16LE(room->roomImageSize);
	out->write(room->walkAreasGrid, 40 * 28);
	out->write(room->mouseGrid, 40 * 28);

	// read puntos
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 30; j++) {
			for (int k = 0; k < 5; k++) {
				savePoint(room->trajectories[i][j][k], out);
			}
		}
	}

	for (int i = 0; i < 5; i++) {
		saveDoorMetadata(room->doors[i], out);
	}
	for (int i = 0; i < 15; i++) {
		saveBitmapRegister(room->screenLayers[i], out);
	}
	for (int i = 0; i < 51; i++) {
		saveRoomObjectList(*room->screenObjectIndex[i], out);
	}
	out->writeByte(room->animationFlag);

	out->writeByte(room->animationName.size());
	out->writeString(room->animationName);
	int paddingSize = 8 - room->animationName.size();
	if (paddingSize > 0) {
		char *padding = (char *)calloc(paddingSize, 1);
		// 8 max char name
		out->write(padding, paddingSize);

		free(padding);
	}
	out->writeByte(room->paletteAnimationFlag);
	out->writeUint16LE(room->palettePointer);
	for (int i = 0; i < 300; i++) {
		savePoint(room->secondaryAnimTrajectory[i], out);
	}
	out->write(room->secondaryAnimDirections, 600);
	out->writeUint16LE(room->secondaryTrajectoryLength);
}

void TotEngine::saveRoomData(RoomFileRegister *room, Common::SeekableWriteStream *out) {
	_rooms->seek(room->code * kRoomRegSize, SEEK_SET);
	saveRoom(room, out);
}

void TotEngine::readObject(Common::SeekableReadStream *stream, uint itemPos, ScreenObject &thisRegObj) {
	stream->seek(itemPos * kItemRegSize);
	clearCurrentInventoryObject();
	thisRegObj.code = stream->readUint16LE();
	thisRegObj.height = stream->readByte();

	thisRegObj.name = stream->readPascalString();

	stream->skip(kObjectNameLength - thisRegObj.name.size());

	thisRegObj.lookAtTextRef = stream->readUint16LE();
	thisRegObj.beforeUseTextRef = stream->readUint16LE();
	thisRegObj.afterUseTextRef = stream->readUint16LE();
	thisRegObj.pickTextRef = stream->readUint16LE();
	thisRegObj.useTextRef = stream->readUint16LE();
	thisRegObj.speaking = stream->readByte();
	thisRegObj.openable = stream->readByte();
	thisRegObj.closeable = stream->readByte();

	stream->read(thisRegObj.used, 8);

	thisRegObj.pickupable = stream->readByte();
	thisRegObj.useWith = stream->readUint16LE();
	thisRegObj.replaceWith = stream->readUint16LE();
	thisRegObj.depth = stream->readByte();
	thisRegObj.bitmapPointer = stream->readUint32LE();
	thisRegObj.bitmapSize = stream->readUint16LE();
	thisRegObj.rotatingObjectAnimation = stream->readUint32LE();
	thisRegObj.rotatingObjectPalette = stream->readUint16LE();
	thisRegObj.dropOverlayX = stream->readUint16LE();
	thisRegObj.dropOverlayY = stream->readUint16LE();
	thisRegObj.dropOverlay = stream->readUint32LE();
	thisRegObj.dropOverlaySize = stream->readUint16LE();
	thisRegObj.objectIconBitmap = stream->readUint16LE();
	thisRegObj.xgrid1 = stream->readByte();
	thisRegObj.ygrid1 = stream->readByte();
	thisRegObj.xgrid2 = stream->readByte();
	thisRegObj.ygrid2 = stream->readByte();
	stream->read(thisRegObj.walkAreasPatch, 100);
	stream->read(thisRegObj.mouseGridPatch, 100);
}

void TotEngine::readObject(uint itemPosition) {
	readObject(_sceneObjectsData, itemPosition, _curObject);
}

void TotEngine::updateObject(uint itemPosition) {
	_curObject.used[0] = 9;
	_sceneObjectsData->seek(itemPosition);
	saveObject(_curObject, _sceneObjectsData);
}

/**
 * Object files contain a single register per object, with a set of 8 flags, to mark them as used in each save.
 */
void TotEngine::initializeObjectFile() {
	Common::File objFile;
	if (!objFile.open(Common::Path("OBJETOS.DAT"))) {
		showError(261);
	}
	delete (_sceneObjectsData);
	byte *objectData = (byte *)malloc(objFile.size());
	objFile.read(objectData, objFile.size());
	_sceneObjectsData = new Common::MemorySeekableReadWriteStream(objectData, objFile.size(), DisposeAfterUse::NO);
	objFile.close();
}

void TotEngine::saveObjectsData(ScreenObject object, Common::SeekableWriteStream *out) {
	out->writeUint16LE(object.code);
	out->writeByte(object.height);

	out->writeByte(object.name.size());
	out->writeString(object.name);
	int paddingSize = kObjectNameLength - object.name.size();
	if (paddingSize > 0) {
		char *padding = (char *)calloc(paddingSize, 1);
		// 8 max char name
		out->write(padding, paddingSize);
		free(padding);
	}

	out->writeUint16LE(object.lookAtTextRef);
	out->writeUint16LE(object.beforeUseTextRef);
	out->writeUint16LE(object.afterUseTextRef);
	out->writeUint16LE(object.pickTextRef);
	out->writeUint16LE(object.useTextRef);

	out->writeByte(object.speaking);
	out->writeByte(object.openable);
	out->writeByte(object.closeable);

	out->write(object.used, 8);

	out->writeByte(object.pickupable);

	out->writeUint16LE(object.useWith);
	out->writeUint16LE(object.replaceWith);
	out->writeByte(object.depth);
	out->writeUint32LE(object.bitmapPointer);
	out->writeUint16LE(object.bitmapSize);
	out->writeUint16LE(object.rotatingObjectAnimation);
	out->writeUint16LE(object.rotatingObjectPalette);
	out->writeUint16LE(object.dropOverlayX);
	out->writeUint16LE(object.dropOverlayY);
	out->writeUint32LE(object.dropOverlay);
	out->writeUint16LE(object.dropOverlaySize);
	out->writeUint16LE(object.objectIconBitmap);

	out->writeByte(object.xgrid1);
	out->writeByte(object.ygrid1);
	out->writeByte(object.xgrid2);
	out->writeByte(object.ygrid2);

	out->write(object.walkAreasPatch, 100);
	out->write(object.mouseGridPatch, 100);
}

void TotEngine::saveObject(ScreenObject object, Common::SeekableWriteStream *out) {
	_sceneObjectsData->seek(object.code * kItemRegSize, SEEK_SET);
	saveObjectsData(object, out);
}

DoorRegistry readDoorMetadata(Common::SeekableReadStream *in) {
	DoorRegistry doorMetadata;
	doorMetadata.nextScene = in->readUint16LE();
	doorMetadata.exitPosX = in->readUint16LE();
	doorMetadata.exitPosY = in->readUint16LE();
	doorMetadata.openclosed = in->readByte();
	doorMetadata.doorcode = in->readByte();

	return doorMetadata;
}

Common::Point readPoint(Common::SeekableReadStream *in) {
	Common::Point point;
	point.x = in->readUint16LE();
	point.y = in->readUint16LE();
	return point;
}

RoomBitmapRegister readAuxBitmaps(Common::SeekableReadStream *in) {
	RoomBitmapRegister bitmapMetadata = RoomBitmapRegister();
	bitmapMetadata.bitmapPointer = in->readSint32LE();
	bitmapMetadata.bitmapSize = in->readUint16LE();
	bitmapMetadata.coordx = in->readUint16LE();
	bitmapMetadata.coordy = in->readUint16LE();
	bitmapMetadata.depth = in->readUint16LE();
	return bitmapMetadata;
}

RoomObjectListEntry *readRoomObjects(Common::SeekableReadStream *in) {
	RoomObjectListEntry *objectMetadata = new RoomObjectListEntry();
	objectMetadata->fileIndex = in->readUint16LE();
	objectMetadata->objectName = in->readPascalString();

	in->skip(20 - objectMetadata->objectName.size());

	return objectMetadata;
}

RoomFileRegister *TotEngine::readScreenDataFile(Common::SeekableReadStream *in) {
	RoomFileRegister *screenData = new RoomFileRegister();
	screenData->code = in->readUint16LE();
	screenData->roomImagePointer = in->readUint32LE();
	screenData->roomImageSize = in->readUint16LE();
	in->read(screenData->walkAreasGrid, 40 * 28);
	in->read(screenData->mouseGrid, 40 * 28);
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 30; j++) {
			for (int k = 0; k < 5; k++) {
				screenData->trajectories[i][j][k] = readPoint(in);
			}
		}
	}

	for (int i = 0; i < 5; i++) {
		screenData->doors[i] = readDoorMetadata(in);
	}
	for (int i = 0; i < 15; i++) {
		screenData->screenLayers[i] = readAuxBitmaps(in);
	}
	for (int i = 0; i < 51; i++) {
		screenData->screenObjectIndex[i] = readRoomObjects(in);
	}

	screenData->animationFlag = in->readByte();
	screenData->animationName = in->readPascalString();
	in->skip(8 - screenData->animationName.size());
	screenData->paletteAnimationFlag = in->readByte();
	screenData->palettePointer = in->readUint16LE();
	for (int i = 0; i < 300; i++) {
		screenData->secondaryAnimTrajectory[i] = readPoint(in);
	}
	in->read(screenData->secondaryAnimDirections, 600);
	screenData->secondaryTrajectoryLength = in->readUint16LE();
	return screenData;
}

} // End of namespace Tot
