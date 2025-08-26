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

	verifyCopyProtection();

	for (int i = 0; i <= 3; i++)
		for (int j = 0; j < kWalkFrameCount; j++) {
			_mainCharAnimation.bitmap[i][j] = (byte *)malloc(_mainCharFrameSize);
			characterFile.read(_mainCharAnimation.bitmap[i][j], _mainCharFrameSize);
		}
	for (int i = 0; i < 4; i++)
		for (int j = kWalkFrameCount; j < (kWalkFrameCount + 10 * 3); j++) {
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

	_conversationData = new Common::MemorySeekableReadWriteStream(buf, fileSize, DisposeAfterUse::NO);
	conversationFile.close();
}

void saveDoorMetadata(DoorRegistry doors, Common::SeekableWriteStream *screenDataStream) {
	screenDataStream->writeUint16LE(doors.nextScene);
	screenDataStream->writeUint16LE(doors.exitPosX);
	screenDataStream->writeUint16LE(doors.exitPosY);
	screenDataStream->writeByte(doors.openclosed);
	screenDataStream->writeByte(doors.doorcode);
}

void savePoint(Common::Point point, Common::SeekableWriteStream *screenDataStream) {
	screenDataStream->writeUint16LE(point.x);
	screenDataStream->writeUint16LE(point.y);
}

void saveBitmapRegister(RoomBitmapRegister bitmap, Common::SeekableWriteStream *screenDataStream) {
	screenDataStream->writeSint32LE(bitmap.bitmapPointer);
	screenDataStream->writeUint16LE(bitmap.bitmapSize);
	screenDataStream->writeUint16LE(bitmap.coordx);
	screenDataStream->writeUint16LE(bitmap.coordy);
	screenDataStream->writeUint16LE(bitmap.depth);
}

void saveRoomObjectList(RoomObjectListEntry objectList, Common::SeekableWriteStream *screenDataStream) {

	screenDataStream->writeUint16LE(objectList.fileIndex);
	screenDataStream->writeByte(objectList.objectName.size());
	int paddingSize = 20 - objectList.objectName.size();
	if (paddingSize < 20) {
		screenDataStream->writeString(objectList.objectName);
	}
	if (paddingSize > 0) {
		char *padding = (char *)malloc(paddingSize);
		for (int i = 0; i < paddingSize; i++) {
			padding[i] = '\0';
		}
		screenDataStream->write(padding, paddingSize);

		free(padding);
	}
}

void saveRoom(RoomFileRegister *room, Common::SeekableWriteStream *screenDataStream) {
	screenDataStream->writeUint16LE(room->code);
	screenDataStream->writeUint32LE(room->roomImagePointer);
	screenDataStream->writeUint16LE(room->roomImageSize);
	screenDataStream->write(room->walkAreasGrid, 40 * 28);
	screenDataStream->write(room->mouseGrid, 40 * 28);

	// read puntos
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 30; j++) {
			for (int k = 0; k < 5; k++) {
				savePoint(room->trajectories[i][j][k], screenDataStream);
			}
		}
	}

	for (int i = 0; i < 5; i++) {
		saveDoorMetadata(room->doors[i], screenDataStream);
	}
	for (int i = 0; i < 15; i++) {
		saveBitmapRegister(room->screenLayers[i], screenDataStream);
	}
	for (int i = 0; i < 51; i++) {
		saveRoomObjectList(*room->screenObjectIndex[i], screenDataStream);
	}
	screenDataStream->writeByte(room->animationFlag);

	screenDataStream->writeByte(room->animationName.size());
	screenDataStream->writeString(room->animationName);
	int paddingSize = 8 - room->animationName.size();
	if (paddingSize > 0) {
		char *padding = (char *)malloc(paddingSize);
		for (int i = 0; i < paddingSize; i++) {
			padding[i] = '\0';
		}
		// 8 max char name
		screenDataStream->write(padding, paddingSize);

		free(padding);
	}
	screenDataStream->writeByte(room->paletteAnimationFlag);
	screenDataStream->writeUint16LE(room->palettePointer);
	for (int i = 0; i < 300; i++) {
		savePoint(room->secondaryAnimTrajectory[i], screenDataStream);
	}
	screenDataStream->write(room->secondaryAnimDirections, 600);
	screenDataStream->writeUint16LE(room->secondaryTrajectoryLength);
}

void TotEngine::saveRoomData(RoomFileRegister *room, Common::SeekableWriteStream *stream) {
	_rooms->seek(room->code * kRoomRegSize, SEEK_SET);
	saveRoom(room, stream);
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

void TotEngine::saveObjectsData(ScreenObject object, Common::SeekableWriteStream *objectDataStream) {
	objectDataStream->writeUint16LE(object.code);
	objectDataStream->writeByte(object.height);

	objectDataStream->writeByte(object.name.size());
	objectDataStream->writeString(object.name);
	int paddingSize = kObjectNameLength - object.name.size();
	if (paddingSize > 0) {
		char *padding = (char *)malloc(paddingSize);
		for (int i = 0; i < paddingSize; i++) {
			padding[i] = '\0';
		}
		// 8 max char name
		objectDataStream->write(padding, paddingSize);
		free(padding);
	}

	objectDataStream->writeUint16LE(object.lookAtTextRef);
	objectDataStream->writeUint16LE(object.beforeUseTextRef);
	objectDataStream->writeUint16LE(object.afterUseTextRef);
	objectDataStream->writeUint16LE(object.pickTextRef);
	objectDataStream->writeUint16LE(object.useTextRef);

	objectDataStream->writeByte(object.speaking);
	objectDataStream->writeByte(object.openable);
	objectDataStream->writeByte(object.closeable);

	objectDataStream->write(object.used, 8);

	objectDataStream->writeByte(object.pickupable);

	objectDataStream->writeUint16LE(object.useWith);
	objectDataStream->writeUint16LE(object.replaceWith);
	objectDataStream->writeByte(object.depth);
	objectDataStream->writeUint32LE(object.bitmapPointer);
	objectDataStream->writeUint16LE(object.bitmapSize);
	objectDataStream->writeUint16LE(object.rotatingObjectAnimation);
	objectDataStream->writeUint16LE(object.rotatingObjectPalette);
	objectDataStream->writeUint16LE(object.dropOverlayX);
	objectDataStream->writeUint16LE(object.dropOverlayY);
	objectDataStream->writeUint32LE(object.dropOverlay);
	objectDataStream->writeUint16LE(object.dropOverlaySize);
	objectDataStream->writeUint16LE(object.objectIconBitmap);

	objectDataStream->writeByte(object.xgrid1);
	objectDataStream->writeByte(object.ygrid1);
	objectDataStream->writeByte(object.xgrid2);
	objectDataStream->writeByte(object.ygrid2);

	objectDataStream->write(object.walkAreasPatch, 100);
	objectDataStream->write(object.mouseGridPatch, 100);
}

void TotEngine::saveObject(ScreenObject object, Common::SeekableWriteStream *stream) {
	_sceneObjectsData->seek(object.code * kItemRegSize, SEEK_SET);
	saveObjectsData(object, stream);
}

DoorRegistry readDoorMetadata(Common::SeekableReadStream *screenDataFile) {
	DoorRegistry doorMetadata;
	doorMetadata.nextScene = screenDataFile->readUint16LE();
	doorMetadata.exitPosX = screenDataFile->readUint16LE();
	doorMetadata.exitPosY = screenDataFile->readUint16LE();
	doorMetadata.openclosed = screenDataFile->readByte();
	doorMetadata.doorcode = screenDataFile->readByte();

	return doorMetadata;
}

Common::Point readPoint(Common::SeekableReadStream *screenDataFile) {
	Common::Point point;
	point.x = screenDataFile->readUint16LE();
	point.y = screenDataFile->readUint16LE();
	return point;
}

RoomBitmapRegister readAuxBitmaps(Common::SeekableReadStream *screenDataFile) {
	RoomBitmapRegister bitmapMetadata = RoomBitmapRegister();
	bitmapMetadata.bitmapPointer = screenDataFile->readSint32LE();
	bitmapMetadata.bitmapSize = screenDataFile->readUint16LE();
	bitmapMetadata.coordx = screenDataFile->readUint16LE();
	bitmapMetadata.coordy = screenDataFile->readUint16LE();
	bitmapMetadata.depth = screenDataFile->readUint16LE();
	return bitmapMetadata;
}

RoomObjectListEntry *readRoomObjects(Common::SeekableReadStream *screenDataFile) {
	RoomObjectListEntry *objectMetadata = new RoomObjectListEntry();
	objectMetadata->fileIndex = screenDataFile->readUint16LE();
	objectMetadata->objectName = screenDataFile->readPascalString();

	screenDataFile->skip(20 - objectMetadata->objectName.size());

	return objectMetadata;
}

RoomFileRegister *TotEngine::readScreenDataFile(Common::SeekableReadStream *screenDataFile) {
	RoomFileRegister *screenData = new RoomFileRegister();
	screenData->code = screenDataFile->readUint16LE();
	screenData->roomImagePointer = screenDataFile->readUint32LE();
	screenData->roomImageSize = screenDataFile->readUint16LE();
	screenDataFile->read(screenData->walkAreasGrid, 40 * 28);
	screenDataFile->read(screenData->mouseGrid, 40 * 28);
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 30; j++) {
			for (int k = 0; k < 5; k++) {
				screenData->trajectories[i][j][k] = readPoint(screenDataFile);
			}
		}
	}

	for (int i = 0; i < 5; i++) {
		screenData->doors[i] = readDoorMetadata(screenDataFile);
	}
	for (int i = 0; i < 15; i++) {
		screenData->screenLayers[i] = readAuxBitmaps(screenDataFile);
	}
	for (int i = 0; i < 51; i++) {
		screenData->screenObjectIndex[i] = readRoomObjects(screenDataFile);
	}

	screenData->animationFlag = screenDataFile->readByte();
	screenData->animationName = screenDataFile->readPascalString();
	screenDataFile->skip(8 - screenData->animationName.size());
	screenData->paletteAnimationFlag = screenDataFile->readByte();
	screenData->palettePointer = screenDataFile->readUint16LE();
	for (int i = 0; i < 300; i++) {
		screenData->secondaryAnimTrajectory[i] = readPoint(screenDataFile);
	}
	screenDataFile->read(screenData->secondaryAnimDirections, 600);
	screenData->secondaryTrajectoryLength = screenDataFile->readUint16LE();
	return screenData;
}

} // End of namespace Tot
