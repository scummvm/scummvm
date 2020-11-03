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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/misc.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/conf/config_node.h"
#include "ultima/shared/conf/xml_tree.h"
#include "common/config-manager.h"
#include "common/file.h"

namespace Ultima {
namespace Nuvie {

Configuration::Configuration() : _configChanged(false) {
	// Set up keys that will be stored locally, since we don't want them being
	// written out to the ScummVM configuration
	_localKeys["GameType"] = "";
	_localKeys["GameName"] = "";
	_localKeys["GameID"] = "";
	_localKeys["datadir"] = "data";		// This maps to ultima6/ in ultima.dat
	_localKeys["quit"] = "no";
}

Configuration::~Configuration() {
	for (Std::vector<Shared::XMLTree *>::iterator i = _trees.begin();
	        i != _trees.end(); ++i) {
		delete(*i);
	}

	if (_configChanged)
		ConfMan.flushToDisk();
}

bool Configuration::readConfigFile(Std::string fname, Std::string root,
                                   bool readonly) {
	_configFilename = fname;
	Shared::XMLTree *tree = new Shared::XMLTree();

	if (!tree->readConfigFile(fname)) {
		delete tree;
		return false;
	}

	_trees.push_back(tree);
	return true;
}

void Configuration::write() {
	for (Std::vector<Shared::XMLTree *>::iterator i = _trees.begin();
	        i != _trees.end(); ++i) {
		if (!(*i)->isReadonly())
			(*i)->write();
	}
}

void Configuration::clear() {
	for (Std::vector<Shared::XMLTree *>::iterator i = _trees.begin();
	        i != _trees.end(); ++i) {
		delete(*i);
	}
	_trees.clear();
}

void Configuration::value(const Std::string &key, Std::string &ret,
                          const char *defaultvalue) {
	// Check for a .cfg file value in the trees
	for (Std::vector<Shared::XMLTree *>::reverse_iterator i = _trees.rbegin();
	        i != _trees.rend(); ++i) {
		if ((*i)->hasNode(key)) {
			(*i)->value(key, ret, defaultvalue);
			return;
		}
	}

	assert(key.hasPrefix("config/"));
	Std::string k = key.substr(7);

	// Check for local entry
	if (_localKeys.contains(k)) {
		ret = _localKeys[k];
		return;
	}

	// Check for ScummVM entry
	if (_settings.contains(k)) {
		ret = _settings[k];
		return;
	}

	ret = defaultvalue;
}

void Configuration::value(const Std::string &key, int &ret, int defaultvalue) {
	// Check for a .cfg file value in the trees
	for (Std::vector<Shared::XMLTree *>::reverse_iterator i = _trees.rbegin();
	        i != _trees.rend(); ++i) {
		if ((*i)->hasNode(key)) {
			(*i)->value(key, ret, defaultvalue);
			return;
		}
	}

	assert(key.hasPrefix("config/"));
	Std::string k = key.substr(7);

	// Check for local entry
	if (_localKeys.contains(k)) {
		ret = atoi(_localKeys[k].c_str());
		return;
	}

	// Check for ScummVM key
	if (_settings.contains(k)) {
		ret = atoi(_settings[k].c_str());
		return;
	}

	ret = defaultvalue;
}

void Configuration::value(const Std::string &key, bool &ret, bool defaultvalue) {
	// Check for a .cfg file value in the trees
	for (Std::vector<Shared::XMLTree *>::reverse_iterator i = _trees.rbegin();
	        i != _trees.rend(); ++i) {
		if ((*i)->hasNode(key)) {
			(*i)->value(key, ret, defaultvalue);
			return;
		}
	}

	assert(key.hasPrefix("config/"));
	Std::string k = key.substr(7);

	// Check for local entry
	if (_localKeys.contains(k)) {
		ret = _localKeys[k].hasPrefixIgnoreCase("y") ||
			_localKeys[k].hasPrefixIgnoreCase("t");
		return;
	}

	// Check for ScummVM key
	if (_settings.contains(k)) {
		ret = _settings[k].hasPrefixIgnoreCase("y") ||
			_settings[k].hasPrefixIgnoreCase("t");
		return;
	}

	ret = defaultvalue;
}

void Configuration::pathFromValue(const Std::string &key, Std::string file, Std::string &full_path) {
	value(key, full_path);

	if (full_path.length() > 0 && full_path[full_path.length() - 1] != U6PATH_DELIMITER)
		full_path += U6PATH_DELIMITER + file;
	else
		full_path += file;
}

bool Configuration::set(const Std::string &key, const Std::string &value) {
	// Currently a value is written to the last writable tree with
	// the correct root.

	for (Std::vector<Shared::XMLTree *>::reverse_iterator i = _trees.rbegin();
	        i != _trees.rend(); ++i) {
		if (!((*i)->isReadonly()) &&
		        (*i)->checkRoot(key)) {
			(*i)->set(key, value);
			return true;
		}
	}

	assert(key.hasPrefix("config/"));
	Std::string k = key.substr(7);

	if (_localKeys.contains(k)) {
		_localKeys[k] = value;
		return true;
	}

	_settings[k] = value;
	ConfMan.set(k, value);
	_configChanged = true;

	return true;
}

bool Configuration::set(const Std::string &key, const char *value) {
	return set(key, Std::string(value));
}


bool Configuration::set(const Std::string &key, int value) {
	// Currently a value is written to the last writable tree with
	// the correct root.

	for (Std::vector<Shared::XMLTree *>::reverse_iterator i = _trees.rbegin();
	        i != _trees.rend(); ++i) {
		if (!((*i)->isReadonly()) &&
		        (*i)->checkRoot(key)) {
			(*i)->set(key, value);
			return true;
		}
	}

	assert(key.hasPrefix("config/"));
	Std::string k = key.substr(7);

	if (_localKeys.contains(k)) {
		_localKeys[k] = Common::String::format("%d", value);
		return true;
	}

	_settings[k] = Common::String::format("%d", value);
	ConfMan.setInt(k, value);
	_configChanged = true;

	return true;
}

bool Configuration::set(const Std::string &key, bool value) {
	// Currently a value is written to the last writable tree with
	// the correct root.

	for (Std::vector<Shared::XMLTree *>::reverse_iterator i = _trees.rbegin();
	        i != _trees.rend(); ++i) {
		if (!((*i)->isReadonly()) &&
		        (*i)->checkRoot(key)) {
			(*i)->set(key, value);
			return true;
		}
	}

	assert(key.hasPrefix("config/"));
	Std::string k = key.substr(7);
	Common::String strValue = value ? "yes" : "no";

	if (_localKeys.contains(k)) {
		_localKeys[k] = strValue;
	} else {
		_settings[k] = strValue;
		ConfMan.setBool(k, value);
		_configChanged = true;
	}

	return true;
}

ConfigNode *Configuration::getNode(const Std::string &key) {
	return new ConfigNode(*this, key);
}

Std::set<Std::string> Configuration::listKeys(const Std::string &key, bool longformat) {
	Std::set<Std::string> keys;
	for (Common::Array<Shared::XMLTree *>::iterator i = _trees.begin();
	        i != _trees.end(); ++i) {
		Common::Array<Common::String> k = (*i)->listKeys(key, longformat);
		for (Common::Array<Common::String>::iterator iter = k.begin();
		        iter != k.end(); ++iter) {
			keys.insert(*iter);
		}
	}
	return keys;
}

void Configuration::getSubkeys(KeyTypeList &ktl, Std::string basekey) {
	for (Std::vector<Shared::XMLTree *>::iterator tree = _trees.begin();
	        tree != _trees.end(); ++tree) {
		Shared::XMLTree::KeyTypeList l;
		(*tree)->getSubkeys(l, basekey);

		for (Shared::XMLTree::KeyTypeList::iterator i = l.begin();
		        i != l.end(); ++i) {
			bool found = false;
			for (KeyTypeList::iterator j = ktl.begin();
			        j != ktl.end() && !found; ++j) {
				if (j->first == i->first) {
					// already have this subkey, so just replace the value
					j->second = i->second;
					found = true;
				}
			}
			if (!found) {
				// new subkey
				ktl.push_back(*i);
			}
		}
	}
}

bool Configuration::isDefaultsSet() const {
	return ConfMan.hasKey("config/video/screen_width");
}

void Configuration::load(GameId gameId, bool isEnhanced) {
	// Load basic defaults for enhanced vs unehanced
	if (isEnhanced)
		setEnhancedDefaults(gameId);
	else
		setUnenhancedDefaults(gameId);

	// nuvie.cfg in the game folder can supercede any ScummVM settings
	if (Common::File::exists("nuvie.cfg"))
		(void)readConfigFile("nuvie.cfg", "config");

	// Load any further settings from scummvm.ini
	const Common::ConfigManager::Domain &domain = *ConfMan.getActiveDomain();
	Common::ConfigManager::Domain::const_iterator it;
	for (it = domain.begin(); it != domain.end(); ++it) {
		_settings[(*it)._key] = (*it)._value;
	}
}

void Configuration::setCommonDefaults(GameId gameType) {
	_settings["video/non_square_pixels"] = "no";

#ifdef TODO
	_settings["audio/enabled"] = true;
	_settings["audio/enable_music"] = true;
	_settings["audio/enable_sfx"] = true;
	_settings["audio/music_volume"] = 100;
	_settings["audio/sfx_volume"] = 255;
#endif
	_settings["audio/combat_changes_music"] = "yes";
	_settings["audio/vehicles_change_music"] = "yes";
	_settings["audio/conversations_stop_music"] = "no"; // original stopped music - maybe due to memory and disk swapping
	_settings["audio/stop_music_on_group_change"] = "yes";

	_settings["input/enable_doubleclick"] = "yes";
	_settings["input/enabled_dragging"] = "yes";
	_settings["input/look_on_left_click"] = "yes";
	_settings["input/walk_with_left_button"] = "yes";
	_settings["input/direction_selects_target"] = "yes";

	_settings["general/dither_mode"] = "none";
	_settings["general/enable_cursors"] = "yes";
	_settings["general/party_formation"] = "standard";

	// Only show the startup console if in ScummVM debug mode
	_settings["general/show_console"] = gDebugLevel > 0 ? "yes" : "no";

	_settings["cheats/enabled"] = "no";
	_settings["cheats/enable_hackmove"] = "no";
	_settings["cheats/min_brightness"] = "0";
	_settings["cheats/party_all_the_time"] = "no";

	// game specific settings
	uint8 bg_color[] = { 218, 136, 216 }; // U6, MD, SE
	uint8 border_color[] = { 220, 133, 219 }; // U6, MD, SE

	int i = 0;
	if (gameType == GAME_MARTIAN_DREAMS)
		i = 1;
	else if (gameType == GAME_SAVAGE_EMPIRE)
		i = 2;

#ifdef TODO
	_settings["language", "en";
	_settings["music", "native";
	_settings["sfx", "native";

	if (i == 0) // U6
		_settings["enable_speech", "yes";
#endif
	_settings["skip_intro"] = "no";
	_settings["show_eggs"] = "no";
	if (i == 0) { // U6
		_settings["show_stealing"] = "no";
		_settings["roof_mode"] = "no";
	}
	_settings["use_new_dolls"] = "no";
	_settings["cb_position"] = "default";
	_settings["show_orig_style_cb"] = "default";
	if (i == 0) // U6
		_settings["cb_text_color"] = "115";
	_settings["map_tile_lighting"] = i == 1 ? "no": "yes"; // MD has canals lit up so disable
	_settings["custom_actor_tiles"] = "default";
	_settings["converse_solid_bg"] = "no";
	_settings["converse_bg_color"] =
		Common::String::format("%d", bg_color[i]);
	_settings["converse_width"] = "default";
	_settings["converse_height"] = "default";
	if (i == 0) { // U6
		_settings["displayed_wind_dir"] = "from";
		_settings["free_balloon_movement"] = "no";
	}
	_settings["game_specific_keys"] = "(default)";
	_settings["newscroll/width"] = "30";
	_settings["newscroll/height"] = "19";
	_settings["newscroll/solid_bg"] = "no";
	_settings["newscroll/bg_color"] =
		Common::String::format("%d", bg_color[i]);
	_settings["newscroll/border_color"] =
		Common::String::format("%d", border_color[i]);

	_settings["townsdir"] = "townsu6";

	//	_settings["newgamedata/name"] = "Avatar";
	//	_settings["newgamedata/gender"] = "0";
	//	_settings["newgamedata/portrait"] = "0";
	//	_settings["newgamedata/str"] = "15";
	//	_settings["newgamedata/dex"] = "15";
	//	_settings["newgamedata/int"] = "15";
}

void Configuration::setUnenhancedDefaults(GameId gameType) {
	setCommonDefaults(gameType);

	_settings["video/screen_width"] = "320";
	_settings["video/screen_height"] = "200";
	_settings["video/game_width"] = "320";
	_settings["video/game_height"] = "200";
	_settings["video/game_style"] = "original";
	_settings["video/game_position"] = "center";

	_settings["general/converse_gump"] = "default";
	_settings["general/lighting"] = "original";
	_settings["general/use_text_gumps"] = "no";

	_settings["input/doubleclick_opens_containers"] = "no";
	_settings["input/party_view_targeting"] = "no";
	_settings["input/new_command_bar"] = "no";
	_settings["input/interface"] = "normal";
}

void Configuration::setEnhancedDefaults(GameId gameType) {
	setCommonDefaults(gameType);

	_settings["video/screen_width"] = "640";
	_settings["video/screen_height"] = "400";
	_settings["video/game_width"] = "640";
	_settings["video/game_height"] = "400";
	_settings["video/game_style"] = "original+_full_map";
	_settings["video/game_position"] = "center";

	_settings["general/converse_gump"] = "yes";
	_settings["general/lighting"] = "smooth";
	_settings["general/use_text_gumps"] = "yes";

	_settings["input/doubleclick_opens_containers"] = "yes";
	_settings["input/party_view_targeting"] = "yes";
	_settings["input/new_command_bar"] = "yes";
	_settings["input/interface"] = "fullscreen";
}

} // End of namespace Nuvie
} // End of namespace Ultima
