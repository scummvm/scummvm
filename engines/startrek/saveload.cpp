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

#include "gui/saveload.h"

#include "graphics/thumbnail.h"

#include "common/file.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/translation.h"

#include "startrek/resource.h"
#include "startrek/room.h"
#include "startrek/startrek.h"

namespace StarTrek {

bool StarTrekEngine::showSaveMenu() {
	GUI::SaveLoadChooser *dialog;
	Common::String desc;
	int slot;

	dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

	slot = dialog->runModalWithCurrentTarget();
	desc = dialog->getResultString();

	if (desc.empty()) {
		// create our own description for the saved game, the user didnt enter it
		desc = dialog->createDefaultSaveDescription(slot);
	}

	if (desc.size() > 28)
		desc = Common::String(desc.c_str(), 28);

	delete dialog;

	if (slot < 0)
		return true;

	return saveGame(slot, desc);
}

bool StarTrekEngine::showLoadMenu() {
	GUI::SaveLoadChooser *dialog;
	int slot;

	dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
	slot = dialog->runModalWithCurrentTarget();

	delete dialog;

	if (slot < 0)
		return true;

	return loadGame(slot);
}

const uint32 CURRENT_SAVEGAME_VERSION = 1;

bool StarTrekEngine::saveGame(int slot, Common::String desc) {
	Common::String filename = getSavegameFilename(slot);
	Common::OutSaveFile *out;

	if (!(out = _saveFileMan->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename.c_str());
		return false;
	} else {
		debug(3, "Successfully opened %s for writing", filename.c_str());
	}

	SavegameMetadata meta;
	meta.version = CURRENT_SAVEGAME_VERSION;
	memset(meta.description, 0, sizeof(meta.description));
	strncpy(meta.description, desc.c_str(), SAVEGAME_DESCRIPTION_LEN);

	TimeDate curTime;
	_system->getTimeAndDate(curTime);
	meta.setSaveTimeAndDate(curTime);
	meta.playTime = g_engine->getTotalPlayTime();

	if (!saveOrLoadMetadata(nullptr, out, &meta)) {
		delete out;
		return false;
	}
	if (!saveOrLoadGameData(nullptr, out, &meta)) {
		delete out;
		return false;
	}

	out->finalize();
	delete out;
	return true;
}

bool StarTrekEngine::loadGame(int slot) {
	Common::String filename = getSavegameFilename(slot);
	Common::InSaveFile *in;

	if (!(in = _saveFileMan->openForLoading(filename))) {
		warning("Can't open file '%s', game not loaded", filename.c_str());
		return false;
	} else {
		debug(3, "Successfully opened %s for loading", filename.c_str());
	}

	SavegameMetadata meta;
	if (!saveOrLoadMetadata(in, nullptr, &meta)) {
		delete in;
		return false;
	}

	if (meta.version > CURRENT_SAVEGAME_VERSION) {
		delete in;
		error("Savegame version (%u) is newer than current version (%u). A newer version of ScummVM is needed", meta.version, CURRENT_SAVEGAME_VERSION);
	}

	if (!saveOrLoadGameData(in, nullptr, &meta)) {
		delete in;
		return false;
	}

	delete in;

	_lastGameMode = _gameMode;

	if (_gameMode == GAMEMODE_AWAYMISSION) {
		for (int i = 0; i < NUM_ACTORS; i++) {
			Actor *a = &_actorList[i];
			if (a->spriteDrawn) {
				if (a->animType != 1)
					a->animFile = SharedPtr<Common::MemoryReadStreamEndian>(_resource->loadFile(a->animFilename + ".anm"));
				_gfx->addSprite(&a->sprite);
				a->sprite.setBitmap(loadAnimationFrame(a->bitmapFilename, a->scale));
			}
		}
	} else if (_gameMode == -1) {
		initBridge(true);
		_lastGameMode = GAMEMODE_BRIDGE;
		// TODO: mode change
	} else {
		_resource->setTxtFileName(_missionToLoad);
		initBridge(false);
		// TODO: mode change
	}

	return true;
}

bool StarTrekEngine::saveOrLoadGameData(Common::SeekableReadStream *in, Common::WriteStream *out, SavegameMetadata *meta) {
	Common::Serializer ser(in, out);

	if (ser.isLoading()) {
		if (_lastGameMode == GAMEMODE_BRIDGE)
			cleanupBridge();
		else // Assume GAMEMODE_AWAYMISSION
			unloadRoom();
	}

	ser.syncAsUint16LE(_gameMode);
	// TODO: sub_1d8eb (save) / sub_1d958 (load) (probably bridge / space combat state)

	Common::String midiFilename = _sound->_loadedMidiFilename;
	ser.syncString(midiFilename);
	ser.syncAsSint16LE(_sound->_loopingMidiTrack);

	if (ser.isLoading()) {
		if (midiFilename.empty())
			_sound->clearAllMidiSlots();
		else {
			_sound->loadMusicFile(midiFilename);
			_sound->playMidiMusicTracks(_sound->_loopingMidiTrack, _sound->_loopingMidiTrack);
		}
	}

	ser.syncAsUint16LE(_frameIndex);
	ser.syncAsUint16LE(_mouseControllingShip);
	if (meta->version >= 1) {
		ser.syncAsSint16LE(_enterpriseState.inOrbit);
		ser.syncAsSint16LE(_enterpriseState.underAttack);
		ser.syncAsSint16LE(_randomEncounterType);

		int16 unkFlag1 = 0; // TODO: word_5082e (either 0 or 1)
		ser.syncAsSint16LE(unkFlag1);

		int unkVar1 = 0;	// TODO: dword_519b0
		ser.syncAsSint32LE(unkVar1);

		ser.syncAsSint16LE(_currentPlanet);
		ser.syncAsSint16LE(_targetPlanet);

		int16 unkFlag2 = 0; // TODO: word_45ab8 (either 0 or -1)
		ser.syncAsSint16LE(unkFlag2);
	}

	ser.syncString(_missionToLoad);

	if (meta->version >= 1) {
		int16 unkFlag3 = 0; // TODO: word_4b032 (either 0 or 1)
		ser.syncAsSint16LE(unkFlag3);

		ser.syncAsUint16LE(_hailedTarget);
		ser.syncAsSint16LE(_lastMissionId);
		for (int i = 0; i < 7; i++) {
			ser.syncAsUint16LE(_missionPoints[i]);
		}
	}

	ser.syncString(_sound->_loopingAudioName);

	if (ser.isLoading()) {
		if (!_sound->_loopingAudioName.empty())
			_sound->playVoc(_sound->_loopingAudioName);
	}

	if (meta->version >= 1) {
		ser.syncAsSint16LE(_bridgeSequenceToLoad);
	}

	for (int i = 0; i < NUM_OBJECTS; i++) {
		ser.syncAsByte(_itemList[i].have);
	}

	if (_gameMode == GAMEMODE_AWAYMISSION) {
		ser.syncString(_missionName);
		ser.syncAsSint16LE(_roomIndex);

		if (ser.isLoading()) {
			_gfx->fadeoutScreen();
			_resource->setTxtFileName("ground");

			// This must be done before loading the actor variables, since this clears
			// them.
			loadRoom(_missionName, _roomIndex);
		}

		ser.syncAsUint32LE(_roomFrameCounter);
		ser.syncAsUint32LE(_frameIndex); // FIXME: redundant

		byte filler = 0;

		// Serialize the "actor" class
		for (int i = 0; i < NUM_ACTORS; i++) {
			Actor *a = &_actorList[i];
			ser.syncAsUint16LE(a->spriteDrawn);
			ser.syncString(a->animFilename);
			if (a->animFilename.size() < 15) {
				filler = 0;
				for (uint j = 0; j < 16 - a->animFilename.size() - 1; ++j)
					ser.syncAsByte(filler);	// make sure that exactly 16 bytes are synced
			}
			a->animFilename.trim();

			ser.syncAsUint16LE(a->animType);

			a->sprite.saveLoadWithSerializer(ser);

			ser.syncString(a->bitmapFilename);
			if (a->bitmapFilename.size() < 9) {
				filler = 0;
				for (uint j = 0; j < 10 - a->bitmapFilename.size() - 1; ++j)
					ser.syncAsByte(filler);	// make sure that exactly 10 bytes are synced
			}
			a->bitmapFilename.trim();

			a->scale.saveLoadWithSerializer(ser);
			// Can't save "animFile" (will be reloaded)
			ser.syncAsUint16LE(a->numAnimFrames);
			ser.syncAsUint16LE(a->animFrame);
			ser.syncAsUint32LE(a->frameToStartNextAnim);
			ser.syncAsSint16LE(a->pos.x);
			ser.syncAsSint16LE(a->pos.y);
			ser.syncAsUint16LE(a->field60);
			ser.syncAsUint16LE(a->field62);
			ser.syncAsUint16LE(a->triggerActionWhenAnimFinished);
			ser.syncAsUint16LE(a->finishedAnimActionParam);
			ser.syncString(a->animationString2);
			if (a->animationString2.size() < 7) {
				filler = 0;
				for (uint j = 0; j < 8 - a->animationString2.size() - 1; ++j)
					ser.syncAsByte(filler);	// make sure that exactly 8 bytes are synced
			}
			ser.syncAsUint16LE(a->field70);
			ser.syncAsUint16LE(a->field72);
			ser.syncAsUint16LE(a->field74);
			ser.syncAsUint16LE(a->field76);
			ser.syncAsSint16LE(a->iwSrcPosition);
			ser.syncAsSint16LE(a->iwDestPosition);
			a->granularPosX.saveLoadWithSerializer(ser);
			a->granularPosY.saveLoadWithSerializer(ser);
			a->speedX.saveLoadWithSerializer(ser);
			a->speedY.saveLoadWithSerializer(ser);
			ser.syncAsSint16LE(a->dest.x);
			ser.syncAsSint16LE(a->dest.y);
			ser.syncAsUint16LE(a->field90);
			ser.syncAsByte(a->field92);
			ser.syncAsByte(a->direction);
			ser.syncAsUint16LE(a->field94);
			ser.syncAsUint16LE(a->field96);
			ser.syncString(a->animationString);
			if (a->animationString.size() < 9) {
				filler = 0;
				for (uint j = 0; j < 10 - a->animationString.size() - 1; ++j)
					ser.syncAsByte(filler);	// make sure that exactly 10 bytes are synced
			}
			ser.syncAsUint16LE(a->fielda2);
			ser.syncAsUint16LE(a->fielda4);
			ser.syncAsUint16LE(a->fielda6);
		}

		Common::String unused = getScreenName();
		ser.syncString(unused);

		// Away mission struct
		for (int i = 0; i < 8; i++)
			ser.syncAsSint16LE(_awayMission.timers[i]);
		ser.syncAsSint16LE(_awayMission.mouseX);
		ser.syncAsSint16LE(_awayMission.mouseY);
		for (int i = 0; i < 4; i++)
			ser.syncAsSint16LE(_awayMission.crewGetupTimers[i]);
		ser.syncAsByte(_awayMission.disableWalking);
		ser.syncAsByte(_awayMission.disableInput);
		ser.syncAsByte(_awayMission.redshirtDead);
		ser.syncAsByte(_awayMission.activeAction);
		ser.syncAsByte(_awayMission.activeObject);
		ser.syncAsByte(_awayMission.passiveObject);
		ser.syncAsByte(_awayMission.rdfStillDoDefaultAction);
		ser.syncAsByte(_awayMission.crewDownBitset);
		for (int i = 0; i < 4; i++)
			ser.syncAsByte(_awayMission.crewDirectionsAfterWalk[i]);

		if (_missionName == "DEMON") {
			_awayMission.demon.saveLoadWithSerializer(ser);
			_room->_roomVar.demon.saveLoadWithSerializer(ser);
		} else if (_missionName == "TUG") {
			_awayMission.tug.saveLoadWithSerializer(ser);
			_room->_roomVar.tug.saveLoadWithSerializer(ser);
		} else if (_missionName == "LOVE") {
			_awayMission.love.saveLoadWithSerializer(ser);
			_room->_roomVar.love.saveLoadWithSerializer(ser);
		} else if (_missionName == "MUDD") {
			_awayMission.mudd.saveLoadWithSerializer(ser);
			_room->_roomVar.mudd.saveLoadWithSerializer(ser);
		} else if (_missionName == "FEATHER") {
			_awayMission.feather.saveLoadWithSerializer(ser);
			_room->_roomVar.feather.saveLoadWithSerializer(ser);
		} else if (_missionName == "TRIAL") {
			_awayMission.trial.saveLoadWithSerializer(ser);
			_room->_roomVar.trial.saveLoadWithSerializer(ser);
		} else if (_missionName == "SINS") {
			_awayMission.sins.saveLoadWithSerializer(ser);
			_room->_roomVar.sins.saveLoadWithSerializer(ser);
		} else if (_missionName == "VENG") {
			_awayMission.veng.saveLoadWithSerializer(ser);
			_room->_roomVar.veng.saveLoadWithSerializer(ser);
		}

		// The action queue
		if (ser.isLoading()) {
			_actionQueue = Common::Queue<Action>();
			int16 n = 0;
			ser.syncAsSint16LE(n);
			for (int i = 0; i < n; i++) {
				Action a;
				a.saveLoadWithSerializer(ser);
				_actionQueue.push(a);
			}
		} else { // Saving
			int16 n = _actionQueue.size();
			ser.syncAsSint16LE(n);
			for (int i = 0; i < n; i++) {
				Action a = _actionQueue.pop();
				a.saveLoadWithSerializer(ser);
				_actionQueue.push(a);
			}
		}

		// Original game located changes in RDF files and saved them. Since RDF files
		// aren't modified directly here, that's skipped.

		ser.syncAsSint16LE(_objectHasWalkPosition);
		ser.syncAsSint16LE(_objectWalkPosition.x);
		ser.syncAsSint16LE(_objectWalkPosition.y);

		for (int i = 0; i < MAX_BUFFERED_WALK_ACTIONS; i++) {
			_actionOnWalkCompletion[i].saveLoadWithSerializer(ser);
			ser.syncAsByte(_actionOnWalkCompletionInUse[i]);
		}

		ser.syncAsSint16LE(_warpHotspotsActive);
	}

	return true;
}

Common::String StarTrekEngine::getSavegameFilename(int slotId) const {
	Common::String saveLoadSlot = _targetName;
	saveLoadSlot += Common::String::format(".%.3d", slotId);
	return saveLoadSlot;
}


// Static function (reused in detection.cpp)
bool saveOrLoadMetadata(Common::SeekableReadStream *in, Common::WriteStream *out, SavegameMetadata *meta) {
	Common::Serializer ser(in, out);

	ser.syncAsUint32LE(meta->version);
	ser.syncBytes((byte *)meta->description, SAVEGAME_DESCRIPTION_LEN + 1);

	// Thumbnail
	if (ser.isLoading()) {
		if (!::Graphics::loadThumbnail(*in, meta->thumbnail))
			meta->thumbnail = nullptr;
	} else
		::Graphics::saveThumbnail(*out);

	// Creation date/time
	ser.syncAsUint32LE(meta->saveDate);
	debugC(5, kDebugSavegame, "Save date: %d", meta->saveDate);
	ser.syncAsUint16LE(meta->saveTime);
	debugC(5, kDebugSavegame, "Save time: %d", meta->saveTime);
	ser.syncAsByte(meta->saveTimeSecs); // write seconds of save time as well
	ser.syncAsUint32LE(meta->playTime);
	debugC(5, kDebugSavegame, "Play time: %d", meta->playTime);

	return true;
}

} // End of namespace StarTrek
