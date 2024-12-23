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

#include "audio/audiostream.h"
#include "common/config-manager.h"
#include "common/timer.h"
#include "engines/engine.h"
#include "scumm/cdda.h"
#include "scumm/music.h"
#include "scumm/scumm.h"
#include "scumm/soundcd.h"
#include "scumm/soundse.h"

namespace Scumm {

SoundCD::SoundCD(ScummEngine *parent, Audio::Mixer *mixer, SoundSE *soundSE, bool useReplacementAudioTracks):
	_vm(parent), _mixer(mixer), _soundSE(soundSE), _useReplacementAudioTracks(useReplacementAudioTracks) {
	_fileBasedCDStatus.playing = false;
	_fileBasedCDStatus.track = 0;
	_fileBasedCDStatus.start = 0;
	_fileBasedCDStatus.duration = 0;
	_fileBasedCDStatus.numLoops = 0;
	_fileBasedCDStatus.volume = Audio::Mixer::kMaxChannelVolume;
	_fileBasedCDStatus.balance = 0;

	_hasFileBasedCDAudio =
		(_vm->_game.id == GID_LOOM && Common::File::exists("CDDA.SOU")) ||
		((_vm->_game.features & GF_DOUBLEFINE_PAK) && _vm->_game.id == GID_MONKEY);

	_loomOvertureTransition = DEFAULT_LOOM_OVERTURE_TRANSITION + ConfMan.getInt("loom_overture_ticks");

	_fileBasedCDAudioHandle = new Audio::SoundHandle();
}

SoundCD::~SoundCD() {
	stopCDTimer();
	stopCD();
	delete _fileBasedCDAudioHandle;
}

void SoundCD::stopCDSound(int sound) {
	if (sound != 0 && sound == _currentCDSound) {
		_currentCDSound = 0;
		_musicTimer = 0;
		_replacementTrackStartTime = 0;
		stopCD();
		stopCDTimer();
	}
}

void SoundCD::stopAllCDSounds() {
	if (_currentCDSound != 0) {
		_currentCDSound = 0;
		_musicTimer = 0;
		_replacementTrackStartTime = 0;
		stopCD();
		stopCDTimer();
	}
}

static void cdTimerHandler(void *refCon) {
	SoundCD *snd = (SoundCD *)refCon;

	// FIXME: Turn off the timer when it's no longer needed. In theory, it
	// should be possible to check with pollCD(), but since CD sound isn't
	// properly restarted when reloading a saved game, I don't dare to.
	if ((snd->_cdMusicTimerMod++ & 3) == 0) {
		snd->_cdMusicTimer++;
	}
}

void SoundCD::startCDTimer() {
	if (_useReplacementAudioTracks)
		return;

	// This CD timer implementation strictly follows the original interpreters for
	// Monkey Island 1 CD and Loom CD: it works by incrementing _cdMusicTimerMod and _cdMusicTimer
	// at each quarter frame (see ScummEngine::setTimerAndShakeFrequency() for what the exact
	// frequency rate is for the particular game and engine version being ran).
	//
	// Again as per the interpreters, VAR_MUSIC_TIMER is then updated inside the SCUMM main loop.
	int32 interval = 1000000 / _vm->getTimerFrequency();

	// LOOM Steam uses a fixed 240Hz rate. This was probably done to get rid of some
	// audio glitches which are confirmed to be in the original. So let's activate this
	// fix for the DOS version of LOOM as well, if enhancements are enabled.
	if (_vm->_game.id == GID_LOOM && (_hasFileBasedCDAudio || _vm->enhancementEnabled(kEnhMinorBugFixes)))
		interval = 1000000 / LOOM_STEAM_CDDA_RATE;

	_vm->getTimerManager()->removeTimerProc(&cdTimerHandler);
	_vm->getTimerManager()->installTimerProc(&cdTimerHandler, interval, this, "scummCDtimer");
}

void SoundCD::stopCDTimer() {
	if (_useReplacementAudioTracks)
		return;

	_vm->getTimerManager()->removeTimerProc(&cdTimerHandler);
}

void SoundCD::playCDTrack(int track, int numLoops, int startFrame, int duration) {
	// Reset the music timer variable at the start of a new track
	_vm->VAR(_vm->VAR_MUSIC_TIMER) = 0;
	_cdMusicTimerMod = 0;
	_cdMusicTimer = 0;

	// Play it
	if (!_soundsPaused)
		playCDTrackInternal(track, numLoops, startFrame, duration);

	// Start the timer after starting the track. Starting an MP3 track is
	// almost instantaneous, but a CD player may take some time. Hopefully
	// playCD() will block during that delay.
	startCDTimer();
}

void SoundCD::playCDTrackInternal(int track, int numLoops, int startFrame, int duration) {
	_fileBasedCDStatus.track = track;
	_fileBasedCDStatus.numLoops = numLoops;
	_fileBasedCDStatus.start = startFrame;
	_fileBasedCDStatus.duration = duration;

	if (!_hasFileBasedCDAudio) {
		g_system->getAudioCDManager()->play(track, numLoops, startFrame, duration);
	} else {
		// Stop any currently playing track
		_mixer->stopHandle(*_fileBasedCDAudioHandle);

		Audio::SeekableAudioStream *stream = nullptr;

		if (_vm->_game.id == GID_LOOM) {
			stream = makeCDDAStream("CDDA.SOU", DisposeAfterUse::YES);
		} else if (_soundSE) {
			stream = _soundSE->getXWBTrack(track);
		}

		if (!stream)
			return;

		Audio::Timestamp start = Audio::Timestamp(0, startFrame, 75);
		Audio::Timestamp end = Audio::Timestamp(0, startFrame + duration, 75);

		_mixer->playStream(Audio::Mixer::kMusicSoundType, _fileBasedCDAudioHandle,
						   Audio::makeLoopingAudioStream(stream, start, end, (numLoops < 1) ? numLoops + 1 : numLoops));
	}
}

void SoundCD::stopCD() {
	if (!_hasFileBasedCDAudio)
		g_system->getAudioCDManager()->stop();
	else
		_mixer->stopHandle(*_fileBasedCDAudioHandle);
}

int SoundCD::pollCD() const {
	if (!_hasFileBasedCDAudio)
		return g_system->getAudioCDManager()->isPlaying();
	else
		return _mixer->isSoundHandleActive(*_fileBasedCDAudioHandle);
}

void SoundCD::updateCD() {
	if (!_hasFileBasedCDAudio)
		g_system->getAudioCDManager()->update();
}

AudioCDManager::Status SoundCD::getCDStatus() {
	if (!_hasFileBasedCDAudio)
		return g_system->getAudioCDManager()->getStatus();
	else {
		AudioCDManager::Status info = _fileBasedCDStatus;
		info.playing = _mixer->isSoundHandleActive(*_fileBasedCDAudioHandle);
		return info;
	}
}

int SoundCD::getCDTrackIdFromSoundId(int soundId, int &loops, int &start) {
	if (_vm->_game.id == GID_LOOM && _vm->_game.version == 4) {
		loops = 0;
		start = -1;
		return 1;
	}

	if (soundId != -1 && _vm->getResourceAddress(rtSound, soundId)) {
		uint8 *ptr = _vm->getResourceAddress(rtSound, soundId) + 0x18;
		loops = ptr[1];
		start = (ptr[2] * 60 + ptr[3]) * 75 + ptr[4];
		return ptr[0];
	}

	loops = 1;
	return -1;
}

bool SoundCD::isRolandLoom() const {
	return
		(_vm->_game.id == GID_LOOM) &&
		(_vm->_game.version == 3) &&
		(_vm->_game.platform == Common::kPlatformDOS) &&
		(_vm->VAR(_vm->VAR_SOUNDCARD) == 4);
}

#define JIFFIES_TO_TICKS(x) (40 * ((double)(x)) / _vm->getTimerFrequency())
#define TICKS_TO_JIFFIES(x) ((double)(x) * (_vm->getTimerFrequency() / 40))

#define TICKS_TO_TIMER(x) ((((x)*204) / _loomOvertureTransition) + 1)
#define TIMER_TO_TICKS(x) ((((x)-1) * _loomOvertureTransition) / 204)

void SoundCD::updateMusicTimer() {
	bool isLoomOverture = (isRolandLoom() && _currentCDSound == 56 && !(_vm->_game.features & GF_DEMO));

	// If the replacement track has ended, reset the timer to 0 like when
	// playing the original music. We make an exception for the Overture,
	// since it may need to keep running after the track has ended.
	//
	// This is also why we can't query the CD audio manager for the current
	// position. That, and the fact that the CD manager does not provide
	// this information at the time of writing.

	if (!pollCD() && !isLoomOverture) {
		_currentCDSound = 0;
		_musicTimer = 0;
		_replacementTrackStartTime = 0;
		return;
	}

	// Time is measured in "ticks", with ten ticks per second. This should
	// be exact enough, while providing an easily understandable unit of
	// measurement for the adjustment slider.

	// The rate at which the timer is advanced is hard-coded for the Loom
	// Overture. When playing the original music the rate is apparently
	// based on the MIDI tempo of it. But at least for Loom, the Overture
	// seems to be the only piece of music where timing matters.

	// These are the values the timer will have to reach or exceed for the
	// Overture to work correctly:

	// 4   - Fade in the "OVERTURE" text
	// 198 - Fade down the "OVERTURE" text
	// 204 - Show the LucasFilm logo
	// 278 - End the Overture

	// VAR_TOTAL_TIMER measures time in "jiffies", or frames. This will
	// eventually overflow, but I don't expect that to ever be a problem.

	int32 now = _vm->VAR(_vm->VAR_TIMER_TOTAL);

	int32 ticks = JIFFIES_TO_TICKS(now - _replacementTrackStartTime);

	// If the track ends before the timer reaches 198, skip ahead. (If the
	// timer didn't even reach 4 you weren't really trying, and must be
	// punished for that!)

	if (isLoomOverture && !pollCD()) {
		int32 fadeDownTick = TIMER_TO_TICKS(198);
		if (ticks < fadeDownTick) {
			_replacementTrackStartTime = now - TICKS_TO_JIFFIES(fadeDownTick);
			ticks = fadeDownTick;
		}
	}

	_musicTimer = TICKS_TO_TIMER(ticks);

	// But don't let the timer exceed 278 until the Overture has ended, or
	// the music will be cut off.

	if (isLoomOverture && pollCD() && _musicTimer >= 278)
		_musicTimer = 277;
}

void SoundCD::restoreAfterLoad() {
	_musicTimer = 0;
	_replacementTrackStartTime = 0;
	int trackNr = -1;
	int loops = 1;
	int start = 0;
	if (_currentCDSound) {
		if (_useReplacementAudioTracks) {
			trackNr = getReplacementAudioTrack(_currentCDSound);
		} else if (_vm->_game.platform != Common::kPlatformFMTowns) {
			trackNr = getCDTrackIdFromSoundId(_currentCDSound, loops, start);
		}

		if (trackNr != -1) {
			if (_useReplacementAudioTracks) {
				int32 now = _vm->VAR(_vm->VAR_TIMER_TOTAL);
				uint32 frame;

				_musicTimer = _vm->VAR(_vm->VAR_MUSIC_TIMER);

				// We try to resume the audio track from where it was
				// saved. The timer isn't very accurate, but it should
				// be good enough.
				//
				// NOTE: This does not seem to work at the moment, since
				// the track immediately gets restarted in the cases I
				// tried.

				if (_musicTimer > 0) {
					int32 ticks = TIMER_TO_TICKS(_musicTimer);

					_replacementTrackStartTime = now - TICKS_TO_JIFFIES(ticks);
					frame = (75 * ticks) / 10;
				} else {
					_replacementTrackStartTime = now;
					frame = 0;
				}

				// If the user has fiddled with the Loom overture
				// setting, the calculated position could be outside
				// the track. But it seems a warning message is as bad
				// as it gets.

				g_system->getAudioCDManager()->play(trackNr, 1, frame, 0, true);
			} else if (_vm->_game.platform != Common::kPlatformFMTowns) {
				g_system->getAudioCDManager()->play(trackNr, loops, start + _vm->VAR(_vm->VAR_MUSIC_TIMER), 0, true);
			}
		}
	}
}

void SoundCD::restoreCDAudioAfterLoad(AudioCDManager::Status &info) {
	if (info.numLoops < 0 && _vm->_game.platform != Common::kPlatformFMTowns) {
		// If we are loading, and the music being loaded was supposed to loop
		// forever, then resume playing it. This helps a lot when the audio CD
		// is used to provide ambient music (see bug #1150).
		// FM-Towns versions handle this in Player_Towns_v1::restoreAfterLoad().
		playCDTrackInternal(info.track, info.numLoops, info.start, info.duration);
	} else if (_vm->_game.id == GID_LOOM && info.start != 0 && info.duration != 0) {
		// Reload audio for LOOM CD/Steam. We move the offset forward by a little bit
		// to restore the correct sync.
		int startOffset = (int)(_vm->VAR(_vm->VAR_MUSIC_TIMER) * 1.25);

		_cdMusicTimer = _vm->VAR(_vm->VAR_MUSIC_TIMER);
		playCDTrackInternal(info.track, info.numLoops, info.start + startOffset, info.duration - _vm->VAR(_vm->VAR_MUSIC_TIMER));
	}
}

bool SoundCD::triggerCDSound(int soundID) {
	if (_useReplacementAudioTracks) {
		// Note that music does not loop. Probably because it's likely
		// to be interrupted by sound effects before it's over anyway.
		//
		// In the FM Towns version, music does play continuously (each
		// track has two versions), probably because CD audio and sound
		// effects are played independent of each other. Personally I
		// find the game harder when the music is allowed to drown out
		// the sound effects.

		int trackNr = getReplacementAudioTrack(soundID);
		if (trackNr != -1) {
			_currentCDSound = soundID;
			_replacementTrackStartTime = _vm->VAR(_vm->VAR_TIMER_TOTAL);
			_musicTimer = 0;
			g_system->getAudioCDManager()->play(trackNr, 1, 0, 0, true);
			return true;
		}
	}

	if (_vm->_game.id == GID_LOOM && _vm->_game.platform == Common::kPlatformPCEngine) {
		if (soundID >= 13 && soundID <= 32) {
			static const char tracks[20] = {3, 4, 5, 7, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 21, 19, 20, 21};

			_currentCDSound = soundID;

			// The original game had hard-coded lengths for all
			// tracks, but this one track is the only one (as far
			// as we know) where this actually matters. See bug
			// #4914 - LOOM-PCE: Music stops prematurely.

			int track = tracks[soundID - 13];
			if (track == 6) {
				playCDTrack(track, 1, 0, 260);
			} else {
				playCDTrack(track, 1, 0, 0);
			}
		} else {
			if (_vm->_musicEngine) {
				_vm->_musicEngine->startSound(soundID);
			}
		}
		return true;
	}

	return false;
}

int SoundCD::getReplacementAudioTrack(int soundID) {
	int trackNr = -1;

	if (_vm->_game.id == GID_LOOM) {
		if (_vm->_game.features & GF_DEMO) {
			// If I understand correctly, the shorter demo only
			// has the Loom intro music. The longer demo has a
			// couple of tracks that it will cycle through if
			// you leave the demo running.

			if (isRolandLoom())
				soundID -= 10;

			switch (soundID) {
			case 19:
				trackNr = 2;
				break;
			case 20:
				trackNr = 4;
				break;
			case 21:
				trackNr = 7;
				break;
			case 23:
				trackNr = 8;
				break;
			case 26:
				trackNr = 3;
				break;
			}
		} else {
			if (isRolandLoom())
				soundID -= 32;

			// The first track, the Overture, only exists as a
			// Roland track.
			if (soundID >= 24 && soundID <= 32) {
				trackNr = soundID - 23;
			} else if (soundID == 19) {
				trackNr = 10;
			} else if (soundID == 21) {
				trackNr = 11;
			}
		}
	}

	if (trackNr != -1 && !_vm->existExtractedCDAudioFiles(trackNr))
		trackNr = -1;

	return trackNr;
}

void SoundCD::pauseCDSounds(bool pause) {
	if ((_vm->_game.features & GF_AUDIOTRACKS) && _vm->VAR_MUSIC_TIMER != 0xFF && _vm->VAR(_vm->VAR_MUSIC_TIMER) > 0) {
		if (pause)
			stopCDTimer();
		else
			startCDTimer();
	}
}

} // End of namespace Scumm
