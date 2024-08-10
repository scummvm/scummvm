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

#ifndef QDENGINE_MINIGAMES_BOOK_ALL_H
#define QDENGINE_MINIGAMES_BOOK_ALL_H

#include "common/debug.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

const float bookLesCZ[95 * 2] = {
	 0.000,  0.200,  0.459,  0.959,  1.210,  1.627,  2.628,  2.836,  3.045,  3.837,
	 3.962,  4.254,  4.493,  4.796,  4.880,  5.422,  6.673,  7.299,  7.466,  8.133,
	 8.800,  9.927, 10.719, 11.303, 11.637, 12.512, 13.388, 13.680, 13.972, 14.306,
	15.432, 15.557, 15.807, 16.308, 16.683, 16.892, 17.768, 18.560, 19.186, 20.145,
	20.437, 20.854, 21.146, 21.980, 22.189, 22.856, 23.732, 24.399, 25.108, 25.400,
	25.609, 26.109, 26.568, 27.277, 28.320, 28.487, 29.029, 29.196, 29.363, 29.863,
	30.405, 30.614, 32.074, 32.616, 32.950, 33.825, 34.117, 34.618, 35.285, 36.536,
	36.745, 37.204, 37.704, 38.288, 38.789, 39.039, 40.374, 40.874, 41.250, 42.084,
	43.126, 43.460, 44.586, 45.045, 45.545, 45.879, 46.588, 46.880, 47.130, 47.547,
	48.382, 49.883, 50.467, 51.009,  0.000,

	 0.000,  0.027,  0.686,  1.208,  1.510,  1.702,  2.471,  2.677,  3.267,  3.405,
	 3.940,  4.517,  5.986,  6.521,  6.700,  6.988,  7.153,  7.304,  7.880,  8.306,
	 8.773,  9.830, 10.530, 11.038, 11.381, 12.205, 12.576, 13.056, 14.429, 14.709,
	15.121, 15.725, 16.000, 16.892, 17.249, 17.977, 19.212, 19.487, 19.665, 19.748,
	20.022, 21.464, 22.027, 22.317, 22.740, 22.800, 23.468, 23.899, 24.284, 25.176,
	25.862, 26.027, 26.727, 26.906, 27.441, 27.565, 28.045, 28.210, 29.185, 29.473,
	29.803, 30.750, 30.956, 31.532, 32.191, 32.521, 33.605, 33.984, 34.450, 34.752,
	34.945, 35.315, 35.453, 35.617, 35.837, 36.647, 37.004, 37.594, 37.883, 38.075,
	38.638, 38.996,  0.000,  0.000,  0.000
};

class qdBookAllMiniGameInterface : public qdMiniGameInterface {
public:
	~qdBookAllMiniGameInterface() { };

	//! Инициализация игры.
	bool init(const qdEngineInterface *engine_interface) {
		debugC(1, kDebugMinigames, "BookAll::init()");

		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return 0;

		_artObject = _scene->object_interface("art");
		_startReading = _scene->object_interface("$start_reading");
		_recordPlayer = _scene->object_interface("\xe3\xf0\xe0\xec\xee\xf4\xee\xed"); // "грамофон"

		_artTimeStamps = bookLesCZ;

		_pageDurations[0] = 0.0;
		_pageDurations[1] = 51.84;		// 59.72
		_pageDurations[2] = 39.832001;  // 42.24

#if 0
		for (int i = 0; i < 95; i++) {
			debugN("%2.3f, ", _artTimeStamps[95 + i]);

			if ((i + 1) % 10 == 0)
				debug("");
		}
		debug("");

		for (int i = 0; i < 95; i++) {
			debugN("%2.3f, ", _artTimeStamps[95 * 2 + i]);

			if ((i + 1) % 10 == 0)
				debug("");
		}
		debug("");
#endif

		_pageNum = 0;
		_playbackOn = 0;

		return true;
	}

	//! Обсчёт логики игры, параметр - время, которое должно пройти в игре (в секундах).
	bool quant(float dt) {
		debugC(3, kDebugMinigames, "BookAll::quant(%f). _playbackOn: %d _playbackOn: %d _currentPageArt: %d _totalPageArts: %d _time: %f",
				dt, _playbackOn, _playbackOn, _currentPageArt, _totalPageArts, _time);

		if (!_playbackOn) {
			if (_startReading->is_state_active("page1")) {
				debugC(1, kDebugMinigames, "BookAll::quant(). Resetting to PAGE1");
				_pageNum = 1;
				_startReading->set_state("reading_page1");
				_currentPageArt = 1;
				_time = 0.0;
				_playbackOn = true;
				_totalPageArts = 93;
			} else if (_startReading->is_state_active("page2")) {
				debugC(1, kDebugMinigames, "BookAll::quant(). Resetting to PAGE2");
				_pageNum = 2;
				_startReading->set_state("reading_page2");
				_currentPageArt = 1;
				_time = 0.0;
				_playbackOn = true;
				_totalPageArts = 81;
			}
		}

		if (_playbackOn && _recordPlayer->is_state_active("\xf1\xf2\xe0\xf2\xe8\xea")) { // "статик"
			debugC(1, kDebugMinigames, "BookAll::quant(). Stopping playback externally");
			_playbackOn = false;
			_startReading->set_state("no");
		}

		if (_playbackOn) {
			if (_currentPageArt > _totalPageArts) {
				_time = _time + dt;
				if (_pageDurations[_pageNum] < (double)_time) {
					debugC(1, kDebugMinigames, "BookAll::quant(). Stopping playback by time");
					_startReading->set_state("stopping");
				}
			} else {
				_time = _time + dt;
				if (_artTimeStamps[95 * (_pageNum - 1) + _currentPageArt] <= (double)_time) {
					_artObject->set_state(Common::String::format("page%i_art_%02i", _pageNum, _currentPageArt).c_str());
					++_currentPageArt;

					debugC(1, kDebugMinigames, "BookAll::quant(). Switching pageArt to %d", _currentPageArt);
				}
			}
		}

		return true;
	}

	//! Деинициализация игры.
	bool finit() {
		debugC(1, kDebugMinigames, "BookAll::finit()");

		if (_scene)  {
			_engine->release_scene_interface(_scene);
			_scene = 0;
		}

 		return true;
	}

	/// Инициализация миниигры, вызывается при старте и перезапуске игры.
	bool new_game(const qdEngineInterface *engine_interface) {
		return true;
	}
	/// Сохранение данных, вызывается при сохранении сцены, на которую повешена миниигра.
	int save_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, char *buffer, int buffer_size) {
		return 0;
	}
	/// Загрузка данных, вызывается при загрузке сцены, на которую повешена миниигра.
	int load_game(const qdEngineInterface *engine_interface, const qdMinigameSceneInterface *scene_interface, const char *buffer, int buffer_size) {
		return 0;
	}

	/// Версия интерфейса игры, трогать не надо.
	enum { INTERFACE_VERSION = 112 };
	int version() const {
		return INTERFACE_VERSION;
	}

private:
	const float *_artTimeStamps;
	float _pageDurations[3];

	const qdEngineInterface *_engine = nullptr;
	qdMinigameSceneInterface *_scene = nullptr;

	qdMinigameObjectInterface *_artObject = nullptr;
	qdMinigameObjectInterface *_startReading = nullptr;
	qdMinigameObjectInterface *_recordPlayer = nullptr;

	int _pageNum = 0;
	bool _playbackOn = false;
	int _currentPageArt = 1;
	int _totalPageArts = 0;
	float _time = 0.0;

};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_BOOK_LES_H
