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

#include "lastexpress/sound/sound.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/entry.h"
#include "lastexpress/sound/queue.h"

#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

// Letters & messages
static const char *const messages[24] = {
	"",
	"TXT1001",  // 1
	"TXT1001A", // 2
	"TXT1011",  // 3
	"TXT1012",  // 4
	"TXT1013",  // 5
	"TXT1014",  // 6
	"TXT1020",  // 7
	"TXT1030",  // 8
	"END1009B", // 50
	"END1046",  // 51
	"END1047",  // 52
	"END1112",  // 53
	"END1112A", // 54
	"END1503",  // 55
	"END1505A", // 56
	"END1505B", // 57
	"END1610",  // 58
	"END1612A", // 59
	"END1612C", // 61
	"END1612D", // 62
	"ENDALRM1", // 63
	"ENDALRM2", // 64
	"ENDALRM3"  // 65
};

static const char *const cities[17] = {
	"EPERNAY",
	"CHALONS",
	"BARLEDUC",
	"NANCY",
	"LUNEVILL",
	"AVRICOUR",
	"DEUTSCHA",
	"STRASBOU",
	"BADENOOS",
	"SALZBURG",
	"ATTNANG",
	"WELS",
	"LINZ",
	"VIENNA",
	"POZSONY",
	"GALANTA",
	"POLICE"
};

static const char *const locomotiveSounds[5] = {
	"ZFX1005",
	"ZFX1006",
	"ZFX1007",
	"ZFX1007A",
	"ZFX1007B"
};

static const SoundFlag soundFlags[32] = {
	kVolumeFull,
	kVolume15,
	kVolume14,
	kVolume13,
	kVolume12,
	kVolume11, kVolume11,
	kVolume10, kVolume10,
	kVolume9,  kVolume9,
	kVolume8,  kVolume8,
	kVolume7,  kVolume7, kVolume7,
	kVolume6,  kVolume6, kVolume6,
	kVolume5,  kVolume5, kVolume5, kVolume5,
	kVolume4,  kVolume4, kVolume4, kVolume4,
	kVolume3,  kVolume3, kVolume3, kVolume3, kVolume3
};

SoundManagerOld::SoundManagerOld(LastExpressEngine *engine) : _engine(engine) {
	_ambientSoundDuration = 0;

	_queue = new SoundQueue(engine);

	memset(&_lastWarning, 0, sizeof(_lastWarning));

	_ambientVolumeChangeTimeMS = _ambientVolumeChangeDelayMS = 0;
	_ambientScheduledVolume = kVolumeNone;
}

SoundManagerOld::~SoundManagerOld() {
	SAFE_DELETE(_queue);

	// Zero passed pointers
	_engine = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Sound-related functions
//////////////////////////////////////////////////////////////////////////
void SoundManagerOld::playSound(CharacterIndex entity, Common::String filename, SoundFlag flag, byte activateDelay) {
	if (_queue->isBuffered(entity) && entity && entity < kCharacterClerk)
		_queue->stop(entity);

	SoundFlag currentFlag = (flag == kSoundVolumeEntityDefault) ? getSoundFlag(entity) : (SoundFlag)(flag | kSoundFlagFixedVolume);

	// Add .SND at the end of the filename if needed
	if (!filename.contains('.'))
		filename += ".SND";

	if (!playSoundWithSubtitles(filename, currentFlag, entity, activateDelay))
		if (entity)
			getSavePoints()->push(kCharacterCath, entity, kCharacterActionEndSound);
}

bool SoundManagerOld::playSoundWithSubtitles(Common::String filename, uint32 flag, CharacterIndex entity, unsigned activateDelay) {
	SoundEntry *entry = new SoundEntry(_engine);

	entry->open(filename, (SoundFlag)flag, 30);
	entry->setEntity(entity);

	// BUG: the original game skips adjustVolumeIfNISPlaying() for delayed-activate sounds.
	// (the original code is structured in a slightly different way)
	// Not sure whether it can be actually triggered,
	// most delayed-activate sounds originate from user actions,
	// all user actions are disabled while NIS is playing.
	entry->adjustVolumeIfNISPlaying();

	if (activateDelay) {
		entry->initDelayedActivate(activateDelay);
	} else {
		entry->play();
	}

	// Add entry to sound list
	_queue->addToQueue(entry);

	return (entry->getTag() != kSoundTagNone);
}

bool SoundManagerOld::needToChangeAmbientVolume() {
	return _ambientScheduledVolume && _engine->_system->getMillis() - _ambientVolumeChangeTimeMS >= _ambientVolumeChangeDelayMS;
}

void SoundManagerOld::playSoundEvent(CharacterIndex entity, byte action, byte activateDelay) {
	int values[5];

	if (getEntityData(entity)->car != getEntityData(kCharacterCath)->car)
		return;

	if (getEntities()->isInSalon(entity) != getEntities()->isInSalon(kCharacterCath))
		return;

	int _action = (int)action;
	SoundFlag flag = getSoundFlag(entity);

	switch (action) {
	case 36: {
		uint newVolume = (flag <= kVolumeFull - 7) ? flag + 7 : kVolumeFull;

		if (newVolume > kVolume7) {
			_ambientScheduledVolume = (SoundFlag)newVolume;
			_ambientVolumeChangeTimeMS = _engine->_system->getMillis();
			_ambientVolumeChangeDelayMS = activateDelay * 1000 / 15;
		}
		break;
		}

	case 37:
		_ambientScheduledVolume = kVolume7;
		_ambientVolumeChangeTimeMS = _engine->_system->getMillis();
		_ambientVolumeChangeDelayMS = activateDelay * 1000 / 15;
		break;

	case 150:
	case 156:
	case 162:
	case 168:
	case 188:
	case 198:
		_action += 1 + (int)rnd(5);
		break;

	case 174:
	case 184:
	case 194:
		_action += 1 + (int)rnd(3);
		break;

	case 180:
		_action += 1 + (int)rnd(4);
		break;

	case 246:
		values[0] = 0;
		values[1] = 104;
		values[2] = 105;
		values[3] = 106;
		values[4] = 116;
		_action = values[rnd(5)];
		break;

	case 247:
		values[0] = 11;
		values[1] = 123;
		values[2] = 124;
		_action = values[rnd(3)];
		break;

	case 248:
		values[0] = 0;
		values[1] = 103;
		values[2] = 108;
		values[3] = 109;
		_action = values[rnd(4)];
		break;

	case 249:
		values[0] = 0;
		values[1] = 56;
		values[2] = 112;
		values[3] = 113;
		_action = values[rnd(4)];
		break;

	case 250:
		values[0] = 0;
		values[1] = 107;
		values[2] = 115;
		values[3] = 117;
		_action = values[rnd(4)];
		break;

	case 251:
		values[0] = 0;
		values[1] = 11;
		values[2] = 56;
		values[3] = 113;
		_action = values[rnd(4)];
		break;

	case 252:
		values[0] = 0;
		values[1] = 6;
		values[2] = 109;
		values[3] = 121;
		_action = values[rnd(4)];
		break;

	case 254:
		values[0] = 0;
		values[1] = 104;
		values[2] = 120;
		values[3] = 121;
		_action = values[rnd(4)];
		break;

	case 255:
		values[0] = 0;
		values[1] = 106;
		values[2] = 115;
		_action = values[rnd(3)];
		break;

	default:
		break;
	}

	if (_action && flag)
		playSoundWithSubtitles(Common::String::format("LIB%03d.SND", _action), flag, kCharacterCath, activateDelay);
}

void SoundManagerOld::playSteam(CityIndex index) {
	if (index >= ARRAYSIZE(cities))
		error("[SoundManagerOld::playSteam] Invalid city index (was %d, max %d)", index, ARRAYSIZE(cities));

	_queue->setAmbientToSteam();

	if (!_queue->getEntry(kSoundTagAmbient))
		playSoundWithSubtitles("STEAM.SND", kSoundTypeAmbient | kSoundFlagLooped | kVolume7, kCharacterSteam);

	// Get the new sound entry and show subtitles
	SoundEntry *entry = _queue->getEntry(kSoundTagAmbient);
	if (entry)
		entry->setSubtitles(cities[index]);
}

void SoundManagerOld::playFightSound(byte action, byte a4) {
	int _action = (int)action;
	int values[5];

	switch (action) {
	default:
		break;

	case 174:
	case 184:
	case 194:
		values[0] = action + 1;
		values[1] = action + 2;
		values[2] = action + 3;
		_action = values[rnd(3)];
		break;

	case 180:
		values[0] = action + 1;
		values[1] = action + 2;
		values[2] = action + 3;
		values[3] = action + 4;
		_action = values[rnd(4)];
		break;

	case 150:
	case 156:
	case 162:
	case 168:
	case 188:
	case 198:
		values[0] = action + 1;
		values[1] = action + 2;
		values[2] = action + 3;
		values[3] = action + 4;
		values[4] = action + 5;
		_action = values[rnd(5)];
		break;
	}

	if (_action)
		playSound(kCharacterClerk, Common::String::format("LIB%03d.SND", _action), kVolumeFull, a4);
}

void SoundManagerOld::playDialog(CharacterIndex entity, CharacterIndex entityDialog, SoundFlag flag, byte a4) {
	if (_queue->isBuffered(getDialogName(entityDialog)))
		_queue->stop(getDialogName(entityDialog));

	playSound(entity, getDialogName(entityDialog), flag, a4);
}

void SoundManagerOld::playLocomotiveSound() {
	playSound(kCharacterCath, locomotiveSounds[rnd(5)], (SoundFlag)(rnd(15) + 2));
}

const char *SoundManagerOld::getDialogName(CharacterIndex entity) const {
	if (_queue->isBuffered(kCharacterTableE))
		return nullptr;
	switch (entity) {
	case kCharacterAnna:
		if (HELPERgetEvent(kEventAnnaDialogGoToJerusalem))
			return "XANN12";

		if (HELPERgetEvent(kEventLocomotiveRestartTrain))
			return "XANN11";

		if (HELPERgetEvent(kEventAnnaBaggageTies) || HELPERgetEvent(kEventAnnaBaggageTies2) || HELPERgetEvent(kEventAnnaBaggageTies3) || HELPERgetEvent(kEventAnnaBaggageTies4))
			return "XANN10";

		if (HELPERgetEvent(kEventAnnaTired) || HELPERgetEvent(kEventAnnaTiredKiss))
			return "XANN9";

		if (HELPERgetEvent(kEventAnnaBaggageArgument))
			return "XANN8";

		if (HELPERgetEvent(kEventKronosVisit))
			return "XANN7";

		if (HELPERgetEvent(kEventAbbotIntroduction))
			return "XANN6A";

		if (HELPERgetEvent(kEventVassiliSeizure))
			return "XANN6";

		if (HELPERgetEvent(kEventAugustPresentAnna) || HELPERgetEvent(kEventAugustPresentAnnaFirstIntroduction))
			return "XANN5";

		if (getProgress().field_60)
			return "XANN4";

		if (HELPERgetEvent(kEventAnnaGiveScarf) || HELPERgetEvent(kEventAnnaGiveScarfDiner) || HELPERgetEvent(kEventAnnaGiveScarfSalon)
		 || HELPERgetEvent(kEventAnnaGiveScarfMonogram) || HELPERgetEvent(kEventAnnaGiveScarfDinerMonogram) || HELPERgetEvent(kEventAnnaGiveScarfSalonMonogram))
			return "XANN3";

		if (HELPERgetEvent(kEventDinerMindJoin))
			return "XANN2";

		if (HELPERgetEvent(kEventGotALight) || HELPERgetEvent(kEventGotALightD))
			return "XANN1";

		break;

	case kCharacterAugust:
		if (HELPERgetEvent(kEventAugustTalkCigar))
			return "XAUG6";

		if (HELPERgetEvent(kEventAugustBringBriefcase))
			return "XAUG5";

		// Getting closer to Vienna...
		if (getState()->time > kTime2200500 && !HELPERgetEvent(kEventAugustMerchandise))
			return "XAUG4A";

		if (HELPERgetEvent(kEventAugustMerchandise))
			return "XAUG4";

		if (HELPERgetEvent(kEventDinerAugust) || HELPERgetEvent(kEventDinerAugustAlexeiBackground) || HELPERgetEvent(kEventMeetAugustTylerCompartment)
		 || HELPERgetEvent(kEventMeetAugustTylerCompartmentBed) || HELPERgetEvent(kEventMeetAugustHisCompartment) || HELPERgetEvent(kEventMeetAugustHisCompartmentBed))
			return "XAUG3";

		if (HELPERgetEvent(kEventAugustPresentAnnaFirstIntroduction))
			return "XAUG2";

		if (getProgress().eventMertensAugustWaiting)
			return "XAUG1";

		break;

	case kCharacterTatiana:
		if (HELPERgetEvent(kEventTatianaTylerCompartment))
			return "XTAT6";

		if (HELPERgetEvent(kEventTatianaCompartmentStealEgg))
			return "XTAT5";

		if (HELPERgetEvent(kEventTatianaGivePoem))
			return "XTAT3";

		if (getProgress().field_64)
			return "XTAT1";

		break;

	case kCharacterVassili:
		if (HELPERgetEvent(kEventCathFreePassengers))
			return "XVAS4";

		if (HELPERgetEvent(kEventVassiliCompartmentStealEgg))
			return "XVAS3";

		if (HELPERgetEvent(kEventAbbotIntroduction))
			return "XVAS2";

		if (HELPERgetEvent(kEventVassiliSeizure))
			return "XVAS1A";

		if (getProgress().field_64)
			return "XVAS1";

		break;

	case kCharacterAlexei:
		if (getProgress().field_88)
			return "XALX6";

		if (getProgress().field_8C)
			return "XALX5";

		if (getProgress().field_90)
			return "XALX4A";

		if (getProgress().field_68)
			return "XALX4";

		if (HELPERgetEvent(kEventAlexeiSalonPoem))
			return "XALX3";

		if (HELPERgetEvent(kEventAlexeiSalonVassili))
			return "XALX2";

		if (HELPERgetEvent(kEventAlexeiDiner) || HELPERgetEvent(kEventAlexeiDinerOriginalJacket))
			return "XALX1";

		break;

	case kCharacterAbbot:
		if (HELPERgetEvent(kEventAbbotDrinkDefuse))
			return "XABB4";

		if (HELPERgetEvent(kEventAbbotInvitationDrink) || HELPERgetEvent(kEventDefuseBomb))
			return "XABB3";

		if (HELPERgetEvent(kEventAbbotWrongCompartment) || HELPERgetEvent(kEventAbbotWrongCompartmentBed))
			return "XABB2";

		if (HELPERgetEvent(kEventAbbotIntroduction))
			return "XABB1";

		break;

	case kCharacterMilos:
		if (HELPERgetEvent(kEventLocomotiveMilosDay) || HELPERgetEvent(kEventLocomotiveMilosNight))
			return "XMIL5";

		if (HELPERgetEvent(kEventMilosCompartmentVisitTyler) && (getProgress().chapter == kChapter3 || getProgress().chapter == kChapter4))
			return "XMIL4";

		if (HELPERgetEvent(kEventMilosCorridorThanks) || getProgress().chapter == kChapter5)
			return "XMIL3";

		if (HELPERgetEvent(kEventMilosCompartmentVisitAugust))
			return "XMIL2";

		if (HELPERgetEvent(kEventMilosTylerCompartmentDefeat))
			return "XMIL1";

		break;

	case kCharacterVesna:
		if (getProgress().field_94)
			return "XVES2";

		if (getProgress().field_98)
			return "XVES1";

		break;

	case kCharacterKronos:
		if (HELPERgetEvent(kEventKronosReturnBriefcase))
			return "XKRO6";

		if (HELPERgetEvent(kEventKronosBringEggCeiling) || HELPERgetEvent(kEventKronosBringEgg))
			return "XKRO5";

		if (HELPERgetEvent(kEventKronosConversation) || HELPERgetEvent(kEventKronosConversationFirebird)) {
			ObjectLocation location = getInventory()->get(kItemFirebird)->location;
			if (location != kObjectLocation6 && location != kObjectLocation5 && location != kObjectLocation2 && location != kObjectLocation1)
				return "XKRO4A";
		}

		if (HELPERgetEvent(kEventKronosConversationFirebird))
			return "XKRO4";

		if (HELPERgetEvent(kEventKronosConversation)) {
			if (HELPERgetEvent(kEventMilosCompartmentVisitAugust))
				return "XKRO3";
			else
				return "XKRO2";
		}

		if (getProgress().eventMertensKronosInvitation)
			return "XKRO1";

		break;

	case kCharacterFrancois:
		if (getProgress().field_9C)
			return "XFRA3";

		if (getProgress().field_A0
		 || HELPERgetEvent(kEventFrancoisWhistle) || HELPERgetEvent(kEventFrancoisWhistleD)
		 || HELPERgetEvent(kEventFrancoisWhistleNight) || HELPERgetEvent(kEventFrancoisWhistleNightD))
			return "XFRA2";

		if (getState()->time > kTimeParisEpernay) // Between Paris and Epernay
			return "XFRA1";

		break;

	case kCharacterMadame:
		if (getProgress().field_A4)
			return "XMME4";

		if (getProgress().field_A8)
			return "XMME3";

		if (getProgress().field_A0)
			return "XMME2";

		if (getProgress().field_AC)
			return "XMME1";

		break;

	case kCharacterMonsieur:
		if (getProgress().eventMetBoutarel)
			return "XMRB1";

		break;

	case kCharacterRebecca:
		if (getProgress().field_B4)
			return "XREB1A";

		if (getProgress().field_B8)
			return "XREB1";

		break;

	case kCharacterSophie:
		if (getProgress().field_B0)
			return "XSOP2";

		if (getProgress().field_BC)
			return "XSOP1B";

		if (getProgress().field_B4)
			return "XSOP1A";

		if (getProgress().field_B8)
			return "XSOP1";

		break;

	case kCharacterMahmud:
		if (getProgress().field_C4)
			return "XMAH1";

		break;

	case kCharacterYasmin:
		if (getProgress().eventMetYasmin)
			return "XHAR2";

		break;

	case kCharacterHadija:
		if (getProgress().eventMetHadija)
			return "XHAR1";

		break;

	case kCharacterAlouan:
		if (getProgress().field_DC)
			return "XHAR3";

		break;

	case kCharacterPolice:
		if (getProgress().field_E0)
			return "XHAR4";

		break;

	case kCharacterMaster:
		if (HELPERgetEvent(kEventCathDream) || HELPERgetEvent(kEventCathWakingUp))
			return "XTYL3";

		return "XTYL1";

	default:
		break;
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Letters & Messages
//////////////////////////////////////////////////////////////////////////
void SoundManagerOld::readText(int id) {
	if (_queue->isBuffered(kCharacterTableE))
		return;

	if (id < 0 || (id > 8 && id < 50) || id > 64)
		error("[Sound::readText] Attempting to use invalid id. Valid values [1;8] - [50;64], was %d", id);

	// Get proper message file (names are stored in sequence in the array but id is [1;8] - [50;64])
	const char *text = messages[id <= 8 ? id : id - 41];

	// Check if file is in cache for id [1;8]
	if (id <= 8)
		if (_queue->isBuffered(text))
			_queue->stop(text);

	playSound(kCharacterTableE, text, kVolumeFull);
}

//////////////////////////////////////////////////////////////////////////
// Sound bites
//////////////////////////////////////////////////////////////////////////
void SoundManagerOld::playWarningCompartment(CharacterIndex entity, ObjectIndex compartment) {

#define PLAY_WARNING(index, sound1, sound2, sound3, sound4, sound5, sound6) { \
	if (_lastWarning[index] + 450 >= getState()->timeTicks) { \
		if (rnd(2)) \
			playSound(kCharacterCond1, sound1, kVolumeFull); \
		else \
			playSound(kCharacterCond1, rnd(2) ? sound2 : sound3, kVolumeFull); \
	} else { \
		if (rnd(2)) \
			playSound(kCharacterCond1, sound4, kVolumeFull); \
		else \
			playSound(kCharacterCond1, rnd(2) ? sound5 : sound6, kVolumeFull); \
	} \
	_lastWarning[index] = getState()->timeTicks; \
}

	if (entity != kCharacterCond1 && entity != kCharacterCond2)
		return;

	//////////////////////////////////////////////////////////////////////////
	// Mertens
	if (entity == kCharacterCond1) {

		switch (compartment) {
		default:
			break;

		case kObjectCompartment2:
			PLAY_WARNING(0, "Con1502A", "Con1500B", "Con1500C", "Con1502", "Con1500", "Con1500A");
			break;

		case kObjectCompartment3:
			PLAY_WARNING(1, "Con1501A", "Con1500B", "Con1500C", "Con1501", "Con1500", "Con1500A");
			break;

		case kObjectCompartment4:
			PLAY_WARNING(2, "Con1503", "Con1500B", "Con1500C", "Con1503", "Con1500", "Con1500A");
			break;

		case kObjectCompartment5:
		case kObjectCompartment6:
		case kObjectCompartment7:
		case kObjectCompartment8:
			++_lastWarning[3];

			switch (_lastWarning[3]) {
			default:
				break;

			case 1:
				getSound()->playSound(kCharacterCond1, "Con1503C", kVolumeFull);
				break;

			case 2:
				getSound()->playSound(kCharacterCond1, rnd(2) ? "Con1503E" : "Con1503A", kVolumeFull);
				break;

			case 3:
				getSound()->playSound(kCharacterCond1, rnd(2) ? "Con1503B" : "Con1503D", kVolumeFull);
				_lastWarning[3] = 0;
				break;
			}
		}

		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Coudert
	switch (compartment) {
		default:
			break;

		case kObjectCompartmentA:
			if (_lastWarning[4] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1500" : "Jac1500A", kVolumeFull);
				break;
			}

			getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1508" : "Jac1508A", kVolumeFull);
			break;

		case kObjectCompartmentB:
			if (_lastWarning[5] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1500" : "Jac1500A", kVolumeFull);
				break;
			}

			if (getProgress().field_40 || (getState()->time > kTimeCityLinz && getState()->time < kTime2133000))
				getSound()->playSound(kCharacterCond2, "Jac1507A", kVolumeFull);
			else
				getSound()->playSound(kCharacterCond2, "Jac1507", kVolumeFull);
			break;

		case kObjectCompartmentC:
			if (_lastWarning[6] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1500" : "Jac1500A", kVolumeFull);
				break;
			}

			if (getProgress().chapter < kChapter3)
				getSound()->playSound(kCharacterCond2, "Jac1506", kVolumeFull);
			else
				getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1506A" : "Jac1506B", kVolumeFull);
			break;

		case kObjectCompartmentD:
			if (_lastWarning[7] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1500" : "Jac1500A", kVolumeFull);
				break;
			}

			getSound()->playSound(kCharacterCond2, "Jac1505", kVolumeFull);
			break;

		case kObjectCompartmentE:
			if (_lastWarning[8] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1500" : "Jac1500A", kVolumeFull);
				break;
			}

			if (getProgress().field_40 || (getState()->time > kTime2115000 && getState()->time < kTime2133000)) {
				getSound()->playSound(kCharacterCond2, "Jac1504B", kVolumeFull);
				break;
			}

			if (getEntities()->isInsideCompartment(kCharacterRebecca, kCarRedSleeping, kPosition_4840))
				getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1500" : "Jac1500A", kVolumeFull);
			else
				getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1504" : "Jac1504A", kVolumeFull);
			break;

		case kObjectCompartmentF:
			if (_lastWarning[9] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1500" : "Jac1500A", kVolumeFull);
				break;
			}

			if (getProgress().field_40 || (getState()->time > kTime2083500 && getState()->time < kTime2133000)) {
				getSound()->playSound(kCharacterCond2, "Jac1503B", kVolumeFull);
				break;
			}

			if (rnd(2) || getEntities()->isInsideCompartment(kCharacterAnna, kCarRedSleeping, kPosition_4070))
				getSound()->playSound(kCharacterCond2, "Jac1503A", kVolumeFull);
			else
				getSound()->playSound(kCharacterCond2, "Jac1503", kVolumeFull);
			break;

		case kObjectCompartmentG:
			if (_lastWarning[10] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1500" : "Jac1500A", kVolumeFull);
				break;
			}

			// BUG: the original game got isInsideCompartment() inverted
			// Jac1502A is "the serbian gentleman is not in, monsier",
			// Jac1502 is a generic response,
			// so Coudert only says "Milos is not in" when Milos is actually in.
			if (rnd(2) || getEntities()->isInsideCompartment(kCharacterMilos, kCarRedSleeping, kPosition_3050))
				getSound()->playSound(kCharacterCond2, "Jac1502", kVolumeFull);
			else
				getSound()->playSound(kCharacterCond2, "Jac1502A", kVolumeFull);
			break;

		case kObjectCompartmentH:
			if (_lastWarning[11] + 450 >= getState()->timeTicks) {
				getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1500" : "Jac1500A", kVolumeFull);
				break;
			}

			if (getEntities()->isInsideCompartment(kCharacterIvo, kCarRedSleeping, kPosition_2740))
				getSound()->playSound(kCharacterCond2, rnd(2) ? "Jac1500" : "Jac1500A", kVolumeFull);
			else
				getSound()->playSound(kCharacterCond2, "Jac1501", kVolumeFull);
			break;
	}

	// Update ticks (Compartments A - H are indexes 4 - 11)
	_lastWarning[compartment - 28] = getState()->timeTicks;
}

void SoundManagerOld::excuseMe(CharacterIndex entity, CharacterIndex entity2, SoundFlag flag) {
	if (_queue->isBuffered(entity) && entity != kCharacterCath && entity != kCharacterMaster && entity != kCharacterClerk)
		return;

	if (entity2 == kCharacterFrancois || entity2 == kCharacterMax)
		return;

	if (entity == kCharacterFrancois && getEntityData(kCharacterFrancois)->field_4A3 != 30)
		return;

	if (flag == kVolumeNone)
		flag = getSoundFlag(entity);

	switch (entity) {
	default:
		break;

	case kCharacterAnna:
		playSound(kCharacterCath, "ANN1107A", flag);
		break;

	case kCharacterAugust:
		switch(rnd(4)) {
		default:
			break;

		case 0:
			playSound(kCharacterCath, "AUG1100A", flag);
			break;

		case 1:
			playSound(kCharacterCath, "AUG1100B", flag);
			break;

		case 2:
			playSound(kCharacterCath, "AUG1100C", flag);
			break;

		case 3:
			playSound(kCharacterCath, "AUG1100D", flag);
			break;
		}
		break;

	case kCharacterCond1:
		if (Entities::isFemale(entity2)) {
			playSound(kCharacterCath, (rnd(2) ? "CON1111" : "CON1111A"), flag);
		} else {
			if (entity2 != kCharacterCath || getProgress().jacket != kJacketGreen || !rnd(2)) {
				switch(rnd(3)) {
				default:
					break;

				case 0:
					playSound(kCharacterCath, "CON1110A", flag);
					break;

				case 1:
					playSound(kCharacterCath, "CON1110C", flag);
					break;

				case 2:
					playSound(kCharacterCath, "CON1110", flag);
					break;
				}
			} else {
				if (isNightOld()) {
					playSound(kCharacterCath, (getProgress().field_18 == 2 ? "CON1110F" : "CON1110E"), flag);
				} else {
					playSound(kCharacterCath, "CON1110D", flag);
				}
			}
		}
		break;

	case kCharacterCond2:
		if (Entities::isFemale(entity2)) {
			playSound(kCharacterCath, "JAC1111D", flag);
		} else {
			if (entity2 != kCharacterCath || getProgress().jacket != kJacketGreen || !rnd(2)) {
				switch(rnd(4)) {
				default:
					break;

				case 0:
					playSound(kCharacterCath, "JAC1111", flag);
					break;

				case 1:
					playSound(kCharacterCath, "JAC1111A", flag);
					break;

				case 2:
					playSound(kCharacterCath, "JAC1111B", flag);
					break;

				case 3:
					playSound(kCharacterCath, "JAC1111C", flag);
					break;
				}
			} else {
				playSound(kCharacterCath, "JAC1113B", flag);
			}
		}
		break;

	case kCharacterHeadWait:
		playSound(kCharacterCath, (rnd(2) ? "HED1002" : "HED1002A"), flag);
		break;

	case kCharacterWaiter1:
	case kCharacterWaiter2:
		switch(rnd(3)) {
		default:
			break;

		case 0:
			playSound(kCharacterCath, (entity == kCharacterWaiter1) ? "WAT1002" : "WAT1003", flag);
			break;

		case 1:
			playSound(kCharacterCath, (entity == kCharacterWaiter1) ? "WAT1002A" : "WAT1003A", flag);
			break;

		case 2:
			playSound(kCharacterCath, (entity == kCharacterWaiter1) ? "WAT1002B" : "WAT1003B", flag);
			break;
		}
		break;

	case kCharacterTrainM:
		if (Entities::isFemale(entity2)) {
			playSound(kCharacterCath, (rnd(2) ? "TRA1113A" : "TRA1113B"), flag);
		} else {
			playSound(kCharacterCath, "TRA1112", flag);
		}
		break;

	case kCharacterTatiana:
		playSound(kCharacterCath, (rnd(2) ? "TAT1102A" : "TAT1102B"), flag);
		break;

	case kCharacterAlexei:
		playSound(kCharacterCath, (rnd(2) ? "ALX1099C" : "ALX1099D"), flag);
		break;

	case kCharacterAbbot:
		if (Entities::isFemale(entity2)) {
			playSound(kCharacterCath, "ABB3002C", flag);
		} else {
			switch(rnd(3)) {
			default:
				break;

			case 0:
				playSound(kCharacterCath, "ABB3002", flag);
				break;

			case 1:
				playSound(kCharacterCath, "ABB3002A", flag);
				break;

			case 2:
				playSound(kCharacterCath, "ABB3002B", flag);
				break;
			}
		}
		break;

	case kCharacterVesna:
		switch(rnd(3)) {
		default:
			break;

		case 0:
			playSound(kCharacterCath, "VES1109A", flag);
			break;

		case 1:
			playSound(kCharacterCath, "VES1109B", flag);
			break;

		case 2:
			playSound(kCharacterCath, "VES1109C", flag);
			break;
		}
		break;

	case kCharacterKahina:
		playSound(kCharacterCath, (rnd(2) ? "KAH1001" : "KAH1001A"), flag);
		break;

	case kCharacterFrancois:
	case kCharacterMadame:
		switch(rnd(4)) {
		default:
			break;

		case 0:
			playSound(kCharacterCath, (entity == kCharacterFrancois) ? "FRA1001" : "MME1103A", flag);
			break;

		case 1:
			playSound(kCharacterCath, (entity == kCharacterFrancois) ? "FRA1001A" : "MME1103B", flag);
			break;

		case 2:
			playSound(kCharacterCath, (entity == kCharacterFrancois) ? "FRA1001B" : "MME1103C", flag);
			break;

		case 3:
			playSound(kCharacterCath, (entity == kCharacterFrancois) ? "FRA1001C" : "MME1103D", flag);
			break;
		}
		break;

	case kCharacterMonsieur:
		playSound(kCharacterCath, "MRB1104", flag);
		if (flag > 2)
			getProgress().eventMetBoutarel = true;
		break;

	case kCharacterRebecca:
		playSound(kCharacterCath, (rnd(2) ? "REB1106" : "Reb1106A"), flag);
		break;

	case kCharacterSophie: {
		switch(rnd(3)) {
		default:
			break;

		case 0:
			playSound(kCharacterCath, "SOP1105", flag);
			break;

		case 1:
			playSound(kCharacterCath, Entities::isFemale(entity2) ? "SOP1105C" : "SOP1105A", flag);
			break;

		case 2:
			playSound(kCharacterCath, Entities::isFemale(entity2) ? "SOP1105D" : "SOP1105B", flag);
			break;
		}
		break;
	}

	case kCharacterMahmud:
		playSound(kCharacterCath, "MAH1101", flag);
		break;

	case kCharacterYasmin:
		playSound(kCharacterCath, "HAR1002", flag);
		if (flag > 2)
			getProgress().eventMetYasmin = true;
		break;

	case kCharacterHadija:
		playSound(kCharacterCath, (rnd(2) ? "HAR1001" : "HAR1001A"), flag);
		if (flag > 2)
			getProgress().eventMetHadija = true;
		break;

	case kCharacterAlouan:
		playSound(kCharacterCath, "HAR1004", flag);
		break;
	}
}

void SoundManagerOld::excuseMeCath() {
	switch(rnd(3)) {
	default:
		playSound(kCharacterCath, "CAT1126B");
		break;

	case 1:
		playSound(kCharacterCath, "CAT1126C");
		break;

	case 2:
		playSound(kCharacterCath, "CAT1126D");
		break;
	}
}

const char *SoundManagerOld::justCheckingCath() const {
	switch(rnd(4)) {
	default:
		break;

	case 0:
		return "CAT5001";

	case 1:
		return "CAT5001A";

	case 2:
		return "CAT5001B";

	case 3:
		return "CAT5001C";
	}

	return "CAT5001";
}

const char *SoundManagerOld::wrongDoorCath() const {
	switch(rnd(5)) {
	default:
		break;

	case 0:
		return "CAT1125";

	case 1:
		return "CAT1125A";

	case 2:
		return "CAT1125B";

	case 3:
		return "CAT1125C";

	case 4:
		return "CAT1125D";
	}

	return "CAT1125";
}

const char *SoundManagerOld::justAMinuteCath() const {
	switch(rnd(3)) {
	default:
		break;

	case 0:
		return "CAT1520";

	case 1:
		return "CAT1521";

	case 2:
		return "CAT1125";    // ?? is this a bug in the original?
	}

	return "CAT1520";
}

//////////////////////////////////////////////////////////////////////////
// Sound flags
//////////////////////////////////////////////////////////////////////////
SoundFlag SoundManagerOld::getSoundFlag(CharacterIndex entity) const {
	if (entity == kCharacterCath)
		return kVolumeFull;

	if (getEntityData(entity)->car != getEntityData(kCharacterCath)->car)
		return kVolumeNone;

	// Compute sound value
	SoundFlag ret = kVolume2;

	// Get default value if valid
	int index = ABS(getEntityData(entity)->entityPosition - getEntityData(kCharacterCath)->entityPosition) / 230;
	if (index < 32)
		ret = soundFlags[index];

	if (getEntityData(kCharacterCath)->location == kLocationOutsideTrain) {
		if (getEntityData(kCharacterCath)->car != kCarKronos
		&& !getEntities()->isOutsideAlexeiWindow()
		&& !getEntities()->isOutsideAnnaWindow())
			return kVolumeNone;

		return (SoundFlag)(ret / 6);
	}

	switch (getEntityData(entity)->car) {
	default:
		break;

	case kCarKronos:
		if (getEntities()->isInKronosSalon(entity) != getEntities()->isInKronosSalon(kCharacterCath))
			ret = (SoundFlag)(ret / 2);
		break;

	case kCarGreenSleeping:
	case kCarRedSleeping:
		if (getEntities()->isInGreenCarEntrance(kCharacterCath) && !getEntities()->isInGreenCarEntrance(entity))
			ret = (SoundFlag)(ret / 2);

		if (getEntityData(kCharacterCath)->location == kLocationInsideCompartment
		&& (getEntityData(entity)->location != kLocationInsideCompartment || !getEntities()->isDistanceBetweenEntities(kCharacterCath, entity, 400)))
			ret = (SoundFlag)(ret / 2);
		break;

	case kCarRestaurant:
		if (getEntities()->isInSalon(entity) != getEntities()->isInSalon(kCharacterCath))
			ret = (SoundFlag)(ret / 4);
		else if (getEntities()->isInRestaurant(entity) != getEntities()->isInRestaurant(kCharacterCath))
			ret = (SoundFlag)(ret / 2);
		break;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
void SoundManagerOld::playAmbientSound(int param) {
	SoundEntry *entry = _queue->getEntry(kSoundTagAmbient);

	static const EntityPosition positions[8] = { kPosition_8200, kPosition_7500,
	                                             kPosition_6470, kPosition_5790,
	                                             kPosition_4840, kPosition_4070,
	                                             kPosition_3050, kPosition_2740 };

	byte numLoops[8];
	numLoops[1] = 4;
	numLoops[2] = 2;
	numLoops[3] = 2;
	numLoops[4] = 2;
	numLoops[5] = 2;
	numLoops[6] = 2;

	char tmp[80];
	tmp[0] = 0;

	int partNumber = 1;
	int fnameLen = 6;

	if (_queue->getAmbientState() & kAmbientSoundEnabled && param >= 0x45 && param <= 0x46) {
		if (_queue->getAmbientState() & kAmbientSoundSteam) {
			Common::strcpy_s(tmp, "STEAM.SND");

			_ambientSoundDuration = 32767;
		} else {
			if (getEntityData(kCharacterCath)->location == kLocationOutsideTrain) {
				partNumber = 6;
			} else {
				if (getEntities()->isInsideCompartments(kCharacterCath)) {
					int objNum = (getEntityData(kCharacterCath)->car == kCarGreenSleeping) ? 9 : 40; // Weird numbers

					numLoops[0] = 0;

					for (int pos = 0; pos < 8; pos++) {
						if (numLoops[0])
							break;
						if (getEntities()->isInsideCompartment(kCharacterCath, getEntityData(kCharacterCath)->car, positions[pos])) {
							numLoops[0] = 1;
							partNumber = (getObjects()->get((ObjectIndex)objNum).status == kObjectLocation2) ? 6 : 1;
						}
						objNum++;
					}
				} else {
					switch (getEntityData(kCharacterCath)->car) {
					case kCarBaggageRear:
					case kCarBaggage:
						partNumber = 4;
						break;
					case kCarKronos:
					case kCarGreenSleeping:
					case kCarRedSleeping:
					case kCarRestaurant:
						partNumber = 1;
						break;
					case kCarCoalTender:
						partNumber = 5;
						break;
					case kCarLocomotive:
						partNumber = 99;
						break;
					case kCarVestibule:
						partNumber = 3;
						break;
					default:
						partNumber = 6;
						break;
					}
				}
			}

			if (partNumber != 99)
				Common::sprintf_s(tmp, "LOOP%d%c.SND", partNumber, (char)(rnd(numLoops[partNumber] - 1) + 'A'));
		}

		if (getFlags()->flag_3)
			fnameLen = 5;

		if (!entry || scumm_strnicmp(entry->getName().c_str(), tmp, (uint)fnameLen)) {
			_ambientSoundDuration = rnd(319) + 260;

			if (partNumber != 99) {
				playSoundWithSubtitles(tmp, kSoundTypeAmbient | kSoundFlagLooped | kVolume1, kCharacterSteam);

				if (entry)
					entry->fade();

				SoundEntry *entry1 = _queue->getEntry(kSoundTagAmbient);
				if (entry1)
					entry1->setVolumeSmoothly(kVolume7);
			}
		}
	}
}

SoundManager::SoundManager(LastExpressEngine *engine) {
	_engine = engine;
	_mixer = _engine->_mixer;

	for (int i = 0; i < ARRAYSIZE(_soundSlotChannels); i++) {
		_soundSlotChannels[i] = nullptr;
	}

	memset(_soundChannelsMixBuffers, 0, sizeof(_soundChannelsMixBuffers));

	soundDriverInit();
}

SoundManager::~SoundManager() {
	_engine = nullptr;
}

int SoundManager::playSoundFile(const char *sndName, int typeFlags, int character, int delay) {
	Slot *slot = new Slot(this, sndName, typeFlags, 30);

	slot->setAssociatedCharacter(character);

	if (delay) {
		slot->setDelayedStartTime(getSoundDriver30HzCounter() + 2 * delay);
		slot->addStatusFlag(kSoundFlagDelayedActivate);
	} else {
		Common::String subName = sndName;
		subName.replace('.', '\0');

		slot->setSub(subName.c_str());
		slot->play();
	}

	return slot->getTag();
}

void SoundManager::startAmbient() {
	_soundSlotAmbientFlag |= kAmbientSoundEnabled;
}

void SoundManager::startSteam(int cityIndex) {
	Slot *cachedSlot;
	bool playSoundNeeded = true;

	_soundSlotAmbientFlag |= kAmbientSoundSteam;

	if (_soundCache) {
		cachedSlot = _soundCache;
		do {
			if (cachedSlot->hasTag(kSoundTagAmbient))
				break;

			cachedSlot = cachedSlot->getNext();
		} while (cachedSlot);

		if (cachedSlot)
			playSoundNeeded = false;
	}

	if (playSoundNeeded)
		playSoundFile("STEAM.SND", kSoundTypeAmbient | kSoundFlagLooped | kVolume7, kCharacterSteam, 0);

	cachedSlot = _soundCache;
	if (_soundCache) {
		do {
			if (cachedSlot->hasTag(kSoundTagAmbient))
				break;

			cachedSlot = cachedSlot->getNext();
		} while (cachedSlot);

		if (cachedSlot) {
			cachedSlot = _soundCache;
			if (_soundCache) {
				do {
					if (cachedSlot->hasTag(kSoundTagAmbient))
						break;

					cachedSlot = cachedSlot->getNext();
				} while (cachedSlot);

				if (cachedSlot) {
					cachedSlot->setSub(_cities[cityIndex]);
				}
			}
		}
	}
}

void SoundManager::endAmbient() {
	_soundSlotAmbientFlag = 0;

	Slot *cachedSlot = _soundCache;
	if (_soundCache) {
		do {
			if (cachedSlot->hasTag(kSoundTagAmbient))
				break;

			cachedSlot = cachedSlot->getNext();
		} while (cachedSlot);

		if (cachedSlot)
			cachedSlot->setFade(0);
	}

	cachedSlot = _soundCache;
	if (_soundCache) {
		do {
			if (cachedSlot->hasTag(kSoundTagOldAmbient))
				break;

			cachedSlot = cachedSlot->getNext();
		} while (cachedSlot);

		if (cachedSlot)
			cachedSlot->setFade(0);
	}
}

void SoundManager::killAmbient() {
	_soundSlotAmbientFlag = 0;

	Slot *cachedSlot = _soundCache;
	if (_soundCache) {
		do {
			if (cachedSlot->hasTag(kSoundTagAmbient))
				break;

			cachedSlot = cachedSlot->getNext();
		} while (cachedSlot);

		if (cachedSlot) {
			cachedSlot->addStatusFlag(kSoundFlagCloseRequested);
			cachedSlot->setAssociatedCharacter(0);

			cachedSlot->closeArchive();
		}
	}

	cachedSlot = _soundCache;
	if (_soundCache) {
		do {
			if (cachedSlot->hasTag(kSoundTagOldAmbient))
				break;

			cachedSlot = cachedSlot->getNext();
		} while (cachedSlot);

		if (cachedSlot) {
			cachedSlot->addStatusFlag(kSoundFlagCloseRequested);
			cachedSlot->setAssociatedCharacter(0);

			cachedSlot->closeArchive();
		}
	}
}

void SoundManager::raiseAmbient(int level, int delay) {
	if (level > 7) {
		_soundAmbientFadeLevel = level;
		_soundAmbientFadeTime = getSoundDriver30HzCounter() + 2 * delay;
	}
}

void SoundManager::levelAmbient(int delay) {
	_soundAmbientFadeLevel = 7;
	_soundAmbientFadeTime = getSoundDriver30HzCounter() + 2 * delay;
}

Slot *SoundManager::findSlotWho(int32 character) {
	Slot *result = _soundCache;

	if (!_soundCache)
		return nullptr;

	while (result->getAssociatedCharacter() != character) {
		result = result->getNext();

		if (!result)
			return nullptr;
	}

	return result;
}

Slot *SoundManager::findSlotName(char *name) {
	Common::StackLock lock(*_engine->_soundMutex);
	Slot *result = _soundCache;

	if (!_soundCache)
		return nullptr;

	while (scumm_stricmp(name, result->getName2())) {
		result = result->getNext();

		if (!result)
			return nullptr;
	}

	return result;
}

void SoundManager::ambientAI(int id) {
	Slot *oldAmbientSlot1;
	Slot *oldAmbientSlot2;

	uint fileNameLen = 6;
	int soundId = 1;

	byte numLoops[9] = {0, 4, 2, 2, 2, 2, 2, 0, 0};

	int positions[8] = {
		kPosition_8200, kPosition_7500, kPosition_6470, kPosition_5790,
		kPosition_4840, kPosition_4070, kPosition_3050, kPosition_2740
	};

	char newAmbientSoundName[80];
	memset(newAmbientSoundName, 0, sizeof(newAmbientSoundName));

	for (oldAmbientSlot1 = _soundCache; oldAmbientSlot1; oldAmbientSlot1 = oldAmbientSlot1->getNext()) {
		if (oldAmbientSlot1->hasTag(kSoundTagAmbient))
			break;
	}

	if ((_soundSlotAmbientFlag & kAmbientSoundEnabled) != 0 && (id == 69 || id == 70)) {
		if ((_soundSlotAmbientFlag & kAmbientSoundSteam) != 0) {
			Common::strcpy_s(newAmbientSoundName, "STEAM.SND");
			_loopingSoundDuration = 0x7FFF;
		} else {
			if (getCharacter(kCharacterCath).characterPosition.location == kLocationOutsideTrain) {
				soundId = 6;
			} else if (_engine->getLogicManager()->inComp(kCharacterCath)) {
				numLoops[0] = 0;

				int objNum = (getCharacter(kCharacterCath).characterPosition.car == kCarGreenSleeping) ? 9 : 40;

				for (int pos = 0; pos < ARRAYSIZE(positions); pos++) {
					if (numLoops[0] == 1)
						break;

					if (_engine->getLogicManager()->inComp(kCharacterCath, getCharacter(kCharacterCath).characterPosition.car, positions[pos])) {
						numLoops[0] = 1;
						soundId = _engine->getLogicManager()->_gameObjects[objNum].door == kObjectLocation2 ? 6 : 1;
					}

					objNum++;
				}
			} else {
				switch (getCharacter(kCharacterCath).characterPosition.car) {
				case kCarBaggageRear:
				case kCarBaggage:
					soundId = 4;
					break;
				case kCarKronos:
				case kCarGreenSleeping:
				case kCarRedSleeping:
				case kCarRestaurant:
					soundId = 1;
					break;
				case kCarCoalTender:
					soundId = 5;
					break;
				case kCarLocomotive:
					soundId = 99;
					break;
				case kCarVestibule:
					soundId = 3;
					break;
				default:
					soundId = 6;
					break;
				}
			}

			if (soundId != 99) {
				char soundSection = (char)((rnd(UINT_MAX)) % numLoops[soundId]) + 'A';
				Common::sprintf_s(newAmbientSoundName, "LOOP%d%c.SND", soundId, soundSection);
			}
		}

		if (_scanAnySoundLoopingSection)
			fileNameLen = 5;

		if (!oldAmbientSlot1 || scumm_strnicmp(oldAmbientSlot1->_name2, newAmbientSoundName, fileNameLen)) {
			_loopingSoundDuration = ((rnd(UINT_MAX)) % 320) + 260;

			if (soundId != 99) {
				if (_engine->isDemo()) {
					playSoundFile(newAmbientSoundName, kSoundTypeAmbient | kSoundFlagLooped | kVolume2, kCharacterSteam, 0);
				} else {
					playSoundFile(newAmbientSoundName, kSoundTypeAmbient | kSoundFlagLooped | kVolume1, kCharacterSteam, 0);
				}

				if (oldAmbientSlot1)
					oldAmbientSlot1->setFade(kVolumeNone);

				oldAmbientSlot2 = _soundCache;

				if (_soundCache) {
					do {
						if (oldAmbientSlot2->hasTag(kSoundTagAmbient))
							break;

						oldAmbientSlot2 = oldAmbientSlot2->getNext();
					} while (oldAmbientSlot2);

					if (oldAmbientSlot2)
						oldAmbientSlot2->setFade(kVolume7);
				}
			}
		}
	}
}

void SoundManager::soundThread() {
	int priority;
	int maxPriority = 0;
	bool loopedPlaying = false;
	Slot *ambientSlot1;
	Slot *ambientSlot2;
	Slot *slotToDevirtualize;
	Slot *cachedSlot;
	Slot *next;

	if (!isCopyingDataToSoundDriver()) {
		ambientSlot1 = _soundCache;
		_inSoundThreadFunction++;

		if (_soundCache) {
			do {
				if (ambientSlot1->hasTag(kSoundTagAmbient))
					break;

				ambientSlot1 = ambientSlot1->getNext();
			} while (ambientSlot1);
		}

		if ((_soundSlotAmbientFlag & kAmbientSoundEnabled) != 0) {
			ambientSlot2 = _soundCache;
			if (!_soundCache) {
				loopedPlaying = true;
			} else {
				do {
					if (ambientSlot2->hasTag(kSoundTagAmbient))
						break;

					ambientSlot2 = ambientSlot2->getNext();
				} while (ambientSlot2);

				if (!ambientSlot2 || _scanAnySoundLoopingSection || ambientSlot1 && ambientSlot1->getTime() > _loopingSoundDuration)
					loopedPlaying = true;
			}

			if (loopedPlaying) {
				ambientAI(kAmbientLooping);
			} else if (_soundAmbientFadeTime && getSoundDriver30HzCounter() >= _soundAmbientFadeTime) {
				ambientSlot1->setFade(_soundAmbientFadeLevel);
				_soundAmbientFadeTime = 0;
			}
		}

		slotToDevirtualize = nullptr;
		cachedSlot = _soundCache;
		if (_soundCache) {
			do {
				next = cachedSlot->getNext();

				if ((cachedSlot->getStatusFlags() & kSoundFlagMuteProcessed) != 0) {
					if (cachedSlot->getSoundBuffer())
						cachedSlot->releaseBuffer();

					if (cachedSlot->closeArchive()) {
						cachedSlot->setNumLoadedBytes(3);
					}

					if (_numActiveChannels < 6 && (cachedSlot->getStatusFlags() & kSoundVolumeMask) != 0) {
						priority = cachedSlot->getPriority();

						if (priority + (cachedSlot->getStatusFlags() & kSoundVolumeMask) > maxPriority) {
							slotToDevirtualize = cachedSlot;
							maxPriority = (cachedSlot->getStatusFlags() & kSoundVolumeMask) + priority;
						}
					}
				}

				if (!cachedSlot->update() && (cachedSlot->getStatusFlags() & kSoundFlagKeepAfterFinish) == 0) {
					if (slotToDevirtualize == cachedSlot) {
						maxPriority = 0;
						slotToDevirtualize = nullptr;
					}

					if (cachedSlot) {
						if (cachedSlot == _engine->getNISManager()->getChainedSound()) {
							// The original deleted the cachedSlot and probably set
							// all its values to zero, which might not be the case on
							// modern compilers and might instead trigger an exception
							// on the NIS code...
							_engine->getNISManager()->setChainedSound(nullptr);
						}

						delete cachedSlot;
						cachedSlot = nullptr;
					}
				}

				cachedSlot = next;
			} while (next);
		}

		if (slotToDevirtualize)
			slotToDevirtualize->devirtualize();

		_scanAnySoundLoopingSection = false;
		_inSoundThreadFunction--;
	}
}

void SoundManager::killAllSlots() {
	for (Slot *i = _soundCache; i; i = i->getNext())
		i->addStatusFlag(kSoundFlagCloseRequested);
}

void SoundManager::killAllExcept(int tag1, int tag2, int tag3, int tag4, int tag5, int tag6, int tag7) {
	Common::StackLock lock(*_engine->_soundMutex);

	Slot *slot = _soundCache;

	if (!tag2)
		tag2 = tag1;

	if (!tag3)
		tag3 = tag1;

	if (!tag4)
		tag4 = tag1;

	if (!tag5)
		tag5 = tag1;

	if (!tag6)
		tag6 = tag1;

	if (!tag7)
		tag7 = tag1;

	if (_soundCache) {
		do {
			int tag = slot->getTag();
			if (tag1 != tag && tag2 != tag && tag3 != tag && tag4 != tag && tag5 != tag && tag6 != tag && tag7 != tag) {
				slot->addStatusFlag(kSoundFlagCloseRequested);
				slot->setAssociatedCharacter(0);

				slot->closeArchive();
			}

			slot = slot->getNext();
		} while (slot);
	}
}

void SoundManager::saveSoundInfo(CVCRFile *file) {
	Common::StackLock lock(*_engine->_soundMutex);

	SaveSlot *saveSlot = new SaveSlot();
	int numSounds = 0;

	file->writeRLE(&_soundSlotAmbientFlag, 4, 1);
	file->writeRLE(&_curSoundSlotTag, 4, 1);
	for (Slot *i = _soundCache; i; i = i->_next) {
		if (scumm_stricmp("NISSND?", i->_name2) && (i->_statusFlags & kSoundTypeMask) != kSoundTypeMenu)
			numSounds++;
	}

	file->writeRLE(&numSounds, 4, 1);

	for (Slot *j = _soundCache; j; j = j->_next) {
		if (scumm_stricmp("NISSND?", j->_name2) && (j->_statusFlags & kSoundTypeMask) != kSoundTypeMenu) {
			saveSlot->tag = j->_tag;
			saveSlot->blockCount = j->_blockCount;
			saveSlot->status = j->_statusFlags;
			saveSlot->time = j->_time;
			saveSlot->fadeDelayCounter = j->_fadeDelayCounter;
			saveSlot->unusedVar = j->_unusedVar;
			saveSlot->character = j->_character;
			saveSlot->delayTicks = j->_delayedStartTime - _sound30HzCounter;
			if (saveSlot->delayTicks > 0x8000000)
				saveSlot->delayTicks = 0;

			saveSlot->priority = j->_priority;
			strncpy(saveSlot->name1, j->_name1, sizeof(saveSlot->name1));
			strncpy(saveSlot->name2, j->_name2, sizeof(saveSlot->name2));
			file->writeRLE(saveSlot, sizeof(SaveSlot), 1);
		}
	}

	delete saveSlot;
}

void SoundManager::destroyAllSound() {
	Slot *i;
	Slot *next;
	int32 waitCycles = 0;

	addSoundDriverFlags(kSoundDriverClearBufferRequested);

	// Wait for the driver to clear the mix buffer
	for (i = _soundCache; (getSoundDriverFlags() & kSoundDriverClearBufferProcessed) == 0; waitCycles++) {
		if (waitCycles >= 3000000)
			break;
	}

	addSoundDriverFlags(kSoundDriverClearBufferProcessed);

	if (_soundCache) {
		do {
			next = i->getNext();
			i->setAssociatedCharacter(0);

			if (i->getSoundBuffer())
				i->releaseBuffer();

			if (i) {
				delete i;
				i = nullptr;
			}

			i = next;
		} while (next);
	}

	_engine->getSubtitleManager()->subThread();
}

void SoundManager::loadSoundInfo(CVCRFile *file, bool skipSoundLoading) {
	Common::StackLock lock(*_engine->_soundMutex);

	int numSounds;

	SaveSlot *saveSlot = new SaveSlot();

	if (skipSoundLoading) {
		int skippedValue;
		
		file->readRLE(&skippedValue, 4, 1);
		file->readRLE(&skippedValue, 4, 1);
		file->readRLE(&numSounds, 4, 1);

		for (int j = 0; j < numSounds; j++) {
			file->readRLE(saveSlot, sizeof(SaveSlot), 1);
		}
	} else {
		file->readRLE(&_soundSlotAmbientFlag, 4, 1);
		file->readRLE(&_curSoundSlotTag, 4, 1);
		file->readRLE(&numSounds, 4, 1);

		for (int j = 0; j < numSounds; j++) {
			file->readRLE(saveSlot, sizeof(SaveSlot), 1);
			// This apparently useless instruction automatically adds the saveSlot pointer to the cache
			Slot *tmp = new Slot(this, saveSlot);
			assert(tmp);
		}

		for (Slot *i = _soundCache; i; i = i->_next) {
			if ((i->_statusFlags & kSoundFlagHasLinkAfter) != 0) {
				Slot *cachedSlot = _soundCache;

				if (_soundCache) {
					while (scumm_stricmp(cachedSlot->_name2, i->_name1)) {
						cachedSlot = cachedSlot->_next;

						if (!cachedSlot)
							break;
					}

					if (cachedSlot)
						i->_chainedSound = cachedSlot;
				}
			}
		}

		_soundDriverFlags &= ~(kSoundDriverClearBufferRequested | kSoundDriverClearBufferProcessed);
	}

	delete saveSlot;
}

void SoundManager::addSlot(Slot *entry) {
	Slot *cachedSlot = _soundCache;
	if (_soundCache) {
		if (_soundCache->getNext()) {
			do {
				cachedSlot = cachedSlot->getNext();
			} while (cachedSlot->getNext());
		}

		cachedSlot->setNext(entry);
		_soundCacheCount++;
	} else {
		_soundCacheCount++;
		_soundCache = entry;
	}
}

void SoundManager::removeSlot(Slot *entry) {
	Slot *cachedSlot;
	Slot *next;

	cachedSlot = _soundCache;

	if (_soundCache && entry) {
		if (_soundCache == entry) {
			_soundCache = _soundCache->getNext();
			_soundCacheCount--;
		} else {
			if (_soundCache->getNext() != entry) {
				do {
					next = cachedSlot->getNext();
					if (!next)
						break;

					cachedSlot = cachedSlot->getNext();
				} while (next->getNext() != entry);
			}

			if (cachedSlot->getNext()) {
				cachedSlot->setNext(cachedSlot->getNext()->getNext());
				_soundCacheCount--;
			}
		}
	}
}

void SoundManager::NISFadeOut() {
	for (Slot *i = _soundCache; i; i = i->getNext()) {
		i->assignDirectVolume(i->getStatusFlags() & kSoundVolumeMask);

		if (i->getVolume())
			i->setFade((i->getVolume() >> 1) + 1);
	}

	addSoundDriverFlags(kSoundDriverNISHasRequestedFade);
}

void SoundManager::NISFadeIn() {
	removeSoundDriverFlags(kSoundDriverNISHasRequestedFade);

	for (Slot *i = _soundCache; i; i = i->getNext()) {
		if (i->getVolume()) {
			if (!i->hasTag(kSoundTagNIS) && !i->hasTag(kSoundTagLink))
				i->setFade(i->getVolume());
		}
	}
}

int SoundManager::getMasterVolume() {
	int result = soundDriverGetVolume();

	if (result < 0)
		return 0;

	if (result > 7)
		return 7;

	return result;
}

void SoundManager::setMasterVolume(int volume) {
	int effVolume = volume;

	if (volume < 0) {
		effVolume = 0;
	}

	if (volume > 7)
		effVolume = 7;

	soundDriverSetVolume(effVolume);
}

} // End of namespace LastExpress
