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

#include "common/scummsys.h"

#include "zvision/scripting/effects/music_effect.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/sound/midi.h"
#include "zvision/sound/zork_raw.h"

#include "common/stream.h"
#include "common/file.h"
#include "audio/decoders/wave.h"

namespace ZVision {

// FIXME: This is wrong, and not what the original is doing.
// Using actual linear volume values fixes bug #7176.
/*static const uint8 dbMapLinear[256] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,
8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 13, 13, 14,
14, 15, 15, 16, 16, 17, 18, 18, 19, 20, 21, 21, 22, 23, 24, 25,
26, 27, 28, 29, 30, 31, 32, 33, 34, 36, 37, 38, 40, 41, 43, 45,
46, 48, 50, 52, 53, 55, 57, 60, 62, 64, 67, 69, 72, 74, 77, 80,
83, 86, 89, 92, 96, 99, 103, 107, 111, 115, 119, 123, 128, 133, 137, 143,
148, 153, 159, 165, 171, 177, 184, 191, 198, 205, 212, 220, 228, 237, 245, 255};*/

MusicNode::MusicNode(ZVision *engine, uint32 key, Common::String &filename, bool loop, uint8 volume)
	: MusicNodeBASE(engine, key, SCRIPTING_EFFECT_AUDIO) {
	_loop = loop;
	_volume = volume;
	_deltaVolume = 0;
	_balance = 0;
	_crossfade = false;
	_crossfadeTarget = 0;
	_crossfadeTime = 0;
	_sub = NULL;
	_stereo = false;
	_loaded = false;

	Audio::RewindableAudioStream *audioStream = NULL;

	if (filename.contains(".wav")) {
		Common::File *file = new Common::File();
		if (_engine->getSearchManager()->openFile(*file, filename)) {
			audioStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
		}
	} else {
		audioStream = makeRawZorkStream(filename, _engine);
	}

	if (audioStream) {
		_stereo = audioStream->isStereo();

		if (_loop) {
			Audio::LoopingAudioStream *loopingAudioStream = new Audio::LoopingAudioStream(audioStream, 0, DisposeAfterUse::YES);
			_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, loopingAudioStream, -1, _volume /*dbMapLinear[_volume]*/);
		} else {
			_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, audioStream, -1, _volume /*dbMapLinear[_volume]*/);
		}

		if (_key != StateKey_NotSet)
			_engine->getScriptManager()->setStateValue(_key, 1);

		// Change filename.raw into filename.sub
		Common::String subname = filename;
		subname.setChar('s', subname.size() - 3);
		subname.setChar('u', subname.size() - 2);
		subname.setChar('b', subname.size() - 1);

		if (_engine->getSearchManager()->hasFile(subname))
			_sub = new Subtitle(_engine, subname);

		_loaded = true;
	}
}

MusicNode::~MusicNode() {
	if (_loaded)
		_engine->_mixer->stopHandle(_handle);
	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 2);
	if (_sub)
		delete _sub;
	debug(1, "MusicNode: %d destroyed", _key);
}

void MusicNode::setDeltaVolume(uint8 volume) {
	_deltaVolume = volume;
	setVolume(_volume);
}

void MusicNode::setBalance(int8 balance) {
	_balance = balance;
	_engine->_mixer->setChannelBalance(_handle, _balance);
}

void MusicNode::setFade(int32 time, uint8 target) {
	_crossfadeTarget = target;
	_crossfadeTime = time;
	_crossfade = true;
}

bool MusicNode::process(uint32 deltaTimeInMillis) {
	if (!_loaded || ! _engine->_mixer->isSoundHandleActive(_handle))
		return stop();
	else {
		uint8 _newvol = _volume;

		if (_crossfade) {
			if (_crossfadeTime > 0) {
				if ((int32)deltaTimeInMillis > _crossfadeTime)
					deltaTimeInMillis = _crossfadeTime;
				_newvol += (int)(floor(((float)(_crossfadeTarget - _newvol) / (float)_crossfadeTime)) * (float)deltaTimeInMillis);
				_crossfadeTime -= deltaTimeInMillis;
			} else {
				_crossfade = false;
				_newvol = _crossfadeTarget;
			}
		}

		if (_volume != _newvol)
			setVolume(_newvol);

		if (_sub && _engine->getScriptManager()->getStateValue(StateKey_Subtitles) == 1)
			_sub->process(_engine->_mixer->getSoundElapsedTime(_handle) / 100);
	}
	return false;
}

void MusicNode::setVolume(uint8 newVolume) {
	if (!_loaded)
		return;

	_volume = newVolume;

	if (_deltaVolume >= _volume)
		_engine->_mixer->setChannelVolume(_handle, 0);
	else
		_engine->_mixer->setChannelVolume(_handle, _volume - _deltaVolume /*dbMapLinear[_volume - _deltaVolume]*/);
}

uint8 MusicNode::getVolume() {
	return _volume;
}

PanTrackNode::PanTrackNode(ZVision *engine, uint32 key, uint32 slot, int16 pos)
	: ScriptingEffect(engine, key, SCRIPTING_EFFECT_PANTRACK) {
	_slot = slot;
	_position = pos;

	// Try to set pan value for music node immediately
	process(0);
}

PanTrackNode::~PanTrackNode() {
}

bool PanTrackNode::process(uint32 deltaTimeInMillis) {
	ScriptManager * scriptManager = _engine->getScriptManager();
	ScriptingEffect *fx = scriptManager->getSideFX(_slot);
	if (fx && fx->getType() == SCRIPTING_EFFECT_AUDIO) {
		MusicNodeBASE *mus = (MusicNodeBASE *)fx;

		int curPos = scriptManager->getStateValue(StateKey_ViewPos);
		int16 _width = _engine->getRenderManager()->getBkgSize().x;
		int16 _halfWidth = _width / 2;
		int16 _quarterWidth = _width / 4;

		int tmp = 0;
		if (curPos <= _position)
			tmp = _position - curPos;
		else
			tmp = _position - curPos + _width;

		int balance = 0;

		if (tmp > _halfWidth)
			tmp -= _width;

		if (tmp > _quarterWidth) {
			balance = 1;
			tmp = _halfWidth - tmp;
		} else if (tmp < -_quarterWidth) {
			balance = -1;
			tmp = -_halfWidth - tmp;
		}

		// Originally it's value -90...90 but we use -127...127 and therefore 360 replaced by 508
		mus->setBalance( (508 * tmp) / _width );

		tmp = (360 * tmp) / _width;

		int deltaVol = balance;

		// This value sets how fast volume goes off than sound source back of you
		// By this value we can hack some "bugs" have place in originall game engine like beat sound in ZGI-dc10
		int volumeCorrection = 2;

		if (_engine->getGameId() == GID_GRANDINQUISITOR) {
			if (scriptManager->getCurrentLocation() == "dc10")
				volumeCorrection = 5;
		}

		if (deltaVol != 0)
			deltaVol = (mus->getVolume() * volumeCorrection) * (90 - tmp * balance) / 90;
		if (deltaVol > 255)
			deltaVol = 255;

		mus->setDeltaVolume(deltaVol);
	}
	return false;
}

MusicMidiNode::MusicMidiNode(ZVision *engine, uint32 key, int8 program, int8 note, int8 volume)
	: MusicNodeBASE(engine, key, SCRIPTING_EFFECT_AUDIO) {
	_volume = volume;
	_prog = program;
	_noteNumber = note;
	_pan = 0;

	_chan = _engine->getMidiManager()->getFreeChannel();

	if (_chan >= 0) {
		_engine->getMidiManager()->setVolume(_chan, _volume);
		_engine->getMidiManager()->setPan(_chan, _pan);
		_engine->getMidiManager()->setProgram(_chan, _prog);
		_engine->getMidiManager()->noteOn(_chan, _noteNumber, _volume);
	}

	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 1);
}

MusicMidiNode::~MusicMidiNode() {
	if (_chan >= 0) {
		_engine->getMidiManager()->noteOff(_chan);
	}
	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 2);
}

void MusicMidiNode::setDeltaVolume(uint8 volume) {
}

void MusicMidiNode::setBalance(int8 balance) {
}

void MusicMidiNode::setFade(int32 time, uint8 target) {
}

bool MusicMidiNode::process(uint32 deltaTimeInMillis) {
	return false;
}

void MusicMidiNode::setVolume(uint8 newVolume) {
	if (_chan >= 0) {
		_engine->getMidiManager()->setVolume(_chan, newVolume);
	}
	_volume = newVolume;
}

uint8 MusicMidiNode::getVolume() {
	return _volume;
}

} // End of namespace ZVision
