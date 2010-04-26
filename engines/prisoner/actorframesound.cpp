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

#include "prisoner/prisoner.h"

namespace Prisoner {

/* ActorFrameSounds */

void PrisonerEngine::updateActorFrameSounds() {
	// TODO
}

int16 PrisonerEngine::addActorFrameSound(int16 actorIndex, int16 soundIndex, int16 volume, int16 frameNum, int16 unk1) {

	int16 actorFrameSoundIndex = _actorFrameSounds.getFreeSlot();
	ActorFrameSound *actorFrameSound = &_actorFrameSounds[actorFrameSoundIndex];

	actorFrameSound->actorIndex = actorIndex;
	actorFrameSound->soundIndex = soundIndex;
	actorFrameSound->frameNum = frameNum;
	actorFrameSound->unk1 = unk1;
	actorFrameSound->volume = volume;
	actorFrameSound->unk2 = 0;

	_actorFrameSoundItemsCount++;

	return actorFrameSoundIndex;
}

void PrisonerEngine::removeActorFrameSound(int16 actorFrameSoundIndex) {
	_actorFrameSounds[actorFrameSoundIndex].actorIndex = -1;
	_actorFrameSoundItemsCount--;
}

void PrisonerEngine::clearActorFrameSoundsBySoundIndex(int16 soundIndex) {
	for (int16 actorFrameSoundIndex = 0; actorFrameSoundIndex < kMaxActorFrameSounds; actorFrameSoundIndex++) {
		ActorFrameSound *actorFrameSound = &_actorFrameSounds[actorFrameSoundIndex];
		if (actorFrameSound->soundIndex == soundIndex) {
			actorFrameSound->actorIndex = -1;
		}
	}
}

void PrisonerEngine::clearActorFrameSoundsByActorIndex(int16 actorIndex) {
	for (int16 actorFrameSoundIndex = 0; actorFrameSoundIndex < kMaxActorFrameSounds; actorFrameSoundIndex++) {
		ActorFrameSound *actorFrameSound = &_actorFrameSounds[actorFrameSoundIndex];
		if (actorFrameSound->actorIndex == actorIndex) {
			actorFrameSound->actorIndex = -1;
		}
	}
}

void PrisonerEngine::setActorFrameSound(int16 actorFrameSoundIndex, int16 soundIndex, int16 volume) {
	ActorFrameSound *actorFrameSound = &_actorFrameSounds[actorFrameSoundIndex];
	if (soundIndex != -1)
		actorFrameSound->soundIndex = soundIndex;
	if (volume != -1)
		actorFrameSound->volume = volume;
}

void PrisonerEngine::clearActorFrameSounds() {
	_actorFrameSoundItemsCount = 0;
	for (int16 actorFrameSoundIndex = 0; actorFrameSoundIndex < kMaxActorFrameSounds; actorFrameSoundIndex++) {
		_actorFrameSounds[actorFrameSoundIndex].actorIndex = -1;
	}
}

} // End of namespace Prisoner
