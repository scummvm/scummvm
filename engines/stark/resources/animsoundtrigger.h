/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARK_RESOURCES_ANIM_SOUND_TRIGGER_H
#define STARK_RESOURCES_ANIM_SOUND_TRIGGER_H

#include "common/str.h"

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class AnimSkeleton;

/**
 * An AnimSoundTrigger plays a sound when a certain time of an animation is reached
 *
 * The sound is played at most once per animation loop.
 */
class AnimSoundTrigger : public Object {
public:
	static const Type::ResourceType TYPE = Type::kAnimSoundTrigger;

	enum SubType {
		kAnimTriggerSound = 1
	};

	AnimSoundTrigger(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~AnimSoundTrigger();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void printData() override;
	void onAllLoaded() override;
	void onGameLoop() override;

private:
	uint32 _soundStockType;
	uint32 _soundTriggerTime;

	AnimSkeleton *_anim;
	bool _alreadyPlayed;
	uint _timeRemainingBeforeLoop;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_ANIM_SOUND_TRIGGER_H
