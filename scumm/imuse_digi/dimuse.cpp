/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "common/timer.h"

#include "scumm/actor.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Scumm {

IMuseDigital::Track::Track()
	: soundId(-1), used(false), stream(NULL) {
}

void IMuseDigital::timer_handler(void *refCon) {
	IMuseDigital *imuseDigital = (IMuseDigital *)refCon;
	imuseDigital->callback();
}

IMuseDigital::IMuseDigital(ScummEngine *scumm)
	: _vm(scumm) {
	_mutex = g_system->create_mutex();
	_pause = false;
	_sound = new ImuseDigiSndMgr(_vm);
	_vm->_timer->installTimerProc(timer_handler, 1000000 / 25, this);

	_curMusicState = 0;
	_curMusicSeq = 0;
	_curMusicCue = 0;
	memset(_attributesSeq, 0, sizeof(_attributesSeq));
	memset(_attributesState, 0, sizeof(_attributesState));
	memset(_attributesTable, 0, sizeof(_attributesTable));
	_curSeqAtribPos = 0;
}

IMuseDigital::~IMuseDigital() {
	stopAllSounds(true);
	_vm->_timer->removeTimerProc(timer_handler);
	delete _sound;
	g_system->delete_mutex(_mutex);
}

void IMuseDigital::callback() {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::callback()");
	int l = 0;

	if (_pause || !_vm)
		return;

	for (l = 0; l < MAX_DIGITAL_TRACKS;l ++) {
		if (_track[l].used) {
			if (_track[l].waitForEndSeq) {
				if ((_curMusicState != 0) && (_curMusicSeq == 0))
					_track[l].waitForEndSeq = false;
				else
					continue;
			}
			if (_track[l].stream2) {
				if (!_track[l].handle.isActive() && _track[l].started) {
					debug(5, "IMuseDigital::callback(): stoped sound: %d", _track[l].soundId);
					delete _track[l].stream2;
					_track[l].stream2 = NULL;
					_track[l].used = false;
					continue;
				}
			} else if (_track[l].stream) {
				if ((_track[l].toBeRemoved)) {
					debug(5, "IMuseDigital::callback(): stoped sound: %d", _track[l].soundId);
					_track[l].stream->finish();
					_track[l].stream = NULL;
					_sound->closeSound(_track[l].soundHandle);
					_curMusicSeq = 0;
					_track[l].used = false;
					continue;
				}
			}

			if (_track[l].volFadeUsed) {
				if (_track[l].volFadeStep < 0) {
					if (_track[l].vol > _track[l].volFadeDest) {
						_track[l].vol += _track[l].volFadeStep;
						if (_track[l].vol < _track[l].volFadeDest) {
							_track[l].vol = _track[l].volFadeDest;
							_track[l].volFadeUsed = false;
						}
						if (_track[l].vol == 0) {
							_track[l].toBeRemoved = true;
						}
					}
				} else if (_track[l].volFadeStep > 0) {
					if (_track[l].vol < _track[l].volFadeDest) {
						_track[l].vol += _track[l].volFadeStep;
						if (_track[l].vol > _track[l].volFadeDest) {
							_track[l].vol = _track[l].volFadeDest;
							_track[l].volFadeUsed = false;
						}
					}
				}
				debug(5, "Fade: sound(%d), Vol(%d)", _track[l].soundId, _track[l].vol / 1000);
			}

			int pan = (_track[l].pan != 64) ? 2 * _track[l].pan - 127 : 0;

			if (_vm->_mixer->isReady()) {
				if (_track[l].stream2) {
					if (!_track[l].started) {
						_track[l].started = true;
						_vm->_mixer->playInputStream(&_track[l].handle, _track[l].stream2, true, _track[l].vol / 1000, _track[l].pan, -1, false);
					} else {
						_vm->_mixer->setChannelVolume(_track[l].handle, _track[l].vol / 1000);
						_vm->_mixer->setChannelBalance(_track[l].handle, pan);
					}
					continue;
				}
			}

			if (_track[l].stream) {
				int32 mixer_size = _track[l].pullSize;
				byte *data = NULL;
				int32 result = 0;

				if (_track[l].stream->endOfData()) {
					mixer_size *= 2;
				}

				if (_track[l].curRegion == -1)
					switchToNextRegion(l);

				int bits = _sound->getBits(_track[l].soundHandle);
				do {
					if (bits == 12) {
						byte *ptr = NULL;

						mixer_size += _track[l].mod;
						int mixer_size_12 = (mixer_size * 3) / 4;
						int length = (mixer_size_12 / 3) * 4;
						_track[l].mod = mixer_size - length;

						int32 offset = (_track[l].regionOffset * 3) / 4;
						int result2 = _sound->getDataFromRegion(_track[l].soundHandle, _track[l].curRegion, &ptr, offset, mixer_size_12);
						result = BundleCodecs::decode12BitsSample(ptr, &data, result2);

						free(ptr);
					} else if (bits == 16) {
						result = _sound->getDataFromRegion(_track[l].soundHandle, _track[l].curRegion, &data, _track[l].regionOffset, mixer_size);
						if (_sound->getChannels(_track[l].soundHandle) == 1) {
							result &= ~1;
						}
						if (_sound->getChannels(_track[l].soundHandle) == 2) {
							if (result & 2)
								result &= ~2;
						}
					} else if (bits == 8) {
						result = _sound->getDataFromRegion(_track[l].soundHandle, _track[l].curRegion, &data, _track[l].regionOffset, mixer_size);
						if (_sound->getChannels(_track[l].soundHandle) == 2) {
							result &= ~1;
						}
					}

					if (result > mixer_size)
						result = mixer_size;

					if (_vm->_mixer->isReady()) {
						_vm->_mixer->setChannelVolume(_track[l].handle, _track[l].vol / 1000);
						_vm->_mixer->setChannelBalance(_track[l].handle, pan);
						_track[l].stream->append(data, result);
						_track[l].regionOffset += result;
						_track[l].trackOffset += result;
						free(data);
					}
					
					if (_sound->isEndOfRegion(_track[l].soundHandle, _track[l].curRegion)) {
						switchToNextRegion(l);
						if (_track[l].toBeRemoved)
							break;
					}
					mixer_size -= result;
					assert(mixer_size >= 0);
				} while (mixer_size != 0);
			}
		}
	}
}

void IMuseDigital::switchToNextRegion(int track) {
	int num_regions = _sound->getNumRegions(_track[track].soundHandle);
	int num_jumps = _sound->getNumJumps(_track[track].soundHandle);
	if (_vm->_gameId == GID_FT) {
		if (_track[track].curRegion == -1) {
			_track[track].curRegion = 0;
			_track[track].regionOffset = 0;
			return;
		}
		if (num_jumps != 0)
			_track[track].regionOffset = 0;
		else
			_track[track].toBeRemoved = true;
		return;
	}

	if (++_track[track].curRegion == num_regions) {
		_track[track].toBeRemoved = true;
		return;
	}

	int jumpId = _sound->getJumpIdByRegionAndHookId(_track[track].soundHandle, _track[track].curRegion, _track[track].curHookId);
	if (jumpId != -1) {
		int region = _sound->getRegionIdByJumpId(_track[track].soundHandle, jumpId);
		if (region != -1) {
			_track[track].curRegion = region;
			debug(5, "switchToNextRegion-sound(%d) jump to %d region, curHookId: %d", _track[track].soundId, _track[track].curRegion, _track[track].curHookId);
			_track[track].curHookId = 0;
		}
	}

	debug(5, "switchToNextRegion-sound(%d) select %d region, curHookId: %d", _track[track].soundId, _track[track].curRegion, _track[track].curHookId);
	_track[track].dataOffset = _sound->getRegionOffset(_track[track].soundHandle, _track[track].curRegion);
	_track[track].regionOffset = 0;
}

void IMuseDigital::startSound(int soundId, const char *soundName, int soundType, int soundGroup, AudioStream *input, bool sequence, int hookId, int volume, bool wait) {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::startSound()");
	debug(5, "IMuseDigital::startSound(%d)", soundId);
	int l;

	for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if (!_track[l].used && !_track[l].handle.isActive()) {
			_track[l].pan = 64;
			_track[l].vol = volume * 1000;
			_track[l].volFadeDest = 0;
			_track[l].volFadeStep = 0;
			_track[l].volFadeDelay = 0;
			_track[l].volFadeUsed = false;
			_track[l].soundId = soundId;
			_track[l].started = false;
			_track[l].soundGroup = soundGroup;
			_track[l].curHookId = hookId;
			_track[l].curRegion = -1;
			_track[l].dataOffset = 0;
			_track[l].regionOffset = 0;
			_track[l].trackOffset = 0;
			_track[l].mod = 0;
			_track[l].toBeRemoved = false;
			_track[l].sequence = sequence;
			_track[l].waitForEndSeq = wait;

			int bits = 0, freq = 0, channels = 0, mixerFlags = 0;

			if (input) {
				_track[l].iteration = 1; // ?
				// Do nothing here, we already have an audio stream
			} else {
				_track[l].soundHandle = _sound->openSound(soundId, soundName, soundType, soundGroup);

				if (_track[l].soundHandle == NULL)
					return;

				bits = _sound->getBits(_track[l].soundHandle);
				channels = _sound->getChannels(_track[l].soundHandle);
				freq = _sound->getFreq(_track[l].soundHandle);

				if ((soundId == kTalkSoundID) && (soundType == IMUSE_BUNDLE)) {
					if (_vm->_actorToPrintStrFor != 0xFF && _vm->_actorToPrintStrFor != 0) {
						Actor *a = _vm->derefActor(_vm->_actorToPrintStrFor, "IMuseDigital::startSound");
						freq = (freq * a->talkFrequency) / 256;
						_track[l].pan = a->talkPan;
						_track[l].vol = a->talkVolume * 1000;
					}
				}

				assert(bits == 8 || bits == 12 || bits == 16);
				assert(channels == 1 || channels == 2);
				assert(0 < freq && freq <= 65535);

				// Round the frequency to a multiple of 25. This is done to 
				// ensure we don't run into data under-/overflows (this is a
				// design limitation of the current IMuseDigital code, which
				// pushes data 'blindly' into the mixer, instead of providing
				// a pull based interface, i.e. a custom AudioInputStream
				// subclass).
				freq -= (fraq % 25);

				_track[l].iteration = _track[l].pullSize = freq * channels;

				if (channels == 2)
					mixerFlags = SoundMixer::FLAG_STEREO | SoundMixer::FLAG_REVERSE_STEREO;

				if ((bits == 12) || (bits == 16)) {
					mixerFlags |= SoundMixer::FLAG_16BITS;
					_track[l].iteration = _track[l].pullSize *= 2;
				} else if (bits == 8) {
					mixerFlags |= SoundMixer::FLAG_UNSIGNED;
				} else
					error("IMuseDigital::startSound(): Can't handle %d bit samples", bits);

				_track[l].pullSize /= 25;	// We want a "frame rate" of 25 audio blocks per second
			}

			if (input) {
				_track[l].stream2 = input;
				_track[l].stream = NULL;
			} else {
				_track[l].stream2 = NULL;
				_track[l].stream = makeAppendableAudioStream(freq, mixerFlags, 100000);
				_vm->_mixer->playInputStream(&_track[l].handle, _track[l].stream, true, _track[l].vol / 1000, _track[l].pan, -1);
			}

			_track[l].used = true;
			return;
		}
	}
	warning("IMuseDigital::startSound(): All slots are full");
}

void IMuseDigital::stopSound(int soundId) {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::stopSound()");
	debug(5, "IMuseDigital::stopSound(%d)", soundId);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].soundId == soundId) && _track[l].used) {
			if (_track[l].stream) {
				_track[l].toBeRemoved = true;
			}
			else if (_track[l].stream2)
				_vm->_mixer->stopHandle(_track[l].handle);
		}
	}
}

void IMuseDigital::setVolume(int soundId, int volume) {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::setVolume()");
	debug(5, "IMuseDigital::setVolumeSound(%d, %d)", soundId, volume);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].soundId == soundId) && _track[l].used) {
			_track[l].vol = volume * 1000;
		}
	}
}

void IMuseDigital::setPan(int soundId, int pan) {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::setPan()");
	debug(5, "IMuseDigital::setVolumeSound(%d, %d)", soundId, pan);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].soundId == soundId) && _track[l].used) {
			_track[l].pan = pan;
		}
	}
}

void IMuseDigital::setFade(int soundId, int destVolume, int delay60HzTicks) {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::setFade()");
	debug(5, "IMuseDigital::setFade(%d, %d, %d)", soundId, destVolume, delay60HzTicks);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].soundId == soundId) && _track[l].used) {
			_track[l].volFadeDelay = delay60HzTicks;
			_track[l].volFadeDest = destVolume * 1000;
			_track[l].volFadeStep = (_track[l].volFadeDest - _track[l].vol) * 60 * 40 / (1000 * delay60HzTicks);
			_track[l].volFadeUsed = true;
		}
	}
}

void IMuseDigital::stopAllSounds(bool waitForStop) {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::stopAllSounds()");
	debug(5, "IMuseDigital::stopAllSounds");
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if (_track[l].used) {
			if (_track[l].stream) {
				_track[l].toBeRemoved = true;
			} else if (_track[l].stream2)
				_vm->_mixer->stopHandle(_track[l].handle);
		}
	}
/*
	if (waitForStop) {
		bool used;
		do {
			used = false;
			for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
				if (_track[l].used)
					used = true;
			}
			g_system->delay_msecs(10);
		} while (used);
	}*/
}

void IMuseDigital::fadeOutMusic(int fadeDelay) {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::fadeOutMusic()");
	debug(5, "IMuseDigital::fadeOutMusic");
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].used) && (!_track[l].waitForEndSeq) && (_track[l].soundGroup == IMUSE_MUSIC) && (!_track[l].volFadeUsed)) {
			_track[l].volFadeDelay = fadeDelay;
			_track[l].volFadeDest = 0;
			_track[l].volFadeStep = (_track[l].volFadeDest - _track[l].vol) * 60 * 40 / (1000 * fadeDelay);
			_track[l].volFadeUsed = true;
		}
	}
}

void IMuseDigital::pause(bool p) {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::pause()");
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if (_track[l].used) {
			_vm->_mixer->pauseHandle(_track[l].handle, p);
		}
	}
	_pause = p;
}

void IMuseDigital::parseScriptCmds(int a, int b, int c, int d, int e, int f, int g, int h) {
	int cmd = a;
	int soundId = b;
	int sub_cmd = c;

	if (!cmd)
		return;

	switch (cmd) {
	case 10: // ImuseStopAllSounds
		stopAllSounds();
		break;
	case 12: // ImuseSetParam
		switch (sub_cmd) {
		case 0x400: // set group volume
			debug(5, "set group volume (0x400), soundId(%d), group volume(%d)", soundId, d);
			break;
		case 0x500: // set priority - could be ignored
			break;
		case 0x600: // set volume
			setVolume(soundId, d);
			break;
		case 0x700: // set pan
			setPan(soundId, d);
			break;
		default:
			warning("IMuseDigital::doCommand SetParam DEFAULT command %d", sub_cmd);
			break;
		}
		break;
	case 14: // ImuseFadeParam
		switch (sub_cmd) {
		case 0x600: // set volume fading
			if ((d != 0) && (e == 0))
				setVolume(soundId, d);
			else if ((d == 0) && (e == 0))
				stopSound(soundId);
			else
				setFade(soundId, d, e);
			break;
		default:
			warning("IMuseDigital::doCommand FadeParam DEFAULT sub command %d", sub_cmd);
			break;
		}
		break;
	case 25: // ImuseStartStream
		debug(5, "ImuseStartStream (%d, %d, %d)", soundId, c, d);
		break;
	case 26: // ImuseSwitchStream
		debug(5, "ImuseSwitchStream (%d, %d, %d, %d, %d)", soundId, c, d, e, f);
		break;
	case 0x1000: // ImuseSetState
		debug(5, "ImuseSetState (%d)", b);
		if ((_vm->_gameId == GID_DIG) && (_vm->_features & GF_DEMO)) {
			if (b == 1) {
				fadeOutMusic(200);
				startMusic(1, false, 127, false);
			} else {
				if (getSoundStatus(2) == 0) {
					fadeOutMusic(200);
					startMusic(2, false, 127, false);
				}
			}
		} else if ((_vm->_gameId == GID_CMI) && (_vm->_features & GF_DEMO)) {
			if (b == 2) {
				fadeOutMusic(120);
				startMusic("in1.imx", 2002, false, 0, 127, false);
			} else if (b == 4) {
				fadeOutMusic(120);
				startMusic("in2.imx", 2004, false, 0, 127, false);
			} else if (b == 8) {
				fadeOutMusic(120);
				startMusic("out1.imx", 2008, false, 0, 127, false);
			} else if (b == 9) {
				fadeOutMusic(120);
				startMusic("out2.imx", 2009, false, 0, 127, false);
			} else if (b == 16) {
				fadeOutMusic(120);
				startMusic("gun.imx", 2016, false, 0, 127, false);
			} else {
				warning("imuse digital: set state unknown for cmi demo: %d, room: %d", b, this->_vm->_currentRoom);
			}
		} else if (_vm->_gameId == GID_DIG) {
			setDigMusicState(b);
		} else if (_vm->_gameId == GID_CMI) {
			setComiMusicState(b);
		} else if (_vm->_gameId == GID_FT) {
			setFtMusicState(b);
		}
		break;
	case 0x1001: // ImuseSetSequence
		debug(5, "ImuseSetSequence (%d)", b);
		if (_vm->_gameId == GID_DIG) {
			setDigMusicSequence(b);
		} else if (_vm->_gameId == GID_CMI) {
			setComiMusicSequence(b);
		} else if (_vm->_gameId == GID_FT) {
			setFtMusicSequence(b);
		}
		break;
	case 0x1002: // ImuseSetCuePoint
		debug(5, "ImuseSetCuePoint (%d)", b);
		if (_vm->_gameId == GID_FT) {
			setFtMusicCuePoint(b);
		}
		break;
	case 0x1003: // ImuseSetAttribute
		debug(5, "ImuseSetAttribute (%d, %d)", b, c);
		if (_vm->_gameId == GID_DIG) {
			assert(b >= 0 && b < 11);
			_attributesTable[b] = c;
		}
		break;
	case 0x2000: // ImuseSetMasterSFXVolume
		debug(5, "ImuseSetMasterSFXVolume (%d)", b);
		// TODO
		break;
	case 0x2001: // ImuseSetMasterVoiceVolume
		debug(5, "ImuseSetMasterVoiceVolume (%d)", b);
		// TODO
		break;
	case 0x2002: // ImuseSetMasterMusicVolume
		debug(5, "ImuseSetMasterMusicVolume (%d)", b);
		// TODO
		break;
	default:
		warning("IMuseDigital::doCommand DEFAULT command %d", cmd);
	}
}

int IMuseDigital::getSoundStatus(int sound) const {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::getSoundStatus()");
	debug(5, "IMuseDigital::getSoundStatus(%d)", sound);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].soundId == sound) && _track[l].used) {
			return 1;
		}
	}

	return 0;
}

void IMuseDigital::getLipSync(int soundId, int syncId, int32 msPos, int32 &width, int32 &height) {
	int32 sync_size;
	byte *sync_ptr;

	msPos /= 16;
	if (msPos < 65536) {
		for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
			if ((_track[l].soundId == soundId) && _track[l].used) {
				_sound->getSyncSizeAndPtrById(_track[l].soundHandle, syncId, sync_size, &sync_ptr);
				if ((sync_size != 0) && (sync_ptr != NULL)) {
					sync_size /= 4;
					while (sync_size--) {
						if (READ_BE_UINT16(sync_ptr) >= msPos)
							break;
						sync_ptr += 4;
					}
					if (sync_size < 0)
						sync_ptr -= 4;
					else
						if (READ_BE_UINT16(sync_ptr) > msPos)
							sync_ptr -= 4;

					width = sync_ptr[2];
					height = sync_ptr[3];
					return;
				}
			}
		}
	}
}

int32 IMuseDigital::getPosInMs(int soundId) {
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].soundId == soundId) && (_track[l].used)) {
			int32 pos = (5 * (_track[l].dataOffset + _track[l].regionOffset)) / (_track[l].iteration / 200);
			return pos;
		}
	}

	return 0;
}

int32 IMuseDigital::getCurMusicPosInMs() {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::getCurMusicPosInMs()");
	int soundId = -1;

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].used) && (!_track[l].waitForEndSeq) && (_track[l].soundGroup == IMUSE_MUSIC) && (!_track[l].volFadeUsed)) {
			soundId = _track[l].soundId;
		}
	}

	int32 msPos = getPosInMs(soundId);
	debug(5, "IMuseDigital::getCurMusicPosInMs(%d) = %d", soundId, msPos);
	return msPos;
}

int32 IMuseDigital::getCurVoiceLipSyncWidth() {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::getCutVoiceLipSyncWidth()");
	int32 msPos = getPosInMs(kTalkSoundID) + _vm->VAR(_vm->VAR_SYNC) + 50;
	int32 width = 0, height = 0;

	debug(5, "IMuseDigital::getCurVoiceLipSyncWidth(%d)", kTalkSoundID);
	getLipSync(kTalkSoundID, 0, msPos, width, height);
	return width;
}

int32 IMuseDigital::getCurVoiceLipSyncHeight() {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::getCurVoiceLipSyncHeight()");
	int32 msPos = getPosInMs(kTalkSoundID) + _vm->VAR(_vm->VAR_SYNC) + 50;
	int32 width = 0, height = 0;

	debug(5, "IMuseDigital::getCurVoiceLipSyncHeight(%d)", kTalkSoundID);
	getLipSync(kTalkSoundID, 0, msPos, width, height);
	return height;
}

int32 IMuseDigital::getCurMusicLipSyncWidth(int syncId) {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::getCurMusicLipSyncWidth()");
	int soundId = -1;

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].used) && (!_track[l].waitForEndSeq) && (_track[l].soundGroup == IMUSE_MUSIC) && (!_track[l].volFadeUsed)) {
			soundId = _track[l].soundId;
		}
	}

	int32 msPos = getPosInMs(soundId) + _vm->VAR(_vm->VAR_SYNC) + 50;
	int32 width = 0, height = 0;

	debug(5, "IMuseDigital::getCurVoiceLipSyncWidth(%d, %d)", soundId, msPos);
	getLipSync(soundId, syncId, msPos, width, height);
	return width;
}

int32 IMuseDigital::getCurMusicLipSyncHeight(int syncId) {
	Common::StackLock lock(_mutex, g_system, "IMuseDigital::getCurMusicLipSyncHeight()");
	int soundId = -1;

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].used) && (!_track[l].waitForEndSeq) && (_track[l].soundGroup == IMUSE_MUSIC) && (!_track[l].volFadeUsed)) {
			soundId = _track[l].soundId;
		}
	}

	int32 msPos = getPosInMs(soundId) + _vm->VAR(_vm->VAR_SYNC) + 50;
	int32 width = 0, height = 0;

	debug(5, "IMuseDigital::getCurVoiceLipSyncHeight(%d, %d)", soundId, msPos);
	getLipSync(soundId, syncId, msPos, width, height);
	return height;
}

} // End of namespace Scumm
