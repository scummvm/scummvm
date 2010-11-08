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
 * $URL$
 * $Id$
 *
 */

#include "asylum/system/speech.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/config.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

Speech::Speech(AsylumEngine *engine): _vm(engine) {
	_tick            = _vm->getTick();
	_soundResourceId = kResourceNone;
	_textResourceId  = kResourceNone;
}

Speech::~Speech() {
}

ResourceId Speech::play(ResourceId soundResourceId, ResourceId textResourceId) {
	if (soundResourceId)
		if (getSound()->isPlaying(soundResourceId))
			getSound()->stopSound(soundResourceId);

	_soundResourceId = soundResourceId;
	_textResourceId = textResourceId;

	prepareSpeech();

	return soundResourceId;
}

ResourceId Speech::playIndexed(int32 index) {
	int processedIndex = 0;

	if (getWorld()->actorType || index != -1) {

	} else {
		switch(_vm->getRandom(3)) {
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
		return play(MAKE_RESOURCE(kResourcePackSpeech, index), MAKE_RESOURCE(kResourcePackText, index + 83));

	case kActorSarah:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 1927), MAKE_RESOURCE(kResourcePackText, index + 586));

	case kActorCyclops:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2084), MAKE_RESOURCE(kResourcePackText, index + 743));

	case kActorAztec:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2234), MAKE_RESOURCE(kResourcePackText, index + 893));
	}

	return kResourceNone;
}

ResourceId Speech::playScene(int32 type, int32 index) {
	switch (type) {
	default:
		play(kResourceNone, kResourceNone);
		break;

	case 0:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2363), MAKE_RESOURCE(kResourcePackText, index + 1022));

	case 1:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2366), MAKE_RESOURCE(kResourcePackText, index + 1025));

	case 2:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2371), MAKE_RESOURCE(kResourcePackText, index + 1030));

	case 3:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2398), MAKE_RESOURCE(kResourcePackText, index + 1057));

	case 4:
		return play(MAKE_RESOURCE(kResourcePackSpeech, index + 503), MAKE_RESOURCE(kResourcePackText, index + 1060));

	case 5:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2401), MAKE_RESOURCE(kResourcePackText, index + 1068));

	case 6:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2409), MAKE_RESOURCE(kResourcePackText, index + 1076));

	case 7:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2415), MAKE_RESOURCE(kResourcePackText, index + 1082));

	case 8:
		return play(MAKE_RESOURCE(kResourcePackSpeech, index + 511), MAKE_RESOURCE(kResourcePackText, index + 1084));

	case 9:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2417), MAKE_RESOURCE(kResourcePackText, index + 1088));

	case 10:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2417), MAKE_RESOURCE(kResourcePackText, index + 1093));

	case 11:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2424), MAKE_RESOURCE(kResourcePackText, index + 1100));

	case 12:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2424), MAKE_RESOURCE(kResourcePackText, index + 1102));

	case 13:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2430), MAKE_RESOURCE(kResourcePackText, index + 1108));

	case 14:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2432), MAKE_RESOURCE(kResourcePackText, index + 1110));

	case 15:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2434), MAKE_RESOURCE(kResourcePackText, index + 1112));

	case 16:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2435), MAKE_RESOURCE(kResourcePackText, index + 1113));

	case 17:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2436), MAKE_RESOURCE(kResourcePackText, index + 1114));

	case 18:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2438), MAKE_RESOURCE(kResourcePackText, index + 1116));

	case 19:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2439), MAKE_RESOURCE(kResourcePackText, index + 1117));
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

		ResourceId soundResourceId = MAKE_RESOURCE(kResourcePackSpeech, soundResourceIndex);

		return play(soundResourceId, MAKE_RESOURCE(kResourcePackText, textResourceIndex + 83));
		}
		break;

	case kActorSarah:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 1927), MAKE_RESOURCE(kResourcePackText, index + 586));

	case kActorCyclops:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2084), MAKE_RESOURCE(kResourcePackText, index + 743));

	case kActorAztec:
		return play(MAKE_RESOURCE(kResourcePackSpeechOther, index + 2234), MAKE_RESOURCE(kResourcePackText, index + 893));
	}

	return kResourceNone;
}

void Speech::resetResourceIds() {
	_soundResourceId = kResourceNone;
	_textResourceId = kResourceNone;
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

			getScene()->adjustCoordinates(actor->x1, actor->y1, &point);

			int32 posY = (((point.y >= 240) - 1) & 280) + 40;

			getText()->loadFont(getWorld()->font3);
			getText()->draw(20, posY, _textDataPos);

			getText()->loadFont(getWorld()->font1);
			getText()->draw(20, posY, _textData);
		}
	}
}


void Speech::process() {
	_tick = 0;

	char *txt = getText()->get(_textResourceId);

	if (*(txt + strlen((const char *)txt) - 2) == 1) {
		_textResourceId = kResourceNone;
		_textData = 0;
		_textDataPos = 0;
	} else if (*txt == '{') {
		_textData = txt + 3;
		_textDataPos = 0;

		getText()->loadFont(getWorld()->font1);
		getSound()->playSpeech(_soundResourceId);
	} else {
		_textData = 0;
		_textDataPos = txt;

		if (*txt == '/') {
			_textDataPos = txt + 2;
		}

		getText()->loadFont(getWorld()->font3);
		getSound()->playSpeech(_soundResourceId);
	}
}

} // end of namespace Asylum
