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
#include "common/debug.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "math/utils.h"
#include "math/angle.h"
#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/scripting/effects/music_effect.h"
#include "zvision/sound/midi.h"
#include "zvision/sound/volume_manager.h"
#include "zvision/sound/zork_raw.h"


namespace ZVision {

void MusicNodeBASE::setDirection(Math::Angle azimuth, uint8 magnitude) {
	if (_engine->getScriptManager()->getStateValue(StateKey_Qsound) >= 1) {
		_azimuth = azimuth;
		_directionality = magnitude;
		_balance = ((int)(127 * _azimuth.getSine()) * _directionality) / 255;
	} else
		setBalance(0);
	updateMixer();
}

void MusicNodeBASE::setBalance(int8 balance) {
	_balance = balance;
	_azimuth.setDegrees(0);
	_directionality = 255;
	updateMixer();
}

void MusicNodeBASE::updateMixer() {
	if (_engine->getScriptManager()->getStateValue(StateKey_Qsound) >= 1)
		_volumeOut = _engine->getVolumeManager()->convert(_volume, _azimuth, _directionality);  // Apply game-specific volume profile and then attenuate according to azimuth
	else
		_volumeOut = _engine->getVolumeManager()->convert(_volume);  // Apply game-specific volume profile and ignore azimuth
	outputMixer();
}

MusicNode::MusicNode(ZVision *engine, uint32 key, Common::Path &filename, bool loop, uint8 volume)
	: MusicNodeBASE(engine, key, SCRIPTING_EFFECT_AUDIO) {
	_loop = loop;
	_volume = volume;
	_balance = 0;
	_fade = false;
	_fadeStartVol = volume;
	_fadeEndVol = 0;
	_fadeTime = 0;
	_fadeElapsed = 0;
	_sub = 0;
	_stereo = false;
	_loaded = false;

	Audio::RewindableAudioStream *audioStream = NULL;

	if (filename.baseName().contains(".wav")) {
		Common::File *file = new Common::File();
		if (file->open(filename)) {
			audioStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
		}
	} else {
		audioStream = makeRawZorkStream(filename, _engine);
	}

	if (audioStream) {
		_stereo = audioStream->isStereo();

		if (_loop) {
			Audio::LoopingAudioStream *loopingAudioStream = new Audio::LoopingAudioStream(audioStream, 0, DisposeAfterUse::YES);
			_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, loopingAudioStream, -1, _volume);
		} else {
			_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, audioStream, -1, _volume);
		}

		if (_key != StateKey_NotSet) {
			debugC(3, kDebugSound, "setting musicnode state value to 1");
			_engine->getScriptManager()->setStateValue(_key, 1);
		}

		// Change filename.raw into filename.sub
		Common::String subname = filename.baseName();
		subname.setChar('s', subname.size() - 3);
		subname.setChar('u', subname.size() - 2);
		subname.setChar('b', subname.size() - 1);

		Common::Path subpath(filename.getParent().appendComponent(subname));
		if (SearchMan.hasFile(subpath))
			_sub = _engine->getSubtitleManager()->create(subpath, _handle); // NB automatic subtitle!

		_loaded = true;
		updateMixer();
	}
	debugC(3, kDebugSound, "MusicNode: %d created", _key);
}

MusicNode::~MusicNode() {
	if (_loaded)
		_engine->_mixer->stopHandle(_handle);
	if (_key != StateKey_NotSet)
		_engine->getScriptManager()->setStateValue(_key, 2);
	if (_sub)
		_engine->getSubtitleManager()->destroy(_sub);
	debugC(3, kDebugSound, "MusicNode: %d destroyed", _key);
}

void MusicNode::outputMixer() {
	_engine->_mixer->setChannelBalance(_handle, _balance);
	_engine->_mixer->setChannelVolume(_handle, _volumeOut);
}

void MusicNode::setFade(int32 time, uint8 target) {
	_fadeStartVol = _volume;
	_fadeEndVol = target;
	_fadeElapsed = 0;
	_fadeTime = time <= 0 ? 0 : (uint32)time;
	_fade = true;
}

bool MusicNode::process(uint32 deltaTimeInMillis) {
	if (!_loaded || ! _engine->_mixer->isSoundHandleActive(_handle))
		return stop();
	else {
		if (_fade) {
			debugC(3, kDebugSound, "Fading music, endVol %d, startVol %d, current %d, fade time %d, elapsed time %dms", _fadeEndVol, _fadeStartVol, _volume, _fadeTime, _fadeElapsed);
			uint8 _newvol = 0;
			_fadeElapsed += deltaTimeInMillis;
			if ((_fadeTime <= 0) | (_fadeElapsed >= _fadeTime)) {
				_newvol = _fadeEndVol;
				_fade = false;
			} else {
				if (_fadeEndVol > _fadeStartVol)
					_newvol = _fadeStartVol + (_fadeElapsed * (_fadeEndVol - _fadeStartVol)) / _fadeTime;
				else
					_newvol = _fadeStartVol - (_fadeElapsed * (_fadeStartVol - _fadeEndVol)) / _fadeTime;
			}
			if (_volume != _newvol)
				setVolume(_newvol);
		}
	}
	return false;
}

void MusicNode::setVolume(uint8 newVolume) {
	if (_loaded) {
		debugC(4, kDebugSound, "Changing volume of music node %d from %d to %d", _key, _volume, newVolume);
		_volume = newVolume;
		updateMixer();
	}
}


PanTrackNode::PanTrackNode(ZVision *engine, uint32 key, uint32 slot, int16 pos, uint8 mag, bool resetMixerOnDelete, bool staticScreen)
	: ScriptingEffect(engine, key, SCRIPTING_EFFECT_PANTRACK),
	  _slot(slot),
	  _sourcePos(0),
	  _viewPos(0),
	  _mag(mag),
	  _width(0),
	  _pos(pos),
	  _staticScreen(staticScreen),
	  _resetMixerOnDelete(resetMixerOnDelete) {
	debugC(3, kDebugSound, "Created PanTrackNode, key %d, slot %d", _key, _slot);
	process(0);     // Try to set pan value for music node immediately
}

PanTrackNode::~PanTrackNode() {
	debugC(1, kDebugSound, "Deleting PanTrackNode, key %d, slot %d", _key, _slot);
	ScriptManager *scriptManager = _engine->getScriptManager();
	ScriptingEffect *fx = scriptManager->getSideFX(_slot);
	if (fx && fx->getType() == SCRIPTING_EFFECT_AUDIO && _resetMixerOnDelete) {
		debugC(1, kDebugSound, "Resetting mixer, slot %d", _slot);
		MusicNodeBASE *mus = (MusicNodeBASE *)fx;
		mus->setBalance(0);
	} else
		debugC(1, kDebugSound, "NOT resetting mixer, slot %d", _slot);
}

bool PanTrackNode::process(uint32 deltaTimeInMillis) {
	debugC(3, kDebugSound, "Processing PanTrackNode, key %d", _key);
	ScriptManager *scriptManager = _engine->getScriptManager();
	ScriptingEffect *fx = scriptManager->getSideFX(_slot);
	if (fx && fx->getType() == SCRIPTING_EFFECT_AUDIO) {
		MusicNodeBASE *mus = (MusicNodeBASE *)fx;
		if (!_staticScreen)
			// Original game scripted behaviour
			switch (_engine->getRenderManager()->getRenderTable()->getRenderState()) {
			case RenderTable::PANORAMA:
				debugC(3, kDebugSound, "PanTrackNode in panorama mode");
				_width = _engine->getRenderManager()->getBkgSize().x;
				if (_width) {
					_sourcePos.setDegrees(360 * _pos / _width);
					_viewPos.setDegrees(360 * scriptManager->getStateValue(StateKey_ViewPos) / _width);
				} else {
					warning("Encountered zero background width whilst processing PanTrackNode in panoramic mode!");
				}
				break;
			case RenderTable::FLAT:
			case RenderTable::TILT:
			default:
				debugC(3, kDebugSound, "PanTrackNode in FLAT/TILT mode");
				_sourcePos.setDegrees(0);
				_viewPos.setDegrees(0);
				break;
			} else {
			// Used for auxiliary scripts only
			_sourcePos.setDegrees(_pos);
			_viewPos.setDegrees(0);
		}
		Math::Angle azimuth;
		azimuth = _sourcePos - _viewPos;
		debugC(3, kDebugSound, "soundPos: %f, _viewPos: %f, azimuth: %f, width %d", _sourcePos.getDegrees(), _viewPos.getDegrees(), azimuth.getDegrees(), _width);
		// azimuth is sound source position relative to player, clockwise from centre of camera axis to front when viewed top-down
		mus->setDirection(azimuth, _mag);
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
	if (_chan >= 0) {
		_engine->getMidiManager()->noteOff(_chan);
	}
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
	_engine->getMidiManager()->setVolume(_chan, _volumeOut);
}

} // End of namespace ZVision
