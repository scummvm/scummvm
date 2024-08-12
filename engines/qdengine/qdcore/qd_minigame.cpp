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

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/input/mouse_input.h"
#include "qdengine/system/input/keyboard_input.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_minigame.h"
#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/qdcore/qd_minigame_interface.h"
#include "qdengine/qdcore/qd_engine_interface.h"

#include "qdengine/minigames/qd_empty_minigame_interface.h"
#include "qdengine/minigames/book_all.h"
#include "qdengine/minigames/puzzle_all.h"


namespace QDEngine {

qdMiniGame::qdMiniGame() : _dll_handle(NULL),
	_interface(NULL) {
}

qdMiniGame::qdMiniGame(const qdMiniGame &mg) : qdNamedObject(mg),
	_dll_name(mg._dll_name),
	_dll_handle(mg._dll_handle),
	_interface(mg._interface),
	_config(mg._config) {
}

qdMiniGame::~qdMiniGame() {
}

bool qdMiniGame::start() {
	if (load_interface()) {
		return _interface->init(&qdmg::qdEngineInterfaceImpl::instance());
	}

	return false;
}

bool qdMiniGame::is_started() const {
	return _interface != 0;
}

bool qdMiniGame::quant(float dt) {
	if (_interface)
		_interface->quant(dt);
	return true;
}

bool qdMiniGame::end() {
	if (_interface)
		_interface->finit();

	return release_interface();
}

int qdMiniGame::save_game(char *buffer, int buffer_size, qdGameScene *scene) {
	bool need_release = false;
	if (!is_started()) {
		if (!load_interface()) return 0;
		need_release = true;
	}

	qdMinigameSceneInterface *scene_int = qdmg::qdEngineInterfaceImpl::instance().scene_interface(scene);
	int size = _interface->save_game(&qdmg::qdEngineInterfaceImpl::instance(), scene_int, buffer, buffer_size);
	qdmg::qdEngineInterfaceImpl::instance().release_scene_interface(scene_int);

	if (need_release)
		release_interface();

	return size;
}

int qdMiniGame::load_game(const char *buffer, int buffer_size, qdGameScene *scene) {
	bool need_release = false;
	if (!is_started()) {
		if (!load_interface()) return 0;
		need_release = true;
	}

	qdMinigameSceneInterface *scene_int = qdmg::qdEngineInterfaceImpl::instance().scene_interface(scene);
	int size = _interface->load_game(&qdmg::qdEngineInterfaceImpl::instance(), scene_int, buffer, buffer_size);
	qdmg::qdEngineInterfaceImpl::instance().release_scene_interface(scene_int);

	if (need_release)
		release_interface();

	return size;
}

bool qdMiniGame::load_script(const xml::tag *p) {
	int config_size = 0;
	for (xml::tag::subtag_iterator it1 = p->subtags_begin(); it1 != p->subtags_end(); ++it1) {
		if (it1->ID() == QDSCR_MINIGAME_CONFIG_PARAMETER)
			config_size++;
	}

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_MINIGAME_DLL_NAME:
			set_dll_name(it->data());
			break;
		case QDSCR_MINIGAME_GAME_NAME:
			set_game_name(it->data());
			break;
		case QDSCR_MINIGAME_CONFIG_FILE:
			set_config_file_name(it->data());
			load_config();
			_config.reserve(_config.size() + config_size);
			break;
		case QDSCR_MINIGAME_CONFIG_PARAMETER: {
			if (!qdGameConfig::get_config().minigame_read_only_ini()) {
				qdMinigameConfigParameter prm;
				prm.load_script(&*it);
				config_container_t::iterator cfg_it = Common::find(_config.begin(), _config.end(), prm);
				if (cfg_it != _config.end()) {
					cfg_it->set_data_string(prm.data_string());
					cfg_it->set_data_count(prm.data_count());
				}
			}
		}
		break;
		}
	}

	return true;
}

bool qdMiniGame::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<minigame");

	if (name()) {
		fh.writeString(Common::String::format(" name=\"%s\"", qdscr_XML_string(name())));
	}

	if (flags()) {
		fh.writeString(Common::String::format(" flags=\"%d\"", flags()));
	}

	if (!_config_file_name.empty()) {
		fh.writeString(Common::String::format(" config_file=\"%s\"", qdscr_XML_string(config_file_name())));
	}

	if (!_dll_name.empty()) {
		fh.writeString(Common::String::format(" dll_name=\"%s\"", qdscr_XML_string(dll_name())));
	}

	if (!_game_name.empty()) {
		fh.writeString(Common::String::format(" game_name=\"%s\"", qdscr_XML_string(game_name())));
	}

	if (!_config.empty()) {
		fh.writeString(">\r\n");

		for (auto &it: _config) {
			it.save_script(fh, indent + 1);
		}

		for (int i = 0; i < indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString("</minigame>\r\n");
	} else {
		fh.writeString("/>\r\n");
	}

	return true;
}

qdMiniGame &qdMiniGame::operator = (const qdMiniGame &mg) {
	if (this == &mg) return *this;

	*static_cast<qdNamedObject *>(this) = mg;

	_dll_name = mg._dll_name;
	_dll_handle = mg._dll_handle;
	_interface = mg._interface;

	_config_file_name = mg._config_file_name;
	_config = mg._config;

	return *this;
}

bool qdMiniGame::init() {
	if (load_interface()) {
		_interface->new_game(&qdmg::qdEngineInterfaceImpl::instance());
		release_interface();
		return true;
	}
	return false;
}

bool qdMiniGame::load_config() {
	if (_config_file_name.empty())
		return false;

	_config.clear();
	Common::INIFile::SectionList section_list;
	enumerateIniSections(config_file_name(), section_list);

	_config.reserve(section_list.size());

	for (auto &it : section_list) {
		qdMinigameConfigParameter prm;
		prm.load_ini(config_file_name(), it.name.c_str());
		_config.push_back(prm);
	}

	return true;
}

bool qdMiniGame::get_files_list(qdFileNameList &files_to_copy, qdFileNameList &files_to_pack) const {
	if (has_dll())
		files_to_copy.push_back(dll_name());

	if (has_config_file())
		files_to_copy.push_back(config_file_name());

	for (config_container_t::const_iterator it = _config.begin(); it != _config.end(); ++it) {
		if (it->data_type() == qdMinigameConfigParameter::PRM_DATA_FILE)
			files_to_copy.push_back(it->data_string());
	}

	return true;
}

const char *qdMiniGame::config_parameter_value(const char *cfg_param_name) const {
	config_container_t::const_iterator it = Common::find(_config.begin(), _config.end(), cfg_param_name);
	if (it != _config.end())
		return it->data_string();
	return NULL;
}

bool qdMiniGame::load_interface() {
	if (!_dll_name.empty()) {
		if (_dll_name == "DLL\\Book_gusenica.dll" || _dll_name == "DLL\\Book_les.dll"
				|| _dll_name == "DLL\\Book_buhta.dll") {
			_interface = new qdBookAllMiniGameInterface(_dll_name, g_engine->getLanguage());
			return true;
		} else if (_dll_name == "DLL\\Puzzle_ep01.dll" || _dll_name == "DLL\\Puzzle_ep02.dll"
				|| _dll_name == "DLL\\Puzzle_ep07.dll" || _dll_name == "DLL\\Puzzle_ep08.dll") {
			_interface = new qdPuzzleAllMiniGameInterface(_dll_name, g_engine->getLanguage());
			return true;
		} else {
			warning("STUB: Trying to load dll: %s", _dll_name.c_str());
			// call here dll->open_game_interface(game_name())
			_interface = new qdEmptyMiniGameInterface;
			return true;
		}
	}
	return false;
}

bool qdMiniGame::release_interface() {
	return true;
}
} // namespace QDEngine
