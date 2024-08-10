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

const float bookGusenica[95 * 2] = {
	 0.000,  0.036,  0.514,  1.154,  1.787,  2.370,  2.862,  4.242,  4.921,  5.979,
	 6.434,  6.820,  7.228,  8.297,  8.963,  9.170,  9.825, 10.698, 11.401, 12.169,
	12.868, 13.416, 14.107, 15.232, 16.679, 17.048, 17.618, 18.119, 19.437, 19.882,
	20.492, 21.583, 22.646, 23.566, 25.343, 25.521, 25.758, 26.371, 26.561, 27.310,
	27.692, 28.216, 28.685, 29.035, 29.596, 30.499, 31.886, 32.309, 32.441, 33.276,
	33.689, 34.260, 34.769, 35.776, 36.569, 37.678, 38.885, 39.007, 39.608, 40.160,
	41.874, 42.118, 42.637, 42.775, 43.555, 43.949, 44.187, 44.761, 45.475, 45.861,
	47.240, 47.428, 47.639, 48.227, 48.746, 49.456, 49.690, 50.298, 50.961, 51.173,
	51.770, 52.395, 52.937,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,

	 0.000,  0.665,  1.136,  1.633,  2.600,  2.973,  3.558,  3.996,  4.657,  6.140,
	 7.132,  8.223,  9.114,  9.288, 10.437, 10.624, 11.307, 11.479, 11.913, 12.505,
	13.162, 13.287, 14.580, 15.192, 16.004, 16.552, 17.128, 17.805, 19.214, 19.860,
	20.571, 22.766, 23.256, 24.098, 24.888, 26.421, 26.786, 26.893, 27.351, 27.804,
	28.530, 29.882, 30.415, 31.506, 31.614, 32.150, 33.647, 34.333, 34.811, 35.659,
	35.809, 36.192, 36.612, 37.062, 37.756, 39.359, 40.266, 41.407, 41.828, 41.901,
	42.447, 43.290, 43.980, 45.047, 46.263, 46.407, 47.836, 48.311, 49.430, 49.752,
	 0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,
};

const float bookGusenicaCZ[95 * 2] = {
	 0.000,  0.075,  0.372,  1.112,  1.399,  1.851,  1.975,  2.232,  3.738,  3.987,
	 4.133,  4.479,  5.108,  5.316,  5.754,  6.463,  7.273,  7.406,  7.636,  8.092,
	 8.797,  9.319,  9.833, 10.238, 10.968, 11.075, 11.876, 12.620, 14.243, 14.438,
	15.089, 15.530, 16.004, 16.088, 17.014, 17.328, 17.962, 18.477, 18.561, 19.695,
	19.965, 20.253, 20.585, 20.939, 21.223, 21.577, 22.161, 22.369, 22.950, 24.185,
	24.474, 24.797, 25.107, 25.678, 26.258, 27.228, 27.990, 28.158, 29.028, 29.134,
	29.688, 30.675, 31.189, 31.627, 31.769, 32.035, 32.455, 32.741, 33.277, 33.503,
	34.048, 34.442, 34.836, 35.009, 35.629, 36.094, 36.856, 36.993, 37.596, 37.914,
	38.561, 39.106, 39.612,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,

	 0.000,  0.042,  0.500,  0.667,  1.460,  1.627,  1.960,  2.336,  3.587,  3.712,
	 4.004,  4.713,  5.339,  6.215,  6.381,  7.674,  8.091,  8.425,  8.925,  9.676,
	10.052, 10.302, 11.220, 11.595, 12.137, 12.221, 12.721, 13.430, 13.597, 13.722,
	14.097, 15.390, 15.766, 16.558, 16.975, 17.226, 18.352, 19.269, 19.603, 20.646,
	21.355, 21.897, 22.105, 23.065, 23.607, 23.732, 24.316, 25.817, 26.276, 26.568,
	26.944, 27.402, 27.569, 28.403, 28.695, 29.154, 29.279, 30.447, 31.615, 32.032,
	33.075, 33.283, 33.784, 34.660, 34.910, 35.369, 35.661, 36.036, 36.995, 37.329,
	37.996, 38.455, 38.580, 38.997, 39.581, 39.998, 40.290, 40.874,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,
};

const float bookLes[95 * 2] = {
	 0.000,  0.046,  0.577,  0.809,  1.623,  1.985,  2.635,  3.536,  3.642,  4.560,
	 5.071,  6.558,  7.007,  7.876,  8.998,  9.548, 10.387, 10.471, 11.054, 12.062,
	12.081, 12.462, 12.647, 12.900, 13.987, 14.265, 14.809, 15.558, 16.332, 17.643,
	18.000, 19.285, 19.512, 19.867, 20.158, 20.517, 20.822, 21.240, 21.741, 23.193,
	23.704, 24.338, 25.117, 26.058, 26.845, 26.989, 27.455, 28.551, 28.859, 29.181,
	30.094, 30.833, 31.519, 32.497, 33.339, 34.638, 34.723, 35.053, 35.474, 36.280,
	37.317, 38.190, 38.846, 39.988, 40.098, 40.768, 41.784, 42.443, 42.580, 43.299,
	44.911, 45.276, 45.679, 45.935, 46.393, 47.825, 48.345, 49.492, 49.723, 49.915,
	50.109, 50.629, 51.285, 51.536, 52.749, 52.903, 53.364, 54.450, 54.539, 54.998,
	55.853, 56.888, 57.261, 58.080, 58.723,

	 0.000,  0.221,  0.891,  1.479,  2.604,  2.712,  3.110,  3.786,  3.917,  4.529,
	 5.952,  6.322,  6.555,  6.676,  7.046,  7.185,  7.567,  8.643,  8.802,  9.154,
	 9.889, 10.886, 11.069, 11.385, 12.181, 12.403, 12.578, 12.732, 13.013, 14.520,
	14.680, 15.121, 15.278, 16.030, 16.396, 16.897, 18.072, 18.506, 18.687, 19.255,
	19.370, 20.322, 20.484, 20.640, 20.981, 21.615, 21.820, 22.347, 22.901, 23.924,
	24.430, 24.810, 25.187, 26.132, 26.958, 27.109, 28.126, 28.286, 28.851, 29.557,
	31.111, 31.308, 31.858, 32.155, 32.680, 33.127, 33.368, 34.789, 35.089, 35.577,
	35.664, 36.126, 36.316, 36.537, 36.897, 37.138, 38.507, 38.776, 39.316, 39.511,
	39.876, 40.096, 41.040, 41.315, 41.513, 41.688,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,
};

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
	38.638, 38.996,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,
};

const float bookBuhta[95 * 2] = {
	 0.000,  0.284,  0.750,  1.287,  1.965,  2.410,  2.975,  3.819,  4.682,  6.184,
	 6.735,  7.182,  7.918,  8.786, 10.161, 11.182, 11.970, 12.297, 13.910, 14.518,
	15.989, 16.419, 17.332, 18.960, 19.471, 20.762, 21.473, 22.749, 23.452, 24.547,
	26.140, 26.339, 26.598, 27.266, 28.012, 28.609, 29.459, 29.746, 30.216, 31.683,
	32.613, 34.180, 34.987, 35.505, 36.508, 37.457, 39.046, 39.387, 40.471, 40.896,
	41.958, 42.591, 42.847, 43.120,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,

	 0.000,  0.811,  1.190,  1.754,  2.475,  3.367,  5.026,  5.580,  6.202,  6.455,
	 7.050,  8.709,  9.114,  9.551, 10.241, 11.304, 11.936, 13.122, 13.897, 15.227,
	16.105, 16.782, 17.773, 19.608, 20.655, 21.015, 21.520, 22.579, 24.167, 24.942,
	25.727, 27.149, 28.267, 28.736, 29.372, 30.030, 30.841, 31.751, 32.170, 32.743,
	33.234, 34.740, 35.880, 36.294, 37.218, 38.147, 38.543, 39.309, 40.896, 41.469,
	42.046, 42.461, 42.944, 43.079, 44.093, 45.324, 46.062, 47.153, 47.798, 48.970,
	49.612, 50.230, 51.343, 51.780, 52.083, 52.326, 52.728, 53.143, 54.139, 55.112,
	 0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,  0.000,
	 0.000,  0.000,  0.000,  0.000,  0.000,
};

class qdBookAllMiniGameInterface : public qdMiniGameInterface {
public:
	qdBookAllMiniGameInterface(Common::String dll, Common::Language language) : _dll(dll), _language(language) {}
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

		_pageDurations[0] = 0.0;
		_totalPageArts[0] = 0;

		if (_dll == "DLL\\Book_gusenica.dll") {
			if (_language == Common::CS_CZE) {
				_artTimeStamps = bookGusenicaCZ;
				_pageDurations[1] = 40.607;
				_pageDurations[2] = 41.854;

				_totalPageArts[1] = 82;
				_totalPageArts[2] = 77;
			} else {
				_artTimeStamps = bookGusenica;
				_pageDurations[1] = 54.300;
				_pageDurations[2] = 50.400;

				_totalPageArts[1] = 82;
				_totalPageArts[2] = 69;
			}
		} else if (_dll == "DLL\\Book_les.dll") {
			if (_language == Common::CS_CZE) {
				_artTimeStamps = bookLesCZ;
				_pageDurations[1] = 51.84;
				_pageDurations[2] = 39.832;

				_totalPageArts[1] = 93;
				_totalPageArts[2] = 81;
			} else {
				_artTimeStamps = bookLes;
				_pageDurations[1] = 59.809;
				_pageDurations[2] = 42.30;

				_totalPageArts[1] = 94;
				_totalPageArts[2] = 85;
			}
		} else if (_dll == "DLL\\Book_buhta.dll") {
			if (_language == Common::CS_CZE) {
				_artTimeStamps = bookLesCZ;
				_pageDurations[1] = 0.0;
				_pageDurations[2] = 0.0;

				_totalPageArts[1] = 0;
				_totalPageArts[2] = 0;
			} else {
				_artTimeStamps = bookBuhta;
				_pageDurations[1] = 43.913;
				_pageDurations[2] = 55.698;

				_totalPageArts[1] = 53;
				_totalPageArts[2] = 69;
			}
		}

#if 0
		const int pageSize = 70;
		float artTimeStamps[pageSize * 3];
		memset(artTimeStamps, 0, sizeof(artTimeStamps));

		for (int i = 0; i < pageSize; i++) {
			debugN("%2.3f, ", artTimeStamps[pageSize + i]);

			if ((i + 1) % 10 == 0)
				debug("");
		}
		debug("");

		for (int i = 0; i < pageSize; i++) {
			debugN("%2.3f, ", artTimeStamps[pageSize * 2 + i]);

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
				dt, _playbackOn, _playbackOn, _currentPageArt, _totalPageArts[_pageNum], _time);

		if (!_playbackOn) {
			if (_startReading->is_state_active("page1")) {
				debugC(1, kDebugMinigames, "BookAll::quant(). Resetting to PAGE1");
				_pageNum = 1;
				_startReading->set_state("reading_page1");
				_currentPageArt = 1;
				_time = 0.0;
				_playbackOn = true;
			} else if (_startReading->is_state_active("page2")) {
				debugC(1, kDebugMinigames, "BookAll::quant(). Resetting to PAGE2");
				_pageNum = 2;
				_startReading->set_state("reading_page2");
				_currentPageArt = 1;
				_time = 0.0;
				_playbackOn = true;
			}
		}

		if (_playbackOn && _recordPlayer->is_state_active("\xf1\xf2\xe0\xf2\xe8\xea")) { // "статик"
			debugC(1, kDebugMinigames, "BookAll::quant(). Stopping playback externally");
			_playbackOn = false;
			_startReading->set_state("no");
		}

		if (_playbackOn) {
			if (_currentPageArt > _totalPageArts[_pageNum]) {
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
	int _totalPageArts[3];
	float _time = 0.0;

	Common::String _dll;
	Common::Language _language;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_BOOK_LES_H
