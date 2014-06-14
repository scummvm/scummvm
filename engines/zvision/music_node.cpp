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
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/sound/zork_raw.h"

#include "common/stream.h"
#include "common/file.h"
#include "audio/decoders/wave.h"


namespace ZVision {

MusicNode::MusicNode(ZVision *engine, uint32 key, Common::String &filename, bool loop, int8 volume)
	: SideFX(engine, key, SIDEFX_AUDIO) {
	_loop = loop;
	_volume = volume;
	_crossfade = false;
	_crossfade_target = 0;
	_crossfade_time = 0;
	_attenuate = 0;
	_pantrack = false;
	_pantrack_X = 0;
	_sub = NULL;

	Audio::RewindableAudioStream *audioStream;

	if (filename.contains(".wav")) {
		Common::File *file = new Common::File();
		if (_engine->getSearchManager()->openFile(*file, filename)) {
			audioStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
		}
	} else {
		audioStream = makeRawZorkStream(filename, _engine);
	}

	_stereo = audioStream->isStereo();

	if (_loop) {
		Audio::LoopingAudioStream *loopingAudioStream = new Audio::LoopingAudioStream(audioStream, 0, DisposeAfterUse::YES);
		_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, loopingAudioStream, -1, _volume);
	} else {
		_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, audioStream, -1, _volume);
	}

	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 1);

	Common::String subname = filename;
	subname.setChar('s', subname.size() - 3);
	subname.setChar('u', subname.size() - 2);
	subname.setChar('b', subname.size() - 1);

	if (_engine->getSearchManager()->hasFile(subname))
		_sub = new Subtitle(_engine, subname);
}

MusicNode::~MusicNode() {
	_engine->_mixer->stopHandle(_handle);
	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 2);
	if (_sub)
		delete _sub;
	debug(1, "MusicNode: %d destroyed\n", _key);
}

void MusicNode::setPanTrack(int16 pos) {
	if (!_stereo) {
		_pantrack = true;
		_pantrack_X = pos;
		setVolume(_volume);
	}
}

void MusicNode::unsetPanTrack() {
	_pantrack = false;
	setVolume(_volume);
}

void MusicNode::setFade(int32 time, uint8 target) {
	_crossfade_target = target;
	_crossfade_time = time;
	_crossfade = true;
}

bool MusicNode::process(uint32 deltaTimeInMillis) {
	if (! _engine->_mixer->isSoundHandleActive(_handle))
		return stop();
	else {
		uint8 _newvol = _volume;

		if (_crossfade) {
			if (_crossfade_time > 0) {
				if ((int32)deltaTimeInMillis > _crossfade_time)
					deltaTimeInMillis = _crossfade_time;
				_newvol += floor(((float)(_crossfade_target - _newvol) / (float)_crossfade_time)) * (float)deltaTimeInMillis;
				_crossfade_time -= deltaTimeInMillis;
			} else {
				_crossfade = false;
				_newvol = _crossfade_target;
			}
		}

		if (_pantrack || _volume != _newvol)
			setVolume(_newvol);

		if (_sub)
			_sub->process(_engine->_mixer->getSoundElapsedTime(_handle) / 100);
	}
	return false;
}

void MusicNode::setVolume(uint8 new_volume) {
	if (_pantrack) {
		int cur_x = _engine->getScriptManager()->getStateValue(StateKey_ViewPos);
		cur_x -= _pantrack_X;
		int32 _width = _engine->getRenderManager()->getBkgSize().x;
		if (cur_x < (-_width) / 2)
			cur_x += _width;
		else if (cur_x >= _width / 2)
			cur_x -= _width;

		float norm = (float)cur_x / ((float)_width / 2.0);
		float lvl = fabs(norm);
		if (lvl > 0.5)
			lvl = (lvl - 0.5) * 1.7;
		else
			lvl = 1.0;

		float bal = sin(-norm * 3.1415926) * 127.0;

		if (_engine->_mixer->isSoundHandleActive(_handle)) {
			_engine->_mixer->setChannelBalance(_handle, bal);
			_engine->_mixer->setChannelVolume(_handle, new_volume * lvl);
		}
	} else {
		if (_engine->_mixer->isSoundHandleActive(_handle)) {
			_engine->_mixer->setChannelBalance(_handle, 0);
			_engine->_mixer->setChannelVolume(_handle, new_volume);
		}
	}

	_volume = new_volume;
}

PanTrackNode::PanTrackNode(ZVision *engine, uint32 key, uint32 slot, int16 pos)
	: SideFX(engine, key, SIDEFX_PANTRACK) {
	_slot = slot;

	SideFX *fx = _engine->getScriptManager()->getSideFX(slot);
	if (fx && fx->getType() == SIDEFX_AUDIO) {
		MusicNode *mus = (MusicNode *)fx;
		mus->setPanTrack(pos);
	}
}

PanTrackNode::~PanTrackNode() {
	SideFX *fx = _engine->getScriptManager()->getSideFX(_slot);
	if (fx && fx->getType() == SIDEFX_AUDIO) {
		MusicNode *mus = (MusicNode *)fx;
		mus->unsetPanTrack();
	}
}

} // End of namespace ZVision
