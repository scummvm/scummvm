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

#include "common/scummsys.h"

#include "zvision/scripting/effects/music_effect.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/sound/midi.h"
#include "zvision/sound/zork_raw.h"

#include "zvision/sound/volume_manager.h"

#include "common/stream.h"
#include "common/file.h"
#include "audio/decoders/wave.h"

#include "math/utils.h"
#include "math/angle.h"

namespace ZVision {

void MusicNodeBASE::setAzimuth(Math::Angle azimuth) {
  if(_engine->getScriptManager()->getStateValue(StateKey_Qsound) >= 1) {
    _azimuth = azimuth;
    _balance = (int)(127*_azimuth.getSine());
  }
	updateMixer();
}

void MusicNodeBASE::setBalance(int8 balance) {
  _balance = balance;
  _azimuth.setDegrees(0);
	updateMixer();
}

void MusicNodeBASE::updateMixer() {
  uint16 tmpVol = _volume * fadeGain;
  tmpVol /= 0xFF;
  if(_engine->getScriptManager()->getStateValue(StateKey_Qsound) >= 1)
    volumeOut = _engine->getVolumeManager()->convert(tmpVol, _azimuth);  //Apply volume profile and then attenuate according to azimuth
  else
    volumeOut = _engine->getVolumeManager()->convert(tmpVol, kVolumeLinear);  //Apply linear volume profile and ignore azimuth
  outputMixer();
}

MusicNode::MusicNode(ZVision *engine, uint32 key, Common::Path &filename, bool loop, uint8 volume)
	: MusicNodeBASE(engine, key, SCRIPTING_EFFECT_AUDIO) {
	_loop = loop;
	_volume = volume;
	_balance = 0;
	_crossfade = false;
	_crossfadeTarget = 0;
	_crossfadeTime = 0;
	_sub = 0;
	_stereo = false;
	_loaded = false;

	Audio::RewindableAudioStream *audioStream = NULL;

	if (filename.baseName().contains(".wav")) {
		Common::File *file = new Common::File();
		if (_engine->getSearchManager()->openFile(*file, filename))
			audioStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
	} 
	else
		audioStream = makeRawZorkStream(filename, _engine);
		
	if (audioStream) {
		_stereo = audioStream->isStereo();
		if (_loop) {
			Audio::LoopingAudioStream *loopingAudioStream = new Audio::LoopingAudioStream(audioStream, 0, DisposeAfterUse::YES);
			_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, loopingAudioStream, -1, _volume);
		}
		else
			_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, audioStream, -1, _volume);
		if (_key != StateKey_NotSet) {
		  debug(1,"setting musicnode state value to 1");
			_engine->getScriptManager()->setStateValue(_key, 1);
		}

		// Change filename.raw into filename.sub
		Common::String subname = filename.baseName();
		subname.setChar('s', subname.size() - 3);
		subname.setChar('u', subname.size() - 2);
		subname.setChar('b', subname.size() - 1);

		Common::Path subpath(filename.getParent().appendComponent(subname));
		if (_engine->getSearchManager()->hasFile(subpath))
		  _sub = _engine->getSubtitleManager()->create(subpath);
		_loaded = true;
		updateMixer();
	}
	debug(1, "MusicNode: %d created", _key);
}

MusicNode::~MusicNode() {
	if (_loaded)
		_engine->_mixer->stopHandle(_handle);
	if (_key != StateKey_NotSet) {
		_engine->getScriptManager()->setStateValue(_key, 2);
		  debug(1,"setting musicnode state value to 2");
		}
	if (_sub)
		_engine->getSubtitleManager()->destroy(_sub);
	debug(1, "MusicNode: %d destroyed", _key);
}

void MusicNode::outputMixer() {
	_engine->_mixer->setChannelBalance(_handle, _balance);
	_engine->_mixer->setChannelVolume(_handle, volumeOut);
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
			_engine->getSubtitleManager()->update(_engine->_mixer->getSoundElapsedTime(_handle) / 100, _sub);
	}
	return false;
}

void MusicNode::setVolume(uint8 newVolume) {
	if (_loaded) {
	  _volume = newVolume;
    updateMixer();
  }
}


PanTrackNode::PanTrackNode(ZVision *engine, uint32 key, uint32 slot, int16 pos)
	: ScriptingEffect(engine, key, SCRIPTING_EFFECT_PANTRACK) {
	_slot = slot;
	sourcePos = pos;
	process(0); 	// Try to set pan value for music node immediately
}

PanTrackNode::~PanTrackNode() {
}

bool PanTrackNode::process(uint32 deltaTimeInMillis) {
	ScriptManager * scriptManager = _engine->getScriptManager();
	ScriptingEffect *fx = scriptManager->getSideFX(_slot);
	if (fx && fx->getType() == SCRIPTING_EFFECT_AUDIO) {
		MusicNodeBASE *mus = (MusicNodeBASE *)fx;
		int viewPos = scriptManager->getStateValue(StateKey_ViewPos);
		int16 _width = _engine->getRenderManager()->getBkgSize().x;
//		int16 _halfWidth = _width / 2;
//		int16 _quarterWidth = _width / 4;

		int deltaPos = 0;
		if (viewPos <= sourcePos)
			deltaPos = sourcePos - viewPos;
		else
			deltaPos = sourcePos - viewPos + _width;
		debug(3,"soundPos: %d, viewPos: %d, deltaPos: %d, width: %d", sourcePos, viewPos, deltaPos, _width);
    //deltaPos is sound source position relative to player, clockwise from centre of camera axis to front when viewed top-down
//*/
    //NEW SYSTEM
    mus->setAzimuth(Math::Angle(360*deltaPos/_width));
      
/*/
    //OLD SYSTEM;		
		int balance = 0;
		if (deltaPos > _halfWidth)  //Source to left
			deltaPos -= _width; //Make angle negative relative to datum
		if (deltaPos > _quarterWidth) {
			balance = 1;
			deltaPos = _halfWidth - deltaPos; //Make relative to right centre?
		} else if (deltaPos < -_quarterWidth) {
			balance = -1;
			deltaPos = -_halfWidth - deltaPos;  //Make relative to left centre?
		}

		// Originally it's value -90...90 but we use -127...127 and therefore 360 replaced by 508
		// Left = -127, centre = 0, right = +127
		mus->setBalance( (508 * deltaPos) / _width );
		deltaPos = (360 * deltaPos) / _width;
		int deltaVol = balance;
		debug(1,"Balance %d", balance);

		// This value sets how fast volume goes off than sound source back of you
		// By this value we can hack some "bugs" have place in original game engine like beat sound in ZGI-dc10
		int volumeCorrection = 2;

		if (_engine->getGameId() == GID_GRANDINQUISITOR) {
			if (scriptManager->getCurrentLocation() == "dc10")
				volumeCorrection = 5;
		}
		if (deltaVol != 0)
			deltaVol = (mus->getVolume() * volumeCorrection) * (90 - deltaPos * balance) / 90;
		if (deltaVol > 255)
			deltaVol = 255;
		mus->setDeltaVolume(deltaVol);
//*/
	}
	return false;
}

MusicMidiNode::MusicMidiNode(ZVision *engine, uint32 key, uint8 program, uint8 note, uint8 volume)
	: MusicNodeBASE(engine, key, SCRIPTING_EFFECT_AUDIO) {
	_volume = volume;
	_prog = program;
	_noteNumber = note;
	_pan = 0;
	_chan = _engine->getMidiManager()->getFreeChannel();
	if (_chan >= 0) {
	  updateMixer();
		_engine->getMidiManager()->setProgram(_chan, _prog);
		_engine->getMidiManager()->noteOn(_chan, _noteNumber, _volume);
	}
	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 1);
}

MusicMidiNode::~MusicMidiNode() {
	if (_chan >= 0)
		_engine->getMidiManager()->noteOff(_chan);
	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 2);
}

void MusicMidiNode::setFade(int32 time, uint8 target) {
}

bool MusicMidiNode::process(uint32 deltaTimeInMillis) {
	return false;
}

void MusicMidiNode::setVolume(uint8 newVolume) {
	if (_chan >= 0) {
	  _volume = newVolume;
	  updateMixer();
	}
}

void MusicMidiNode::outputMixer() {
	_engine->getMidiManager()->setBalance(_chan, _balance);
	_engine->getMidiManager()->setPan(_chan, _pan);
  _engine->getMidiManager()->setVolume(_chan, volumeOut);
}

} // End of namespace ZVision
