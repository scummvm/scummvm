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
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
AdGeomExtNode::AdGeomExtNode(BaseGame *inGame) : BaseClass(inGame) {
	_namePattern = nullptr;
	_receiveShadows = false;
	_type = GEOM_GENERIC;
}

//////////////////////////////////////////////////////////////////////////
AdGeomExtNode::~AdGeomExtNode() {
	SAFE_DELETE_ARRAY(_namePattern);
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
bool AdGeomExtNode::loadBuffer(char *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
		TOKEN_TABLE(NODE)
		TOKEN_TABLE(NAME)
		TOKEN_TABLE(WALKPLANE)
		TOKEN_TABLE(BLOCKED)
		TOKEN_TABLE(WAYPOINT)
		TOKEN_TABLE(RECEIVE_SHADOWS)
	TOKEN_TABLE_END

	char *params;
	int cmd = 2;
	BaseParser parser(_game);

	if (complete) {
		if (parser.getCommand(&buffer, commands, &params) != TOKEN_NODE) {
			_game->LOG(0, "'NODE' keyword expected.");
			return false;
		}

		buffer = params;
	}

	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_NAME:
			BaseUtils::setString(&_namePattern, params);
			break;

		case TOKEN_RECEIVE_SHADOWS:
			parser.scanStr(params, "%b", &_receiveShadows);
			break;

		case TOKEN_WALKPLANE: {
			bool isWalkplane = false;
			parser.scanStr(params, "%b", &isWalkplane);
			if (isWalkplane) {
				_type = GEOM_WALKPLANE;
			}
			break;
		}

		case TOKEN_BLOCKED: {
			bool isBlocked = false;
			parser.scanStr(params, "%b", &isBlocked);
			if (isBlocked) {
				_type = GEOM_BLOCKED;
			}
			break;
		}

		case TOKEN_WAYPOINT: {
			bool isWaypoint = false;
			parser.scanStr(params, "%b", &isWaypoint);
			if (isWaypoint) {
				_type = GEOM_WAYPOINT;
			}
			break;
		}
		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND) {
		_game->LOG(0, "Syntax error in geometry description file");
		return false;
	}
	if (cmd == PARSERR_GENERIC) {
		_game->LOG(0, "Error loading geometry description");
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
	return BaseUtils::matchesPattern(_namePattern, name);
}

} // namespace Wintermute
