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

#ifndef ZVISION_SYNCSOUND_NODE_H
#define ZVISION_SYNCSOUND_NODE_H

#include "audio/mixer.h"
#include "zvision/scripting/scripting_effect.h"
#include "zvision/text/subtitles.h"

namespace Common {
class String;
}

namespace ZVision {
class SyncSoundNode : public ScriptingEffect {
public:
	SyncSoundNode(ZVision *engine, uint32 key, Common::String &file, int32 syncto);
	~SyncSoundNode() override;

	/**
	 * Decrement the timer by the delta time. If the timer is finished, set the status
	 * in _globalState and let this node be deleted
	 *
	 * @param deltaTimeInMillis    The number of milliseconds that have passed since last frame
	 * @return                     If true, the node can be deleted after process() finishes
	 */
	bool process(uint32 deltaTimeInMillis) override;
private:
	int32 _syncto;
	Audio::SoundHandle _handle;
	Subtitle *_sub;
};

} // End of namespace ZVision

#endif
