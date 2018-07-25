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

	/*
	dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
	slot = dialog->runModalWithCurrentTarget();
	*/

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

const uint32 CURRENT_SAVEGAME_VERSION = 0;

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
		error("Savegame version (%d) is newer than current version (%d). A newer version of ScummVM is needed", meta.version, CURRENT_SAVEGAME_VERSION);
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
					a->animFile = loadFile(Common::String(a->animFilename) + ".anm");
				_gfx->addSprite(&a->sprite);
				a->sprite.setBitmap(loadAnimationFrame(a->bitmapFilename, a->scale));
			}
		}
	} else if (_gameMode == -1) {
		initBridge(true);
		_lastGameMode = GAMEMODE_BRIDGE;
		// TODO: mode change
	} else {
		_txtFilename = _missionToLoad;
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

	ser.syncString(_sound->_loadedMidiFilename);
	ser.syncAsSint16LE(_sound->_loopingMidiTrack);

	if (ser.isLoading()) {
		if (_sound->_loadedMidiFilename.empty())
			_sound->clearAllMidiSlots();
		else {
			_sound->loadMusicFile(_sound->_loadedMidiFilename);
			_sound->playMidiMusicTracks(_sound->_loopingMidiTrack, _sound->_loopingMidiTrack);
		}
	}

	ser.syncAsUint16LE(_frameIndex);
	ser.syncAsUint16LE(_mouseControllingShip);
	// TODO: word_45aa8
	// TODO: word_45aaa
	// TODO: word_45aac
	// TODO: word_5082e
	// TODO: dword_519b0
	// TODO: word_45ab2
	// TODO: word_45ab4
	// TODO: word_45ab8

	ser.syncString(_missionToLoad);
	// TODO: word_4b032
	// TODO: word_519bc
	// TODO: word_45c5c
	// TODO: unk_52afe
	ser.syncString(_sound->_loopingAudioName);

	if (ser.isLoading()) {
		if (!_sound->_loopingAudioName.empty())
			_sound->playVoc(_sound->_loopingAudioName);
	}

	// TODO: word_45a50

	for (int i = 0; i < NUM_OBJECTS; i++) {
		ser.syncAsByte(_itemList[i].have);
	}

	if (_gameMode == GAMEMODE_AWAYMISSION) {
		ser.syncString(_missionName);
		ser.syncAsSint16LE(_roomIndex);

		if (ser.isLoading()) {
			_gfx->fadeoutScreen();
			_txtFilename = "ground";

			// This must be done before loading the actor variables, since this clears
			// them.
			loadRoom(_missionName, _roomIndex);
		}

		ser.syncAsUint32LE(_roomFrameCounter);
		ser.syncAsUint32LE(_frameIndex); // FIXME: redundant

		// Serialize the "actor" class
		for (int i = 0; i < NUM_ACTORS; i++) {
			Actor *a = &_actorList[i];
			ser.syncAsUint16LE(a->spriteDrawn);
			ser.syncBytes((byte *)a->animFilename, 16);
			ser.syncAsUint16LE(a->animType);

			a->sprite.saveLoadWithSerializer(ser);

			ser.syncBytes((byte *)a->bitmapFilename, 10);
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
			ser.syncBytes((byte *)a->animationString2, 8);
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
			ser.syncBytes((byte *)a->animationString, 10);
			ser.syncAsUint16LE(a->fielda2);
			ser.syncAsUint16LE(a->fielda4);
			ser.syncAsUint16LE(a->fielda6);
		}

		ser.syncString(_mapFilename);

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
			ser.syncAsByte(_awayMission.demon.wasRudeToPrelate);
			ser.syncAsByte(_awayMission.demon.insultedStephen);
			ser.syncAsByte(_awayMission.demon.field2d);
			ser.syncAsByte(_awayMission.demon.beatKlingons);
			ser.syncAsByte(_awayMission.demon.tookKlingonHand);
			ser.syncAsByte(_awayMission.demon.talkedToPrelate);
			ser.syncAsByte(_awayMission.demon.stephenWelcomedToStudy);
			ser.syncAsByte(_awayMission.demon.prelateWelcomedCrew);
			ser.syncAsByte(_awayMission.demon.askedPrelateAboutSightings);
			ser.syncAsByte(_awayMission.demon.field37);
			ser.syncAsByte(_awayMission.demon.mccoyMentionedFlora);
			ser.syncAsByte(_awayMission.demon.numBouldersGone);
			ser.syncAsByte(_awayMission.demon.enteredFrom);
			ser.syncAsByte(_awayMission.demon.repairedHand);
			ser.syncAsByte(_awayMission.demon.healedMiner);
			ser.syncAsByte(_awayMission.demon.curedChub);
			ser.syncAsByte(_awayMission.demon.field3e);
			ser.syncAsByte(_awayMission.demon.knowAboutHypoDytoxin);
			ser.syncAsByte(_awayMission.demon.minerDead);
			ser.syncAsByte(_awayMission.demon.field41);
			ser.syncAsByte(_awayMission.demon.foundMiner);
			ser.syncAsByte(_awayMission.demon.field45);
			ser.syncAsByte(_awayMission.demon.gaveSkullToNauian);
			ser.syncAsByte(_awayMission.demon.warpsDisabled);
			ser.syncAsByte(_awayMission.demon.boulder1Gone);
			ser.syncAsByte(_awayMission.demon.boulder2Gone);
			ser.syncAsByte(_awayMission.demon.boulder3Gone);
			ser.syncAsByte(_awayMission.demon.boulder4Gone);
			ser.syncAsByte(_awayMission.demon.doorOpened);
			ser.syncAsByte(_awayMission.demon.solvedSunPuzzle);
			ser.syncAsByte(_awayMission.demon.itemsTakenFromCase);
			ser.syncAsByte(_awayMission.demon.gotBerries);
			ser.syncAsByte(_awayMission.demon.madeHypoDytoxin);
			ser.syncAsByte(_awayMission.demon.metNauian);
			ser.syncAsByte(_awayMission.demon.gavePointsForDytoxin);
			ser.syncAsByte(_awayMission.demon.lookedAtComputer);
			ser.syncAsByte(_awayMission.demon.field56);
			ser.syncAsByte(_awayMission.demon.foundAlienRoom);
			ser.syncAsSint16LE(_awayMission.demon.missionScore);

			// demon0
			ser.syncAsByte(_room->_roomVar.demon.bottomDoorCounter);
			ser.syncAsByte(_room->_roomVar.demon.topDoorCounter);
			ser.syncAsByte(_room->_roomVar.demon.movingToTopDoor);
			ser.syncAsByte(_room->_roomVar.demon.movingToBottomDoor);

			// demon1
			ser.syncBytes((byte *)_room->_roomVar.demon.klingonShot, 3);
			ser.syncAsSint16LE(_room->_roomVar.demon.numKlingonsKilled);
			ser.syncAsByte(_room->_roomVar.demon.attackIndex);
			ser.syncAsByte(_room->_roomVar.demon.kirkShooting);
			ser.syncBytes((byte *)_room->_roomVar.demon.d6, 10);

			// demon3
			ser.syncAsByte(_room->_roomVar.demon.shootingBoulder);
			ser.syncAsByte(_room->_roomVar.demon.boulder1Shot);
			ser.syncAsByte(_room->_roomVar.demon.boulderBeingShot);
			ser.syncAsByte(_room->_roomVar.demon.kirkInPosition);
			ser.syncAsByte(_room->_roomVar.demon.redshirtInPosition);
			ser.syncAsByte(_room->_roomVar.demon.spockInPosition);
			ser.syncAsByte(_room->_roomVar.demon.mccoyInPosition);
			ser.syncAsByte(_room->_roomVar.demon.inFiringPosition);
			ser.syncAsByte(_room->_roomVar.demon.kirkPhaserOut);
			ser.syncBytes((byte *)_room->_roomVar.demon.boulderAnim, 10);
			ser.syncAsSint16LE(_room->_roomVar.demon.usedPhaserOnDoor);

			// demon4
			ser.syncAsByte(_room->_roomVar.demon.triedToShootNauian);
			ser.syncAsByte(_room->_roomVar.demon.nauianEmerged);
			ser.syncAsByte(_room->_roomVar.demon.disabledSecurity);
			ser.syncAsByte(_room->_roomVar.demon.cd);
			ser.syncAsByte(_room->_roomVar.demon.crewReadyToBeamOut);
			ser.syncAsSint16LE(_room->_roomVar.demon.crewmanUsingPanel);

			// demon5
			ser.syncAsByte(_room->_roomVar.demon.scannedRoberts);
			ser.syncAsByte(_room->_roomVar.demon.scannedChub);
			ser.syncAsByte(_room->_roomVar.demon.scannedGrisnash);
			ser.syncAsByte(_room->_roomVar.demon.scannedStephen);
			ser.syncAsByte(_room->_roomVar.demon.numScanned);
			ser.syncAsByte(_room->_roomVar.demon.numTalkedTo);
			ser.syncAsByte(_room->_roomVar.demon.talkedToRoberts);
			ser.syncAsByte(_room->_roomVar.demon.talkedToChub);
			ser.syncAsByte(_room->_roomVar.demon.talkedToGrisnash);
			ser.syncAsByte(_room->_roomVar.demon.talkedToStephen);

			// demon6
			ser.syncAsByte(_room->_roomVar.demon.insultedStephenRecently);
			ser.syncAsByte(_room->_roomVar.demon.stephenInRoom);
			ser.syncAsByte(_room->_roomVar.demon.caseOpened);

			// common
			ser.syncAsByte(_room->_roomVar.demon.movingToDoor);
			ser.syncAsByte(_room->_roomVar.demon.doorCounter);
		} else if (_missionName == "TUG") {
			ser.syncAsSint16LE(_awayMission.tug.missionScore);
			ser.syncAsSint16LE(_awayMission.tug.field2b);
			ser.syncAsSint16LE(_awayMission.tug.field2d);
			ser.syncAsByte(_awayMission.tug.engineerConscious);
			ser.syncAsByte(_awayMission.tug.field35);
			ser.syncAsByte(_awayMission.tug.gotWires);
			ser.syncAsByte(_awayMission.tug.orbitalDecayCounter);
			ser.syncAsByte(_awayMission.tug.bridgeElasiDrewPhasers);
			ser.syncAsByte(_awayMission.tug.talkedToCereth);
			ser.syncAsByte(_awayMission.tug.gotJunkPile);
			ser.syncAsByte(_awayMission.tug.gotTransmogrifier);
			ser.syncAsByte(_awayMission.tug.transporterRepaired);
			ser.syncAsByte(_awayMission.tug.spockExaminedTransporter);
			ser.syncAsByte(_awayMission.tug.usedTransmogrifierOnTransporter);
			ser.syncAsByte(_awayMission.tug.bridgeForceFieldDown);
			ser.syncAsByte(_awayMission.tug.savedPrisoners);
			ser.syncAsByte(_awayMission.tug.haveBomb);
			ser.syncAsByte(_awayMission.tug.brigElasiPhasersOnKill);
			ser.syncAsByte(_awayMission.tug.field4b);
			ser.syncAsByte(_awayMission.tug.guard1Status);
			ser.syncAsByte(_awayMission.tug.guard2Status);
			ser.syncAsByte(_awayMission.tug.field4e);
			ser.syncBytes(_awayMission.tug.crewmanKilled, 4);
			ser.syncAsByte(_awayMission.tug.bridgeElasi1Status);
			ser.syncAsByte(_awayMission.tug.bridgeElasi2Status);
			ser.syncAsByte(_awayMission.tug.bridgeElasi3Status);
			ser.syncAsByte(_awayMission.tug.bridgeElasi4Status);
			ser.syncAsByte(_awayMission.tug.brigForceFieldDown);
			ser.syncAsByte(_awayMission.tug.field59);
			ser.syncAsByte(_awayMission.tug.field5b);
			ser.syncAsByte(_awayMission.tug.elasiSurrendered);
			ser.syncAsByte(_awayMission.tug.kirkPhaserDrawn);
			ser.syncAsByte(_awayMission.tug.bridgeWinMethod);
			ser.syncAsByte(_awayMission.tug.talkedToBrigCrewman);

			// tug2
			ser.syncAsByte(_room->_roomVar.tug.shootingObject);
			ser.syncAsByte(_room->_roomVar.tug.shootingTarget);
			ser.syncAsByte(_room->_roomVar.tug.elasiPhaserOnKill);
			ser.syncAsByte(_room->_roomVar.tug.shootKirkOverride);
		} else if (_missionName == "LOVE") {
			ser.syncAsByte(_awayMission.love.alreadyStartedMission);
			ser.syncAsByte(_awayMission.love.knowAboutVirus);
			ser.syncAsByte(_awayMission.love.romulansUnconsciousFromLaughingGas);
			ser.syncAsByte(_awayMission.love.releasedHumanLaughingGas);
			ser.syncAsByte(_awayMission.love.releasedRomulanLaughingGas);
			ser.syncAsByte(_awayMission.love.chamberHasCure);
			ser.syncAsByte(_awayMission.love.freezerOpen);
			ser.syncAsByte(_awayMission.love.chamberHasDish);
			ser.syncAsByte(_awayMission.love.bottleInNozzle);
			ser.syncAsByte(_awayMission.love.cabinetOpen);
			ser.syncAsByte(_awayMission.love.gasFeedOn);
			ser.syncAsByte(_awayMission.love.synthesizerBottleIndex);
			ser.syncAsByte(_awayMission.love.synthesizerContents);
			ser.syncAsByte(_awayMission.love.canister1);
			ser.syncAsByte(_awayMission.love.canister2);
			ser.syncAsByte(_awayMission.love.servicePanelOpen);
			ser.syncAsByte(_awayMission.love.gasTankUnscrewed);
			ser.syncAsByte(_awayMission.love.wrenchTaken);
			ser.syncAsByte(_awayMission.love.tookN2TankFromServicePanel);
			ser.syncAsByte(_awayMission.love.field3c);
			ser.syncAsByte(_awayMission.love.grateRemoved);
			ser.syncAsByte(_awayMission.love.insulationOnGround);
			ser.syncAsByte(_awayMission.love.visitedRoomWithRomulans);
			ser.syncAsByte(_awayMission.love.romulansCured);
			ser.syncAsByte(_awayMission.love.romulansUnconsciousFromVirus);
			ser.syncAsByte(_awayMission.love.freedMarcusAndCheever);
			ser.syncAsByte(_awayMission.love.preaxCured);
			ser.syncAsByte(_awayMission.love.spockInfectionCounter);
			ser.syncAsByte(_awayMission.love.spockCured);
			ser.syncAsByte(_awayMission.love.contactedEnterpriseBeforeCure);
			ser.syncAsByte(_awayMission.love.contactedEnterpriseAfterCure);
			ser.syncAsByte(_awayMission.love.spockAccessedConsole);
			ser.syncAsByte(_awayMission.love.mccoyAccessedConsole);
			ser.syncAsByte(_awayMission.love.gotPolyberylcarbonate);
			ser.syncAsByte(_awayMission.love.gotTLDH);
			ser.syncAsByte(_awayMission.love.gotPointsForOpeningGrate);
			ser.syncAsByte(_awayMission.love.gotPointsForGassingRomulans);
			ser.syncAsByte(_awayMission.love.gotCure);
			ser.syncAsByte(_awayMission.love.gotPointsForHydratingPreax);
			ser.syncAsByte(_awayMission.love.gotPointsForHydratingRomulans);
			ser.syncAsSint16LE(_awayMission.love.missionScore);

			// love0
			ser.syncAsByte(_room->_roomVar.love.heardSummaryOfVirus);
			ser.syncAsSint16LE(_room->_roomVar.love.consoleCrewman);
			ser.syncBytes((byte *)_room->_roomVar.love.consoleAnimation, 10);
			ser.syncAsSint32LE(_room->_roomVar.love.consoleSpeaker);
			ser.syncAsSint32LE(_room->_roomVar.love.consoleText);

			// love1
			ser.syncAsSint32LE(_room->_roomVar.love.dyingSpeaker);
			ser.syncAsSint16LE(_room->_roomVar.love.crewmanUsingFreezerRetX);
			ser.syncAsSint16LE(_room->_roomVar.love.crewmanUsingFreezerRetY);
			ser.syncAsSint16LE(_room->_roomVar.love.crewmanUsingDevice);
			ser.syncAsSint16LE(_room->_roomVar.love.itemInNozzle);
			ser.syncBytes((byte *)_room->_roomVar.love.bottleAnimation, 10);

			// love2
			ser.syncAsByte(_room->_roomVar.love.canisterType);
			ser.syncAsByte(_room->_roomVar.love.cb);
			ser.syncAsSint16LE(_room->_roomVar.love.canisterItem);
			ser.syncBytes((byte *)_room->_roomVar.love.canisterAnim, 10);
			ser.syncAsSint16LE(_room->_roomVar.love.chamberObject);
			ser.syncBytes((byte *)_room->_roomVar.love.chamberInputAnim, 10);
			ser.syncBytes((byte *)_room->_roomVar.love.chamberOutputAnim, 10);

			// love3
			ser.syncAsByte(_room->_roomVar.love.activeCrewman);

			// love4
			ser.syncAsByte(_room->_roomVar.love.gaveWaterToRomulans);

			// love5
			ser.syncAsByte(_room->_roomVar.love.numCrewmenReadyToBeamOut);

			// common
			ser.syncAsByte(_room->_roomVar.love.walkingToDoor);
			ser.syncAsByte(_room->_roomVar.love.doorOpenCounter);
			ser.syncAsByte(_room->_roomVar.love.spockAndMccoyReadyToUseCure);
			ser.syncAsByte(_room->_roomVar.love.cmnXPosToCureSpock);
			ser.syncAsByte(_room->_roomVar.love.cmnYPosToCureSpock);
		} else if (_missionName == "MUDD") {
			ser.syncAsByte(_awayMission.mudd.muddFirstRoomState);
			ser.syncAsByte(_awayMission.mudd.torpedoLoaded);
			ser.syncAsByte(_awayMission.mudd.knowAboutTorpedo);
			ser.syncAsByte(_awayMission.mudd.discoveredBase3System);
			ser.syncAsByte(_awayMission.mudd.translatedAlienLanguage);
			ser.syncAsByte(_awayMission.mudd.databaseDestroyed);
			ser.syncAsByte(_awayMission.mudd.muddInDatabaseRoom);
			ser.syncAsByte(_awayMission.mudd.muddCurrentlyInsane);
			ser.syncAsByte(_awayMission.mudd.computerDataErasedOrDestroyed);
			ser.syncAsByte(_awayMission.mudd.muddErasedDatabase);
			ser.syncAsByte(_awayMission.mudd.discoveredLenseAndDegrimerFunction);
			ser.syncAsSint16LE(_awayMission.mudd.torpedoStatus);
			ser.syncAsByte(_awayMission.mudd.muddUnavailable);
			ser.syncAsByte(_awayMission.mudd.muddVisitedDatabaseRoom);
			ser.syncAsByte(_awayMission.mudd.accessedAlienDatabase);
			ser.syncAsByte(_awayMission.mudd.tookRepairTool);
			ser.syncAsByte(_awayMission.mudd.gotPointsForDownloadingData);
			ser.syncAsByte(_awayMission.mudd.contactedEnterpriseFirstTime);
			ser.syncAsByte(_awayMission.mudd.viewScreenEnabled);
			ser.syncAsByte(_awayMission.mudd.lifeSupportMalfunctioning);
			ser.syncAsByte(_awayMission.mudd.numTimesEnteredRoom5);
			ser.syncAsByte(_awayMission.mudd.gotMemoryDisk);
			ser.syncAsByte(_awayMission.mudd.gotLense);
			ser.syncAsByte(_awayMission.mudd.gotDegrimer);
			ser.syncAsByte(_awayMission.mudd.putCapsuleInMedicalMachine);
			ser.syncAsByte(_awayMission.mudd.muddUnconscious);
			ser.syncAsByte(_awayMission.mudd.muddInsanityState);
			ser.syncAsByte(_awayMission.mudd.muddInhaledGas);
			ser.syncAsSint16LE(_awayMission.mudd.lifeSupportTimer);
			ser.syncAsByte(_awayMission.mudd.startedLifeSupportTimer);
			ser.syncAsByte(_awayMission.mudd.enteredRoom0ForFirstTime);
			ser.syncAsByte(_awayMission.mudd.gotPointsForLoadingTorpedo);
			ser.syncAsByte(_awayMission.mudd.gotPointsForPressingRedButton);
			ser.syncAsByte(_awayMission.mudd.gotPointsForEnablingViewscreen);
			ser.syncAsByte(_awayMission.mudd.enteredRoom1ForFirstTime);
			ser.syncAsByte(_awayMission.mudd.repairedLifeSupportGenerator);
			ser.syncAsSint16LE(_awayMission.mudd.missionScore);

			// mudd3
			ser.syncAsByte(_room->_roomVar.mudd.suggestedUsingTricorders);
			ser.syncAsByte(_room->_roomVar.mudd.tricordersUnavailable);

			// mudd4
			ser.syncAsByte(_room->_roomVar.mudd.usingLeftConsole);
			ser.syncAsByte(_room->_roomVar.mudd.kirkUsingRightConsole);

			// common
			ser.syncAsByte(_room->_roomVar.mudd.walkingToDoor);
		} else if (_missionName == "FEATHER") {
			ser.syncAsByte(_awayMission.feather.diedFromStalactites);
			ser.syncAsByte(_awayMission.feather.vineState);
			ser.syncAsByte(_awayMission.feather.gotRock);
			ser.syncAsByte(_awayMission.feather.gotSnake);
			ser.syncAsByte(_awayMission.feather.tookKnife);
			ser.syncAsByte(_awayMission.feather.field2e);
			ser.syncAsByte(_awayMission.feather.numRocksThrownAtTlaoxac);
			ser.syncAsByte(_awayMission.feather.gotFern);
			ser.syncAsByte(_awayMission.feather.holeBlocked);
			ser.syncAsByte(_awayMission.feather.tlaoxacTestPassed);
			ser.syncAsByte(_awayMission.feather.knockedOutTlaoxac);
			ser.syncAsByte(_awayMission.feather.waterMonsterRetreated);
			ser.syncAsByte(_awayMission.feather.showedSnakeToTlaoxac);
			ser.syncAsSint16LE(_awayMission.feather.missionScore);

			// feather1
			ser.syncAsByte(_room->_roomVar.feather.snakeInHole);
			ser.syncAsByte(_room->_roomVar.feather.scannedSnake);
			ser.syncBytes(_room->_roomVar.feather.crewEscaped, 4);
			ser.syncAsByte(_room->_roomVar.feather.crewmanClimbingVine);

			// feather2
			ser.syncAsByte(_room->_roomVar.feather.showedSnakeToTlaoxac);
			ser.syncAsByte(_room->_roomVar.feather.tlaoxacUnconscious);

			// feather6
			ser.syncAsByte(_room->_roomVar.feather.usedRockOnCrystalsOnce);

			// feather7
			ser.syncAsByte(_room->_roomVar.feather.insultedQuetzecoatl);
		} else if (_missionName == "TRIAL") {
			ser.syncAsSint16LE(_awayMission.trial.missionScore);
			ser.syncAsSint16LE(_awayMission.trial.field2b);
			ser.syncAsByte(_awayMission.trial.entityDefeated);
			ser.syncAsByte(_awayMission.trial.doorOpen);
			ser.syncAsByte(_awayMission.trial.scannedLock);
			ser.syncAsByte(_awayMission.trial.doorCodeBehaviour);
			ser.syncAsByte(_awayMission.trial.globSplitInTwo);
			ser.syncAsByte(_awayMission.trial.globDefeated);
			ser.syncBytes(_awayMission.trial.globEnergyLevels, 3);
			ser.syncAsByte(_awayMission.trial.enteredTrial3FirstTime);
			ser.syncAsByte(_awayMission.trial.klingonShootIndex);
			ser.syncAsByte(_awayMission.trial.shotKlingons);
			ser.syncAsSint16LE(_awayMission.trial.shotKlingonState);
			ser.syncAsByte(_awayMission.trial.neuralInterfaceActive);
			for (int i = 0; i < 3; i++)
				ser.syncAsSint16LE(_awayMission.trial.holeContents[i]);
			ser.syncAsByte(_awayMission.trial.enteredGlobRoom);
			ser.syncAsByte(_awayMission.trial.forceFieldDown);
			ser.syncAsByte(_awayMission.trial.uhuraAnalyzedCode);
			ser.syncAsSint16LE(_awayMission.trial.missionEndMethod);
			ser.syncAsByte(_awayMission.trial.gotPointsForGettingRod);
			ser.syncAsByte(_awayMission.trial.gotPointsForCoatingRodWithIron);
			ser.syncAsByte(_awayMission.trial.gotPointsForActivatingInterface);
			ser.syncAsByte(_awayMission.trial.gotPointsForScanningGlob);
			ser.syncAsByte(_awayMission.trial.gotPointsForBeamingOut);

			// trial2
			ser.syncAsByte(_room->_roomVar.trial.globBeingShot);
			ser.syncAsByte(_room->_roomVar.trial.phaserOnKill);

			// trial5
			ser.syncAsSint16LE(_room->_roomVar.trial.itemToUse);
			ser.syncAsSint16LE(_room->_roomVar.trial.objectToUse);
			ser.syncAsSint16LE(_room->_roomVar.trial.hole);
		}

		// The action queue
		if (ser.isLoading()) {
			_actionQueue = Common::Queue<Action>();
			int16 n;
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

}
