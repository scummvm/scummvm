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

#include "asylum/resources/reaction.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/sound.h"
#include "asylum/system/speech.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/shared.h"

namespace Asylum {

Reaction::Reaction(AsylumEngine *engine) : _vm(engine) {}

Reaction::~Reaction() {
	// Zero-out passed pointers
	_vm = NULL;
}

void Reaction::run(int32 reactionIndex) {
	//debugC(kDebugLevelSound, "[SPEC] Running special function for chapter %d",getWorld()->numChapter);

	switch (getWorld()->chapter) {
	default:
	case kChapterNone:
		// Do nothing
		break;

	case kChapter1:
		chapter1(reactionIndex);
		break;

	case kChapter2:
		chapter2(reactionIndex);
		break;

	case kChapter3:
		chapter3(reactionIndex);
		break;

	case kChapter4:
		chapter4(reactionIndex);
		break;

	case kChapter5:
		chapter5(reactionIndex);
		break;

	case kChapter6:
		chapter6(reactionIndex);
		break;

	case kChapter7:
		chapter7(reactionIndex);
		break;

	case kChapter8:
		chapter8(reactionIndex);
		break;

	case kChapter9:
		chapter9(reactionIndex);
		break;

	case kChapter10:
		chapter10(reactionIndex);
		break;

	case kChapter11:
		chapter11(reactionIndex);
		break;

	case kChapter12:
	case kChapter13:
		chapter12(reactionIndex);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// Reaction functions
//////////////////////////////////////////////////////////////////////////
void Reaction::chapter1(int32 reactionIndex) {
	switch (reactionIndex) {
	default:
		play(0);
		break;

	case 0:
		play(61);
		break;

	case 1:
		play(69);
		break;

	case 2:
		play(0);
		break;
	}
}

void Reaction::chapter2(int32 reactionIndex) {
	switch (reactionIndex) {
	default:
		play(0);
		break;

	case 0:
		play(107);
		break;

	case 1:
		play(134);
		break;

	case 2:
		play(104);
		break;

	case 3:
		play(113);
		break;

	case 4:
		play(_vm->isGameFlagSet(kGameFlag186) ? 362 : 110);
		break;

	case 5:
		play(112);
		break;

	case 6:
		play(117);
		break;

	case 7:
		play(109);
		break;

	case 8:
		play(108);
		break;

	case 9:
		play(111);
		break;

	case 10:
		play(106);
		break;
	}
}

void Reaction::chapter3(int32 reactionIndex) {
	switch (reactionIndex) {
	default:
		play(reactionIndex);
		break;

	case 0:
		play(170);
		break;

	case 1:
		play(182);
		break;

	case 2:
		play(181);
		break;

	case 3:
		play(172);
		break;

	case 4:
		play(171);
		break;

	case 5:
		play(169);
		break;
	}
}

void Reaction::chapter4(int32 reactionIndex) {
	switch (reactionIndex) {
	default:
		play(0);
		break;

	case 0:
		play(61);
		break;

	case 1:
		play((getScene()->getActor()->getNumberValue01() != 1) ? 65 : 64);
		break;

	case 2:
		play(66);
		break;

	case 3:
		play(67);
		break;

	case 4:
		play(68);
		break;

	case 5:
		play(69);
		break;

	case 6:
		play(70);
		break;

	case 7:
		play(78);
		break;

	case 8:
		play(77);
		break;
	}
}

void Reaction::chapter5(int32 reactionIndex) {
	if (reactionIndex)
		play(0);
	else
		play(197);
}

void Reaction::chapter6(int32 reactionIndex) {
	switch (reactionIndex) {
	default:
		play(0);
		break;

	case 0:
		play(59);
		break;

	case 1:
		play(81);
		break;

	case 2:
		play(60);
		break;

	case 3:
		play(84);
		break;

	case 4:
		play(88);
		break;

	case 5:
		play(54);
		break;

	case 6:
		play(74);
		break;

	case 7:
		play(139);
		break;

	case 8:
		play(97);
		break;

	case 9:
		play(121);
		break;
	}
}

void Reaction::chapter7(int32 reactionIndex) {
	switch (reactionIndex) {
	default:
		play(0);
		break;

	case 0:
		play(239);
		break;

	case 1:
		play(234);
		break;

	case 2:
		play(249);
		break;

	case 3:
		play(259);
		break;

	case 4:
		play(260);
		break;

	case 5:
		play(272);
		break;

	case 6:
		play(237);
		break;

	case 7:
		play(262);
		break;
	}
}

void Reaction::chapter8(int32 reactionIndex) {
	switch (reactionIndex) {
	default:
		play(0);
		break;

	case 0:
		play(58);
		break;

	case 1:
		play(59);
		break;

	case 2:
		play(60);
		break;

	case 3:
		play(111);
		break;

	case 4:
		play(75);
		break;

	case 5:
		play(76);
		break;

	case 6:
		play(77);
		break;

	case 7:
		play(78);
		break;
	}
}

void Reaction::chapter9(int32 reactionIndex) {
	switch (reactionIndex) {
	default:
		play(0);
		break;

	case 0:
		play(284);
		break;

	case 1:
		play(285);
		break;

	case 2:
		play(286);
		break;

	case 3:
		play(329);
		break;

	case 4:
		play(330);
		break;

	case 5:
		play(331);
		break;

	case 6:
		play(332);
		break;

	case 7:
		play(322);
		break;

	case 9:
		play(465);
		break;
	}
}

void Reaction::chapter10(int32 reactionIndex) {
	switch (reactionIndex) {
	default:
		play(0);
		break;

	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		play(91 + reactionIndex);
		break;
	}
}

void Reaction::chapter11(int32 reactionIndex) {
	switch (reactionIndex) {
	default:
		play(0);
		break;

	case 0:
		play(69);
		break;

	case 1:
		play(70);
		break;

	case 2:
		play(78);
		break;
	}
}

void Reaction::chapter12(int32) {
	play(0);
}

//////////////////////////////////////////////////////////////////////////
// Utils
//////////////////////////////////////////////////////////////////////////
void Reaction::play(int32 index) {
	ResourceId resourceId = kResourceNone;

	switch (getWorld()->actorType) {
	default:
		resourceId = (ResourceId)index;
		break;

	case kActorMax:
		resourceId = MAKE_RESOURCE(kResourcePackSpeech, index < 259 ? index : index - 9);
		break;

	case kActorSarah:
		resourceId = MAKE_RESOURCE(kResourcePackSharedSound, index + 1927);
		break;

	case kActorCyclops:
		resourceId = MAKE_RESOURCE(kResourcePackSharedSound, index + 2084);
		break;

	case kActorAztec:
		resourceId = MAKE_RESOURCE(kResourcePackSharedSound, index + 2234);
		break;
	}

	if (getSpeech()->getSoundResourceId() == resourceId || !getSound()->isPlaying(resourceId))
		getSpeech()->playPlayer(index);
}

} // End of namespace Asylum
