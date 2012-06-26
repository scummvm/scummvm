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

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/BBase.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BDynBuffer.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
CBBase::CBBase(CBGame *GameOwner) {
	Game = GameOwner;
	_persistable = true;
}


//////////////////////////////////////////////////////////////////////////
CBBase::CBBase() {
	Game = NULL;
	_persistable = true;
}


//////////////////////////////////////////////////////////////////////
CBBase::~CBBase() {
	_editorProps.clear();
}


//////////////////////////////////////////////////////////////////////////
const char *CBBase::getEditorProp(const char *PropName, const char *InitVal) {
	_editorPropsIter = _editorProps.find(PropName);
	if (_editorPropsIter != _editorProps.end())
		return _editorPropsIter->_value.c_str();
	//return _editorPropsIter->second.c_str(); // <- TODO Clean
	else return InitVal;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBBase::setEditorProp(const char *PropName, const char *PropValue) {
	if (PropName == NULL) return E_FAIL;

	if (PropValue == NULL) {
		_editorProps.erase(PropName);
	} else {
		_editorProps[PropName] = PropValue;
	}
	return S_OK;
}



TOKEN_DEF_START
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(NAME)
TOKEN_DEF(VALUE)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CBBase::parseEditorProperty(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(VALUE)
	TOKEN_TABLE_END


	if (!Game->_editorMode) return S_OK;


	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_EDITOR_PROPERTY) {
			Game->LOG(0, "'EDITOR_PROPERTY' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	char *PropName = NULL;
	char *PropValue = NULL;

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_NAME:
			delete[] PropName;
			PropName = new char[strlen((char *)params) + 1];
			if (PropName) strcpy(PropName, (char *)params);
			else cmd = PARSERR_GENERIC;
			break;

		case TOKEN_VALUE:
			delete[] PropValue;
			PropValue = new char[strlen((char *)params) + 1];
			if (PropValue) strcpy(PropValue, (char *)params);
			else cmd = PARSERR_GENERIC;
			break;
		}

	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		delete[] PropName;
		delete[] PropValue;
		PropName = NULL;
		PropValue = NULL;
		Game->LOG(0, "Syntax error in EDITOR_PROPERTY definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC || PropName == NULL || PropValue == NULL) {
		delete[] PropName;
		delete[] PropValue;
		PropName = NULL;
		PropValue = NULL;
		Game->LOG(0, "Error loading EDITOR_PROPERTY definition");
		return E_FAIL;
	}


	setEditorProp(PropName, PropValue);

	delete[] PropName;
	delete[] PropValue;
	PropName = NULL;
	PropValue = NULL;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBBase::saveAsText(CBDynBuffer *Buffer, int Indent) {
	_editorPropsIter = _editorProps.begin();
	while (_editorPropsIter != _editorProps.end()) {
		Buffer->putTextIndent(Indent, "EDITOR_PROPERTY\n");
		Buffer->putTextIndent(Indent, "{\n");
		Buffer->putTextIndent(Indent + 2, "NAME=\"%s\"\n", _editorPropsIter->_key.c_str());
		Buffer->putTextIndent(Indent + 2, "VALUE=\"%s\"\n", _editorPropsIter->_value.c_str());
		//Buffer->putTextIndent(Indent + 2, "NAME=\"%s\"\n", (char *)_editorPropsIter->first.c_str()); // <- TODO, remove
		//Buffer->putTextIndent(Indent + 2, "VALUE=\"%s\"\n", _editorPropsIter->second.c_str()); // <- TODO, remove
		Buffer->putTextIndent(Indent, "}\n\n");

		_editorPropsIter++;
	}
	return S_OK;
}

} // end of namespace WinterMute
