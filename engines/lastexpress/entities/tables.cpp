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

#include "lastexpress/entities/tables.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Tables::Tables(LastExpressEngine *engine, CharacterIndex id) : Entity(engine, id) {
	_id = id;

	ADD_CALLBACK_FUNCTION(Tables, chapter1);
	ADD_CALLBACK_FUNCTION(Tables, chapter2);
	ADD_CALLBACK_FUNCTION(Tables, chapter3);
	ADD_CALLBACK_FUNCTION(Tables, chapter4);
	ADD_CALLBACK_FUNCTION(Tables, chapter5);
	ADD_CALLBACK_FUNCTION(Tables, draw);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Tables, chapter1)
	if (savepoint.action == kCharacterActionDefault) {
		if (_id == kCharacterTableC)
			getSound()->playSoundWithSubtitles("LOOP8A.SND", kSoundTypeWalla | kSoundFlagLooped | kVolume8, kCharacterTableC);

		setup_draw();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(2, Tables, chapter2)
	if (savepoint.action == kCharacterActionDefault) {
		if (_id == kCharacterTableC)
			getSound()->playSoundWithSubtitles("LOOP8A.SND", kSoundTypeWalla | kSoundFlagLooped | kVolume8, kCharacterTableC);

		setup_draw();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(3, Tables, chapter3)
	if (savepoint.action == kCharacterActionDefault) {
		if (_id == kCharacterTableC)
			getSound()->playSoundWithSubtitles("LOOP8A.SND", kSoundTypeWalla | kSoundFlagLooped | kVolume8, kCharacterTableC);

		setup_draw();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(4, Tables, chapter4)
	if (savepoint.action == kCharacterActionDefault) {
		if (_id == kCharacterTableC)
			getSound()->playSoundWithSubtitles("LOOP8A.SND", kSoundTypeWalla | kSoundFlagLooped | kVolume8, kCharacterTableC);

		setup_draw();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(5, Tables, chapter5)
	if (savepoint.action == kCharacterActionDefault) {
		if (_id == kCharacterTableC && getSoundQueue()->isBuffered(kCharacterTableC))
			getSoundQueue()->fade(kCharacterTableC);

		setup_draw();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Tables, draw)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		// Only applicable to Tables2 character
		if (_id != kCharacterTableC)
			break;

		switch (getProgress().chapter) {
		default:
			break;

		case kChapter1:
			if (getState()->time > kTime1165500 && !params->param1) {
				params->param1 = 1;
				getSoundQueue()->fade(kCharacterTableC);
			}
			break;

		case kChapter3:
			if (getState()->time > kTime2052000 && !params->param2) {
				params->param2 = 1;
				getSoundQueue()->fade(kCharacterTableC);
			}
			break;

		case kChapter4:
			if (getState()->time > kTime2488500 && !params->param3) {
				params->param3 = 1;
				getSoundQueue()->fade(kCharacterTableC);
			}
			break;

		}
		break;

	case kCharacterActionDefault:
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		switch(_id) {
		default:
			break;

		case kCharacterTableA:
			getData()->entityPosition = kPosition_3970;
			getEntities()->drawSequenceLeft(_id, "001P");
			break;

		case kCharacterTableB:
			getData()->entityPosition = kPosition_3970;
			getEntities()->drawSequenceLeft(_id, "005J");
			break;

		case kCharacterTableC:
			getData()->entityPosition = kPosition_4690;
			getEntities()->drawSequenceLeft(_id, "009G");
			break;

		case kCharacterTableD:
			getData()->entityPosition = kPosition_4690;
			getEntities()->drawSequenceLeft(_id, "010M");
			break;

		case kCharacterTableE:
			getData()->entityPosition = kPosition_5420;
			getEntities()->drawSequenceLeft(_id, "014F");
			break;

		case kCharacterTableF:
			getData()->entityPosition = kPosition_5420;
			getEntities()->drawSequenceLeft(_id, "024D");
			break;
		}

		break;

	case kCharacterActionDrawTablesWithChairs:
		if (!strcmp(savepoint.param.charValue, "")) {
			getEntities()->drawSequenceLeft(_id, savepoint.param.charValue);
		} else {
			switch(_id) {
			default:
				break;

			case kCharacterTableA:
				getEntities()->drawSequenceLeft(_id, "001P");
				break;

			case kCharacterTableB:
				getEntities()->drawSequenceLeft(_id, "005J");
				break;

			case kCharacterTableC:
				getEntities()->drawSequenceLeft(_id, "009G");
				break;

			case kCharacterTableD:
				getEntities()->drawSequenceLeft(_id, "010M");
				break;

			case kCharacterTableE:
				getEntities()->drawSequenceLeft(_id, "014F");
				break;

			case kCharacterTableF:
				getEntities()->drawSequenceLeft(_id, "024D");
				break;
			}
		}
		break;

	case kCharacterAction136455232:
		getEntities()->drawSequenceLeft(_id, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
