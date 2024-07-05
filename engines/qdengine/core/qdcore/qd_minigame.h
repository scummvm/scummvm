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

#ifndef QDENGINE_CORE_QDCORE_QD_MINIGAME_H
#define QDENGINE_CORE_QDCORE_QD_MINIGAME_H

#include "qdengine/core/qdcore/qd_named_object.h"
#include "qdengine/core/qdcore/qd_minigame_config.h"


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

	const char *config_file_name() const {
		return config_file_name_.c_str();
	}
	void set_config_file_name(const char *file_name) {
		config_file_name_ = file_name;
	}
	bool has_config_file() const {
		return !config_file_name_.empty();
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
		return dll_name_.c_str();
	}
	//! Устанавливает имя подгружаемой для игры dll.
	void set_dll_name(const char *p) {
		dll_name_ = p;
	}
	bool has_dll() const {
		return !dll_name_.empty();
	}

	const char *game_name() const {
		return game_name_.c_str();
	}
	void set_game_name(const char *p) {
		game_name_ = p;
	}

	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag *p);
	//! Запись данных в скрипт.
	bool save_script(Common::SeekableWriteStream &fh, int indent = 0) const;

	//! Добавляет в передаваемые списки имена файлов, необходимых для миниигры.
	bool get_files_list(qdFileNameList &files_to_copy, qdFileNameList &files_to_pack) const;

#ifdef _QUEST_EDITOR
	typedef std::list<qdMinigameConfigParameter> config_container_t;
#else
	typedef std::vector<qdMinigameConfigParameter> config_container_t;
#endif

	const config_container_t &config() const {
		return config_;
	}
	void set_config(const config_container_t &cfg) {
		config_ = cfg;
	}
	bool load_config();

	/// Возвращает значение параметра с именем cfg_param_name.
	/**
	Если параметр с таким именем не найден, то возвращает 0.
	*/
	const char *config_parameter_value(const char *cfg_param_name) const;

private:

	//! Имя подгружаемой для игры dll.
	std::string dll_name_;
	//! .ini файл с настройками игры.
	std::string config_file_name_;
	//! Имя игры, по которому она ищется в dll.
	std::string game_name_;
	//! Хэндл подгруженной dll.
	void *dll_handle_;
	//! Интерфейс к игре из dll.
	qdMiniGameInterface *interface_;
	qdEmptyMiniGameInterface *_emptyInterface;

	//! Настройки игры.
	config_container_t config_;

	bool load_interface();
	bool release_interface();
};

} // namespace QDEngine

#endif // QDENGINE_CORE_QDCORE_QD_MINIGAME_H
