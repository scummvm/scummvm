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

#include "common/str.h"
#include "engines/wintermute/ad/ad_geom_ext_node.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/utils/utils.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
AdGeomExtNode::AdGeomExtNode(BaseGame *inGame) : BaseClass(inGame),
                                                 _receiveShadows(false), _type(GEOM_GENERIC), _namePattern(nullptr) {
}

//////////////////////////////////////////////////////////////////////////
AdGeomExtNode::~AdGeomExtNode() {
	delete[] _namePattern;
}

TOKEN_DEF_START
	TOKEN_DEF(NODE)
	TOKEN_DEF(NAME)
	TOKEN_DEF(WALKPLANE)
	TOKEN_DEF(BLOCKED)
	TOKEN_DEF(WAYPOINT)
	TOKEN_DEF(RECEIVE_SHADOWS)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdGeomExtNode::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
		TOKEN_TABLE(NODE)
		TOKEN_TABLE(NAME)
		TOKEN_TABLE(WALKPLANE)
		TOKEN_TABLE(BLOCKED)
		TOKEN_TABLE(WAYPOINT)
		TOKEN_TABLE(RECEIVE_SHADOWS)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_NODE) {
			_gameRef->LOG(0, "'NODE' keyword expected.");
			return false;
		}

		buffer = params;
	}

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_NAME:
			BaseUtils::setString(&_namePattern, (char *)params);
			break;

		case TOKEN_RECEIVE_SHADOWS:
			parser.scanStr((char *)params, "%b", &_receiveShadows);
			break;

		case TOKEN_WALKPLANE: {
			bool isWalkplane = false;
			parser.scanStr((char *)params, "%b", &isWalkplane);
			if (isWalkplane) {
				_type = GEOM_WALKPLANE;
			}
			break;
		}

		case TOKEN_BLOCKED: {
			bool isBlocked = false;
			parser.scanStr((char *)params, "%b", &isBlocked);
			if (isBlocked) {
				_type = GEOM_BLOCKED;
			}
			break;
		}

		case TOKEN_WAYPOINT: {
			bool isWaypoint = false;
			parser.scanStr((char *)params, "%b", &isWaypoint);
			if (isWaypoint) {
				_type = GEOM_WAYPOINT;
			}
			break;
		}
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
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdGeomExtNode::setupNode(const char *namePattern, TGeomNodeType type, bool receiveShadows) {
	BaseUtils::setString(&_namePattern, namePattern);
	_type = type;
	_receiveShadows = receiveShadows;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdGeomExtNode::matchesName(const char *name) {
	return Common::matchString(name, _namePattern);
}

} // namespace Wintermute
