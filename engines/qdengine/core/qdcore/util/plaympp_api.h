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


#ifndef __PLAYMPP_API_H__
#define __PLAYMPP_API_H__

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

	bool play(const char *file, bool loop = false, int vol = 256);
	//! Останавливает проигрывание mp+ файла.
	bool stop();
	//! Приостанавливает проигрывание mp+ файла.
	bool pause();
	//! Продолжает проигрывание mp+ файла.
	bool resume();

	//! Возвращает true, если проигрывание музыки разрешено.
	bool is_enabled() const {
		return is_enabled_;
	}
	//! Разрешает проигрывание музыки.
	void enable(bool need_resume = true) {
		is_enabled_ = true;
		if (need_resume) resume();
	}
	//! Запрещает проигрывание музыки.
	void disable() {
		is_enabled_ = false;
		pause();
	}

	mpeg_status_t status() const;
	bool is_playing() const {
		return (status() == MPEG_PLAYING);
	}

	unsigned int volume() const {
		return volume_;
	}
	void set_volume(unsigned int vol);

	static bool init_library(void *dsound_device);
	static void deinit_library();

	static mpegPlayer &instance();

private:

	mpegPlayer();

	//! Music is off when false
	bool is_enabled_;
	bool _paused;

	//! Music volume, [0, 255].
	unsigned int volume_;
	unsigned int cur_track_volume_;

	Audio::SoundHandle _soundHandle;

	Common::SeekableReadStream *_stream = nullptr;
};

} // namespace QDEngine

#endif /* __PLAYMPP_API_H__ */
