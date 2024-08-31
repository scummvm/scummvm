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


#ifndef QDENGINE_QDCORE_UTIL_PLAYMPP_API_H
#define QDENGINE_QDCORE_UTIL_PLAYMPP_API_H

#include "audio/mixer.h"

namespace Common {
class SeekableReadStream;
}

namespace QDEngine {

class MpegSound;

class mpegPlayer {
public:
	//! Состояние.
	enum mpeg_status_t {
		MPEG_STOPPED,
		MPEG_PAUSED,
		MPEG_PLAYING
	};

	~mpegPlayer();

	bool play(const Common::Path file, bool loop = false, int vol = 256);
	bool stop();
	bool pause();
	bool resume();

	bool is_enabled() const {
		return _is_enabled;
	}
	//! Разрешает проигрывание музыки.
	void enable(bool need_resume = true) {
		_is_enabled = true;
		if (need_resume) resume();
	}
	//! Запрещает проигрывание музыки.
	void disable() {
		_is_enabled = false;
		pause();
	}

	mpeg_status_t status() const;
	bool is_playing() const {
		return (status() == MPEG_PLAYING);
	}

	uint32 volume() const {
		return _volume;
	}
	void set_volume(uint32 vol);

	void syncMusicSettings();

	static bool init_library(void *dsound_device);
	static void deinit_library();

	static mpegPlayer &instance();

private:

	mpegPlayer();

	//! Music is off when false
	bool _is_enabled;
	bool _paused;

	//! Music volume, [0, 255].
	uint32 _volume;
	uint32 _cur_track_volume;

	Audio::SoundHandle _soundHandle;

	Common::SeekableReadStream *_stream = nullptr;

	Common::Path _file;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_UTIL_PLAYMPP_API_H
