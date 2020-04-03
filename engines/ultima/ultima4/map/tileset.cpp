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

#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/map/tile.h"
#include "ultima/ultima4/map/tilemap.h"

namespace Ultima {
namespace Ultima4 {

using Std::vector;

/**
 * TileRule Class Implementation
 */
TileRuleMap TileRule::_rules;

TileRule *TileRule::findByName(const Common::String &name) {
	TileRuleMap::iterator i = _rules.find(name);
	if (i != _rules.end())
		return i->_value;
	return NULL;
}

void TileRule::load() {
	const Config *config = Config::getInstance();
	vector<ConfigElement> rules = config->getElement("tileRules").getChildren();

	for (Std::vector<ConfigElement>::iterator i = rules.begin(); i != rules.end(); i++) {
		TileRule *rule = new TileRule;
		rule->initFromConf(*i);
		TileRule::_rules[rule->_name] = rule;
	}

	if (TileRule::findByName("default") == NULL)
		errorFatal("no 'default' rule found in tile rules");
}

bool TileRule::initFromConf(const ConfigElement &conf) {
	unsigned int i;

	static const struct {
		const char *name;
		unsigned int mask;
	} booleanAttributes[] = {
		{ "dispel", MASK_DISPEL },
		{ "talkover", MASK_TALKOVER },
		{ "door", MASK_DOOR },
		{ "lockeddoor", MASK_LOCKEDDOOR },
		{ "chest", MASK_CHEST },
		{ "ship", MASK_SHIP },
		{ "horse", MASK_HORSE },
		{ "balloon", MASK_BALLOON },
		{ "canattackover", MASK_ATTACKOVER },
		{ "canlandballoon", MASK_CANLANDBALLOON },
		{ "replacement", MASK_REPLACEMENT },
		{ "foreground", MASK_FOREGROUND },
		{ "onWaterOnlyReplacement", MASK_WATER_REPLACEMENT},
		{ "livingthing", MASK_LIVING_THING }

	};

	static const struct {
		const char *name;
		unsigned int mask;
	} movementBooleanAttr[] = {
		{ "swimable", MASK_SWIMABLE },
		{ "sailable", MASK_SAILABLE },
		{ "unflyable", MASK_UNFLYABLE },
		{ "creatureunwalkable", MASK_CREATURE_UNWALKABLE }
	};
	static const char *speedEnumStrings[] = { "fast", "slow", "vslow", "vvslow", NULL };
	static const char *effectsEnumStrings[] = { "none", "fire", "sleep", "poison", "poisonField", "electricity", "lava", NULL };

	this->_mask = 0;
	this->_movementMask = 0;
	this->_speed = FAST;
	this->_effect = EFFECT_NONE;
	this->_walkOnDirs = MASK_DIR_ALL;
	this->_walkOffDirs = MASK_DIR_ALL;
	this->_name = conf.getString("name");

	for (i = 0; i < sizeof(booleanAttributes) / sizeof(booleanAttributes[0]); i++) {
		if (conf.getBool(booleanAttributes[i].name))
			this->_mask |= booleanAttributes[i].mask;
	}

	for (i = 0; i < sizeof(movementBooleanAttr) / sizeof(movementBooleanAttr[0]); i++) {
		if (conf.getBool(movementBooleanAttr[i].name))
			this->_movementMask |= movementBooleanAttr[i].mask;
	}

	Common::String cantwalkon = conf.getString("cantwalkon");
	if (cantwalkon == "all")
		this->_walkOnDirs = 0;
	else if (cantwalkon == "west")
		this->_walkOnDirs = DIR_REMOVE_FROM_MASK(DIR_WEST, this->_walkOnDirs);
	else if (cantwalkon == "north")
		this->_walkOnDirs = DIR_REMOVE_FROM_MASK(DIR_NORTH, this->_walkOnDirs);
	else if (cantwalkon == "east")
		this->_walkOnDirs = DIR_REMOVE_FROM_MASK(DIR_EAST, this->_walkOnDirs);
	else if (cantwalkon == "south")
		this->_walkOnDirs = DIR_REMOVE_FROM_MASK(DIR_SOUTH, this->_walkOnDirs);
	else if (cantwalkon == "advance")
		this->_walkOnDirs = DIR_REMOVE_FROM_MASK(DIR_ADVANCE, this->_walkOnDirs);
	else if (cantwalkon == "retreat")
		this->_walkOnDirs = DIR_REMOVE_FROM_MASK(DIR_RETREAT, this->_walkOnDirs);

	Common::String cantwalkoff = conf.getString("cantwalkoff");
	if (cantwalkoff == "all")
		this->_walkOffDirs = 0;
	else if (cantwalkoff == "west")
		this->_walkOffDirs = DIR_REMOVE_FROM_MASK(DIR_WEST, this->_walkOffDirs);
	else if (cantwalkoff == "north")
		this->_walkOffDirs = DIR_REMOVE_FROM_MASK(DIR_NORTH, this->_walkOffDirs);
	else if (cantwalkoff == "east")
		this->_walkOffDirs = DIR_REMOVE_FROM_MASK(DIR_EAST, this->_walkOffDirs);
	else if (cantwalkoff == "south")
		this->_walkOffDirs = DIR_REMOVE_FROM_MASK(DIR_SOUTH, this->_walkOffDirs);
	else if (cantwalkoff == "advance")
		this->_walkOffDirs = DIR_REMOVE_FROM_MASK(DIR_ADVANCE, this->_walkOffDirs);
	else if (cantwalkoff == "retreat")
		this->_walkOffDirs = DIR_REMOVE_FROM_MASK(DIR_RETREAT, this->_walkOffDirs);

	this->_speed = static_cast<TileSpeed>(conf.getEnum("speed", speedEnumStrings));
	this->_effect = static_cast<TileEffect>(conf.getEnum("effect", effectsEnumStrings));

	return true;
}


/**
 * Tileset Class Implementation
 */

/* static member variables */
Tileset::TilesetMap Tileset::tilesets;

void Tileset::loadAll() {
	Debug dbg("debug/tileset.txt", "Tileset");
	const Config *config = Config::getInstance();
	vector<ConfigElement> conf;

	TRACE(dbg, "Unloading all tilesets");
	unloadAll();

	// get the config element for all tilesets
	TRACE_LOCAL(dbg, "Loading tilesets info from config");
	conf = config->getElement("tilesets").getChildren();

	// load tile rules
	TRACE_LOCAL(dbg, "Loading tile rules");
	if (!TileRule::_rules.size())
		TileRule::load();

	// load all of the tilesets
	for (Std::vector<ConfigElement>::iterator i = conf.begin(); i != conf.end(); i++) {
		if (i->getName() == "tileset") {

			Tileset *tileset = new Tileset;
			tileset->load(*i);

			tilesets[tileset->_name] = tileset;
		}
	}

	// load tile maps, including translations from index to id
	TRACE_LOCAL(dbg, "Loading tilemaps");
	TileMap::loadAll();

	TRACE(dbg, "Successfully Loaded Tilesets");
}

void Tileset::unloadAll() {
	TilesetMap::iterator i;

	// unload all tilemaps
	TileMap::unloadAll();

	for (i = tilesets.begin(); i != tilesets.end(); i++) {
		i->_value->unload();
		delete i->_value;
	}
	tilesets.clear();

	Tile::resetNextId();
}

void Tileset::unloadAllImages() {
	TilesetMap::iterator i;

	for (i = tilesets.begin(); i != tilesets.end(); i++) {
		i->_value->unloadImages();
	}

	Tile::resetNextId();
}

Tileset *Tileset::get(const Common::String &name) {
	if (tilesets.find(name) != tilesets.end())
		return tilesets[name];
	else return NULL;
}

Tile *Tileset::findTileByName(const Common::String &name) {
	TilesetMap::iterator i;
	for (i = tilesets.begin(); i != tilesets.end(); i++) {
		Tile *t = i->_value->getByName(name);
		if (t)
			return t;
	}

	return NULL;
}

Tile *Tileset::findTileById(TileId id) {
	TilesetMap::iterator i;
	for (i = tilesets.begin(); i != tilesets.end(); i++) {
		Tile *t = i->_value->get(id);
		if (t)
			return t;
	}

	return NULL;
}

void Tileset::load(const ConfigElement &tilesetConf) {
	Debug dbg("debug/tileset.txt", "Tileset", true);

	_name = tilesetConf.getString("name");
	if (tilesetConf.exists("imageName"))
		_imageName = tilesetConf.getString("imageName");
	if (tilesetConf.exists("extends"))
		_extends = Tileset::get(tilesetConf.getString("extends"));
	else _extends = NULL;

	TRACE_LOCAL(dbg, "\tLoading Tiles...");

	int index = 0;
	vector<ConfigElement> children = tilesetConf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
		if (i->getName() != "tile")
			continue;

		Tile *tile = new Tile(this);
		tile->loadProperties(*i);

		TRACE_LOCAL(dbg, Common::String("\t\tLoaded '") + tile->getName() + "'");

		/* add the tile to our tileset */
		_tiles[tile->getId()] = tile;
		_nameMap[tile->getName()] = tile;

		index += tile->getFrames();
	}
	_totalFrames = index;
}

void Tileset::unloadImages() {
	Tileset::TileIdMap::iterator i;

	/* free all the image memory and nullify so that reloading can automatically take place lazily */
	for (i = _tiles.begin(); i != _tiles.end(); i++) {
		i->_value->deleteImage();
	}
}

void Tileset::unload() {
	Tileset::TileIdMap::iterator i;

	/* free all the memory for the tiles */
	for (i = _tiles.begin(); i != _tiles.end(); i++)
		delete i->_value;

	_tiles.clear();
	_totalFrames = 0;
	_imageName.clear();
}

Tile *Tileset::get(TileId id) {
	if (_tiles.find(id) != _tiles.end())
		return _tiles[id];
	else if (_extends)
		return _extends->get(id);
	return NULL;
}

Tile *Tileset::getByName(const Common::String &name) {
	if (_nameMap.find(name) != _nameMap.end())
		return _nameMap[name];
	else if (_extends)
		return _extends->getByName(name);
	else return NULL;
}

Common::String Tileset::getImageName() const {
	if (_imageName.empty() && _extends)
		return _extends->getImageName();
	else return _imageName;
}

unsigned int Tileset::numTiles() const {
	return _tiles.size();
}

unsigned int Tileset::numFrames() const {
	return _totalFrames;
}

} // End of namespace Ultima4
} // End of namespace Ultima
