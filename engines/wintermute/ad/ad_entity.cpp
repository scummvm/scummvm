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


#include "engines/wintermute/ad/ad_entity.h"
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_layer.h"
#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_scene_node.h"
#include "engines/wintermute/ad/ad_waypoint_group.h"
#include "engines/wintermute/ad/ad_sentence.h"
#include "engines/wintermute/base/base_active_rect.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_surface_storage.h"
#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/particles/part_emitter.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/video/video_theora_player.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/platform_osystem.h"
#include "common/str.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdEntity, false)

//////////////////////////////////////////////////////////////////////////
AdEntity::AdEntity(BaseGame *inGame) : AdTalkHolder(inGame) {
	_type = OBJECT_ENTITY;
	_subtype = ENTITY_NORMAL;
	_region = nullptr;
	_item = nullptr;

	_walkToX = _walkToY = 0;
	_walkToDir = DI_NONE;

#ifdef ENABLE_FOXTAIL
	_hintX = _hintY = -1;
#endif

	_theora = nullptr;
}


//////////////////////////////////////////////////////////////////////////
AdEntity::~AdEntity() {
	_gameRef->unregisterObject(_region);

	delete _theora;
	_theora = nullptr;

	delete[] _item;
	_item = nullptr;
}

int32 AdEntity::getWalkToX() const {
	return _walkToX;
}

int32 AdEntity::getWalkToY() const {
	return _walkToY;
}

TDirection AdEntity::getWalkToDir() const {
	return _walkToDir;
}

const char *AdEntity::getItemName() const {
	return _item;
}

#ifdef ENABLE_FOXTAIL
int32 AdEntity::getHintX() const {
	return _hintX;
}

int32 AdEntity::getHintY() const {
	return _hintY;
}
#endif

//////////////////////////////////////////////////////////////////////////
bool AdEntity::loadFile(const char *filename) {
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_gameRef->LOG(0, "AdEntity::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing ENTITY file '%s'", filename);
	}


	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(ENTITY)
TOKEN_DEF(SPRITE)
TOKEN_DEF(X)
TOKEN_DEF(Y)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(NAME)
TOKEN_DEF(SCALABLE)
TOKEN_DEF(REGISTRABLE)
TOKEN_DEF(INTERACTIVE)
TOKEN_DEF(SHADOWABLE)
TOKEN_DEF(COLORABLE)
TOKEN_DEF(ACTIVE)
TOKEN_DEF(EVENTS)
TOKEN_DEF(FONT)
TOKEN_DEF(TALK_SPECIAL)
TOKEN_DEF(TALK)
TOKEN_DEF(CURSOR)
TOKEN_DEF(REGION)
TOKEN_DEF(BLOCKED_REGION)
TOKEN_DEF(EDITOR_SELECTED)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(SOUND_START_TIME)
TOKEN_DEF(SOUND_VOLUME)
TOKEN_DEF(SOUND_PANNING)
TOKEN_DEF(SOUND)
TOKEN_DEF(SUBTYPE)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(WAYPOINTS)
TOKEN_DEF(IGNORE_ITEMS)
TOKEN_DEF(ROTABLE)
TOKEN_DEF(ROTATABLE)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(SCALE)
TOKEN_DEF(RELATIVE_SCALE)
TOKEN_DEF(ALPHA)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(ITEM)
TOKEN_DEF(WALK_TO_X)
TOKEN_DEF(WALK_TO_Y)
TOKEN_DEF(WALK_TO_DIR)
TOKEN_DEF(SAVE_STATE)
#ifdef ENABLE_FOXTAIL
TOKEN_DEF(HINT_X)
TOKEN_DEF(HINT_Y)
#endif
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdEntity::loadBuffer(char *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ENTITY)
	TOKEN_TABLE(SPRITE)
	TOKEN_TABLE(X)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(SCALABLE)
	TOKEN_TABLE(REGISTRABLE)
	TOKEN_TABLE(INTERACTIVE)
	TOKEN_TABLE(SHADOWABLE)
	TOKEN_TABLE(COLORABLE)
	TOKEN_TABLE(ACTIVE)
	TOKEN_TABLE(EVENTS)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(TALK_SPECIAL)
	TOKEN_TABLE(TALK)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(REGION)
	TOKEN_TABLE(BLOCKED_REGION)
	TOKEN_TABLE(EDITOR_SELECTED)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(SOUND_START_TIME)
	TOKEN_TABLE(SOUND_VOLUME)
	TOKEN_TABLE(SOUND_PANNING)
	TOKEN_TABLE(SOUND)
	TOKEN_TABLE(SUBTYPE)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(WAYPOINTS)
	TOKEN_TABLE(IGNORE_ITEMS)
	TOKEN_TABLE(ROTABLE)
	TOKEN_TABLE(ROTATABLE)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(SCALE)
	TOKEN_TABLE(RELATIVE_SCALE)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(ITEM)
	TOKEN_TABLE(WALK_TO_X)
	TOKEN_TABLE(WALK_TO_Y)
	TOKEN_TABLE(WALK_TO_DIR)
	TOKEN_TABLE(SAVE_STATE)
#ifdef ENABLE_FOXTAIL
	TOKEN_TABLE(HINT_X)
	TOKEN_TABLE(HINT_Y)
#endif
	TOKEN_TABLE_END

	char *params;
	int cmd;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand(&buffer, commands, &params) != TOKEN_ENTITY) {
			_gameRef->LOG(0, "'ENTITY' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	AdGame *adGame = (AdGame *)_gameRef;
	BaseSprite *spr = nullptr;
	int ar = 0, ag = 0, ab = 0, alpha = 0;
	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile(params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_X:
			parser.scanStr(params, "%d", &_posX);
			break;

		case TOKEN_Y:
			parser.scanStr(params, "%d", &_posY);
			break;

		case TOKEN_SPRITE: {
			delete _sprite;
			_sprite = nullptr;
			spr = new BaseSprite(_gameRef, this);
			if (!spr || DID_FAIL(spr->loadFile(params))) {
				cmd = PARSERR_GENERIC;
			} else {
				_sprite = spr;
			}
		}
		break;

		case TOKEN_TALK: {
			spr = new BaseSprite(_gameRef, this);
			if (!spr || DID_FAIL(spr->loadFile(params, adGame->_texTalkLifeTime))) {
				cmd = PARSERR_GENERIC;
			} else {
				_talkSprites.add(spr);
			}
		}
		break;

		case TOKEN_TALK_SPECIAL: {
			spr = new BaseSprite(_gameRef, this);
			if (!spr || DID_FAIL(spr->loadFile(params, adGame->_texTalkLifeTime))) {
				cmd = PARSERR_GENERIC;
			} else {
				_talkSpritesEx.add(spr);
			}
		}
		break;

		case TOKEN_NAME:
			setName(params);
			break;

		case TOKEN_ITEM:
			setItem(params);
			break;

		case TOKEN_CAPTION:
			setCaption(params);
			break;

		case TOKEN_FONT:
			setFont(params);
			break;

		case TOKEN_SCALABLE:
			parser.scanStr(params, "%b", &_zoomable);
			break;

		case TOKEN_SCALE: {
			int s;
			parser.scanStr(params, "%d", &s);
			_scale = (float)s;

		}
		break;

		case TOKEN_RELATIVE_SCALE: {
			int s;
			parser.scanStr(params, "%d", &s);
			_relativeScale = (float)s;

		}
		break;

		case TOKEN_ROTABLE:
		case TOKEN_ROTATABLE:
			parser.scanStr(params, "%b", &_rotatable);
			break;

		case TOKEN_REGISTRABLE:
		case TOKEN_INTERACTIVE:
			parser.scanStr(params, "%b", &_registrable);
			break;

		case TOKEN_SHADOWABLE:
		case TOKEN_COLORABLE:
			parser.scanStr(params, "%b", &_shadowable);
			break;

		case TOKEN_ACTIVE:
			parser.scanStr(params, "%b", &_active);
			break;

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new BaseSprite(_gameRef);
			if (!_cursor || DID_FAIL(_cursor->loadFile(params))) {
				delete _cursor;
				_cursor = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_EDITOR_SELECTED:
			parser.scanStr(params, "%b", &_editorSelected);
			break;

		case TOKEN_REGION: {
			if (_region) {
				_gameRef->unregisterObject(_region);
			}
			_region = nullptr;
			BaseRegion *rgn = new BaseRegion(_gameRef);
			if (!rgn || DID_FAIL(rgn->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
			} else {
				_region = rgn;
				_gameRef->registerObject(_region);
			}
		}
		break;

		case TOKEN_BLOCKED_REGION: {
			delete _blockRegion;
			_blockRegion = nullptr;
			delete _currentBlockRegion;
			_currentBlockRegion = nullptr;
			BaseRegion *rgn = new BaseRegion(_gameRef);
			BaseRegion *crgn = new BaseRegion(_gameRef);
			if (!rgn || !crgn || DID_FAIL(rgn->loadBuffer(params, false))) {
				delete _blockRegion;
				_blockRegion = nullptr;
				delete _currentBlockRegion;
				_currentBlockRegion = nullptr;
				cmd = PARSERR_GENERIC;
			} else {
				_blockRegion = rgn;
				_currentBlockRegion = crgn;
				_currentBlockRegion->mimic(_blockRegion);
			}
		}
		break;

		case TOKEN_WAYPOINTS: {
			delete _wptGroup;
			_wptGroup = nullptr;
			delete _currentWptGroup;
			_currentWptGroup = nullptr;
			AdWaypointGroup *wpt = new AdWaypointGroup(_gameRef);
			AdWaypointGroup *cwpt = new AdWaypointGroup(_gameRef);
			if (!wpt || !cwpt || DID_FAIL(wpt->loadBuffer(params, false))) {
				delete _wptGroup;
				_wptGroup = nullptr;
				delete _currentWptGroup;
				_currentWptGroup = nullptr;
				cmd = PARSERR_GENERIC;
			} else {
				_wptGroup = wpt;
				_currentWptGroup = cwpt;
				_currentWptGroup->mimic(_wptGroup);
			}
		}
		break;

		case TOKEN_SCRIPT:
			addScript(params);
			break;

		case TOKEN_SUBTYPE: {
			if (scumm_stricmp(params, "sound") == 0) {
				delete _sprite;
				_sprite = nullptr;
				if (_gameRef->_editorMode) {
					spr = new BaseSprite(_gameRef, this);
					if (!spr || DID_FAIL(spr->loadFile("entity_sound.sprite"))) {
						cmd = PARSERR_GENERIC;
					} else {
						_sprite = spr;
					}
				}
				if (_gameRef->_editorMode) {
					_editorOnly = true;
				}
				_zoomable = false;
				_rotatable = false;
				_registrable = _gameRef->_editorMode;
				_shadowable = false;
				_subtype = ENTITY_SOUND;
			}
		}
		break;

		case TOKEN_SOUND:
			playSFX(params, false, false);
			break;

		case TOKEN_SOUND_START_TIME:
			parser.scanStr(params, "%d", &_sFXStart);
			break;

		case TOKEN_SOUND_VOLUME:
			parser.scanStr(params, "%d", &_sFXVolume);
			break;

		case TOKEN_SOUND_PANNING:
			parser.scanStr(params, "%b", &_autoSoundPanning);
			break;

		case TOKEN_SAVE_STATE:
			parser.scanStr(params, "%b", &_saveState);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;

		case TOKEN_IGNORE_ITEMS:
			parser.scanStr(params, "%b", &_ignoreItems);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.scanStr(params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.scanStr(params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		case TOKEN_WALK_TO_X:
			parser.scanStr(params, "%d", &_walkToX);
			break;

		case TOKEN_WALK_TO_Y:
			parser.scanStr(params, "%d", &_walkToY);
			break;

		case TOKEN_WALK_TO_DIR: {
			int i;
			parser.scanStr(params, "%d", &i);
			if (i < 0) {
				i = 0;
			}
			if (i >= NUM_DIRECTIONS) {
				i = DI_NONE;
			}
			_walkToDir = (TDirection)i;
			break;

#ifdef ENABLE_FOXTAIL
		case TOKEN_HINT_X:
			parser.scanStr(params, "%d", &_hintX);
			break;
		case TOKEN_HINT_Y:
			parser.scanStr(params, "%d", &_hintY);
			break;
#endif
		}
		break;

		default:
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in ENTITY definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading ENTITY definition");
		if (spr) {
			delete spr;
		}
		return STATUS_FAILED;
	}

	if (_region && _sprite) {
		_gameRef->LOG(0, "Warning: Entity '%s' has both sprite and region.", getName());
	}

	updatePosition();

	if (alpha != 0 && ar == 0 && ag == 0 && ab == 0) {
		ar = ag = ab = 255;
	}
	_alphaColor = BYTETORGBA(ar, ag, ab, alpha);
	_state = STATE_READY;

	if (_item && ((AdGame *)_gameRef)->isItemTaken(_item)) {
		_active = false;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdEntity::display() {
	if (_active) {
		updateSounds();

		uint32 alpha;
		if (_alphaColor != 0) {
			alpha = _alphaColor;
		} else {
			alpha = _shadowable ? ((AdGame *)_gameRef)->_scene->getAlphaAt(_posX, _posY) : 0xFFFFFFFF;
		}

		float scaleX, scaleY;
		getScale(&scaleX, &scaleY);

		float rotate;
		if (_rotatable) {
			if (_rotateValid) {
				rotate = _rotate;
			} else {
				rotate = ((AdGame *)_gameRef)->_scene->getRotationAt(_posX, _posY) + _relativeRotate;
			}
		} else {
			rotate = 0.0f;
		}


		bool reg = _registrable;
		if (_ignoreItems && ((AdGame *)_gameRef)->_selectedItem) {
			reg = false;
		}

		if (_region && (reg || _editorAlwaysRegister)) {
			_gameRef->_renderer->addRectToList(new BaseActiveRect(_gameRef,  _registerAlias, _region, _gameRef->_offsetX, _gameRef->_offsetY));
		}

		displaySpriteAttachments(true);
		if (_theora && (_theora->isPlaying() || _theora->isPaused())) {
			_theora->display(alpha);
		} else if (_currentSprite) {
			_currentSprite->display(_posX,
			                        _posY,
			                        (reg || _editorAlwaysRegister) ? _registerAlias : nullptr,
			                        scaleX,
			                        scaleY,
			                        alpha,
			                        rotate,
			                        _blendMode);
		}
		displaySpriteAttachments(false);

		if (_partEmitter) {
			_partEmitter->display(_region);
		}

	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdEntity::update() {
	_currentSprite = nullptr;

	if (_state == STATE_READY && _animSprite) {
		delete _animSprite;
		_animSprite = nullptr;
	}

	// finished playing animation?
	if (_state == STATE_PLAYING_ANIM && _animSprite != nullptr && _animSprite->isFinished()) {
		_state = STATE_READY;
		_currentSprite = _animSprite;
	}

	if (_sentence && _state != STATE_TALKING) {
		_sentence->finish();
	}

	// default: stand animation
	if (!_currentSprite) {
		_currentSprite = _sprite;
	}

	switch (_state) {
		//////////////////////////////////////////////////////////////////////////
	case STATE_PLAYING_ANIM:
		_currentSprite = _animSprite;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_READY:
		if (!_animSprite) {
			_currentSprite = _sprite;
		}
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_TALKING: {
		_sentence->update();
		if (_sentence->_currentSprite) {
			_tempSprite2 = _sentence->_currentSprite;
		}

		bool timeIsUp = (_sentence->_sound && _sentence->_soundStarted && (!_sentence->_sound->isPlaying() && !_sentence->_sound->isPaused())) || (!_sentence->_sound && _sentence->_duration <= _gameRef->getTimer()->getTime() - _sentence->_startTime);
		if (_tempSprite2 == nullptr || _tempSprite2->isFinished() || (/*_tempSprite2->_looping &&*/ timeIsUp)) {
			if (timeIsUp) {
				_sentence->finish();
				_tempSprite2 = nullptr;
				_state = STATE_READY;
			} else {
				_tempSprite2 = getTalkStance(_sentence->getNextStance());
				if (_tempSprite2) {
					_tempSprite2->reset();
					_currentSprite = _tempSprite2;
				}
				((AdGame *)_gameRef)->addSentence(_sentence);
			}
		} else {
			_currentSprite = _tempSprite2;
			((AdGame *)_gameRef)->addSentence(_sentence);
		}
	}
	break;
	default: // Silence unhandled enum-warning
		break;
	}


	if (_currentSprite) {
		_currentSprite->getCurrentFrame(_zoomable ? ((AdGame *)_gameRef)->_scene->getZoomAt(_posX, _posY) : 100);
		if (_currentSprite->isChanged()) {
			_posX += _currentSprite->_moveX;
			_posY += _currentSprite->_moveY;
		}
	}

	updateBlockRegion();
	_ready = (_state == STATE_READY);

	if (_theora) {
		int offsetX, offsetY;
		_gameRef->getOffset(&offsetX, &offsetY);
		_theora->_posX = _posX - offsetX;
		_theora->_posY = _posY - offsetY;

		_theora->update();
		if (_theora->isFinished()) {
			_theora->stop();
			delete _theora;
			_theora = nullptr;
		}
	}

	updatePartEmitter();
	updateSpriteAttachments();

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool AdEntity::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// StopSound
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "StopSound") == 0 && _subtype == ENTITY_SOUND) {
		stack->correctParams(0);

		if (DID_FAIL(stopSFX(false))) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlayTheora") == 0) {
		stack->correctParams(4);
		const char *filename = stack->pop()->getString();
		bool looping = stack->pop()->getBool(false);
		ScValue *valAlpha = stack->pop();
		int startTime = stack->pop()->getInt();

		delete _theora;
		_theora = new VideoTheoraPlayer(_gameRef);
		if (_theora && DID_SUCCEED(_theora->initialize(filename))) {
			if (!valAlpha->isNULL()) {
				_theora->setAlphaImage(valAlpha->getString());
			}
			_theora->play(VID_PLAY_POS, 0, 0, false, false, looping, startTime, _scale >= 0.0f ? _scale : -1.0f, _sFXVolume);
			//if (_scale>=0) _theora->_playZoom = _scale;
			stack->pushBool(true);
		} else {
			script->runtimeError("Entity.PlayTheora - error playing video '%s'", filename);
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StopTheora") == 0) {
		stack->correctParams(0);
		if (_theora) {
			_theora->stop();
			delete _theora;
			_theora = nullptr;
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsTheoraPlaying
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsTheoraPlaying") == 0) {
		stack->correctParams(0);
		if (_theora && _theora->isPlaying()) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PauseTheora") == 0) {
		stack->correctParams(0);
		if (_theora && _theora->isPlaying()) {
			_theora->pause();
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResumeTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ResumeTheora") == 0) {
		stack->correctParams(0);
		if (_theora && _theora->isPaused()) {
			_theora->resume();
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsTheoraPaused
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsTheoraPaused") == 0) {
		stack->correctParams(0);
		if (_theora && _theora->isPaused()) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

#if defined(ENABLE_KINJAL) || defined(ENABLE_HEROCRAFT)
	//////////////////////////////////////////////////////////////////////////
	// [WME Kinjal 1.4] SetBeforeEntity / SetAfterEntity
	// Usage at HeroCraft games: ent.SetBeforeEntity("redDuskaEntity")
	// Look for target entity (entity with given name, on the same layer as source entity)
	// If target entity is not found, do nothing
	// Else shift nodes of the layer to put current entity behind/after target entity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetBeforeEntity") == 0 || strcmp(name, "SetAfterEntity") == 0) {
		stack->correctParams(1);
		const char *nodeName = stack->pop()->getString();

		if (strcmp(getName(), nodeName) == 0) {
			warning("%s(%s): source and target have the same name", name, nodeName);
			stack->pushBool(false);
			return STATUS_OK;
		}

		for (uint32 i = 0; i < ((AdGame *)_gameRef)->_scene->_layers.size(); i++) {
			AdLayer *layer = ((AdGame *)_gameRef)->_scene->_layers[i];
			for (uint32 j = 0; j < layer->_nodes.size(); j++) {
				if (layer->_nodes[j]->_type == OBJECT_ENTITY && this == layer->_nodes[j]->_entity) {
					// found source layer and index, looking for target node
					for (uint32 k = 0; k < layer->_nodes.size(); k++) {
						if (layer->_nodes[k]->_type == OBJECT_ENTITY && strcmp(layer->_nodes[k]->_entity->getName(), nodeName) == 0) {
							// update target index, depending on method name and comparison of index values
							if (j < k && strcmp(name, "SetBeforeEntity") == 0) {
								k--;
							} else if (j > k && strcmp(name, "SetAfterEntity") == 0) {
								k++;
							}

							// shift layer nodes array between source and target
							int32 delta = j <= k ? 1 : -1;
							AdSceneNode *tmp = layer->_nodes[j];
							for (int32 x = j; x != (int32)k; x += delta) {
								layer->_nodes[x] = layer->_nodes[x + delta];
							}
							layer->_nodes[k] = tmp;

							// done
							stack->pushBool(true);
							return STATUS_OK;
						}
					}
				}
			}
		}

		warning("%s(%s): not found", name, nodeName);
		stack->pushBool(false);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [WME Kinjal 1.4] GetLayer / GetIndex
	// Find current entity's layer and node index
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetLayer") == 0 || strcmp(name, "GetIndex") == 0) {
		stack->correctParams(0);

		for (uint32 i = 0; i < ((AdGame *)_gameRef)->_scene->_layers.size(); i++) {
			AdLayer *layer = ((AdGame *)_gameRef)->_scene->_layers[i];
			for (uint32 j = 0; j < layer->_nodes.size(); j++) {
				if (layer->_nodes[j]->_type == OBJECT_ENTITY && this == layer->_nodes[j]->_entity) {
					if (strcmp(name, "GetLayer") == 0) {
						stack->pushNative(layer, true);
					} else {
						stack->pushInt(j);
					}
					return STATUS_OK;
				}
			}
		}

		warning("%s(): not found", name);
		stack->pushNULL();
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// CreateRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateRegion") == 0) {
		stack->correctParams(0);
		if (!_region) {
			_region = new BaseRegion(_gameRef);
			_gameRef->registerObject(_region);
		}
		if (_region) {
			stack->pushNative(_region, true);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteRegion") == 0) {
		stack->correctParams(0);
		if (_region) {
			_gameRef->unregisterObject(_region);
			_region = nullptr;
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	} else {
		return AdTalkHolder::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *AdEntity::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("entity");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Item
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Item") {
		if (_item) {
			_scValue->setString(_item);
		} else {
			_scValue->setNULL();
		}

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Subtype (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Subtype") {
		if (_subtype == ENTITY_SOUND) {
			_scValue->setString("sound");
		} else {
			_scValue->setString("normal");
		}

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "WalkToX") {
		_scValue->setInt(_walkToX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "WalkToY") {
		_scValue->setInt(_walkToY);
		return _scValue;
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] HintX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "HintX") {
		_scValue->setInt(_hintX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] HintY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "HintY") {
		_scValue->setInt(_hintY);
		return _scValue;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// WalkToDirection
	//////////////////////////////////////////////////////////////////////////
	else if (name == "WalkToDirection") {
		_scValue->setInt((int)_walkToDir);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Region (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Region") {
		if (_region) {
			_scValue->setNative(_region, true);
		} else {
			_scValue->setNULL();
		}
		return _scValue;
	} else {
		return AdTalkHolder::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdEntity::scSetProperty(const char *name, ScValue *value) {

	//////////////////////////////////////////////////////////////////////////
	// Item
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Item") == 0) {
		setItem(value->getString());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WalkToX") == 0) {
		_walkToX = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WalkToY") == 0) {
		_walkToY = value->getInt();
		return STATUS_OK;
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] HintX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HintX") == 0) {
		_hintX = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HintY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HintY") == 0) {
		_hintY = value->getInt();
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// WalkToDirection
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WalkToDirection") == 0) {
		int dir = value->getInt();
		if (dir >= 0 && dir < NUM_DIRECTIONS) {
			_walkToDir = (TDirection)dir;
		}
		return STATUS_OK;
	} else {
		return AdTalkHolder::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *AdEntity::scToString() {
	return "[entity object]";
}


//////////////////////////////////////////////////////////////////////////
bool AdEntity::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "ENTITY {\n");
	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", getName());
	if (_subtype == ENTITY_SOUND) {
		buffer->putTextIndent(indent + 2, "SUBTYPE=\"SOUND\"\n");
	}
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());
	buffer->putTextIndent(indent + 2, "ACTIVE=%s\n", _active ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "X=%d\n", _posX);
	buffer->putTextIndent(indent + 2, "Y=%d\n", _posY);
	buffer->putTextIndent(indent + 2, "SCALABLE=%s\n", _zoomable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "INTERACTIVE=%s\n", _registrable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "COLORABLE=%s\n", _shadowable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SELECTED=%s\n", _editorSelected ? "TRUE" : "FALSE");
	if (_ignoreItems) {
		buffer->putTextIndent(indent + 2, "IGNORE_ITEMS=%s\n", _ignoreItems ? "TRUE" : "FALSE");
	}
	if (_rotatable) {
		buffer->putTextIndent(indent + 2, "ROTATABLE=%s\n", _rotatable ? "TRUE" : "FALSE");
	}

	if (!_autoSoundPanning) {
		buffer->putTextIndent(indent + 2, "SOUND_PANNING=%s\n", _autoSoundPanning ? "TRUE" : "FALSE");
	}

	if (!_saveState) {
		buffer->putTextIndent(indent + 2, "SAVE_STATE=%s\n", _saveState ? "TRUE" : "FALSE");
	}

	if (_item && _item[0] != '\0') {
		buffer->putTextIndent(indent + 2, "ITEM=\"%s\"\n", _item);
	}

	buffer->putTextIndent(indent + 2, "WALK_TO_X=%d\n", _walkToX);
	buffer->putTextIndent(indent + 2, "WALK_TO_Y=%d\n", _walkToY);
	if (_walkToDir != DI_NONE) {
		buffer->putTextIndent(indent + 2, "WALK_TO_DIR=%d\n", (int)_walkToDir);
	}

#ifdef ENABLE_FOXTAIL
	buffer->putTextIndent(indent + 2, "HINT_X=%d\n", _hintX);
	buffer->putTextIndent(indent + 2, "HINT_Y=%d\n", _hintY);
#endif

	for (uint32 i = 0; i < _scripts.size(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	if (_subtype == ENTITY_NORMAL && _sprite && _sprite->getFilename()) {
		buffer->putTextIndent(indent + 2, "SPRITE=\"%s\"\n", _sprite->getFilename());
	}

	if (_subtype == ENTITY_SOUND && _sFX && _sFX->getFilename()) {
		buffer->putTextIndent(indent + 2, "SOUND=\"%s\"\n", _sFX->getFilename());
		buffer->putTextIndent(indent + 2, "SOUND_START_TIME=%d\n", _sFXStart);
		buffer->putTextIndent(indent + 2, "SOUND_VOLUME=%d\n", _sFXVolume);
	}


	if (RGBCOLGetR(_alphaColor) != 0 || RGBCOLGetG(_alphaColor) != 0 ||  RGBCOLGetB(_alphaColor) != 0) {
		buffer->putTextIndent(indent + 2, "ALPHA_COLOR { %d,%d,%d }\n", RGBCOLGetR(_alphaColor), RGBCOLGetG(_alphaColor), RGBCOLGetB(_alphaColor));
	}

	if (RGBCOLGetA(_alphaColor) != 0) {
		buffer->putTextIndent(indent + 2, "ALPHA = %d\n", RGBCOLGetA(_alphaColor));
	}

	if (_scale >= 0) {
		buffer->putTextIndent(indent + 2, "SCALE = %d\n", (int)_scale);
	}

	if (_relativeScale != 0) {
		buffer->putTextIndent(indent + 2, "RELATIVE_SCALE = %d\n", (int)_relativeScale);
	}

	if (_font && _font->getFilename()) {
		buffer->putTextIndent(indent + 2, "FONT=\"%s\"\n", _font->getFilename());
	}

	if (_cursor && _cursor->getFilename()) {
		buffer->putTextIndent(indent + 2, "CURSOR=\"%s\"\n", _cursor->getFilename());
	}

	AdTalkHolder::saveAsText(buffer, indent + 2);

	if (_region) {
		_region->saveAsText(buffer, indent + 2);
	}

	if (_scProp) {
		_scProp->saveAsText(buffer, indent + 2);
	}

	AdObject::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n\n");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int32 AdEntity::getHeight() {
	if (_region && !_sprite) {
		return _region->_rect.bottom - _region->_rect.top;
	} else {
		if (_currentSprite == nullptr) {
			_currentSprite = _sprite;
		}
		return AdObject::getHeight();
	}
}


//////////////////////////////////////////////////////////////////////////
void AdEntity::updatePosition() {
	if (_region && !_sprite) {
		_posX = _region->_rect.left + (_region->_rect.right - _region->_rect.left) / 2;
		_posY = _region->_rect.bottom;
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdEntity::persist(BasePersistenceManager *persistMgr) {
	AdTalkHolder::persist(persistMgr);

	persistMgr->transferCharPtr(TMEMBER(_item));
	persistMgr->transferPtr(TMEMBER_PTR(_region));
	//persistMgr->transfer(TMEMBER(_sprite));
	persistMgr->transferSint32(TMEMBER_INT(_subtype));
	_talkSprites.persist(persistMgr);
	_talkSpritesEx.persist(persistMgr);

	persistMgr->transferSint32(TMEMBER(_walkToX));
	persistMgr->transferSint32(TMEMBER(_walkToY));
	persistMgr->transferSint32(TMEMBER_INT(_walkToDir));

	persistMgr->transferPtr(TMEMBER_PTR(_theora));

#ifdef ENABLE_FOXTAIL
    if (BaseEngine::instance().isFoxTail(FOXTAIL_1_2_527, FOXTAIL_LATEST_VERSION)) {
	    persistMgr->transferSint32(TMEMBER(_hintX));
	    persistMgr->transferSint32(TMEMBER(_hintY));
	}
#endif

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void AdEntity::setItem(const char *itemName) {
	BaseUtils::setString(&_item, itemName);
}

//////////////////////////////////////////////////////////////////////////
bool AdEntity::setSprite(const char *filename) {
	if (_currentSprite == _sprite) {
		_currentSprite = nullptr;
	}

	delete _sprite;
	_sprite = nullptr;
	BaseSprite *spr = new BaseSprite(_gameRef, this);
	if (!spr || DID_FAIL(spr->loadFile(filename))) {
		delete _sprite;
		_sprite = nullptr;
		return STATUS_FAILED;
	} else {
		_sprite = spr;
		_currentSprite = _sprite;
		return STATUS_OK;
	}
}

Common::String AdEntity::debuggerToString() const {
	return Common::String::format("%p: Entity \"%s\"; (X,Y): (%d, %d), rotate(%d): %f deg, scale(%d): (%f, %f)%%", (const void *)this, getName(), _posX, _posY, _rotatable, _rotate, _zoomable, _scaleX, _scaleY);
}
} // End of namespace Wintermute
