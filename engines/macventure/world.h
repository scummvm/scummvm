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

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#ifndef MACVENTURE_WORLD_H
#define MACVENTURE_WORLD_H

#include "macventure/macventure.h"
#include "macventure/container.h"

namespace MacVenture {

typedef Common::Array<uint16> AttributeGroup;

class SaveGame {
public:
	SaveGame(MacVentureEngine *engine, Common::SeekableReadStream *res);
	~SaveGame();

	const Common::Array<AttributeGroup> &getGroups();
	const Common::Array<uint16> &getGlobals();
	const Common::String &getText();

private:
	void loadGroups(MacVentureEngine *engine, Common::SeekableReadStream *res);
	void loadGlobals(MacVentureEngine *engine, Common::SeekableReadStream *res);
	void loadText(MacVentureEngine *engine, Common::SeekableReadStream *res);

private:
	Common::Array<AttributeGroup> _groups;
	Common::Array<uint16> _globals;
	Common::String _text;
};

class World {
public:
	World(MacVentureEngine *engine, Common::MacResManager *resMan);
	~World();

private:
	bool loadStartGameFileName();

private:
	MacVentureEngine *_engine;
	Common::MacResManager *_resourceManager;

	Common::String _startGameFileName;	

	SaveGame *_saveGame;
	Container *_objectConstants;

};

} // End of namespace MacVenture

#endif

