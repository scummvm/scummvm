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
#include "ultima/nuvie/conf/xml_tree.h"
#include "ultima/nuvie/conf/config_node.h"
#include "common/config-manager.h"

namespace Ultima {
namespace Nuvie {

Configuration::Configuration() : _configChanged(false) {
	// Set up keys that will be stored locally, since we don't want them being
	// written out to the ScummVM configuration
	_localKeys["GameType"] = "";
	_localKeys["GameName"] = "";
	_localKeys["GameID"] = "";
	_localKeys["datadir"] = "data";		// This maps to ultima6/ in ultima.dat
}

Configuration::~Configuration() {
	for (Std::vector<XMLTree *>::iterator i = _trees.begin();
	        i != _trees.end(); ++i) {
		delete(*i);
	}

	if (_configChanged)
		ConfMan.flushToDisk();
}

bool Configuration::readConfigFile(Std::string fname, Std::string root,
                                   bool readonly) {
	_configFilename = fname;
	XMLTree *tree = new XMLTree();
	tree->clear(root);
	if (!tree->readConfigFile(fname)) {
		delete tree;
		return false;
	}

	_trees.push_back(tree);
	return true;
}

void Configuration::write() {
	for (Std::vector<XMLTree *>::iterator i = _trees.begin();
	        i != _trees.end(); ++i) {
		if (!(*i)->isReadonly())
			(*i)->write();
	}
}

void Configuration::clear() {
	for (Std::vector<XMLTree *>::iterator i = _trees.begin();
	        i != _trees.end(); ++i) {
		delete(*i);
	}
	_trees.clear();
}

void Configuration::value(const Std::string &key, Std::string &ret,
                          const char *defaultvalue) {
	// Check for a .cfg file value in the trees
	for (Std::vector<XMLTree *>::reverse_iterator i = _trees.rbegin();
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
	if (ConfMan.hasKey(k)) {
		ret = ConfMan.get(k);
		return;
	}

	ret = defaultvalue;
}

void Configuration::value(const Std::string &key, int &ret, int defaultvalue) {
	// Check for a .cfg file value in the trees
	for (Std::vector<XMLTree *>::reverse_iterator i = _trees.rbegin();
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
	if (ConfMan.hasKey(k)) {
		ret = ConfMan.getInt(k);
		return;
	}

	ret = defaultvalue;
}

void Configuration::value(const Std::string &key, bool &ret, bool defaultvalue) {
	// Check for a .cfg file value in the trees
	for (Std::vector<XMLTree *>::reverse_iterator i = _trees.rbegin();
	        i != _trees.rend(); ++i) {
		if ((*i)->hasNode(key)) {
			(*i)->value(key, ret, defaultvalue);
			return;
		}
	}

	assert(key.hasPrefix("config/"));
	Std::string k = key.substr(7);

	// Check for local entry
	assert(!_localKeys.contains(k));

	// Check for ScummVM key
	if (ConfMan.hasKey(k)) {
		ret = ConfMan.getBool(k);
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

	return;
}

bool Configuration::set(const Std::string &key, const Std::string &value) {
	// Currently a value is written to the last writable tree with
	// the correct root.

	for (Std::vector<XMLTree *>::reverse_iterator i = _trees.rbegin();
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

	for (Std::vector<XMLTree *>::reverse_iterator i = _trees.rbegin();
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

	ConfMan.setInt(k, value);
	_configChanged = true;

	return true;
}

bool Configuration::set(const Std::string &key, bool value) {
	// Currently a value is written to the last writable tree with
	// the correct root.

	for (Std::vector<XMLTree *>::reverse_iterator i = _trees.rbegin();
	        i != _trees.rend(); ++i) {
		if (!((*i)->isReadonly()) &&
		        (*i)->checkRoot(key)) {
			(*i)->set(key, value);
			return true;
		}
	}

	assert(key.hasPrefix("config/"));
	Std::string k = key.substr(7);
	assert(!_localKeys.contains(k));

	ConfMan.setBool(k, value);
	_configChanged = true;

	return true;
}

ConfigNode *Configuration::getNode(const Std::string &key) {
	return new ConfigNode(*this, key);
}

Std::set<Std::string> Configuration::listKeys(const Std::string &key, bool longformat) {
	Std::set<Std::string> keys;
	for (Std::vector<XMLTree *>::iterator i = _trees.begin();
	        i != _trees.end(); ++i) {
		Std::vector<Std::string> k = (*i)->listKeys(key, longformat);
		for (Std::vector<Std::string>::iterator iter = k.begin();
		        iter != k.end(); ++iter) {
			keys.insert(*iter);
		}
	}
	return keys;
}

void Configuration::getSubkeys(KeyTypeList &ktl, Std::string basekey) {
	for (Std::vector<XMLTree *>::iterator tree = _trees.begin();
	        tree != _trees.end(); ++tree) {
		KeyTypeList l;
		(*tree)->getSubkeys(l, basekey);

		for (KeyTypeList::iterator i = l.begin();
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

void Configuration::setCommonDefaults(GameId gameType) {
	set("config/video/non_square_pixels", "no");

#ifdef TODO
	set("config/audio/enabled", true);
	set("config/audio/enable_music", true);
	set("config/audio/enable_sfx", true);
	set("config/audio/music_volume", 100);
	set("config/audio/sfx_volume", 255);
#endif
	set("config/audio/combat_changes_music", true);
	set("config/audio/vehicles_change_music", true);
	set("config/audio/conversations_stop_music", false); // original stopped music - maybe due to memory and disk swapping
	set("config/audio/stop_music_on_group_change", true);

	set("config/input/enable_doubleclick", true);
	set("config/input/enabled_dragging", true);
	set("config/input/look_on_left_click", true);
	set("config/input/walk_with_left_button", true);
	set("config/input/direction_selects_target", true);

	set("config/general/dither_mode", "none");
	set("config/general/enable_cursors", true);
	set("config/general/party_formation", "standard");

	// Only show the startup console if in ScummVM debug mode
	set("config/general/show_console", gDebugLevel > 0);

	set("config/cheats/enabled", false);
	set("config/cheats/enable_hackmove", false);
	set("config/cheats/min_brightness", 0);
	set("config/cheats/party_all_the_time", false);

	// game specific settings
	uint8 bg_color[] = { 218, 136, 216 }; // U6, MD, SE
	uint8 border_color[] = { 220, 133, 219 }; // U6, MD, SE

	int i = 0;
	if (gameType == GAME_MARTIAN_DREAMS)
		i = 1;
	else if (gameType == GAME_SAVAGE_EMPIRE)
		i = 2;

#ifdef TODO
	set("config/language", "en");
	set("config/music", "native");
	set("config/sfx", "native");

	if (i == 0) // U6
		set("config/enable_speech", "yes");
#endif
	set("config/skip_intro", false);
	set("config/show_eggs", false);
	if (i == 0) { // U6
		set("config/show_stealing", false);
		set("config/roof_mode", false);
	}
	set("config/use_new_dolls", false);
	set("config/cb_position", "default");
	set("config/show_orig_style_cb", "default");
	if (i == 0) // U6
		set("config/cb_text_color", 115);
	set("config/map_tile_lighting", i == 1 ? false : true); // MD has canals lit up so disable
	set("config/custom_actor_tiles", "default");
	set("config/converse_solid_bg", false);
	set("config/converse_bg_color", bg_color[i]);
	set("config/converse_width", "default");
	set("config/converse_height", "default");
	if (i == 0) { // U6
		set("config/displayed_wind_dir", "from");
		set("config/free_balloon_movement", false);
	}
	set("config/game_specific_keys", "(default)");
	set("config/newscroll/width", 30);
	set("config/newscroll/height", 19);
	set("config/newscroll/solid_bg", false);
	set("config/newscroll/bg_color", bg_color[i]);
	set("config/newscroll/border_color", border_color[i]);

	//	set("config/newgamedata/name", "Avatar");
	//	set("config/newgamedata/gender", 0);
	//	set("config/newgamedata/portrait", 0);
	//	set("config/newgamedata/str", 0xf);
	//	set("config/newgamedata/dex", 0xf);
	//	set("config/newgamedata/int", 0xf);
}

void Configuration::setUnenhancedDefaults(GameId gameType) {
	setCommonDefaults(gameType);

	set("config/video/screen_width", 320);
	set("config/video/screen_height", 200);
	set("config/video/game_width", 320);
	set("config/video/game_height", 200);
	set("config/video/game_style", "original");
	set("config/video/game_position", "center");

	set("config/general/converse_gump", "default");
	set("config/general/lighting", "original");
	set("config/general/use_text_gumps", false);

	set("config/input/doubleclick_opens_containers", false);
	set("config/input/party_view_targeting", false);
	set("config/input/new_command_bar", false);
	set("config/input/interface", "normal");
}

void Configuration::setEnhancedDefaults(GameId gameType) {
	setCommonDefaults(gameType);

	set("config/video/screen_width", 640);
	set("config/video/screen_height", 400);
	set("config/video/game_width", 640);
	set("config/video/game_height", 400);
	set("config/video/game_style", "original+_full_map");
	set("config/video/game_position", "center");

	set("config/general/converse_gump", "yes");
	set("config/general/lighting", "smooth");
	set("config/general/use_text_gumps", true);

	set("config/input/doubleclick_opens_containers", true);
	set("config/input/party_view_targeting", true);
	set("config/input/new_command_bar", true);
	set("config/input/interface", "fullscreen");
}

} // End of namespace Nuvie
} // End of namespace Ultima
