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

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {


class qdBookAllMiniGameInterface : public qdMiniGameInterface {
public:
	~qdBookAllMiniGameInterface() { };

	//! Инициализация игры.
	bool init(const qdEngineInterface *engine_interface) {
		_engine = engine_interface;
		_scene = engine_interface->current_scene_interface();
		if (!_scene)
			return 0;

		_artObject = _scene->object_interface("art");
		_startReading = _scene->object_interface("$start_reading");
		_recordPlayer = _scene->object_interface("\xe3\xf0\xe0\xec\xee\xf4\xee\xed"); // "грамофон"

		_artTimeStamps[95]  = 0.0;
		_artTimeStamps[96]  = 0.2;
		_artTimeStamps[97]  = 0.45899999;
		_artTimeStamps[98]  = 0.95899999;
		_artTimeStamps[99]  = 1.21;
		_artTimeStamps[100] = 1.627;
		_artTimeStamps[101] = 2.628;
		_artTimeStamps[102] = 2.836;
		_artTimeStamps[103] = 3.0450001;
		_artTimeStamps[104] = 3.8369999;
		_artTimeStamps[105] = 3.9619999;
		_artTimeStamps[106] = 4.2540002;
		_artTimeStamps[107] = 4.493;
		_artTimeStamps[108] = 4.796;
		_artTimeStamps[109] = 4.8800001;
		_artTimeStamps[110] = 5.4219999;
		_artTimeStamps[111] = 6.6729999;
		_artTimeStamps[112] = 7.2989998;
		_artTimeStamps[113] = 7.4660001;
		_artTimeStamps[114] = 8.1330004;
		_artTimeStamps[115] = 8.8000002;
		_artTimeStamps[116] = 9.927;
		_artTimeStamps[117] = 10.719;
		_artTimeStamps[118] = 11.303;
		_artTimeStamps[119] = 11.637;
		_artTimeStamps[120] = 12.512;
		_artTimeStamps[121] = 13.388;
		_artTimeStamps[122] = 13.68;
		_artTimeStamps[123] = 13.972;
		_artTimeStamps[124] = 14.306;
		_artTimeStamps[125] = 15.432;
		_artTimeStamps[126] = 15.557;
		_artTimeStamps[127] = 15.807;
		_artTimeStamps[128] = 16.308001;
		_artTimeStamps[129] = 16.683001;
		_artTimeStamps[130] = 16.892;
		_artTimeStamps[131] = 17.768;
		_artTimeStamps[132] = 18.559999;
		_artTimeStamps[133] = 19.186001;
		_artTimeStamps[134] = 20.145;
		_artTimeStamps[135] = 20.437;
		_artTimeStamps[136] = 20.854;
		_artTimeStamps[137] = 21.146;
		_artTimeStamps[138] = 21.98;
		_artTimeStamps[139] = 22.188999;
		_artTimeStamps[140] = 22.856001;
		_artTimeStamps[141] = 23.732;
		_artTimeStamps[142] = 24.399;
		_artTimeStamps[143] = 25.108;
		_artTimeStamps[144] = 25.4;
		_artTimeStamps[145] = 25.608999;
		_artTimeStamps[146] = 26.108999;
		_artTimeStamps[147] = 26.568001;
		_artTimeStamps[148] = 27.277;
		_artTimeStamps[149] = 28.32;
		_artTimeStamps[150] = 28.487;
		_artTimeStamps[151] = 29.028999;
		_artTimeStamps[152] = 29.195999;
		_artTimeStamps[153] = 29.363001;
		_artTimeStamps[154] = 29.863001;
		_artTimeStamps[155] = 30.405001;
		_artTimeStamps[156] = 30.614;
		_artTimeStamps[157] = 32.074001;
		_artTimeStamps[158] = 32.616001;
		_artTimeStamps[159] = 32.950001;
		_artTimeStamps[160] = 33.825001;
		_artTimeStamps[161] = 34.117001;
		_artTimeStamps[162] = 34.618;
		_artTimeStamps[163] = 35.285;
		_artTimeStamps[164] = 36.535999;
		_artTimeStamps[165] = 36.744999;
		_artTimeStamps[166] = 37.203999;
		_artTimeStamps[167] = 37.703999;
		_artTimeStamps[168] = 38.287998;
		_artTimeStamps[169] = 38.789001;
		_artTimeStamps[170] = 39.039001;
		_artTimeStamps[171] = 40.374001;
		_artTimeStamps[172] = 40.874001;
		_artTimeStamps[173] = 41.25;
		_artTimeStamps[174] = 42.084;
		_artTimeStamps[175] = 43.125999;
		_artTimeStamps[176] = 43.459999;
		_artTimeStamps[177] = 44.585999;
		_artTimeStamps[178] = 45.044998;
		_artTimeStamps[179] = 45.544998;
		_artTimeStamps[180] = 45.879002;
		_artTimeStamps[181] = 46.588001;
		_artTimeStamps[182] = 46.880001;
		_artTimeStamps[183] = 47.130001;
		_artTimeStamps[184] = 47.547001;
		_artTimeStamps[185] = 48.382;
		_artTimeStamps[186] = 49.882999;
		_artTimeStamps[187] = 50.466999;
		_artTimeStamps[188] = 51.008999;

		_artTimeStamps[190] = 0.0;
		_artTimeStamps[191] = 0.027000001;
		_artTimeStamps[192] = 0.68599999;
		_artTimeStamps[193] = 1.2079999;
		_artTimeStamps[194] = 1.51;
		_artTimeStamps[195] = 1.702;
		_artTimeStamps[196] = 2.471;
		_artTimeStamps[197] = 2.677;
		_artTimeStamps[198] = 3.267;
		_artTimeStamps[199] = 3.405;
		_artTimeStamps[200] = 3.9400001;
		_artTimeStamps[201] = 4.5170002;
		_artTimeStamps[202] = 5.9860001;
		_artTimeStamps[203] = 6.5209999;
		_artTimeStamps[204] = 6.6999998;
		_artTimeStamps[205] = 6.9879999;
		_artTimeStamps[206] = 7.1529999;
		_artTimeStamps[207] = 7.3039999;
		_artTimeStamps[208] = 7.8800001;
		_artTimeStamps[209] = 8.3059998;
		_artTimeStamps[210] = 8.7729998;
		_artTimeStamps[211] = 9.8299999;
		_artTimeStamps[212] = 10.53;
		_artTimeStamps[213] = 11.038;
		_artTimeStamps[214] = 11.381;
		_artTimeStamps[215] = 12.205;
		_artTimeStamps[216] = 12.576;
		_artTimeStamps[217] = 13.056;
		_artTimeStamps[218] = 14.429;
		_artTimeStamps[219] = 14.709;
		_artTimeStamps[220] = 15.121;
		_artTimeStamps[221] = 15.725;
		_artTimeStamps[222] = 16.0;
		_artTimeStamps[223] = 16.892;
		_artTimeStamps[224] = 17.249001;
		_artTimeStamps[225] = 17.976999;
		_artTimeStamps[226] = 19.212;
		_artTimeStamps[227] = 19.487;
		_artTimeStamps[228] = 19.665001;
		_artTimeStamps[229] = 19.747999;
		_artTimeStamps[230] = 20.021999;
		_artTimeStamps[231] = 21.464001;
		_artTimeStamps[232] = 22.027;
		_artTimeStamps[233] = 22.316999;
		_artTimeStamps[234] = 22.74;
		_artTimeStamps[235] = 22.799999;
		_artTimeStamps[236] = 23.468;
		_artTimeStamps[237] = 23.899;
		_artTimeStamps[238] = 24.284;
		_artTimeStamps[239] = 25.176001;
		_artTimeStamps[240] = 25.862;
		_artTimeStamps[241] = 26.027;
		_artTimeStamps[242] = 26.726999;
		_artTimeStamps[243] = 26.906;
		_artTimeStamps[244] = 27.441;
		_artTimeStamps[245] = 27.565001;
		_artTimeStamps[246] = 28.045;
		_artTimeStamps[247] = 28.209999;
		_artTimeStamps[248] = 29.184999;
		_artTimeStamps[249] = 29.473;
		_artTimeStamps[250] = 29.802999;
		_artTimeStamps[251] = 30.75;
		_artTimeStamps[252] = 30.955999;
		_artTimeStamps[253] = 31.532;
		_artTimeStamps[254] = 32.191002;
		_artTimeStamps[255] = 32.521;
		_artTimeStamps[256] = 33.605;
		_artTimeStamps[257] = 33.984001;
		_artTimeStamps[258] = 34.450001;
		_artTimeStamps[259] = 34.751999;
		_artTimeStamps[260] = 34.945;
		_artTimeStamps[261] = 35.314999;
		_artTimeStamps[262] = 35.452999;
		_artTimeStamps[263] = 35.617001;
		_artTimeStamps[264] = 35.837002;
		_artTimeStamps[265] = 36.646999;
		_artTimeStamps[266] = 37.004002;
		_artTimeStamps[267] = 37.594002;
		_artTimeStamps[268] = 37.882999;
		_artTimeStamps[269] = 38.075001;
		_artTimeStamps[270] = 38.638;
		_artTimeStamps[271] = 38.995998;

		_pageDurations[1] = 0.0;
		_pageDurations[1] = 51.84;
		_pageDurations[2] = 39.832001;

		_pageNum = 0;
		_playbackOn = 0;

		return true;
	}

	//! Обсчёт логики игры, параметр - время, которое должно пройти в игре (в секундах).
	bool quant(float dt) {
		if (!_playbackOn) {
			if (_startReading->is_state_active("page1")) {
				_pageNum = 1;
				_startReading->set_state("reading_page1");
				_currentPageArt = 1;
				_time = 0.0;
				_playbackOn = true;
				_totalPageArts = 93;
			} else if (_startReading->is_state_active("page2")) {
				warning("PAGE2");
				_pageNum = 2;
				_startReading->set_state("reading_page2");
				_currentPageArt = 1;
				_time = 0.0;
				_playbackOn = true;
				_totalPageArts = 81;
			}
		}

		if (_playbackOn && _recordPlayer->is_state_active("\xf1\xf2\xe0\xf2\xeb\xea")) { // "статик"
			_playbackOn = false;
			_startReading->set_state("no");
		}

		if (_playbackOn) {
			if (_currentPageArt > _totalPageArts) {
				_time = _time + dt;
				if (_pageDurations[_pageNum] < (double)_time)
					_startReading->set_state("stopping");
			} else {
				_time = _time + dt;
				if (_artTimeStamps[95 * _pageNum + _currentPageArt] <= (double)_time) {
					_artObject->set_state(Common::String::format("page%i_art_%02i", _pageNum, _currentPageArt).c_str());
					++_currentPageArt;
				}
			}
		}

		return true;
	}

	//! Деинициализация игры.
	bool finit() {
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
	float _artTimeStamps[95 * 3];
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
