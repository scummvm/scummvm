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

#include "bladerunner/music.h"

#include "bladerunner/audio_mixer.h"
#include "bladerunner/aud_stream.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/game_info.h"
#include "bladerunner/savefile.h"
#include "bladerunner/game_constants.h"

#include "common/timer.h"
namespace BladeRunner {

Music::Music(BladeRunnerEngine *vm) {
	_vm = vm;
	// _musicVolumeFactorOriginalEngine here sets a percentage to be applied on the music tracks' volume
	// before sending them to the audio player.
	// This is how the original engine set the volume via the in-game KIA volume slider controls.
	// Setting _musicVolumeFactorOriginalEngine to 100, for the purposes ScummVM engine, renders it indifferent,
	// so sound volume can be controlled by ScummVM's Global Main Menu / ConfMan/ syncSoundSettings().
	_musicVolumeFactorOriginalEngine = BLADERUNNER_ORIGINAL_SETTINGS ? 65 : 100;
	reset();
}

Music::~Music() {
	stop(0u);
	while (isPlaying()) {
		// wait for the mixer to finish
	}

#if BLADERUNNER_ORIGINAL_BUGS
	_vm->getTimerManager()->removeTimerProc(timerCallbackFadeOut);
	_vm->getTimerManager()->removeTimerProc(timerCallbackNext);
#else
	// probably not really needed, but tidy up anyway
	reset();
	_vm->_audioMixer->stopAppTimerProc(kAudioMixerAppTimerMusicFadeOut);
	_vm->_audioMixer->stopAppTimerProc(kAudioMixerAppTimerMusicNext);
#endif
}

void Music::reset() {
	_current.name = "";
	_next.name = "";
	_channel = -1;
	_isPlaying = false;
	_isPaused = false;
	_current.loop = 0;
	_isNextPresent = false;
	_data = nullptr;
	_stream = nullptr;
}

// volume should be in [0, 100]
// pan should be in [-100, 100]
// A negative (typically -1) value for timePlaySeconds, means "play the whole music track".
bool Music::play(const Common::String &trackName, int volume, int pan, int32 timeFadeInSeconds, int32 timePlaySeconds, int loop, int32 timeFadeOutSeconds) {
	//Common::StackLock lock(_mutex);

	// TODO Does this mean that when music volume slider is at mute (0) (in the original engine),
	// the music track would *not* be "played silently" and would not be loaded nor queued at all?
	if (_musicVolumeFactorOriginalEngine <= 0) {
		return false;
	}

	int volumeAdjusted = (volume * _musicVolumeFactorOriginalEngine) / 100; //volumeAdjusted will be in [0, 100]
	int volumeStart = volumeAdjusted;
	if (timeFadeInSeconds > 0) {
		volumeStart = 1;
	}

	// Queuing mechanism:
	// if a music track is already playing, then:
	//    if the requested track is a different track
	//       queue it as "next", to play after the current one.
	//       However, if a "next" track already exists,
	//       then stop the _current track after 2 seconds (force stop current playing track)
	//       Also the previous "next" track still gets replaced by the new requested one.
	// 	     This can be best tested at Animoid Row, within Hawker's Circle,
	// 	     moving from Izo's Pawn Shop to the Bar.
	//    if the requested track is the same as the currently playing,
	//       update the loop int value of the _current to the new one
	//       and adjust its fadeIn and balance/pan
	//    In both of the above cases, the _current track is not (yet) changed.
	if (isPlaying()) {
		if (!_current.name.equalsIgnoreCase(trackName)) {
			_next.name = trackName;
			_next.volume = volume; // Don't store the adjusted volume - This is a "target" volume (before any adjustments from volume sliders or special mode like VK)
			_next.pan = pan;       // This is a "target" value for the pan (balance)
			_next.timeFadeInSeconds = timeFadeInSeconds;
			_next.timePlaySeconds = timePlaySeconds;
			_next.loop = loop;
			_next.timeFadeOutSeconds = timeFadeOutSeconds;
			if (_isNextPresent) {
				stop(2u);
			}
			_isNextPresent = true;
		} else {
			_current.loop = loop;
			if (timeFadeInSeconds < 0) {
				timeFadeInSeconds = 0;
			}
#if BLADERUNNER_ORIGINAL_BUGS
			adjustVolume(volumeAdjusted, timeFadeInSeconds);
#else
			adjustVolume(volume, timeFadeInSeconds);
#endif // BLADERUNNER_ORIGINAL_BUGS
			adjustPan(pan, timeFadeInSeconds);
		}
		return true;
	}

	// If we reach here, there is no music track currently playing
	// So we load it from the game's resources
	_data = getData(trackName);
	if (_data == nullptr) {
		return false;
	}
	_stream = new AudStream(_data);

	_isNextPresent = false;
	uint32 trackLengthInMillis = _stream->getLength();

	uint32 secondToStart = 0;
	// loop > 1 can only happen in restored content, so no need to check for _vm->_cutContent explicitly here
	if (loop > 1 && trackLengthInMillis > 0) {
		// start at some point within the first half of the track
		if (timePlaySeconds > 0 && trackLengthInMillis/1000 > (uint32)timePlaySeconds) {
			secondToStart = _vm->_rnd.getRandomNumberRng(0, MIN(trackLengthInMillis/2000, (trackLengthInMillis/1000 - (uint32)timePlaySeconds)));
		} else if (timeFadeOutSeconds >= 0 && trackLengthInMillis/1000 > (uint32)timeFadeOutSeconds) {
			secondToStart = _vm->_rnd.getRandomNumberRng(0, MIN(trackLengthInMillis/2000, (trackLengthInMillis/1000 - (uint32)timeFadeOutSeconds)));
		}
	}
	if (secondToStart > 0) {
		 _stream->startAtSecond(secondToStart);
	}

	_channel = _vm->_audioMixer->playMusic(_stream, volumeStart, mixerChannelEnded, this, trackLengthInMillis);
	if (_channel < 0) {
		delete _stream;
		_stream = nullptr;
		delete[] _data;
		_data = nullptr;

		return false;
	}
	if (timeFadeInSeconds > 0) {
#if BLADERUNNER_ORIGINAL_BUGS
		adjustVolume(volumeAdjusted, timeFadeInSeconds);
#else
		adjustVolume(volume, timeFadeInSeconds);
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
	_current.name = trackName;

	if (timePlaySeconds > 0) {
		// Removes any previous fadeout timer and installs a new one.
		// Uses the timeFadeOutSeconds value (see Music::fadeOut())
#if BLADERUNNER_ORIGINAL_BUGS
		_vm->getTimerManager()->removeTimerProc(timerCallbackFadeOut);
		_vm->getTimerManager()->installTimerProc(timerCallbackFadeOut, timePlaySeconds * 1000 * 1000, this, "BladeRunnerMusicFadeoutTimer");
#else
		_vm->_audioMixer->stopAppTimerProc(kAudioMixerAppTimerMusicFadeOut);
		_vm->_audioMixer->startAppTimerProc(kAudioMixerAppTimerMusicFadeOut, timePlaySeconds * 1000u);
#endif //BLADERUNNER_ORIGINAL_BUGS
	} else if (timeFadeOutSeconds > 0) {
#if BLADERUNNER_ORIGINAL_BUGS
		_vm->getTimerManager()->removeTimerProc(timerCallbackFadeOut);
		_vm->getTimerManager()->installTimerProc(timerCallbackFadeOut, (trackLengthInMillis - timeFadeOutSeconds * 1000) * 1000, this, "BladeRunnerMusicFadeoutTimer");
#else
		_vm->_audioMixer->stopAppTimerProc(kAudioMixerAppTimerMusicFadeOut);
		_vm->_audioMixer->startAppTimerProc(kAudioMixerAppTimerMusicFadeOut, (trackLengthInMillis - timeFadeOutSeconds * 1000u));
#endif //BLADERUNNER_ORIGINAL_BUGS
	}
	_isPlaying = true;
	_current.volume = volume; // Don't store the adjusted volume - This is a "target" volume (before any adjustments from volume sliders or special mode like VK)
	_current.pan = pan;       // This is a "target" value for the pan (balance)
	_current.timeFadeInSeconds = timeFadeInSeconds;
	_current.timePlaySeconds = timePlaySeconds;
	_current.loop = loop;
	// loop == kMusicLoopPlayOnceRandomStart can only happen in restored content, so no need to check for _vm->_cutContent explicitly here
	if (_current.loop == kMusicLoopRepeatRandomStart) {
		// loop value to store (and use in next loop) should be kMusicLoopRepeat
		_current.loop = kMusicLoopRepeat;
	}
	_current.timeFadeOutSeconds = timeFadeOutSeconds;
	return true;
}

void Music::stop(uint32 delaySeconds) {
	Common::StackLock lock(_mutex);

	if (_channel < 0) {
		return;
	}

#if !BLADERUNNER_ORIGINAL_BUGS
	// In original game, on queued music was not removed and it started playing after actor left the scene
	_isNextPresent = false;
#endif

	_current.loop = 0;
	_vm->_audioMixer->stop(_channel, 60u * delaySeconds);
}

// adjust() is a public method, and note that it accepts special values for its arguments
// volume should be in [0, 100], with "-1" being a special value for skipping volume adjustment
// pan should be in [-100, 100], with "-101" being a special value for skipping pan (balance) adjustment
void Music::adjust(int volume, int pan, uint32 delaySeconds) {
	// value -1 for volume means "do not adjust volume for this music track (while it's playing)"
	if (volume != -1) {
#if BLADERUNNER_ORIGINAL_BUGS
		adjustVolume((volume * _musicVolumeFactorOriginalEngine)/ 100, delaySeconds);
#else
		adjustVolume(volume, delaySeconds);
#endif // BLADERUNNER_ORIGINAL_BUGS
	}
	// value -101 for pan means "do not adjust pan for this music track (while it's playing)"
	if (pan != -101) {
		adjustPan(pan, delaySeconds);
	}
}

bool Music::isPlaying() {
	return _channel >= 0 && _isPlaying;
}

// TODO Evaluate if for ScummVM we can avoid using and modifying _musicVolumeFactorOriginalEngine altogether.
// While we no longer use the original engine's mechanism to set the music tracks' volume
// with the Music::setVolume() public method (when using the in-game KIA volume slider),
// we do use this method as did the original engine to temporarily set the volume levels
// in VK mode.
// volume should be in [0, 100]. If volume < 0 then it's treated as 0, and the track is stopped with 2 seconds fade out.
void Music::setVolume(int volume) {
	// Just don't set the _musicVolumeFactorOriginalEngine to a negative value
	_musicVolumeFactorOriginalEngine = (volume < 0) ? 0 : volume;
	if (_musicVolumeFactorOriginalEngine == 0) {
		stop(2u);
	} else if (isPlaying()) {
		// delay is 2 seconds (multiplied by 60u as expected by AudioMixer::adjustVolume())
		_vm->_audioMixer->adjustVolume(_channel, (_current.volume * _musicVolumeFactorOriginalEngine) / 100, 120u);
	}
}

int Music::getVolume() const {
	return _musicVolumeFactorOriginalEngine;
}

void Music::playSample() {
	if (!isPlaying()) {
		play(_vm->_gameInfo->getSfxTrack(kSfxMUSVOL8), 100, 0, 2, -1, kMusicLoopPlayOnce, 3);
	}
}

void Music::save(SaveFileWriteStream &f) {
	f.writeBool(_isNextPresent);
	f.writeBool(_isPlaying);
	f.writeBool(_isPaused);
	f.writeStringSz(_current.name, 13);
	f.writeInt(_current.volume);
	f.writeInt(_current.pan);
	f.writeInt(_current.timeFadeInSeconds);
	f.writeInt(_current.timePlaySeconds);
	f.writeInt(_current.loop);
	f.writeInt(_current.timeFadeOutSeconds);
	f.writeStringSz(_next.name, 13);
	f.writeInt(_next.volume);
	f.writeInt(_next.pan);
	f.writeInt(_next.timeFadeInSeconds);
	f.writeInt(_next.timePlaySeconds);
	f.writeInt(_next.loop);
	f.writeInt(_next.timeFadeOutSeconds);
}

void Music::load(SaveFileReadStream &f) {
	_isNextPresent = f.readBool();
	_isPlaying = f.readBool();
	_isPaused = f.readBool();
	_current.name = f.readStringSz(13);
	_current.volume = f.readInt();
	_current.pan = f.readInt();
	_current.timeFadeInSeconds = f.readInt();
	_current.timePlaySeconds = f.readInt();
	_current.loop = f.readInt();
	_current.timeFadeOutSeconds = f.readInt();
	_next.name = f.readStringSz(13);
	_next.volume = f.readInt();
	_next.pan = f.readInt();
	_next.timeFadeInSeconds = f.readInt();
	_next.timePlaySeconds = f.readInt();
	_next.loop = f.readInt();
	_next.timeFadeOutSeconds = f.readInt();

	stop(2u);
	if (_isPlaying) {
		if (_channel == -1) {
			play(_current.name,
				_current.volume,
				_current.pan,
				_current.timeFadeInSeconds,
				_current.timePlaySeconds,
				_current.loop,
				_current.timeFadeOutSeconds);
		} else {
			_isNextPresent = true;
			_next.name = _current.name;
			_next.volume = _current.volume;
			_next.pan = _current.pan;
			_next.timeFadeInSeconds = _current.timeFadeInSeconds;
			_next.timePlaySeconds = _current.timePlaySeconds;
			_next.loop = _current.loop;
			_next.timeFadeOutSeconds = _current.timeFadeOutSeconds;
		}
	}
}

// volume should be in [0, 100]
#if BLADERUNNER_ORIGINAL_BUGS
void Music::adjustVolume(int adjustedVolume, uint32 delaySeconds) {
	// adjustVolume() takes an "adjusted volume" value as an argument
	// We don't store that as the target _current.volume. Music::play() stores the proper value
	// However, adjustVolume() is called from Music::play() but also from Music::adjust() (called by ScriptBase::Music_Adjust()).
	if (_channel >= 0) {
		_vm->_audioMixer->adjustVolume(_channel, adjustedVolume, 60u * delaySeconds);
	}
}
#else
void Music::adjustVolume(int volume, uint32 delaySeconds) {
	// adjustVolume() takes a "target" volume value as an argument
	// We store the new volume value as the _current.volume (which is a target volume).
	// adjustVolume() is called from Music::play() but also from Music::adjust() (called by ScriptBase::Music_Adjust()).
	_current.volume = volume;
	if (_channel >= 0) {
		_vm->_audioMixer->adjustVolume(_channel, (volume * _musicVolumeFactorOriginalEngine) / 100, 60u * delaySeconds);
	}
}
#endif // BLADERUNNER_ORIGINAL_BUGS


// pan should be in [-100, 100]
void Music::adjustPan(int pan, uint32 delaySeconds) {
	// adjustPan() is called from Music::play() but also from Music::adjust().
	// We store the new pan value here as the _current.pan (which is a target pan).
	_current.pan = pan;
	if (_channel >= 0) {
		_vm->_audioMixer->adjustPan(_channel, pan, 60u * delaySeconds);
	}
}

void Music::ended() {
	Common::StackLock lock(_mutex);

	_isPlaying = false;
	_channel = -1;

	delete[] _data;
	_data = nullptr;

	// The timer that checks for a next track is started here.
	// When it expires, it should check for queued music (_isNextPresent) or looping music (_current.loop)
#if BLADERUNNER_ORIGINAL_BUGS
	_vm->getTimerManager()->installTimerProc(timerCallbackNext, 100 * 1000, this, "BladeRunnerMusicNextTimer");
#else
	_vm->_audioMixer->startAppTimerProc(kAudioMixerAppTimerMusicNext, 100u);
#endif // BLADERUNNER_ORIGINAL_BUGS
}

void Music::fadeOut() {
#if BLADERUNNER_ORIGINAL_BUGS
	_vm->getTimerManager()->removeTimerProc(timerCallbackFadeOut);
#else
	_vm->_audioMixer->stopAppTimerProc(kAudioMixerAppTimerMusicFadeOut);
#endif // BLADERUNNER_ORIGINAL_BUGS
	if (_channel >= 0) {
		if (_current.timeFadeOutSeconds < 0) {
			_current.timeFadeOutSeconds = 0;
		}
		_vm->_audioMixer->stop(_channel, 60u * _current.timeFadeOutSeconds);
	}
}

#if BLADERUNNER_ORIGINAL_BUGS
void Music::timerCallbackFadeOut(void *refCon) {
	((Music *)refCon)->fadeOut();
}

void Music::timerCallbackNext(void *refCon) {
	((Music *)refCon)->next();
}

void Music::next() {
	_vm->getTimerManager()->removeTimerProc(timerCallbackNext);
	if (_isNextPresent) {
		if (_isPaused) {
			// postpone loading the next track (re-arm the BladeRunnerMusicNextTimer timer)
			_vm->getTimerManager()->installTimerProc(timerCallbackNext, 2000 * 1000, this, "BladeRunnerMusicNextTimer");
		} else {
			play(_next.name.c_str(), _next.volume, _next.pan, _next.timeFadeInSeconds, _next.timePlaySeconds, _next.loop, _next.timeFadeOutSeconds);
		}
		// This should not come after a possible call to play() which could swap the "_current" for the new (_next) track
		// Setting the loop to 0 here, would then make the new track non-looping, even if it is supposed to be looping
		_current.loop = 0;
	} else if (_current.loop) {
		play(_current.name.c_str(), _current.volume, _current.pan, _current.timeFadeInSeconds, _current.timePlaySeconds, _current.loop, _current.timeFadeOutSeconds);
	}
}
#else
void Music::next() {
	_vm->_audioMixer->stopAppTimerProc(kAudioMixerAppTimerMusicNext);
	if (_isNextPresent) {
		if (_isPaused) {
			// postpone loading the next track (re-arm the BladeRunnerMusicNextTimer timer)
			_vm->_audioMixer->startAppTimerProc(kAudioMixerAppTimerMusicNext, 2000u);
			_current.loop = 0;
		} else {
			_current.loop = 0;
			play(_next.name.c_str(), _next.volume, _next.pan, _next.timeFadeInSeconds, _next.timePlaySeconds, _next.loop, _next.timeFadeOutSeconds);
		}
	} else if (_current.loop) {
		play(_current.name.c_str(), _current.volume, _current.pan, _current.timeFadeInSeconds, _current.timePlaySeconds, _current.loop, _current.timeFadeOutSeconds);
	}
}
#endif // BLADERUNNER_ORIGINAL_BUGS

void Music::mixerChannelEnded(int channel, void *data) {
	if (data != nullptr) {
		((Music *)data)->ended();
	}
}

byte *Music::getData(const Common::String &name) {
	// NOTE: This is not part original game, loading data is done in the mixer and its using buffering to limit memory usage
	Common::SeekableReadStream *stream = _vm->getResourceStream(_vm->_enhancedEdition ? ("audio/" + name) : name);

	if (stream == nullptr) {
		return nullptr;
	}

	uint32 size = stream->size();
	byte *data = new byte[size];
	stream->read(data, size);

	delete stream;

	return data;
}

} // End of namespace BladeRunner
