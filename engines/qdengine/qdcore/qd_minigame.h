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

#ifndef QDENGINE_QDCORE_QD_MINIGAME_H
#define QDENGINE_QDCORE_QD_MINIGAME_H

#include "qdengine/qdcore/qd_named_object.h"
#include "qdengine/qdcore/qd_minigame_config.h"


namespace QDEngine {

class qdMiniGameInterface;
class qdEmptyMiniGameInterface;

//! Мини-игра.
class qdMiniGame : public qdNamedObject {
public:
	qdMiniGame();
	qdMiniGame(const qdMiniGame &mg);
	~qdMiniGame();

	qdMiniGame &operator = (const qdMiniGame &mg);

	int named_object_type() const {
		return QD_NAMED_OBJECT_MINIGAME;
	}

	//! Инициализация данных, вызывается при старте и перезапуске основной игры.
	bool init();

	const Common::Path config_file_name() const {
		return _config_file_name;
	}
	void set_config_file_name(const Common::Path file_name) {
		_config_file_name = file_name;
	}
	bool has_config_file() const {
		return !_config_file_name.empty();
	}

	//! Старт игры, вызывается при заходе на сцену, которой управляет игра.
	bool start();
	bool is_started() const;
	//! Логический квант игры, параметр - время, которое должно пройти в игре (в секундах).
	bool quant(float dt);
	//! Окончание игры, вызывается при уходе со сцены, которая управляется игрой.
	bool end();

	/// Сохранение, вызывается при сохранении сцены \a scene
	int save_game(char *buffer, int buffer_size, qdGameScene *scene);
	/// Загрузка, вызывается при загрузке сцены \a scene
	int load_game(const char *buffer, int buffer_size, qdGameScene *scene);

	//! Возвращает имя подгружаемой для игры dll.
	const char *dll_name() const {
		return _dll_name.c_str();
	}
	//! Устанавливает имя подгружаемой для игры dll.
	void set_dll_name(const char *p) {
		_dll_name = p;
	}
	bool has_dll() const {
		return !_dll_name.empty();
	}

	const char *game_name() const {
		return _game_name.c_str();
	}
	void set_game_name(const char *p) {
		_game_name = p;
	}

	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag *p);
	//! Запись данных в скрипт.
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	typedef Std::vector<qdMinigameConfigParameter> config_container_t;

	const config_container_t &config() const {
		return _config;
	}
	void set_config(const config_container_t &cfg) {
		_config = cfg;
	}
	bool load_config();

	/// Возвращает значение параметра с именем cfg_param_name.
	/**
	Если параметр с таким именем не найден, то возвращает 0.
	*/
	const char *config_parameter_value(const char *cfg_param_name) const;

private:

	//! Имя подгружаемой для игры dll.
	Common::String _dll_name;
	//! .ini файл с настройками игры.
	Common::Path _config_file_name;
	//! Имя игры, по которому она ищется в dll.
	Common::String _game_name;
	//! Хэндл подгруженной dll.
	void *_dll_handle;
	//! Интерфейс к игре из dll.
	qdMiniGameInterface *_interface;

	//! Настройки игры.
	config_container_t _config;

	bool load_interface();
	bool release_interface();
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_MINIGAME_H
