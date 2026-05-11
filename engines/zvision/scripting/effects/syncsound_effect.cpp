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

#include "audio/decoders/wave.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/scripting/effects/syncsound_effect.h"
#include "zvision/sound/zork_raw.h"


namespace ZVision {

SyncSoundNode::SyncSoundNode(ZVision *engine, uint32 key, Common::Path &filename, int32 syncto)
	: MusicNode(engine, key, filename, false, Audio::Mixer::kMaxChannelVolume) {
	_syncto = syncto;
}

SyncSoundNode::~SyncSoundNode() {
}

bool SyncSoundNode::process(uint32 deltaTimeInMillis) {
	if (_engine->getScriptManager()->getSideFX(_syncto) == NULL)
		return stop();
	else
		return MusicNode::process(deltaTimeInMillis);
}

} // End of namespace ZVision
