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
	: idSound(-1), used(false), stream(NULL) {
}

void IMuseDigital::timer_handler(void *refCon) {
	IMuseDigital *imuseDigital = (IMuseDigital *)refCon;
	imuseDigital->callback();
}

IMuseDigital::IMuseDigital(ScummEngine *scumm)
	: _vm(scumm) {
	_pause = false;
	_sound = new ImuseDigiSndMgr(_vm);
	_vm->_timer->installTimerProc(timer_handler, 1000000 / 25, this);
	_curMusicId = -1;
}

IMuseDigital::~IMuseDigital() {
	_vm->_timer->removeTimerProc(timer_handler);
	stopAllSounds();
	delete _sound;
}

void IMuseDigital::callback() {
	int l = 0;

	if (_pause || !_vm)
		return;

	for (l = 0; l < MAX_DIGITAL_TRACKS;l ++) {
		if (_track[l].used) {
			if (_track[l].stream2) {
				if (!_track[l].handle.isActive() && _track[l].started) {
					debug(5, "IMuseDigital::callback(): stoped sound: %d", _track[l].idSound);
					delete _track[l].stream2;
					_track[l].stream2 = NULL;
					_track[l].used = false;
					continue;
				}
			} else if (_track[l].stream) {
				if (_track[l].toBeRemoved) {
					debug(5, "IMuseDigital::callback(): stoped sound: %d", _track[l].idSound);
					if (_track[l].stream)
						_track[l].stream->finish();
					_track[l].stream = NULL;
					_sound->closeSound(_track[l].soundHandle);
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
				debug(5, "Fade: sound(%d), Vol(%d)", _track[l].idSound, _track[l].vol / 1000);
			}

			int pan = (_track[l].pan != 64) ? 2 * _track[l].pan - 127 : 0;

			if (_vm->_mixer->isReady()) {
				if (_track[l].stream2) {
					if (!_track[l].started) {
						_track[l].started = true;
						_vm->_mixer->playInputStream(&_track[l].handle, _track[l].stream2, true, _track[l].vol / 1000, _track[l].pan, -1, false);
					} else {
						_vm->_mixer->setChannelVolume(_track[l].handle, _track[l].vol / 1000);
						_vm->_mixer->setChannelPan(_track[l].handle, pan);
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
						_vm->_mixer->setChannelPan(_track[l].handle, pan);
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
	if ((_vm->_gameId == GID_FT) && (num_jumps != 0)) {
		_track[track].regionOffset = 0;
		return;
	}

				if (_track[track].idSound == 2312) {
					_track[track].curRegion = 4;
					_track[track].regionOffset = 0;
					return;
				}
	if (++_track[track].curRegion == num_regions) {
		_track[track].toBeRemoved = true;
		return;
	}

	int hookid = _sound->getJumpIdByRegionId(_track[track].soundHandle, _track[track].curRegion);
	if (hookid == _track[track].curHookId) {
		int region = _sound->getRegionIdByHookId(_track[track].soundHandle, hookid);
		if (region != -1) {
			_track[track].curRegion = region;
			_track[track].curHookId = 0;
			debug(5, "switchToNextRegion-sound(%d) jump to %d region", _track[track].idSound, _track[track].curRegion);
		}
	}

	debug(5, "switchToNextRegion-sound(%d) select %d region", _track[track].idSound, _track[track].curRegion);
	_track[track].regionOffset = 0;
}

void IMuseDigital::startSound(int soundId, const char *soundName, int soundType, int soundGroup, AudioStream *input) {
	debug(5, "IMuseDigital::startSound(%d)", soundId);
	int l;

	for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if (!_track[l].used && !_track[l].handle.isActive()) {
			_track[l].pan = 64;
			_track[l].vol = 127 * 1000;
			_track[l].volFadeDest = 0;
			_track[l].volFadeStep = 0;
			_track[l].volFadeDelay = 0;
			_track[l].volFadeUsed = false;
			_track[l].idSound = soundId;
			_track[l].started = false;
			_track[l].soundGroup = soundGroup;
			_track[l].curHookId = 0;
			_track[l].curRegion = -1;
			_track[l].regionOffset = 0;
			_track[l].trackOffset = 0;
			_track[l].mod = 0;
			_track[l].toBeRemoved = false;

			int bits = 0, freq = 0, channels = 0, mixerFlags = 0;

			if (input) {
				_track[l].iteration = 1; // ?
				// Do nothing here, we already have an audio stream
			} else {
				if (soundName == NULL)
					_track[l].soundHandle = _sound->openSound(soundId, NULL, soundType, soundGroup);
				else
					_track[l].soundHandle = _sound->openSound(0, soundName, soundType, soundGroup);

				if (_track[l].soundHandle == NULL)
					return;

				bits = _sound->getBits(_track[l].soundHandle);
				channels = _sound->getChannels(_track[l].soundHandle);
				freq = _sound->getFreq(_track[l].soundHandle);
				
				_track[l].iteration = freq * channels;
				if ((bits == 12) || (bits == 16))
					_track[l].iteration *= 2;

				assert(channels == 1 || channels == 2);
				_track[l].pullSize = freq * channels;
				if (channels == 2) {
					mixerFlags = SoundMixer::FLAG_STEREO | SoundMixer::FLAG_REVERSE_STEREO;
				}

				if ((bits == 12) || (bits == 16)) {
					mixerFlags |= SoundMixer::FLAG_16BITS;
					_track[l].pullSize *= 2;
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

			if (soundGroup == IMUSE_MUSIC) {
				stopMusic();
				_curMusicId = soundId;
			}

			_track[l].used = true;
			return;
		}
	}
	warning("IMuseDigital::startSound(): All slots are full");
}

void IMuseDigital::stopMusic() {
	debug(5, "IMuseDigital::stopMusic()");
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].idSound == _curMusicId) && _track[l].used) {
			if (_track[l].stream) {
				_track[l].toBeRemoved = true;
			}
		}
	}
	_curMusicId = -1;
}

void IMuseDigital::stopSound(int soundId) {
	debug(5, "IMuseDigital::stopSound(%d)", soundId);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].idSound == soundId) && _track[l].used) {
			if (_track[l].stream) {
				_track[l].toBeRemoved = true;
			}
			else if (_track[l].stream2)
				_vm->_mixer->stopHandle(_track[l].handle);
		}
	}
}

void IMuseDigital::stopAllSounds(bool waitForStop) {
	debug(5, "IMuseDigital::stopAllSounds");
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if (_track[l].used) {
			if (_track[l].stream) {
				_track[l].toBeRemoved = true;
			} else if (_track[l].stream2)
				_vm->_mixer->stopHandle(_track[l].handle);
		}
	}
	_curMusicId = -1;

	if (waitForStop) {
		bool used;
		do {
			used = false;
			for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
				if (_track[l].used)
					used = true;
			}
		} while (used);
	}
}

void IMuseDigital::pause(bool p) {
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if (_track[l].used) {
			_vm->_mixer->pauseHandle(_track[l].handle, p);
		}
	}
	_pause = p;
}

void IMuseDigital::parseScriptCmds(int a, int b, int c, int d, int e, int f, int g, int h) {
	int cmd = a;
	int sample = b;
	int sub_cmd = c;
	int chan = -1;
	int l, r;

	if (!cmd)
		return;

	switch (cmd) {
	case 10: // ImuseStopAllSounds
		debug(5, "ImuseStopAllSounds()");
		stopAllSounds();
		break;
	case 12: // ImuseSetParam
		switch (sub_cmd) {
		case 0x500: // set priority - could be ignored
			break;
		case 0x600: // set volume
			debug(5, "ImuseSetParam (0x600), sample(%d), volume(%d)", sample, d);
			for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
				if ((_track[l].idSound == sample) && _track[l].used) {
					_track[l].vol = d * 1000;
//					if (_track[l].volFadeUsed)
//						_track[l].volFadeStep = (_track[l].volFadeDest - _track[l].vol) * 60 * 40 / (1000 * _track[chan].volFadeDelay);
				}
			}
			if (l == -1) {
				debug(5, "ImuseSetParam (0x600), sample(%d) not exist in channels", sample);
				return;
			}
			break;
		case 0x700: // set pan
			debug(5, "ImuseSetParam (0x700), sample(%d), pan(%d)", sample, d);
			for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
				if ((_track[l].idSound == sample) && _track[l].used) {
					_track[l].pan = d;
				}
			}
			if (l == -1) {
				debug(5, "ImuseSetParam (0x700), sample(%d) not exist in channels", sample);
				return;
			}
			break;
		default:
			warning("IMuseDigital::doCommand SetParam DEFAULT command %d", sub_cmd);
			break;
		}
		break;
	case 14: // ImuseFadeParam
		switch (sub_cmd) {
		case 0x600: // set new volume with fading
			debug(5, "ImuseFadeParam - fade sample(%d), to volume(%d) with 60hz ticks(%d)", sample, d, e);
			if ((_vm->_gameId == GID_DIG) && (_vm->_features & GF_DEMO)) {
				stopSound(sample);
				return;
			}
			for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
				if ((_track[l].idSound == sample) && _track[l].used) {
					_track[l].volFadeDelay = e;
					_track[l].volFadeDest = d * 1000;
					_track[l].volFadeStep = (_track[l].volFadeDest - _track[l].vol) * 60 * 40 / (1000 * e);
					_track[l].volFadeUsed = true;
					debug(5, "ImuseFadeParam: vol %d, volDest %d, step %d", _track[l].vol, d * 1000, _track[l].volFadeStep);
				}
			}
			if (chan == -1) {
				debug(5, "ImuseFadeParam (0x600), sample %d not exist in channels", sample);
				return;
			}
			break;
		default:
			warning("IMuseDigital::doCommand FadeParam DEFAULT sub command %d", sub_cmd);
			break;
		}
		break;
	case 0x1000: // ImuseSetState
		debug(5, "ImuseSetState (%d)", b);
		if ((_vm->_gameId == GID_DIG) && (_vm->_features & GF_DEMO)) {
			if (b == 1)
				startMusic(1);
			else {
				if (getSoundStatus(2) == 0)
					startMusic(2);
			}
		} else if (_vm->_gameId == GID_DIG) {
			if (b == 1000) {		// STATE_NULL
				stopMusic();
				return;
			}
			for (l = 0; _digStateMusicMap[l].room != -1; l++) {
				if (_digStateMusicMap[l].room == b) {
					int music = _digStateMusicMap[l].table_index;
					debug(5, "Play imuse music: %s, %s, %s", _digStateMusicTable[music].name, _digStateMusicTable[music].title, _digStateMusicTable[music].filename);
					if (_digStateMusicTable[music].filename[0] != 0) {
						startMusic(_digStateMusicTable[music].filename, _digStateMusicTable[music].id);
					}
					break;
				}
			}
		} else if ((_vm->_gameId == GID_CMI) && (_vm->_features & GF_DEMO)) {
			if (b == 2) {
				startMusic("in1.imx", 2002);
			} else if (b == 4) {
				startMusic("in2.imx", 2004);
			} else if (b == 8) {
				startMusic("out1.imx", 2008);
			} else if (b == 9) {
				startMusic("out2.imx", 2009);
			} else if (b == 16) {
				startMusic("gun.imx", 2016);
			} else {
				warning("imuse digital: set state unknown for cmi demo: %d, room: %d", b, this->_vm->_currentRoom);
			}
		} else if (_vm->_gameId == GID_CMI) {
			if (b == 1000) {		// STATE_NULL
				stopMusic();
				return;
			}
			for (l = 0; _comiStateMusicTable[l].id != -1; l++) {
				if ((_comiStateMusicTable[l].id == b)) {
					debug(5, "Play imuse music: %s, %s, %s", _comiStateMusicTable[l].name, _comiStateMusicTable[l].title, _comiStateMusicTable[l].filename);
					if (_comiStateMusicTable[l].filename[0] != 0) {
						startMusic(_comiStateMusicTable[l].filename, b);
					}
					break;
				}
			}
		} else if (_vm->_gameId == GID_FT) {
			for (l = 0;; l++) {
				if (_ftStateMusicTable[l].index == -1) {
					return;
				}
				if (_ftStateMusicTable[l].index == b) {
					debug(5, "Play imuse music: %s, %s", _ftStateMusicTable[l].name, _ftStateMusicTable[l].audioname);
					if (_ftStateMusicTable[l].audioname[0] != 0) {
						for (r = 0; r < _vm->_numAudioNames; r++) {
							if (strcmp(_ftStateMusicTable[l].audioname, &_vm->_audioNames[r * 9]) == 0) {
								startMusic(r);
								parseScriptCmds(12, r, 0x600, _ftStateMusicTable[l].volume, 0, 0, 0, 0);
							}
						}
					}
				}
			}
		}
		break;
	case 0x1001: // ImuseSetSequence
		debug(5, "ImuseSetSequence (%d)", b);
		if (_vm->_gameId == GID_DIG) {
			for (l = 0; _digSeqMusicTable[l].room != -1; l++) {
				if (_digSeqMusicTable[l].room == b) {
					debug(5, "Play imuse music: %s, %s, %s", _digSeqMusicTable[l].name, _digSeqMusicTable[l].title, _digSeqMusicTable[l].filename);
					if (_digSeqMusicTable[l].filename[0] != 0) {
						startMusic(_digSeqMusicTable[l].filename, b);
					}
					break;
				}
			}
		} else if (_vm->_gameId == GID_CMI) {
			for (l = 0; _comiSeqMusicTable[l].id != -1; l++) {
				if (_comiSeqMusicTable[l].id == b) {
					debug(5, "Play imuse music: %s, %s, %s", _comiSeqMusicTable[l].name, _comiSeqMusicTable[l].title, _comiSeqMusicTable[l].filename);
					if (_comiSeqMusicTable[l].filename[0] != 0) {
						startMusic(_comiSeqMusicTable[l].filename, b);
					}
					break;
				}
			}
		} else if (_vm->_gameId == GID_FT) {
			for (l = 0; _ftSeqMusicTable[l].index != -1; l++) {
				if (_ftSeqMusicTable[l].index == b) {
					debug(5, "Play imuse music: %s, %s", _ftSeqMusicTable[l].name, _ftSeqMusicTable[l].audioname);
					if (_ftSeqMusicTable[l].audioname[0] != 0) {
						for (r = 0; r < _vm->_numAudioNames; r++) {
							if (strcmp(_ftSeqMusicTable[l].audioname, &_vm->_audioNames[r * 9]) == 0) {
//								startMusic(r);
//								parseScriptCmds(12, r, 0x600, _ftSeqMusicTable[l].volume, 0, 0, 0, 0);
							}
						}
					}
				}
			}
		}
		break;
	case 0x1002: // ImuseSetCuePoint
		debug(5, "ImuseSetCuePoint (%d)", b);
		// TODO
		break;
	case 0x1003: // ImuseSetAttribute
		debug(5, "ImuseSetAttribute (%d, %d)", b, c);
		// TODO
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
	debug(5, "IMuseDigital::getSoundStatus(%d)", sound);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].idSound == sound) && _track[l].used) {
			return 1;
		}
	}

	return 0;
}

int32 IMuseDigital::getPosInMs(int soundId) {
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].idSound == soundId) && _track[l].used) {
			int32 pos = 1000 * _track[l].trackOffset / _track[l].iteration;
			return pos;
		}
	}

	return 0;
}

int32 IMuseDigital::getCurMusicPosInMs() {
	debug(5, "IMuseDigital::getCurMusicPosInMs(%d)", _curMusicId);
	return getPosInMs(_curMusicId);
}

int32 IMuseDigital::getCurVoiceLipSyncWidth() {
//	int32 pos = getPosInMs(kTalkSoundID);
	return _vm->_rnd.getRandomNumber(255);
}

int32 IMuseDigital::getCurVoiceLipSyncHeight() {
//	int32 pos = getPosInMs(kTalkSoundID);
	return _vm->_rnd.getRandomNumber(255);
}

int32 IMuseDigital::getCurMusicLipSyncWidth(int32 param) {
//	int32 pos = getPosInMs(_curMusicId);
	return _vm->_rnd.getRandomNumber(255);
}

int32 IMuseDigital::getCurMusicLipSyncHeight(int32 param) {
//	int32 pos = getPosInMs(_curMusicId);
	return _vm->_rnd.getRandomNumber(255);
}

} // End of namespace Scumm
