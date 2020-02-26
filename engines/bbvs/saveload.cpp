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

#include "bbvs/bbvs.h"
#include "bbvs/gamemodule.h"
#include "common/savefile.h"
#include "graphics/thumbnail.h"

namespace Bbvs {

WARN_UNUSED_RESULT BbvsEngine::kReadSaveHeaderError BbvsEngine::readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail) {

	header.version = in->readUint32LE();
	if (header.version > BBVS_SAVEGAME_VERSION)
		return kRSHEInvalidVersion;

	byte descriptionLen = in->readByte();
	header.description = "";
	while (descriptionLen--)
		header.description += (char)in->readByte();

	if (!Graphics::loadThumbnail(*in, header.thumbnail, skipThumbnail)) {
		return kRSHEIoError;
	}

	// Not used yet, reserved for future usage
	header.gameID = in->readByte();
	header.flags = in->readUint32LE();

	header.saveDate = in->readUint32LE();
	header.saveTime = in->readUint32LE();
	header.playTime = in->readUint32LE();

	return ((in->eos() || in->err()) ? kRSHEIoError : kRSHENoError);
}

void BbvsEngine::savegame(const char *filename, const char *description) {

	Common::OutSaveFile *out;
	if (!(out = _system->getSavefileManager()->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return;
	}

	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	// Header start
	out->writeUint32LE(BBVS_SAVEGAME_VERSION);

	byte descriptionLen = strlen(description);
	out->writeByte(descriptionLen);
	out->write(description, descriptionLen);

	Graphics::saveThumbnail(*out);

	// Not used yet, reserved for future usage
	out->writeByte(0);
	out->writeUint32LE(0);
	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint32 saveTime = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);
	uint32 playTime = g_engine->getTotalPlayTime() / 1000;
	out->writeUint32LE(saveDate);
	out->writeUint32LE(saveTime);
	out->writeUint32LE(playTime);
	// Header end

	out->write(_snapshot, _snapshotStream->pos());

	out->finalize();
	delete out;
}

void BbvsEngine::loadgame(const char *filename) {
	Common::InSaveFile *in;
	if (!(in = _system->getSavefileManager()->openForLoading(filename))) {
		warning("Can't open file '%s', game not loaded", filename);
		return;
	}

	SaveHeader header;

	kReadSaveHeaderError errorCode = readSaveHeader(in, header);

	if (errorCode != kRSHENoError) {
		warning("Error loading savegame '%s'", filename);
		delete in;
		return;
	}

	g_engine->setTotalPlayTime(header.playTime * 1000);

	memset(_sceneObjects, 0, sizeof(_sceneObjects));
	for (int i = 0; i < kSceneObjectsCount; ++i) {
		_sceneObjects[i].walkDestPt.x = -1;
		_sceneObjects[i].walkDestPt.y = -1;
	}

	_currSceneNum = 0;
	_newSceneNum = in->readUint32LE();

	initScene(false);

	_prevSceneNum = in->readUint32LE();
	_gameState = in->readUint32LE();
	_mouseCursorSpriteIndex = in->readUint32LE();
	_mousePos.x = in->readUint16LE();
	_mousePos.y = in->readUint16LE();
	_currVerbNum = in->readUint32LE();
	_activeItemType = in->readUint32LE();
	_activeItemIndex = in->readUint32LE();
	_verbPos.x = in->readUint16LE();
	_verbPos.y = in->readUint16LE();
	_inventoryButtonIndex = in->readUint32LE();
	_currInventoryItem = in->readUint32LE();
	_currTalkObjectIndex = in->readUint32LE();
	_currCameraNum = in->readUint32LE();
	_cameraPos.x = in->readUint16LE();
	_cameraPos.y = in->readUint16LE();
	_newCameraPos.x = in->readUint16LE();
	_newCameraPos.y = in->readUint16LE();
	_dialogSlotCount = in->readUint32LE();
	_walkMousePos.x = in->readUint16LE();
	_walkMousePos.y = in->readUint16LE();
	in->read(_backgroundSoundsActive, kSceneSoundsCount);
	in->read(_inventoryItemStatus, kInventoryItemStatusCount);
	in->read(_dialogItemStatus, kDialogItemStatusCount);
	in->read(_gameVars, kGameVarsCount);
	in->read(_sceneVisited, kSceneVisitedCount);
	for (int i = 0; i < _gameModule->getSceneObjectDefsCount(); ++i) {
		SceneObject *obj = &_sceneObjects[i];
		obj->x = in->readUint32LE();
		obj->y = in->readUint32LE();
		obj->animIndex = in->readUint32LE();
		obj->frameIndex = in->readUint32LE();
		obj->frameTicks = in->readUint32LE();
		obj->walkCount = in->readUint32LE();
		obj->xIncr = in->readUint32LE();
		obj->yIncr = in->readUint32LE();
		obj->turnValue = in->readUint32LE();
		obj->turnCount = in->readUint32LE();
		obj->turnTicks = in->readUint32LE();
		obj->walkDestPt.x = in->readUint16LE();
		obj->walkDestPt.y = in->readUint16LE();
		obj->anim = obj->animIndex > 0 ? _gameModule->getAnimation(obj->animIndex) : 0;
	}

	updateWalkableRects();

	// Restart scene background sounds
	for (int i = 0; i < _gameModule->getSceneSoundsCount(); ++i) {
		if (_backgroundSoundsActive[i]) {
			SceneSound *sceneSound = _gameModule->getSceneSound(i);
			playSound(sceneSound->soundNum, true);
		}
	}

	_currAction = 0;
	_currActionCommandIndex = -1;

	delete in;

}

Common::Error BbvsEngine::loadGameState(int slot) {
	Common::String fileName = getSaveStateName(slot);
	loadgame(fileName.c_str());
	return Common::kNoError;
}

Common::Error BbvsEngine::saveGameState(int slot, const Common::String &description, bool isAutosave) {
	Common::String fileName = getSaveStateName(slot);
	savegame(fileName.c_str(), description.c_str());
	return Common::kNoError;
}

Common::String BbvsEngine::getSavegameFilename(const Common::String &target, int num) {
	assert(num >= 0 && num <= 999);
	return Common::String::format("%s.%03d", target.c_str(), num);
}

bool BbvsEngine::existsSavegame(int num) {
	return _system->getSavefileManager()->listSavefiles(getSavegameFilename(_targetName, num)).size() != 0;
}

void BbvsEngine::allocSnapshot() {
	_snapshot = new byte[kSnapshotSize];
	_snapshotStream = new Common::SeekableMemoryWriteStream(_snapshot, kSnapshotSize);
}

void BbvsEngine::freeSnapshot() {
	delete _snapshotStream;
	delete[] _snapshot;
}

void BbvsEngine::saveSnapshot() {
	_hasSnapshot = true;
	_snapshotStream->seek(0);
	_snapshotStream->writeUint32LE(_currSceneNum);
	_snapshotStream->writeUint32LE(_prevSceneNum);
	_snapshotStream->writeUint32LE(_gameState);
	_snapshotStream->writeUint32LE(_mouseCursorSpriteIndex);
	_snapshotStream->writeUint16LE(_mousePos.x);
	_snapshotStream->writeUint16LE(_mousePos.y);
	_snapshotStream->writeUint32LE(_currVerbNum);
	_snapshotStream->writeUint32LE(_activeItemType);
	_snapshotStream->writeUint32LE(_activeItemIndex);
	_snapshotStream->writeUint16LE(_verbPos.x);
	_snapshotStream->writeUint16LE(_verbPos.y);
	_snapshotStream->writeUint32LE(_inventoryButtonIndex);
	_snapshotStream->writeUint32LE(_currInventoryItem);
	_snapshotStream->writeUint32LE(_currTalkObjectIndex);
	_snapshotStream->writeUint32LE(_currCameraNum);
	_snapshotStream->writeUint16LE(_cameraPos.x);
	_snapshotStream->writeUint16LE(_cameraPos.y);
	_snapshotStream->writeUint16LE(_newCameraPos.x);
	_snapshotStream->writeUint16LE(_newCameraPos.y);
	_snapshotStream->writeUint32LE(_dialogSlotCount);
	_snapshotStream->writeUint16LE(_walkMousePos.x);
	_snapshotStream->writeUint16LE(_walkMousePos.y);
	_snapshotStream->write(_backgroundSoundsActive, kSceneSoundsCount);
	_snapshotStream->write(_inventoryItemStatus, kInventoryItemStatusCount);
	_snapshotStream->write(_dialogItemStatus, kDialogItemStatusCount);
	_snapshotStream->write(_gameVars, kGameVarsCount);
	_snapshotStream->write(_sceneVisited, kSceneVisitedCount);
	for (int i = 0; i < _gameModule->getSceneObjectDefsCount(); ++i) {
		SceneObject *obj = &_sceneObjects[i];
		_snapshotStream->writeUint32LE(obj->x);
		_snapshotStream->writeUint32LE(obj->y);
		_snapshotStream->writeUint32LE(obj->animIndex);
		_snapshotStream->writeUint32LE(obj->frameIndex);
		_snapshotStream->writeUint32LE(obj->frameTicks);
		_snapshotStream->writeUint32LE(obj->walkCount);
		_snapshotStream->writeUint32LE(obj->xIncr);
		_snapshotStream->writeUint32LE(obj->yIncr);
		_snapshotStream->writeUint32LE(obj->turnValue);
		_snapshotStream->writeUint32LE(obj->turnCount);
		_snapshotStream->writeUint32LE(obj->turnTicks);
		_snapshotStream->writeUint16LE(obj->walkDestPt.x);
		_snapshotStream->writeUint16LE(obj->walkDestPt.y);
	}
}

void BbvsEngine::writeContinueSavegame() {
	if (_hasSnapshot) {
		saveGameState(0, "Continue");
	}
}

} // End of namespace Bbvs
