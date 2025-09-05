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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/ad/ad_layer.h"
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_scene_node.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/dcgf.h"

#include "common/str.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdLayer, false)

//////////////////////////////////////////////////////////////////////////
AdLayer::AdLayer(BaseGame *inGame) : BaseObject(inGame) {
	_main = false;
	_width = _height = 0;
	_active = true;
	_closeUp = false;
}


//////////////////////////////////////////////////////////////////////////
AdLayer::~AdLayer() {
	for (int32 i = 0; i < _nodes.getSize(); i++) {
		delete _nodes[i];
	}
	_nodes.removeAll();
}


//////////////////////////////////////////////////////////////////////////
bool AdLayer::loadFile(const char *filename) {
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_game->LOG(0, "AdLayer::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_game->LOG(0, "Error parsing LAYER file '%s'", filename);
	}

	delete[] buffer;

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
bool AdLayer::loadBuffer(char *buffer, bool complete) {
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

	char *params;
	int cmd;
	BaseParser parser(_game);

	if (complete) {
		if (parser.getCommand(&buffer, commands, &params) != TOKEN_LAYER) {
			_game->LOG(0, "'LAYER' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile(params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_NAME:
			setName(params);
			break;

		case TOKEN_CAPTION:
			setCaption(params);
			break;

		case TOKEN_MAIN:
			parser.scanStr(params, "%b", &_main);
			break;

		case TOKEN_CLOSE_UP:
			parser.scanStr(params, "%b", &_closeUp);
			break;

		case TOKEN_WIDTH:
			parser.scanStr(params, "%d", &_width);
			break;

		case TOKEN_HEIGHT:
			parser.scanStr(params, "%d", &_height);
			break;

		case TOKEN_ACTIVE:
			parser.scanStr(params, "%b", &_active);
			break;

		case TOKEN_REGION: {
			AdRegion *region = new AdRegion(_game);
			AdSceneNode *node = new AdSceneNode(_game);
			if (!region || !node || DID_FAIL(region->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				SAFE_DELETE(region);
				SAFE_DELETE(node);
			} else {
				node->setRegion(region);
				_nodes.add(node);
			}
		}
		break;

		case TOKEN_ENTITY: {
			AdEntity *entity = new AdEntity(_game);
			AdSceneNode *node = new AdSceneNode(_game);
			if (entity) {
				entity->_zoomable = false;    // scene entites default to NOT zoom
			}
			if (!entity || !node || DID_FAIL(entity->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				SAFE_DELETE(entity);
				SAFE_DELETE(node);
			} else {
				node->setEntity(entity);
				_nodes.add(node);
			}
		}
		break;

		case TOKEN_EDITOR_SELECTED:
			parser.scanStr(params, "%b", &_editorSelected);
			break;

		case TOKEN_SCRIPT:
			addScript(params);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		default:
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_game->LOG(0, "Syntax error in LAYER definition");
		return STATUS_FAILED;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool AdLayer::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// GetNode
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetNode") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		int32 node = -1;

		if (val->_type == VAL_INT) {
			node = val->getInt();
		} else { // get by name
			for (int32 i = 0; i < _nodes.getSize(); i++) {
				if ((_nodes[i]->_type == OBJECT_ENTITY && scumm_stricmp(_nodes[i]->_entity->_name, val->getString()) == 0) ||
				        (_nodes[i]->_type == OBJECT_REGION && scumm_stricmp(_nodes[i]->_region->_name, val->getString()) == 0)) {
					node = i;
					break;
				}
			}
		}

		if (node < 0 || node >= _nodes.getSize()) {
			stack->pushNULL();
		} else {
			switch (_nodes[node]->_type) {
			case OBJECT_ENTITY:
				stack->pushNative(_nodes[node]->_entity, true);
				break;
			case OBJECT_REGION:
				stack->pushNative(_nodes[node]->_region, true);
				break;
			default:
				stack->pushNULL();
			}
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddRegion / AddEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddRegion") == 0 || strcmp(name, "AddEntity") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdSceneNode *node = new AdSceneNode(_game);
		if (strcmp(name, "AddRegion") == 0) {
			AdRegion *region = new AdRegion(_game);
			if (!val->isNULL()) {
				region->setName(val->getString());
			}
			node->setRegion(region);
			stack->pushNative(region, true);
		} else {
			AdEntity *entity = new AdEntity(_game);
			if (!val->isNULL()) {
				entity->setName(val->getString());
			}
			node->setEntity(entity);
			stack->pushNative(entity, true);
		}
		_nodes.add(node);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertRegion / InsertEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InsertRegion") == 0 || strcmp(name, "InsertEntity") == 0) {
		stack->correctParams(2);
		int32 index = stack->pop()->getInt();
		ScValue *val = stack->pop();

		AdSceneNode *node = new AdSceneNode(_game);
		if (strcmp(name, "InsertRegion") == 0) {
			AdRegion *region = new AdRegion(_game);
			if (!val->isNULL()) {
				region->setName(val->getString());
			}
			node->setRegion(region);
			stack->pushNative(region, true);
		} else {
			AdEntity *entity = new AdEntity(_game);
			if (!val->isNULL()) {
				entity->setName(val->getString());
			}
			node->setEntity(entity);
			stack->pushNative(entity, true);
		}
		if (index < 0) {
			index = 0;
		}
		if (index <= _nodes.getSize() - 1) {
			_nodes.insertAt(index, node);
		} else {
			_nodes.add(node);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteNode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteNode") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdSceneNode *toDelete = nullptr;
		if (val->isNative()) {
			BaseScriptable *temp = val->getNative();
			for (int32 i = 0; i < _nodes.getSize(); i++) {
				if (_nodes[i]->_region == temp || _nodes[i]->_entity == temp) {
					toDelete = _nodes[i];
					break;
				}
			}
		} else {
			int32 index = val->getInt();
			if (index >= 0 && index < _nodes.getSize()) {
				toDelete = _nodes[index];
			}
		}
		if (toDelete == nullptr) {
			stack->pushBool(false);
			return STATUS_OK;
		}

		for (int32 i = 0; i < _nodes.getSize(); i++) {
			if (_nodes[i] == toDelete) {
				SAFE_DELETE(_nodes[i]);
				_nodes.removeAt(i);
				break;
			}
		}
		stack->pushBool(true);
		return STATUS_OK;
	} else {
		return BaseObject::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *AdLayer::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("layer");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumNodes (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumNodes") == 0) {
		_scValue->setInt(_nodes.getSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Width") == 0) {
		_scValue->setInt(_width);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Height") == 0) {
		_scValue->setInt(_height);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Main (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Main") == 0) {
		_scValue->setBool(_main);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CloseUp
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CloseUp") == 0) {
		_scValue->setBool(_closeUp);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Active") == 0) {
		_scValue->setBool(_active);
		return _scValue;
	} else {
		return BaseObject::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdLayer::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CloseUp
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CloseUp") == 0) {
		_closeUp = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Width") == 0) {
		_width = value->getInt();
		if (_width < 0) {
			_width = 0;
		}
		((AdGame *)_game)->_scene->onLayerResized(this);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Height") == 0) {
		_height = value->getInt();
		if (_height < 0) {
			_height = 0;
		}
		((AdGame *)_game)->_scene->onLayerResized(this);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Active") == 0) {
		bool b = value->getBool();
		if (b == false && _main) {
			_game->LOG(0, "Warning: cannot deactivate scene's main layer");
		} else {
			_active = b;
		}
		return STATUS_OK;
	} else {
		return BaseObject::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *AdLayer::scToString() {
	return "[layer]";
}


//////////////////////////////////////////////////////////////////////////
bool AdLayer::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "LAYER {\n");
	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _name);
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());
	buffer->putTextIndent(indent + 2, "MAIN=%s\n", _main ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "WIDTH=%d\n", _width);
	buffer->putTextIndent(indent + 2, "HEIGHT=%d\n", _height);
	buffer->putTextIndent(indent + 2, "ACTIVE=%s\n", _active ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SELECTED=%s\n", _editorSelected ? "TRUE" : "FALSE");
	if (_closeUp) {
		buffer->putTextIndent(indent + 2, "CLOSE_UP=%s\n", _closeUp ? "TRUE" : "FALSE");
	}

	for (int32 i = 0; i < _scripts.getSize(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	if (_scProp) {
		_scProp->saveAsText(buffer, indent + 2);
	}

	for (int32 i = 0; i < _nodes.getSize(); i++) {
		switch (_nodes[i]->_type) {
		case OBJECT_ENTITY:
			_nodes[i]->_entity->saveAsText(buffer, indent + 2);
			break;
		case OBJECT_REGION:
			_nodes[i]->_region->saveAsText(buffer, indent + 2);
			break;
		default:
			error("AdLayer::SaveAsText - Unhandled enum");
			break;
		}
	}

	BaseClass::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n\n");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdLayer::persist(BasePersistenceManager *persistMgr) {

	BaseObject::persist(persistMgr);

	persistMgr->transferBool(TMEMBER(_active));
	persistMgr->transferBool(TMEMBER(_closeUp));
	persistMgr->transferSint32(TMEMBER(_height));
	persistMgr->transferBool(TMEMBER(_main));
	_nodes.persist(persistMgr);
	persistMgr->transferSint32(TMEMBER(_width));

	return STATUS_OK;
}

} // End of namespace Wintermute
