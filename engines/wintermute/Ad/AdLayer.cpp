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
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Ad/AdLayer.h"
#include "engines/wintermute/Ad/AdSceneNode.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdLayer, false)

//////////////////////////////////////////////////////////////////////////
CAdLayer::CAdLayer(CBGame *inGame): CBObject(inGame) {
	_main = false;
	_width = _height = 0;
	_active = true;
	_closeUp = false;
}


//////////////////////////////////////////////////////////////////////////
CAdLayer::~CAdLayer() {
	for (int i = 0; i < _nodes.GetSize(); i++)
		delete _nodes[i];
	_nodes.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdLayer::LoadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdLayer::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing LAYER file '%s'", Filename);


	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(LAYER)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(NAME)
TOKEN_DEF(WIDTH)
TOKEN_DEF(HEIGHT)
TOKEN_DEF(MAIN)
TOKEN_DEF(ENTITY)
TOKEN_DEF(REGION)
TOKEN_DEF(ACTIVE)
TOKEN_DEF(EDITOR_SELECTED)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(CLOSE_UP)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdLayer::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(LAYER)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(WIDTH)
	TOKEN_TABLE(HEIGHT)
	TOKEN_TABLE(MAIN)
	TOKEN_TABLE(ENTITY)
	TOKEN_TABLE(REGION)
	TOKEN_TABLE(ACTIVE)
	TOKEN_TABLE(EDITOR_SELECTED)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(CLOSE_UP)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_LAYER) {
			Game->LOG(0, "'LAYER' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_MAIN:
			parser.ScanStr((char *)params, "%b", &_main);
			break;

		case TOKEN_CLOSE_UP:
			parser.ScanStr((char *)params, "%b", &_closeUp);
			break;

		case TOKEN_WIDTH:
			parser.ScanStr((char *)params, "%d", &_width);
			break;

		case TOKEN_HEIGHT:
			parser.ScanStr((char *)params, "%d", &_height);
			break;

		case TOKEN_ACTIVE:
			parser.ScanStr((char *)params, "%b", &_active);
			break;

		case TOKEN_REGION: {
			CAdRegion *region = new CAdRegion(Game);
			CAdSceneNode *node = new CAdSceneNode(Game);
			if (!region || !node || FAILED(region->LoadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete region;
				delete node;
				region = NULL;
				node = NULL;
			} else {
				node->SetRegion(region);
				_nodes.Add(node);
			}
		}
		break;

		case TOKEN_ENTITY: {
			CAdEntity *entity = new CAdEntity(Game);
			CAdSceneNode *node = new CAdSceneNode(Game);
			if (entity) entity->_zoomable = false; // scene entites default to NOT zoom
			if (!entity || !node || FAILED(entity->LoadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete entity;
				delete node;
				entity = NULL;
				node = NULL;
			} else {
				node->SetEntity(entity);
				_nodes.Add(node);
			}
		}
		break;

		case TOKEN_EDITOR_SELECTED:
			parser.ScanStr((char *)params, "%b", &_editorSelected);
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
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
		Game->LOG(0, "Syntax error in LAYER definition");
		return E_FAIL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdLayer::scCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// GetNode
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetNode") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		int node = -1;

		if (val->_type == VAL_INT) node = val->GetInt();
		else { // get by name
			for (int i = 0; i < _nodes.GetSize(); i++) {
				if ((_nodes[i]->_type == OBJECT_ENTITY && scumm_stricmp(_nodes[i]->_entity->_name, val->GetString()) == 0) ||
				        (_nodes[i]->_type == OBJECT_REGION && scumm_stricmp(_nodes[i]->_region->_name, val->GetString()) == 0)) {
					node = i;
					break;
				}
			}
		}

		if (node < 0 || node >= _nodes.GetSize()) Stack->PushNULL();
		else {
			switch (_nodes[node]->_type) {
			case OBJECT_ENTITY:
				Stack->PushNative(_nodes[node]->_entity, true);
				break;
			case OBJECT_REGION:
				Stack->PushNative(_nodes[node]->_region, true);
				break;
			default:
				Stack->PushNULL();
			}
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddRegion / AddEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddRegion") == 0 || strcmp(Name, "AddEntity") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CAdSceneNode *Node = new CAdSceneNode(Game);
		if (strcmp(Name, "AddRegion") == 0) {
			CAdRegion *Region = new CAdRegion(Game);
			if (!Val->IsNULL()) Region->setName(Val->GetString());
			Node->SetRegion(Region);
			Stack->PushNative(Region, true);
		} else {
			CAdEntity *Entity = new CAdEntity(Game);
			if (!Val->IsNULL()) Entity->setName(Val->GetString());
			Node->SetEntity(Entity);
			Stack->PushNative(Entity, true);
		}
		_nodes.Add(Node);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertRegion / InsertEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InsertRegion") == 0 || strcmp(Name, "InsertEntity") == 0) {
		Stack->CorrectParams(2);
		int Index = Stack->Pop()->GetInt();
		CScValue *Val = Stack->Pop();

		CAdSceneNode *Node = new CAdSceneNode(Game);
		if (strcmp(Name, "InsertRegion") == 0) {
			CAdRegion *Region = new CAdRegion(Game);
			if (!Val->IsNULL()) Region->setName(Val->GetString());
			Node->SetRegion(Region);
			Stack->PushNative(Region, true);
		} else {
			CAdEntity *Entity = new CAdEntity(Game);
			if (!Val->IsNULL()) Entity->setName(Val->GetString());
			Node->SetEntity(Entity);
			Stack->PushNative(Entity, true);
		}
		if (Index < 0) Index = 0;
		if (Index <= _nodes.GetSize() - 1) _nodes.InsertAt(Index, Node);
		else _nodes.Add(Node);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteNode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteNode") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CAdSceneNode *ToDelete = NULL;
		if (Val->IsNative()) {
			CBScriptable *Temp = Val->GetNative();
			for (int i = 0; i < _nodes.GetSize(); i++) {
				if (_nodes[i]->_region == Temp || _nodes[i]->_entity == Temp) {
					ToDelete = _nodes[i];
					break;
				}
			}
		} else {
			int Index = Val->GetInt();
			if (Index >= 0 && Index < _nodes.GetSize()) {
				ToDelete = _nodes[Index];
			}
		}
		if (ToDelete == NULL) {
			Stack->PushBool(false);
			return S_OK;
		}

		for (int i = 0; i < _nodes.GetSize(); i++) {
			if (_nodes[i] == ToDelete) {
				delete _nodes[i];
				_nodes[i] = NULL;
				_nodes.RemoveAt(i);
				break;
			}
		}
		Stack->PushBool(true);
		return S_OK;
	}

	else return CBObject::scCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdLayer::scGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("layer");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumNodes (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumNodes") == 0) {
		_scValue->SetInt(_nodes.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Width") == 0) {
		_scValue->SetInt(_width);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Height") == 0) {
		_scValue->SetInt(_height);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Main (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Main") == 0) {
		_scValue->SetBool(_main);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CloseUp
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CloseUp") == 0) {
		_scValue->SetBool(_closeUp);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Active") == 0) {
		_scValue->SetBool(_active);
		return _scValue;
	}

	else return CBObject::scGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdLayer::scSetProperty(const char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Name") == 0) {
		setName(Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CloseUp
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CloseUp") == 0) {
		_closeUp = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Width") == 0) {
		_width = Value->GetInt();
		if (_width < 0) _width = 0;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Height") == 0) {
		_height = Value->GetInt();
		if (_height < 0) _height = 0;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Active") == 0) {
		bool b = Value->GetBool();
		if (b == false && _main) {
			Game->LOG(0, "Warning: cannot deactivate scene's main layer");
		} else _active = b;
		return S_OK;
	}

	else return CBObject::scSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
const char *CAdLayer::scToString() {
	return "[layer]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdLayer::saveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "LAYER {\n");
	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", _name);
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", getCaption());
	Buffer->PutTextIndent(Indent + 2, "MAIN=%s\n", _main ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "WIDTH=%d\n", _width);
	Buffer->PutTextIndent(Indent + 2, "HEIGHT=%d\n", _height);
	Buffer->PutTextIndent(Indent + 2, "ACTIVE=%s\n", _active ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_SELECTED=%s\n", _editorSelected ? "TRUE" : "FALSE");
	if (_closeUp)
		Buffer->PutTextIndent(Indent + 2, "CLOSE_UP=%s\n", _closeUp ? "TRUE" : "FALSE");

	int i;

	for (i = 0; i < _scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	if (_scProp) _scProp->saveAsText(Buffer, Indent + 2);

	for (i = 0; i < _nodes.GetSize(); i++) {
		switch (_nodes[i]->_type) {
		case OBJECT_ENTITY:
			_nodes[i]->_entity->saveAsText(Buffer, Indent + 2);
			break;
		case OBJECT_REGION:
			_nodes[i]->_region->saveAsText(Buffer, Indent + 2);
			break;
		default:
			error("CAdLayer::SaveAsText - Unhandled enum");
			break;
		}
	}

	CBBase::saveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdLayer::persist(CBPersistMgr *persistMgr) {

	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_active));
	persistMgr->transfer(TMEMBER(_closeUp));
	persistMgr->transfer(TMEMBER(_height));
	persistMgr->transfer(TMEMBER(_main));
	_nodes.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_width));

	return S_OK;
}

} // end of namespace WinterMute
