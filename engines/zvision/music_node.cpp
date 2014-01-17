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

#include "common/scummsys.h"

#include "zvision/music_node.h"

#include "zvision/zvision.h"
#include "zvision/script_manager.h"
#include "zvision/zork_raw.h"

#include "common/stream.h"
#include "common/file.h"
#include "audio/decoders/wave.h"


namespace ZVision {

MusicNode::MusicNode(ZVision *engine, uint32 key, Common::String &filename, bool loop, int8 volume)
	: SideFX(engine, key, SIDEFX_AUDIO) {
	_loop = loop;
	_volume = volume;
	_crossfade = false;
	_crossfade_delta = 0;
	_crossfade_time = 0;
	_attenuate = 0;
	_pantrack = false;
	_pantrack_X = 0;

	Audio::RewindableAudioStream *audioStream;

	if (filename.contains(".wav")) {
		Common::File *file = new Common::File();
		if (file->open(filename)) {
			audioStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
		}
	} else {
		audioStream = makeRawZorkStream(filename, _engine);
	}


	if (_loop) {
		Audio::LoopingAudioStream *loopingAudioStream = new Audio::LoopingAudioStream(audioStream, 0, DisposeAfterUse::YES);
		_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, loopingAudioStream, -1, _volume);
	} else {
		_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, audioStream, -1, _volume);
	}

	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 1);
}

MusicNode::~MusicNode() {
	_engine->_mixer->stopHandle(_handle);
	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 2);
	debug(1, "MusicNode: %d destroyed\n", _key);
}

bool MusicNode::process(uint32 deltaTimeInMillis) {
	if (! _engine->_mixer->isSoundHandleActive(_handle))
		return stop();
	return false;
}

} // End of namespace ZVision
