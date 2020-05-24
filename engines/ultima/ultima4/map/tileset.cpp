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
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/map/tile.h"
#include "ultima/ultima4/map/tilemap.h"

namespace Ultima {
namespace Ultima4 {

TileRules *g_tileRules;
TileSets *g_tileSets;

TileRules::TileRules() {
	g_tileRules = this;
}

TileRules::~TileRules() {
	// Delete the tile rules
	for (iterator it = begin(); it != end(); ++it)
		delete it->_value;

	g_tileRules = nullptr;
}

void TileRules::load() {
	const Config *config = Config::getInstance();
	Std::vector<ConfigElement> rules = config->getElement("tileRules").getChildren();

	for (Std::vector<ConfigElement>::iterator i = rules.begin(); i != rules.end(); i++) {
		TileRule *rule = new TileRule();
		rule->initFromConf(*i);
		(*this)[rule->_name] = rule;
	}

	if (findByName("default") == nullptr)
		error("no 'default' rule found in tile rules");
}

TileRule *TileRules::findByName(const Common::String &name) {
	TileRuleMap::iterator i = find(name);
	if (i != end())
		return i->_value;
	return nullptr;
}

/*-------------------------------------------------------------------*/

TileSets::TileSets() {
	g_tileSets = this;
	loadAll();
}

TileSets::~TileSets() {
	unloadAll();
	g_tileSets = nullptr;
}

void TileSets::loadAll() {
	const Config *config = Config::getInstance();
	Std::vector<ConfigElement> conf;

	unloadAll();

	// Get the config element for all tilesets
	conf = config->getElement("tilesets").getChildren();

	// Load tile rules
	if (g_tileRules->empty())
		g_tileRules->load();

	// Load all of the tilesets
	for (Std::vector<ConfigElement>::iterator i = conf.begin(); i != conf.end(); i++) {
		if (i->getName() == "tileset") {

			Tileset *tileset = new Tileset();
			tileset->load(*i);

			(*this)[tileset->_name] = tileset;
		}
	}
}

void TileSets::unloadAll() {
	iterator i;

	for (i = begin(); i != end(); i++) {
		i->_value->unload();
		delete i->_value;
	}
	clear();

	Tile::resetNextId();
}

void TileSets::unloadAllImages() {
	iterator i;

	for (i = begin(); i != end(); i++) {
		i->_value->unloadImages();
	}

	Tile::resetNextId();
}

Tileset *TileSets::get(const Common::String &name) {
	if (find(name) != end())
		return (*this)[name];
	else
		return nullptr;
}

Tile *TileSets::findTileByName(const Common::String &name) {
	iterator i;
	for (i = begin(); i != end(); i++) {
		Tile *t = i->_value->getByName(name);
		if (t)
			return t;
	}

	return nullptr;
}

Tile *TileSets::findTileById(TileId id) {
	iterator i;
	for (i = begin(); i != end(); i++) {
		Tile *t = i->_value->get(id);
		if (t)
			return t;
	}

	return nullptr;
}


/*-------------------------------------------------------------------*/

bool TileRule::initFromConf(const ConfigElement &conf) {
	uint i;

	static const struct {
		const char *name;
		uint mask;
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
		const char *_name;
		uint _mask;
	} movementBooleanAttr[] = {
		{ "swimable", MASK_SWIMABLE },
		{ "sailable", MASK_SAILABLE },
		{ "unflyable", MASK_UNFLYABLE },
		{ "creatureunwalkable", MASK_CREATURE_UNWALKABLE }
	};
	static const char *speedEnumStrings[] = { "fast", "slow", "vslow", "vvslow", nullptr };
	static const char *effectsEnumStrings[] = { "none", "fire", "sleep", "poison", "poisonField", "electricity", "lava", nullptr };

	_mask = 0;
	_movementMask = 0;
	_speed = FAST;
	_effect = EFFECT_NONE;
	_walkOnDirs = MASK_DIR_ALL;
	_walkOffDirs = MASK_DIR_ALL;
	_name = conf.getString("name");

	for (i = 0; i < sizeof(booleanAttributes) / sizeof(booleanAttributes[0]); i++) {
		if (conf.getBool(booleanAttributes[i].name))
			_mask |= booleanAttributes[i].mask;
	}

	for (i = 0; i < sizeof(movementBooleanAttr) / sizeof(movementBooleanAttr[0]); i++) {
		if (conf.getBool(movementBooleanAttr[i]._name))
			_movementMask |= movementBooleanAttr[i]._mask;
	}

	Common::String cantwalkon = conf.getString("cantwalkon");
	if (cantwalkon == "all")
		_walkOnDirs = 0;
	else if (cantwalkon == "west")
		_walkOnDirs = DIR_REMOVE_FROM_MASK(DIR_WEST, _walkOnDirs);
	else if (cantwalkon == "north")
		_walkOnDirs = DIR_REMOVE_FROM_MASK(DIR_NORTH, _walkOnDirs);
	else if (cantwalkon == "east")
		_walkOnDirs = DIR_REMOVE_FROM_MASK(DIR_EAST, _walkOnDirs);
	else if (cantwalkon == "south")
		_walkOnDirs = DIR_REMOVE_FROM_MASK(DIR_SOUTH, _walkOnDirs);
	else if (cantwalkon == "advance")
		_walkOnDirs = DIR_REMOVE_FROM_MASK(DIR_ADVANCE, _walkOnDirs);
	else if (cantwalkon == "retreat")
		_walkOnDirs = DIR_REMOVE_FROM_MASK(DIR_RETREAT, _walkOnDirs);

	Common::String cantwalkoff = conf.getString("cantwalkoff");
	if (cantwalkoff == "all")
		_walkOffDirs = 0;
	else if (cantwalkoff == "west")
		_walkOffDirs = DIR_REMOVE_FROM_MASK(DIR_WEST, _walkOffDirs);
	else if (cantwalkoff == "north")
		_walkOffDirs = DIR_REMOVE_FROM_MASK(DIR_NORTH, _walkOffDirs);
	else if (cantwalkoff == "east")
		_walkOffDirs = DIR_REMOVE_FROM_MASK(DIR_EAST, _walkOffDirs);
	else if (cantwalkoff == "south")
		_walkOffDirs = DIR_REMOVE_FROM_MASK(DIR_SOUTH, _walkOffDirs);
	else if (cantwalkoff == "advance")
		_walkOffDirs = DIR_REMOVE_FROM_MASK(DIR_ADVANCE, _walkOffDirs);
	else if (cantwalkoff == "retreat")
		_walkOffDirs = DIR_REMOVE_FROM_MASK(DIR_RETREAT, _walkOffDirs);

	_speed = static_cast<TileSpeed>(conf.getEnum("speed", speedEnumStrings));
	_effect = static_cast<TileEffect>(conf.getEnum("effect", effectsEnumStrings));

	return true;
}

void Tileset::load(const ConfigElement &tilesetConf) {
	_name = tilesetConf.getString("name");
	if (tilesetConf.exists("imageName"))
		_imageName = tilesetConf.getString("imageName");
	if (tilesetConf.exists("extends"))
		_extends = g_tileSets->get(tilesetConf.getString("extends"));
	else
		_extends = nullptr;

	int index = 0;
	Std::vector<ConfigElement> children = tilesetConf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
		if (i->getName() != "tile")
			continue;

		Tile *tile = new Tile(this);
		tile->loadProperties(*i);

		// Add the tile to our tileset
		_tiles[tile->getId()] = tile;
		_nameMap[tile->getName()] = tile;

		index += tile->getFrames();
	}
	_totalFrames = index;
}

void Tileset::unloadImages() {
	Tileset::TileIdMap::iterator i;

	// Free all the image memory and nullify so that reloading can automatically take place lazily
	for (i = _tiles.begin(); i != _tiles.end(); i++) {
		i->_value->deleteImage();
	}
}

void Tileset::unload() {
	Tileset::TileIdMap::iterator i;

	// Free all the memory for the tiles
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
	return nullptr;
}

Tile *Tileset::getByName(const Common::String &name) {
	if (_nameMap.find(name) != _nameMap.end())
		return _nameMap[name];
	else if (_extends)
		return _extends->getByName(name);
	else
		return nullptr;
}

Common::String Tileset::getImageName() const {
	if (_imageName.empty() && _extends)
		return _extends->getImageName();
	else
		return _imageName;
}

uint Tileset::numTiles() const {
	return _tiles.size();
}

uint Tileset::numFrames() const {
	return _totalFrames;
}

} // End of namespace Ultima4
} // End of namespace Ultima
