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
	// TODO Auto-generated destructor stub
}

int32 Speech::play(ResourceId speechResourceId) {
	ResourceId soundResourceId = kResourceNone;

	switch (getWorld()->actorType) {
	case kActorMax: {
		int32 soundResourceId2 = speechResourceId;
		int32 textResourceId = speechResourceId;

		if (speechResourceId >= 259) {
			soundResourceId2 -= 9;
			textResourceId -= 9;
		}

		soundResourceId = (ResourceId)(soundResourceId2 - 0x7FFD0000);
		// setup the right index for sound and text
		setPlayerSpeech(soundResourceId, (ResourceId)(textResourceId - 0x7FFFFFAD));
	}
		break;
	// TODO: check if actor type is right for the following cases
	case kActorSarah: {
		soundResourceId = (ResourceId)(speechResourceId - 0x7FFBF879);
		setPlayerSpeech(soundResourceId, (ResourceId)(speechResourceId - 0x7FFFFDB6));
	}
		break;
	case kActorCyclops: {
		soundResourceId = (ResourceId)(speechResourceId - 0x7FFBF7DC);
		setPlayerSpeech(soundResourceId, (ResourceId)(speechResourceId - 0x7FFFFD19));
	}
		break;
	case kActorAztec: {
		soundResourceId = (ResourceId)(speechResourceId - 0x7FFBF746);
		setPlayerSpeech(soundResourceId, (ResourceId)(speechResourceId - 0x7FFFFC83));
	}
		break;
	default:
		soundResourceId = kResourceNone;
		break;
	}

	return soundResourceId;
}

void Speech::setPlayerSpeech(ResourceId soundResourceId, ResourceId textResourceId) {
	if (soundResourceId) {
		if (getSound()->isPlaying(soundResourceId)) {
			getSound()->stopSound(soundResourceId);
		}
	}

	_soundResourceId = soundResourceId;
	_textResourceId = textResourceId;

	prepareSpeech();
}

void Speech::prepareSpeech() {
	//int32 startTick = _scene->vm()->getTick();

	if (_soundResourceId) {
		if (!getSound()->isPlaying(_soundResourceId)/* || _tick && startTick >= _tick*/) {
			processSpeech();
		}

		if (Config.showEncounterSubtitles) {
			int16 check = 0;
			/*Common::Point *pt;
			_scene->getActorPosition(_scene->getActor(), pt);

			check = pt->y < 240;
			check = pt->y >= 240;*/
			int32 posY = ((check - 1) & 0x118) + 40;

			getText()->loadFont(getWorld()->font3);
			getText()->drawText(20, posY, _textDataPos);

			getText()->loadFont(getWorld()->font1);
			getText()->drawText(20, posY, _textData);
		}
	}
}

void Speech::processSpeech() {
	char * txt;

	_tick = 0;

	txt = getText()->getResText(_textResourceId);

	if (*(txt + strlen((const char *)txt) - 2) == 1) {
		_textResourceId = kResourceNone;
		_textData = 0;
		_textDataPos = 0;
	} else {
		if (*txt == '{') {
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
}

} // end of namespace Asylum
