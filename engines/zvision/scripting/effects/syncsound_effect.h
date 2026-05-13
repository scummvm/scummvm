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

#ifndef ZVISION_SYNCSOUND_NODE_H
#define ZVISION_SYNCSOUND_NODE_H

#include "audio/mixer.h"
#include "zvision/scripting/effects/music_effect.h"
#include "zvision/scripting/scripting_effect.h"
#include "zvision/text/subtitle_manager.h"

namespace Common {
class String;
}

namespace ZVision {
class SyncSoundNode : public MusicNode {
public:
	SyncSoundNode(ZVision *engine, uint32 key, Common::Path &file, int32 syncto);
	~SyncSoundNode();

	bool process(uint32 deltaTimeInMillis) override;
private:
	int32 _syncto;
};

} // End of namespace ZVision

#endif
