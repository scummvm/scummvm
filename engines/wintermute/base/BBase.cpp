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
#include "engines/wintermute/base/BBase.h"
#include "engines/wintermute/base/BGame.h"
#include "engines/wintermute/base/BParser.h"
#include "engines/wintermute/base/BDynBuffer.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
CBBase::CBBase(CBGame *gameOwner) {
	_gameRef = gameOwner;
	_persistable = true;
}


//////////////////////////////////////////////////////////////////////////
CBBase::CBBase() {
	_gameRef = NULL;
	_persistable = true;
}


//////////////////////////////////////////////////////////////////////
CBBase::~CBBase() {
	_editorProps.clear();
}


//////////////////////////////////////////////////////////////////////////
const char *CBBase::getEditorProp(const char *propName, const char *initVal) {
	_editorPropsIter = _editorProps.find(propName);
	if (_editorPropsIter != _editorProps.end())
		return _editorPropsIter->_value.c_str();
	//return _editorPropsIter->second.c_str(); // <- TODO Clean
	else return initVal;
}


//////////////////////////////////////////////////////////////////////////
bool CBBase::setEditorProp(const char *propName, const char *propValue) {
	if (propName == NULL) return STATUS_FAILED;

	if (propValue == NULL) {
		_editorProps.erase(propName);
	} else {
		_editorProps[propName] = propValue;
	}
	return STATUS_OK;
}



TOKEN_DEF_START
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(NAME)
TOKEN_DEF(VALUE)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool CBBase::parseEditorProperty(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(VALUE)
	TOKEN_TABLE_END


	if (!_gameRef->_editorMode)
		return STATUS_OK;


	byte *params;
	int cmd;
	CBParser parser(_gameRef);

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_EDITOR_PROPERTY) {
			_gameRef->LOG(0, "'EDITOR_PROPERTY' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	char *propName = NULL;
	char *propValue = NULL;

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_NAME:
			delete[] propName;
			propName = new char[strlen((char *)params) + 1];
			if (propName) strcpy(propName, (char *)params);
			else cmd = PARSERR_GENERIC;
			break;

		case TOKEN_VALUE:
			delete[] propValue;
			propValue = new char[strlen((char *)params) + 1];
			if (propValue) strcpy(propValue, (char *)params);
			else cmd = PARSERR_GENERIC;
			break;
		}

	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		delete[] propName;
		delete[] propValue;
		propName = NULL;
		propValue = NULL;
		_gameRef->LOG(0, "Syntax error in EDITOR_PROPERTY definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC || propName == NULL || propValue == NULL) {
		delete[] propName;
		delete[] propValue;
		propName = NULL;
		propValue = NULL;
		_gameRef->LOG(0, "Error loading EDITOR_PROPERTY definition");
		return STATUS_FAILED;
	}


	setEditorProp(propName, propValue);

	delete[] propName;
	delete[] propValue;
	propName = NULL;
	propValue = NULL;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBBase::saveAsText(CBDynBuffer *buffer, int indent) {
	_editorPropsIter = _editorProps.begin();
	while (_editorPropsIter != _editorProps.end()) {
		buffer->putTextIndent(indent, "EDITOR_PROPERTY\n");
		buffer->putTextIndent(indent, "{\n");
		buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _editorPropsIter->_key.c_str());
		buffer->putTextIndent(indent + 2, "VALUE=\"%s\"\n", _editorPropsIter->_value.c_str());
		//buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", (char *)_editorPropsIter->first.c_str()); // <- TODO, remove
		//buffer->putTextIndent(indent + 2, "VALUE=\"%s\"\n", _editorPropsIter->second.c_str()); // <- TODO, remove
		buffer->putTextIndent(indent, "}\n\n");

		_editorPropsIter++;
	}
	return STATUS_OK;
}

} // end of namespace WinterMute
