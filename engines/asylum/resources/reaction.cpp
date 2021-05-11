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

#include "asylum/resources/reaction.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/speech.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

static const int32 reactions[12][11] = {
	{ 61,  69,   0,   0,   0,   0,   0,   0,   0,   0,   0},
	{107, 134, 104, 113,  -1, 112, 117, 109, 108, 111, 106},
	{170, 182, 181, 172, 171, 169,   0,   0,   0,   0,   0},
	{ 61,  -1,  66,  67,  68,  69,  70,  78,  77,   0,   0},
	{197,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
	{ 59,  81,  60,  84,  88,  54,  74, 139,  97, 121,   0},
	{239, 234, 249, 259, 260, 272, 237, 262,   0,   0,   0},
	{ 58,  59,  60, 111,  75,  76,  77,  78,   0,   0,   0},
	{284, 285, 286, 329, 330, 331, 332, 322,   0, 465,   0},
	{ -1,  -1,  -1,  -1,  -1,   0,   0,   0,   0,   0,   0},
	{ 69,  70,  78,   0,   0,   0,   0,   0,   0,   0,   0},
	{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}
};

Reaction::Reaction(AsylumEngine *engine) : _vm(engine) {}

void Reaction::run(uint32 reactionIndex) {
	if (getWorld()->chapter == kChapterNone || reactionIndex > 10)
		return;

	if (getWorld()->chapter == kChapter2 && reactionIndex == 4)
		play(_vm->isGameFlagSet(kGameFlag186) ? 362 : 110);
	else if (getWorld()->chapter == kChapter4 && reactionIndex == 1)
		play((getScene()->getActor()->getNumberValue01() != 1) ? 65 : 64);
	else if (getWorld()->chapter == kChapter10 && reactionIndex < 5)
		play(91 + reactionIndex);
	else
		play(reactions[getWorld()->chapter - 1][reactionIndex]);
}

void Reaction::play(uint32 index) {
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

	if (getSpeech()->getSoundResourceId() != resourceId || !getSound()->isPlaying(resourceId))
		getSpeech()->playPlayer(index);
}

} // End of namespace Asylum
