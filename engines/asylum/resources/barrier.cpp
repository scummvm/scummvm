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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/resources/barrier.h"

#include "asylum/views/scene.h"

namespace Asylum {

Barrier::Barrier(Scene *scene) : _scene(scene) {
}

Barrier::~Barrier() {
	// TODO Auto-generated destructor stub
}

void Barrier::destroy() {
	flags &= kBarrierFlagDestroyed;
}

void Barrier::destroyAndRemoveFromQueue() {
	destroy();
	flags|= kBarrierFlag20000;

	_scene->vm()->screen()->deleteGraphicFromQueue(resourceId);
}

int32 Barrier::getRandomId() {
	int32 numRes = 0;
	ResourceId randomResourceIds[5];
	memset(&randomResourceIds, 0, sizeof(randomResourceIds));
	for (int32 i = 0; i < 5; i++) {
		if (field_68C[i]) {
			randomResourceIds[numRes] = field_68C[i];
			numRes++;
		}
	}

	if (numRes > 0)
		return randomResourceIds[rand() % numRes];

	return resourceId;
}

int32 Barrier::checkFlags() {
	return (flags & 1) && (flags & 8 || flags & 0x10000);
}

void Barrier::setNextFrame(int32 targetFlags) {
	int32 newFlag = targetFlags | 1 | flags;
	flags |= targetFlags | 1;

	if (newFlag & 0x10000)
		frameIdx = frameCount - 1;
	else
		frameIdx = 0;
}

void Barrier::updateSoundItems(Sound *snd) {
	for (int32 i = 0; i < 16; i++) {
		SoundItem *item = &soundItems[i];
		if (snd->isPlaying(item->resourceId)) {
			if (item->field_4) {
				snd->stopSound(item->resourceId);
				item->resourceId   = 0;
				item->field_4 = 0;
			}
		}
	}

	// FIXME Barrier::updateSoundItems needs to be followed by a
	// call to Scene::stopSound(barIdx, 0). Since there aren't that
	// many calls to this method, I'm leaving this comment as a reminder
	// until all dependant methods are implemented
}

void Barrier::stopSound() {
	if (_scene->vm()->sound()->isPlaying(soundResourceId))
		_scene->vm()->sound()->stopSound(soundResourceId);
}

} // end of namespace Asylum
