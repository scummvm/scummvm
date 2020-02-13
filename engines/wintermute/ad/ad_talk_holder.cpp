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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/ad/ad_talk_holder.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_ext_array.h"
#include "engines/wintermute/platform_osystem.h"
#include "common/str.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdTalkHolder, false)

//////////////////////////////////////////////////////////////////////////
AdTalkHolder::AdTalkHolder(BaseGame *inGame) : AdObject(inGame) {
	_sprite = nullptr;
}


//////////////////////////////////////////////////////////////////////////
AdTalkHolder::~AdTalkHolder() {
	delete _sprite;
	_sprite = nullptr;

	for (uint32 i = 0; i < _talkSprites.size(); i++) {
		delete _talkSprites[i];
	}
	_talkSprites.clear();

	for (uint32 i = 0; i < _talkSpritesEx.size(); i++) {
		delete _talkSpritesEx[i];
	}
	_talkSpritesEx.clear();
}

//////////////////////////////////////////////////////////////////////////
BaseSprite *AdTalkHolder::getTalkStance(const char *stance) {
	BaseSprite *ret = nullptr;


	// forced stance?
	if (_forcedTalkAnimName && !_forcedTalkAnimUsed) {
		_forcedTalkAnimUsed = true;
		delete _animSprite;
		_animSprite = new BaseSprite(_gameRef, this);
		if (_animSprite) {
			bool res = _animSprite->loadFile(_forcedTalkAnimName);
			if (DID_FAIL(res)) {
				_gameRef->LOG(res, "AdTalkHolder::GetTalkStance: error loading talk sprite (object:\"%s\" sprite:\"%s\")", getName(), _forcedTalkAnimName);
				delete _animSprite;
				_animSprite = nullptr;
			} else {
				return _animSprite;
			}
		}
	}


	if (stance != nullptr) {
		// search special talk stances
		for (uint32 i = 0; i < _talkSpritesEx.size(); i++) {
			if (scumm_stricmp(_talkSpritesEx[i]->getName(), stance) == 0) {
				ret = _talkSpritesEx[i];
				break;
			}
		}
		if (ret == nullptr) {
			// serach generic talk stances
			for (uint32 i = 0; i < _talkSprites.size(); i++) {
				if (scumm_stricmp(_talkSprites[i]->getName(), stance) == 0) {
					ret = _talkSprites[i];
					break;
				}
			}
		}
	}

	// not a valid stance? get a random one
	if (ret == nullptr) {
		if (_talkSprites.size() < 1) {
			ret = _sprite;
		} else {
			// TODO: remember last
			int rnd = BaseEngine::instance().randInt(0, _talkSprites.size() - 1);
			ret = _talkSprites[rnd];
		}
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool AdTalkHolder::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetSprite
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetSprite") == 0) {
		stack->correctParams(1);

		ScValue *val = stack->pop();

		bool setCurrent = false;
		if (_currentSprite && _currentSprite == _sprite) {
			setCurrent = true;
		}

		delete _sprite;
		_sprite = nullptr;

		if (val->isNULL()) {
			_sprite = nullptr;
			if (setCurrent) {
				_currentSprite = nullptr;
			}
			stack->pushBool(true);
		} else {
			const char *filename = val->getString();
			BaseSprite *spr = new BaseSprite(_gameRef, this);
			if (!spr || DID_FAIL(spr->loadFile(filename))) {
				script->runtimeError("SetSprite method failed for file '%s'", filename);
				stack->pushBool(false);
			} else {
				_sprite = spr;
				if (setCurrent) {
					_currentSprite = _sprite;
				}
				stack->pushBool(true);
			}
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSprite") == 0) {
		stack->correctParams(0);

		if (!_sprite || !_sprite->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_sprite->getFilename());
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSpriteObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSpriteObject") == 0) {
		stack->correctParams(0);

		if (!_sprite) {
			stack->pushNULL();
		} else {
			stack->pushNative(_sprite, true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddTalkSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddTalkSprite") == 0) {
		stack->correctParams(2);

		const char *filename = stack->pop()->getString();
		bool ex = stack->pop()->getBool();
		BaseArray<BaseSprite *> &sprites = ex ? _talkSpritesEx : _talkSprites;

		BaseSprite *spr = new BaseSprite(_gameRef, this);
		if (!spr || DID_FAIL(spr->loadFile(filename))) {
			stack->pushBool(false);
			script->runtimeError("AddTalkSprite method failed for file '%s'", filename);
		} else {
			sprites.add(spr);
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveTalkSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveTalkSprite") == 0) {
		stack->correctParams(2);

		const char *filename = stack->pop()->getString();
		bool ex = stack->pop()->getBool();
		BaseArray<BaseSprite *> &sprites = ex ? _talkSpritesEx : _talkSprites;

		for (uint32 i = 0; i < sprites.size(); i++) {
			if (scumm_stricmp(sprites[i]->getFilename(), filename) == 0) {
				if (_currentSprite == sprites[i]) {
					_currentSprite = _sprite;
				}
				if (_tempSprite2 == sprites[i]) {
					_tempSprite2 = _sprite;
				}
				delete sprites[i];
				sprites.remove_at(i);
				break;
			}
		}
		stack->pushBool(true);
		return STATUS_OK;
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] GetTalkSprites
	// This is used once, to store Fenek's TalkSprites array to a temporary var
	// Later state is restored with this.SetTalkSprite(array_talk_sprites[0])
	// Return value should be array
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetTalkSprites") == 0) {
		stack->correctParams(1);
		bool ex = stack->pop()->getBool();
		BaseArray<BaseSprite *> &sprites = ex ? _talkSpritesEx : _talkSprites;

		BaseScriptable *arr;
		stack->pushInt(0);
		arr = makeSXArray(_gameRef, stack);
		for (uint32 i = 0; i < sprites.size(); i++) {
			stack->pushString(sprites[i]->getFilename());
			((SXArray *)arr)->push(stack->pop());
		}
		stack->pushNative(arr, false);
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// SetTalkSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetTalkSprite") == 0) {
		stack->correctParams(2);

		const char *filename = stack->pop()->getString();
		bool ex = stack->pop()->getBool();
		BaseArray<BaseSprite *> &sprites = ex ? _talkSpritesEx : _talkSprites;

		BaseSprite *spr = new BaseSprite(_gameRef, this);
		if (!spr || DID_FAIL(spr->loadFile(filename))) {
			stack->pushBool(false);
			script->runtimeError("SetTalkSprite method failed for file '%s'", filename);
		} else {
			for (uint32 i = 0; i < sprites.size(); i++) {
				if (_currentSprite == sprites[i]) {
					_currentSprite = spr;
				}
				if (_tempSprite2 == sprites[i]) {
					_tempSprite2 = spr;
				}
				delete sprites[i];
			}
			sprites.clear();
			sprites.add(spr);
			stack->pushBool(true);
		}
		return STATUS_OK;
	} else {
		return AdObject::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *AdTalkHolder::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("talk-holder");
		return _scValue;
	} else {
		return AdObject::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdTalkHolder::scSetProperty(const char *name, ScValue *value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Item
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Item")==0) {
	    SetItem(value->getString());
	    return STATUS_OK;
	}

	else*/ return AdObject::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *AdTalkHolder::scToString() {
	return "[talk-holder object]";
}


//////////////////////////////////////////////////////////////////////////
bool AdTalkHolder::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	for (uint32 i = 0; i < _talkSprites.size(); i++) {
		if (_talkSprites[i]->getFilename()) {
			buffer->putTextIndent(indent + 2, "TALK=\"%s\"\n", _talkSprites[i]->getFilename());
		}
	}

	for (uint32 i = 0; i < _talkSpritesEx.size(); i++) {
		if (_talkSpritesEx[i]->getFilename()) {
			buffer->putTextIndent(indent + 2, "TALK_SPECIAL=\"%s\"\n", _talkSpritesEx[i]->getFilename());
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdTalkHolder::persist(BasePersistenceManager *persistMgr) {
	AdObject::persist(persistMgr);

	persistMgr->transferPtr(TMEMBER_PTR(_sprite));
	_talkSprites.persist(persistMgr);
	_talkSpritesEx.persist(persistMgr);

	return STATUS_OK;
}

} // End of namespace Wintermute
