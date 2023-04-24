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

#ifndef BLADERUNNER_MUSIC_H
#define BLADERUNNER_MUSIC_H

#include "common/mutex.h"
#include "common/str.h"

#include "bladerunner/bladerunner.h" // For BLADERUNNER_ORIGINAL_BUGS and BLADERUNNER_ORIGINAL_SETTINGS symbols

namespace BladeRunner {

class AudStream;
class BladeRunnerEngine;
class SaveFileReadStream;
class SaveFileWriteStream;

class Music {
	struct Track {
		Common::String name;
		int            volume;             // A value in [0, 100] - It is the set (target) volume for the track regardless of fadeIn and fadeOut transitions
		int            pan;                // A value in [-100, 100]. -100 is left, 100 is right and 0 is center - It is the set (target) pan/balance for the track regardless of any ongoing adjustments
		int32          timeFadeInSeconds;  // how long will it take for the track to reach target volume (in seconds)
		int32          timePlaySeconds;    // how long the track will play before starting fading out (in seconds) - uses timeFadeOutSeconds for fadeout
		                                   // -1: Special value for playing the whole track
		int            loop;               // values from enum MusicTrackLoop (see game_constants.h)
		int32          timeFadeOutSeconds; // how long the fade out will be for the track at its end (in seconds)
	};

	BladeRunnerEngine *_vm;

	Common::Mutex _mutex;
	int           _musicVolumeFactorOriginalEngine; // should be in [0, 100]
	int           _channel;
	bool          _isNextPresent;
	bool          _isPlaying;
	bool          _isPaused;
	Track         _current;
	Track         _next;
	byte         *_data;
	AudStream    *_stream;

public:
	Music(BladeRunnerEngine *vm);
	~Music();

	bool play(const Common::String &trackName, int volume, int pan, int32 timeFadeInSeconds, int32 timePlaySeconds, int loop, int32 timeFadeOutSeconds);
	void stop(uint32 delaySeconds);
	void adjust(int volume, int pan, uint32 delaySeconds);
	bool isPlaying();

	void setVolume(int volume);
	int getVolume() const;
	void playSample();

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);

#if !BLADERUNNER_ORIGINAL_BUGS
	// moved to public access
	void fadeOut();
	void next();
#endif // !BLADERUNNER_ORIGINAL_BUGS


private:
	void reset();
#if BLADERUNNER_ORIGINAL_BUGS
	void adjustVolume(int adjustedVolume, uint32 delaySeconds);
#else
	void adjustVolume(int volume, uint32 delaySeconds);
#endif // BLADERUNNER_ORIGINAL_BUGS
	void adjustPan(int pan, uint32 delaySeconds);

	void ended();
#if BLADERUNNER_ORIGINAL_BUGS
	void fadeOut();
	void next();
	static void timerCallbackFadeOut(void *refCon);
	static void timerCallbackNext(void *refCon);
#endif // BLADERUNNER_ORIGINAL_BUGS

	static void mixerChannelEnded(int channel, void *data);

	byte *getData(const Common::String &name);
};

} // End of namespace BladeRunner

#endif
