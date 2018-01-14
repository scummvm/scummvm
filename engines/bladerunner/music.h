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

#ifndef BLADERUNNER_MUSIC_H
#define BLADERUNNER_MUSIC_H

#include "common/mutex.h"
#include "common/str.h"

namespace BladeRunner {

class AudStream;
class BladeRunnerEngine;

class Music {
	struct Track {
		Common::String name;
		int            volume;
		int            pan;
		int            timeFadeIn;
		int            timePlay;
		int            loop;
		int            timeFadeOut;
	};

	BladeRunnerEngine *_vm;

	Common::Mutex _mutex;
	int           _musicVolume;
	int           _channel;
	int           _isNextPresent;
	int           _isPlaying;
	int           _isPaused;
	Track         _current;
	Track         _next;
	byte         *_data;
	AudStream    *_stream;

public:
	Music(BladeRunnerEngine *vm);
	~Music();

	bool play(const Common::String &trackName, int volume, int pan, int timeFadeIn, int timePlay, int loop, int timeFadeOut);
	void stop(int delay);
	void adjust(int volume, int pan, int delay);
	bool isPlaying();

	void setVolume(int volume);
	int getVolume();
	void playSample();

private:
	void adjustVolume(int volume, int delay);
	void adjustPan(int pan, int delay);

	void ended();
	void fadeOut();
	void next();

	static void mixerChannelEnded(int channel, void *data);
	static void timerCallbackFadeOut(void *refCon);
	static void timerCallbackNext(void *refCon);

	byte *getData(const Common::String &name);
};

} // End of namespace BladeRunner

#endif
