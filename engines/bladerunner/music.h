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
class SaveFileReadStream;
class SaveFileWriteStream;

class Music {
	struct Track {
		Common::String name;
		int            volume;
		int            pan;
		int32          timeFadeIn;
		int32          timePlay;
		int            loop;
		int32          timeFadeOut;
	};

	BladeRunnerEngine *_vm;

	Common::Mutex _mutex;
	int           _musicVolume;
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

	bool play(const Common::String &trackName, int volume, int pan, int32 timeFadeIn, int32 timePlay, int loop, int32 timeFadeOut);
	void stop(uint32 delay);
	void adjust(int volume, int pan, uint32 delay);
	bool isPlaying();

	void setVolume(int volume);
	int getVolume();
	void playSample();

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);

private:
	void adjustVolume(int volume, uint32 delay);
	void adjustPan(int pan, uint32 delay);

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
