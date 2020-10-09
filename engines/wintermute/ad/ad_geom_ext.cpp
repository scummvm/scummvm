/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/ad/ad_geom_ext.h"
#include "engines/wintermute/ad/ad_geom_ext_node.h"
#include "engines/wintermute/ad/ad_types.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/utils/utils.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
AdGeomExt::AdGeomExt(BaseGame *in_gameRef) : BaseClass(in_gameRef) {
}

//////////////////////////////////////////////////////////////////////////
AdGeomExt::~AdGeomExt() {
	for (uint i = 0; i < _nodes.size(); i++) {
		delete _nodes[i];
	}
	_nodes.clear();
}

//////////////////////////////////////////////////////////////////////////
bool AdGeomExt::loadFile(char *filename) {
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_gameRef->LOG(0, "AdGeomExt::LoadFile failed for file '%s'", filename);
		return false;
	}

	bool ret = loadBuffer(buffer);
	if (!ret) {
		_gameRef->LOG(0, "Error parsing geometry description file '%s'", filename);
	}

	delete[] buffer;

	return ret;
}

TOKEN_DEF_START
	TOKEN_DEF(GEOMETRY)
	TOKEN_DEF(NODE)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdGeomExt::loadBuffer(byte *buffer) {
	TOKEN_TABLE_START(commands)
		TOKEN_TABLE(GEOMETRY)
		TOKEN_TABLE(NODE)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser;

	if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_GEOMETRY) {
		_gameRef->LOG(0, "'GEOMETRY' keyword expected.");
		return false;
	}

	buffer = params;

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_NODE: {
			AdGeomExtNode *node = new AdGeomExtNode(_gameRef);

			if (node && node->loadBuffer(params, false)) {
				_nodes.add(node);
			} else {
				if (node) {
					delete node;
				}

				cmd = PARSERR_GENERIC;
			}
			} break;
		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in geometry description file");
		return false;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading geometry description");
		return false;
	}

	addStandardNodes();
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdGeomExt::addStandardNodes() {
	AdGeomExtNode *node;

	node = new AdGeomExtNode(_gameRef);
	node->setupNode("walk_*", GEOM_WALKPLANE, true);
	_nodes.add(node);

	node = new AdGeomExtNode(_gameRef);
	node->setupNode("blk_*", GEOM_BLOCKED, false);
	_nodes.add(node);

	node = new AdGeomExtNode(_gameRef);
	node->setupNode("wpt_*", GEOM_WAYPOINT, false);
	_nodes.add(node);

	return true;
}

//////////////////////////////////////////////////////////////////////////
AdGeomExtNode *AdGeomExt::matchName(const char *name) {
	if (!name) {
		return nullptr;
	}

	for (uint i = 0; i < _nodes.size(); i++) {
		if (_nodes[i]->matchesName(name)) {
			return _nodes[i];
		}
	}

	return nullptr;
}

} // namespace Wintermute
