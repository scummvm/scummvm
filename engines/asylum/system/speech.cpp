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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/system/speech.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/text.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/staticres.h"

namespace Asylum {

Speech::Speech(AsylumEngine *engine): _vm(engine), _textData(nullptr), _textDataPos(nullptr) {
	_tick            = _vm->getTick();
	_soundResourceId = kResourceNone;
	_textResourceId  = kResourceNone;
}

ResourceId Speech::play(ResourceId soundResourceId, ResourceId textResourceId) {
	if (soundResourceId)
		if (getSound()->isPlaying(_soundResourceId))
			getSound()->stopAll(_soundResourceId);

	_soundResourceId = soundResourceId;
	_textResourceId = textResourceId;

	prepareSpeech();

	return soundResourceId;
}

ResourceId Speech::playIndexed(int32 index) {
	int processedIndex;

	if (_vm->checkGameVersion("Demo")) {
		switch (index) {
		default:
		case 1:
			processedIndex = 43 + _vm->getRandom(5);
			break;
		case 3:
			processedIndex = 58 + _vm->getRandom(5);
			break;
		}
	} else if (getWorld()->actorType || index != -1) {
		processedIndex = (int)speechIndex[index + 5 * getWorld()->actorType] + (int)rnd(speechIndexRandom[index + 5 * getWorld()->actorType]);
	} else {
		switch (_vm->getRandom(3)) {
		default:
		case 0:
			processedIndex = 23;
			break;

		case 1:
			processedIndex = 400;
			break;

		case 2:
			processedIndex = 401;
			break;

		case 3:
			processedIndex = index;
			break;
		}

		if (processedIndex >= 259)
			processedIndex -=9;
	}

	switch (getWorld()->actorType) {
	default:
		break;

	case kActorMax:
		if (_vm->checkGameVersion("Demo"))
			return play(MAKE_RESOURCE(kResourcePackSharedSound, processedIndex), MAKE_RESOURCE(kResourcePackText, processedIndex - 1));
		else
			return play(MAKE_RESOURCE(kResourcePackSpeech, processedIndex), MAKE_RESOURCE(kResourcePackText, processedIndex + 83));

	case kActorSarah:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, processedIndex + 1927), MAKE_RESOURCE(kResourcePackText, processedIndex + 586));

	case kActorCyclops:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, processedIndex + 2084), MAKE_RESOURCE(kResourcePackText, processedIndex + 743));

	case kActorAztec:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, processedIndex + 2234), MAKE_RESOURCE(kResourcePackText, processedIndex + 893));
	}

	return kResourceNone;
}

ResourceId Speech::playScene(int32 type, int32 index) {
	switch (type) {
	default:
		play(kResourceNone, kResourceNone);
		break;

	case 0:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2363), MAKE_RESOURCE(kResourcePackText, index + 1022));

	case 1:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2366), MAKE_RESOURCE(kResourcePackText, index + 1025));

	case 2:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2371), MAKE_RESOURCE(kResourcePackText, index + 1030));

	case 3:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2398), MAKE_RESOURCE(kResourcePackText, index + 1057));

	case 4:
		return play(MAKE_RESOURCE(kResourcePackSpeech, index + 503), MAKE_RESOURCE(kResourcePackText, index + 1060));

	case 5:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2401), MAKE_RESOURCE(kResourcePackText, index + 1068));

	case 6:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2409), MAKE_RESOURCE(kResourcePackText, index + 1076));

	case 7:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2415), MAKE_RESOURCE(kResourcePackText, index + 1082));

	case 8:
		return play(MAKE_RESOURCE(kResourcePackSpeech, index + 511), MAKE_RESOURCE(kResourcePackText, index + 1084));

	case 9:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2417), MAKE_RESOURCE(kResourcePackText, index + 1088));

	case 10:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2417), MAKE_RESOURCE(kResourcePackText, index + 1093));

	case 11:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2424), MAKE_RESOURCE(kResourcePackText, index + 1100));

	case 12:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2424), MAKE_RESOURCE(kResourcePackText, index + 1102));

	case 13:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2430), MAKE_RESOURCE(kResourcePackText, index + 1108));

	case 14:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2432), MAKE_RESOURCE(kResourcePackText, index + 1110));

	case 15:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2434), MAKE_RESOURCE(kResourcePackText, index + 1112));

	case 16:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2435), MAKE_RESOURCE(kResourcePackText, index + 1113));

	case 17:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2436), MAKE_RESOURCE(kResourcePackText, index + 1114));

	case 18:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2438), MAKE_RESOURCE(kResourcePackText, index + 1116));

	case 19:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2439), MAKE_RESOURCE(kResourcePackText, index + 1117));
	}

	return kResourceNone;
}

ResourceId Speech::playPlayer(int32 index) {
	switch (getWorld()->actorType) {
	default:
		break;

	case kActorMax: {
		int32 soundResourceIndex = index;
		int32 textResourceIndex = index;

		if (index >= 259) {
			soundResourceIndex -= 9;
			textResourceIndex -= 9;
		}

		ResourceId soundResourceId;
		if (_vm->checkGameVersion("Demo")) {
			soundResourceId = MAKE_RESOURCE(kResourcePackSharedSound, soundResourceIndex + 21 + 7 * _vm->isAltDemo());
			return play(soundResourceId, MAKE_RESOURCE(kResourcePackText, textResourceIndex + 20));
		} else {
			soundResourceId = MAKE_RESOURCE(kResourcePackSpeech, soundResourceIndex);
			return play(soundResourceId, MAKE_RESOURCE(kResourcePackText, textResourceIndex + 83));
		}
		}

	case kActorSarah:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 1927), MAKE_RESOURCE(kResourcePackText, index + 586));

	case kActorCyclops:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2084), MAKE_RESOURCE(kResourcePackText, index + 743));

	case kActorAztec:
		return play(MAKE_RESOURCE(kResourcePackSharedSound, index + 2234), MAKE_RESOURCE(kResourcePackText, index + 893));
	}

	return kResourceNone;
}

void Speech::resetResourceIds() {
	_soundResourceId = kResourceNone;
	_textResourceId = kResourceNone;
}

void Speech::resetTextData() {
	_textData = nullptr;
	_textDataPos = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Private methods
//////////////////////////////////////////////////////////////////////////

void Speech::prepareSpeech() {
	int32 startTick = _vm->getTick();

	if (_soundResourceId) {
		if (!getSound()->isPlaying(_soundResourceId) || (_tick && startTick >= _tick))
			process();

		if (Config.showEncounterSubtitles) {
			Common::Point point;
			Actor *actor = getScene()->getActor();
			actor->adjustCoordinates(&point);

			int16 posY = (point.y >= 240) ? 40 : 320;

			getText()->draw(_textDataPos, getWorld()->font3, posY);
			getText()->draw(_textData, getWorld()->font1, posY);
		}
	}
}


void Speech::process() {
	_tick = 0;

	char *txt = getText()->get(_textResourceId);

	if (*(txt + strlen((const char *)txt) - 2) == 1) {
		_textResourceId = kResourceNone;
		_textData = nullptr;
		_textDataPos = nullptr;
	} else if (*txt == '{') {
		_textData = txt + 3;
		_textDataPos = nullptr;

		getText()->loadFont(getWorld()->font1);
		getSound()->playSound(_soundResourceId, false, Config.voiceVolume, 0);
	} else {
		_textData = nullptr;
		_textDataPos = txt;

		if (*txt == '/') {
			_textDataPos = txt + 2;
		}

		getText()->loadFont(getWorld()->font3);
		getSound()->playSound(_soundResourceId, false, Config.voiceVolume, 0);
	}
}

} // end of namespace Asylum
