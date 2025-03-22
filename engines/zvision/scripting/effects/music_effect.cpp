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
  if(_engine->getScriptManager()->getStateValue(StateKey_Qsound) >= 1)
    volumeOut = _engine->getVolumeManager()->convert(_volume, _azimuth);  //Apply volume profile and then attenuate according to azimuth
  else
    volumeOut = _engine->getVolumeManager()->convert(_volume, kVolumeLinear);  //Apply linear volume profile and ignore azimuth
  outputMixer();
}

MusicNode::MusicNode(ZVision *engine, uint32 key, Common::Path &filename, bool loop, uint8 volume)
	: MusicNodeBASE(engine, key, SCRIPTING_EFFECT_AUDIO) {
	_loop = loop;
	_volume = volume;
	_balance = 0;
	_fade = false;
  fadeStartVol = volume;
	fadeEndVol = 0;
	fadeTime = 0;
	fadeElapsed = 0;
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
		  debug(3,"setting musicnode state value to 1");
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
	debug(3, "MusicNode: %d created", _key);
}

MusicNode::~MusicNode() {
	if (_loaded)
		_engine->_mixer->stopHandle(_handle);
	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 2);
	if (_sub)
		_engine->getSubtitleManager()->destroy(_sub);
	debug(3, "MusicNode: %d destroyed", _key);
}

void MusicNode::outputMixer() {
	_engine->_mixer->setChannelBalance(_handle, _balance);
	_engine->_mixer->setChannelVolume(_handle, volumeOut);
}

void MusicNode::setFade(int32 time, uint8 target) {
  fadeStartVol = _volume;
	fadeEndVol = target;
	fadeElapsed = 0;
	fadeTime = time <= 0 ? 0 : (uint32)time;
	_fade = true;
}

bool MusicNode::process(uint32 deltaTimeInMillis) {
	if (!_loaded || ! _engine->_mixer->isSoundHandleActive(_handle))
		return stop();
	else {
		if (_fade) {
			debug(3,"Fading music, endVol %d, startVol %d, current %d, fade time %d, elapsed time %dms", fadeEndVol, fadeStartVol, _volume, fadeTime, fadeElapsed);
		  uint8 _newvol = 0;
		  fadeElapsed += deltaTimeInMillis;
		  if( (fadeTime <= 0) | (fadeElapsed >= fadeTime) ) {
        _newvol = fadeEndVol;
		    _fade = false;
		  }
		  else {
		    if(fadeEndVol > fadeStartVol)
		      _newvol = fadeStartVol + (fadeElapsed*(fadeEndVol - fadeStartVol))/fadeTime;
	      else
  		    _newvol = fadeStartVol - (fadeElapsed*(fadeStartVol - fadeEndVol))/fadeTime;
	    }
		  if (_volume != _newvol)
			  setVolume(_newvol);
		}
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


PanTrackNode::PanTrackNode(ZVision *engine, uint32 key, uint32 slot, int16 pos, uint8 mag, bool resetMixerOnDelete)
	: ScriptingEffect(engine, key, SCRIPTING_EFFECT_PANTRACK),
	_slot(slot),
	sourcePos(pos),
	_mag(mag),
	_resetMixerOnDelete(resetMixerOnDelete) {
//	_slot = slot;
//	sourcePos = pos;
//	_mag = mag;
  debug(3,"Created PanTrackNode, key %d, slot %d", _key, _slot);
	process(0); 	// Try to set pan value for music node immediately
}

PanTrackNode::~PanTrackNode() {
  debug(1,"Deleting PanTrackNode, key %d, slot %d", _key, _slot);
	ScriptManager * scriptManager = _engine->getScriptManager();
	ScriptingEffect *fx = scriptManager->getSideFX(_slot);
  if (fx && fx->getType() == SCRIPTING_EFFECT_AUDIO && _resetMixerOnDelete) {
    debug(1,"Resetting mixer, slot %d", _slot);
	  MusicNodeBASE *mus = (MusicNodeBASE *)fx;
    mus->setBalance(0);
  }
  else
    debug(1,"NOT resetting mixer, slot %d", _slot);
}

bool PanTrackNode::process(uint32 deltaTimeInMillis) {
  debug(3,"Processing PanTrackNode, key %d", _key);
	ScriptManager * scriptManager = _engine->getScriptManager();
	ScriptingEffect *fx = scriptManager->getSideFX(_slot);
	if (fx && fx->getType() == SCRIPTING_EFFECT_AUDIO) {
		MusicNodeBASE *mus = (MusicNodeBASE *)fx;
		int viewPos = scriptManager->getStateValue(StateKey_ViewPos);
		int16 _width = _engine->getRenderManager()->getBkgSize().x;
		int deltaPos = 0;
		if (viewPos <= sourcePos)
			deltaPos = sourcePos - viewPos;
		else
			deltaPos = sourcePos - viewPos + _width;
		debug(3,"soundPos: %d, viewPos: %d, deltaPos: %d, width: %d", sourcePos, viewPos, deltaPos, _width);
    //deltaPos is sound source position relative to player, clockwise from centre of camera axis to front when viewed top-down
    mus->setAzimuth(Math::Angle(360*deltaPos/_width));
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
