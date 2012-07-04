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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/Ad/AdWaypointGroup.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BRegion.h"
#include "engines/wintermute/Base/BFileManager.h"
#include <limits.h>

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdWaypointGroup, false)

//////////////////////////////////////////////////////////////////////////
CAdWaypointGroup::CAdWaypointGroup(CBGame *inGame): CBObject(inGame) {
	_active = true;
	_editorSelectedPoint = -1;
	_lastMimicScale = -1;
	_lastMimicX = _lastMimicY = INT_MIN;
}


//////////////////////////////////////////////////////////////////////////
CAdWaypointGroup::~CAdWaypointGroup() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CAdWaypointGroup::cleanup() {
	for (int i = 0; i < _points.GetSize(); i++)
		delete _points[i];
	_points.RemoveAll();
	_editorSelectedPoint = -1;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdWaypointGroup::loadFile(const char *filename) {
	byte *buffer = Game->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		Game->LOG(0, "CAdWaypointGroup::LoadFile failed for file '%s'", filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (FAILED(ret = loadBuffer(buffer, true))) Game->LOG(0, "Error parsing WAYPOINTS file '%s'", filename);


	delete [] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(WAYPOINTS)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(NAME)
TOKEN_DEF(POINT)
TOKEN_DEF(EDITOR_SELECTED_POINT)
TOKEN_DEF(EDITOR_SELECTED)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdWaypointGroup::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(WAYPOINTS)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(POINT)
	TOKEN_TABLE(EDITOR_SELECTED_POINT)
	TOKEN_TABLE(EDITOR_SELECTED)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (complete) {
		if (parser.GetCommand((char **)&buffer, commands, (char **)&params) != TOKEN_WAYPOINTS) {
			Game->LOG(0, "'WAYPOINTS' keyword expected.");
			return E_FAIL;
		}
		buffer = params;
	}

	while ((cmd = parser.GetCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_POINT: {
			int x, y;
			parser.ScanStr((char *)params, "%d,%d", &x, &y);
			_points.Add(new CBPoint(x, y));
		}
		break;

		case TOKEN_EDITOR_SELECTED:
			parser.ScanStr((char *)params, "%b", &_editorSelected);
			break;

		case TOKEN_EDITOR_SELECTED_POINT:
			parser.ScanStr((char *)params, "%d", &_editorSelectedPoint);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in WAYPOINTS definition");
		return E_FAIL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdWaypointGroup::saveAsText(CBDynBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "WAYPOINTS {\n");
	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _name);
	buffer->putTextIndent(indent + 2, "EDITOR_SELECTED=%s\n", _editorSelected ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SELECTED_POINT=%d\n", _editorSelectedPoint);

	if (_scProp) _scProp->saveAsText(buffer, indent + 2);
	CBBase::saveAsText(buffer, indent + 2);

	for (int i = 0; i < _points.GetSize(); i++) {
		buffer->putTextIndent(indent + 2, "POINT {%d,%d}\n", _points[i]->x, _points[i]->y);
	}

	buffer->putTextIndent(indent, "}\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdWaypointGroup::persist(CBPersistMgr *persistMgr) {

	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_active));
	persistMgr->transfer(TMEMBER(_editorSelectedPoint));
	persistMgr->transfer(TMEMBER(_lastMimicScale));
	persistMgr->transfer(TMEMBER(_lastMimicX));
	persistMgr->transfer(TMEMBER(_lastMimicY));
	_points.persist(persistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdWaypointGroup::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("waypoint-group");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Active") == 0) {
		_scValue->setBool(_active);
		return _scValue;
	}

	else return CBObject::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdWaypointGroup::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Active") == 0) {
		_active = value->getBool();
		return S_OK;
	}

	else return CBObject::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdWaypointGroup::Mimic(CAdWaypointGroup *Wpt, float Scale, int X, int Y) {
	if (Scale == _lastMimicScale && X == _lastMimicX && Y == _lastMimicY) return S_OK;

	cleanup();

	for (int i = 0; i < Wpt->_points.GetSize(); i++) {
		int x, y;

		x = (int)((float)Wpt->_points[i]->x * Scale / 100.0f);
		y = (int)((float)Wpt->_points[i]->y * Scale / 100.0f);

		_points.Add(new CBPoint(x + X, y + Y));
	}

	_lastMimicScale = Scale;
	_lastMimicX = X;
	_lastMimicY = Y;

	return S_OK;
}

} // end of namespace WinterMute
