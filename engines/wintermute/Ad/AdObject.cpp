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
#include "engines/wintermute/Ad/AdGame.h"
#include "engines/wintermute/Ad/AdItem.h"
#include "engines/wintermute/Ad/AdObject.h"
#include "engines/wintermute/Ad/AdInventory.h"
#include "engines/wintermute/Ad/AdLayer.h"
#include "engines/wintermute/Ad/AdScene.h"
#include "engines/wintermute/Ad/AdSceneNode.h"
#include "engines/wintermute/Ad/AdSentence.h"
#include "engines/wintermute/Ad/AdWaypointGroup.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFrame.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BSurfaceStorage.h"
#include "engines/wintermute/Base/BSubFrame.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Base/BFontStorage.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BStringTable.h"
#include "engines/wintermute/Base/scriptables/ScEngine.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "common/str.h"
#include "common/util.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdObject, false)

//////////////////////////////////////////////////////////////////////////
CAdObject::CAdObject(CBGame *inGame): CBObject(inGame) {
	_type = OBJECT_NONE;
	_state = _nextState = STATE_NONE;

	_active = true;
	_drawn = false;

	_currentSprite = NULL;
	_animSprite = NULL;
	_tempSprite2 = NULL;

	_font = NULL;

	_sentence = NULL;

	_forcedTalkAnimName = NULL;
	_forcedTalkAnimUsed = false;

	_blockRegion = NULL;
	_wptGroup = NULL;

	_currentBlockRegion = NULL;
	_currentWptGroup = NULL;

	_ignoreItems = false;
	_sceneIndependent = false;

	_stickRegion = NULL;

	_subtitlesModRelative = true;
	_subtitlesModX = 0;
	_subtitlesModY = 0;
	_subtitlesWidth = 0;
	_subtitlesModXCenter = true;

	_inventory = NULL;

	for (int i = 0; i < MAX_NUM_REGIONS; i++) _currentRegions[i] = NULL;

	_partEmitter = NULL;
	_partFollowParent = false;
	_partOffsetX = _partOffsetY = 0;

	_registerAlias = this;
}


//////////////////////////////////////////////////////////////////////////
CAdObject::~CAdObject() {
	_currentSprite = NULL; // reference only, don't delete
	delete _animSprite;
	_animSprite = NULL;
	delete _sentence;
	_sentence = NULL;
	delete[] _forcedTalkAnimName;
	_forcedTalkAnimName = NULL;

	delete _blockRegion;
	_blockRegion = NULL;
	delete _wptGroup;
	_wptGroup = NULL;

	delete _currentBlockRegion;
	_currentBlockRegion = NULL;
	delete _currentWptGroup;
	_currentWptGroup = NULL;

	_tempSprite2 = NULL; // reference only
	_stickRegion = NULL;

	if (_font) Game->_fontStorage->RemoveFont(_font);

	if (_inventory) {
		((CAdGame *)Game)->UnregisterInventory(_inventory);
		_inventory = NULL;
	}

	if (_partEmitter)
		Game->UnregisterObject(_partEmitter);


	for (int i = 0; i < _attachmentsPre.GetSize(); i++) {
		Game->UnregisterObject(_attachmentsPre[i]);
	}
	_attachmentsPre.RemoveAll();

	for (int i = 0; i < _attachmentsPost.GetSize(); i++) {
		Game->UnregisterObject(_attachmentsPost[i]);
	}
	_attachmentsPost.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::playAnim(const char *Filename) {
	delete _animSprite;
	_animSprite = NULL;
	_animSprite = new CBSprite(Game, this);
	if (!_animSprite) {
		Game->LOG(0, "CAdObject::PlayAnim: error creating temp sprite (object:\"%s\" sprite:\"%s\")", _name, Filename);
		return E_FAIL;
	}
	HRESULT res = _animSprite->loadFile(Filename);
	if (FAILED(res)) {
		Game->LOG(res, "CAdObject::PlayAnim: error loading temp sprite (object:\"%s\" sprite:\"%s\")", _name, Filename);
		delete _animSprite;
		_animSprite = NULL;
		return res;
	}
	_state = STATE_PLAYING_ANIM;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::display() {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::update() {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {

	//////////////////////////////////////////////////////////////////////////
	// PlayAnim / PlayAnimAsync
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "PlayAnim") == 0 || strcmp(name, "PlayAnimAsync") == 0) {
		stack->correctParams(1);
		if (FAILED(playAnim(stack->pop()->GetString()))) stack->pushBool(false);
		else {
			if (strcmp(name, "PlayAnimAsync") != 0) script->WaitFor(this);
			stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Reset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Reset") == 0) {
		stack->correctParams(0);
		reset();
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsTalking
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsTalking") == 0) {
		stack->correctParams(0);
		stack->pushBool(_state == STATE_TALKING);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopTalk / StopTalking
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StopTalk") == 0 || strcmp(name, "StopTalking") == 0) {
		stack->correctParams(0);
		if (_sentence) _sentence->finish();
		if (_state == STATE_TALKING) {
			_state = _nextState;
			_nextState = STATE_READY;
			stack->pushBool(true);
		} else stack->pushBool(false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ForceTalkAnim
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ForceTalkAnim") == 0) {
		stack->correctParams(1);
		const char *AnimName = stack->pop()->GetString();
		delete[] _forcedTalkAnimName;
		_forcedTalkAnimName = new char[strlen(AnimName) + 1];
		strcpy(_forcedTalkAnimName, AnimName);
		_forcedTalkAnimUsed = false;
		stack->pushBool(true);
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// Talk / TalkAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Talk") == 0 || strcmp(name, "TalkAsync") == 0) {
		stack->correctParams(5);

		const char *Text    = stack->pop()->GetString();
		CScValue *SoundVal = stack->pop();
		int Duration  = stack->pop()->GetInt();
		CScValue *ValStances = stack->pop();

		const char *Stances = ValStances->IsNULL() ? NULL : ValStances->GetString();

		int Align;
		CScValue *val = stack->pop();
		if (val->IsNULL()) Align = TAL_CENTER;
		else Align = val->GetInt();

		Align = MIN(MAX(0, Align), NUM_TEXT_ALIGN - 1);

		const char *Sound = SoundVal->IsNULL() ? NULL : SoundVal->GetString();

		talk(Text, Sound, Duration, Stances, (TTextAlign)Align);
		if (strcmp(name, "TalkAsync") != 0) script->WaitForExclusive(this);

		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StickToRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StickToRegion") == 0) {
		stack->correctParams(1);

		CAdLayer *Main = ((CAdGame *)Game)->_scene->_mainLayer;
		bool RegFound = false;

		int i;
		CScValue *Val = stack->pop();
		if (Val->IsNULL() || !Main) {
			_stickRegion = NULL;
			RegFound = true;
		} else if (Val->IsString()) {
			const char *RegionName = Val->GetString();
			for (i = 0; i < Main->_nodes.GetSize(); i++) {
				if (Main->_nodes[i]->_type == OBJECT_REGION && Main->_nodes[i]->_region->_name && scumm_stricmp(Main->_nodes[i]->_region->_name, RegionName) == 0) {
					_stickRegion = Main->_nodes[i]->_region;
					RegFound = true;
					break;
				}
			}
		} else if (Val->IsNative()) {
			CBScriptable *Obj = Val->GetNative();

			for (i = 0; i < Main->_nodes.GetSize(); i++) {
				if (Main->_nodes[i]->_type == OBJECT_REGION && Main->_nodes[i]->_region == Obj) {
					_stickRegion = Main->_nodes[i]->_region;
					RegFound = true;
					break;
				}
			}

		}

		if (!RegFound) _stickRegion = NULL;
		stack->pushBool(RegFound);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetFont") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();

		if (Val->IsNULL()) SetFont(NULL);
		else SetFont(Val->GetString());

		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFont") == 0) {
		stack->correctParams(0);
		if (_font && _font->_filename) stack->pushString(_font->_filename);
		else stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TakeItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TakeItem") == 0) {
		stack->correctParams(2);

		if (!_inventory) {
			_inventory = new CAdInventory(Game);
			((CAdGame *)Game)->RegisterInventory(_inventory);
		}

		CScValue *val = stack->pop();
		if (!val->IsNULL()) {
			const char *ItemName = val->GetString();
			val = stack->pop();
			const char *InsertAfter = val->IsNULL() ? NULL : val->GetString();
			if (FAILED(_inventory->InsertItem(ItemName, InsertAfter))) script->RuntimeError("Cannot add item '%s' to inventory", ItemName);
			else {
				// hide associated entities
				((CAdGame *)Game)->_scene->handleItemAssociations(ItemName, false);
			}

		} else script->RuntimeError("TakeItem: item name expected");

		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DropItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DropItem") == 0) {
		stack->correctParams(1);

		if (!_inventory) {
			_inventory = new CAdInventory(Game);
			((CAdGame *)Game)->RegisterInventory(_inventory);
		}

		CScValue *val = stack->pop();
		if (!val->IsNULL()) {
			if (FAILED(_inventory->RemoveItem(val->GetString()))) script->RuntimeError("Cannot remove item '%s' from inventory", val->GetString());
			else {
				// show associated entities
				((CAdGame *)Game)->_scene->handleItemAssociations(val->GetString(), true);
			}
		} else script->RuntimeError("DropItem: item name expected");

		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetItem") == 0) {
		stack->correctParams(1);

		if (!_inventory) {
			_inventory = new CAdInventory(Game);
			((CAdGame *)Game)->RegisterInventory(_inventory);
		}

		CScValue *val = stack->pop();
		if (val->_type == VAL_STRING) {
			CAdItem *item = ((CAdGame *)Game)->GetItemByName(val->GetString());
			if (item) stack->pushNative(item, true);
			else stack->pushNULL();
		} else if (val->IsNULL() || val->GetInt() < 0 || val->GetInt() >= _inventory->_takenItems.GetSize())
			stack->pushNULL();
		else
			stack->pushNative(_inventory->_takenItems[val->GetInt()], true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HasItem") == 0) {
		stack->correctParams(1);

		if (!_inventory) {
			_inventory = new CAdInventory(Game);
			((CAdGame *)Game)->RegisterInventory(_inventory);
		}

		CScValue *val = stack->pop();
		if (!val->IsNULL()) {
			for (int i = 0; i < _inventory->_takenItems.GetSize(); i++) {
				if (val->GetNative() == _inventory->_takenItems[i]) {
					stack->pushBool(true);
					return S_OK;
				} else if (scumm_stricmp(val->GetString(), _inventory->_takenItems[i]->_name) == 0) {
					stack->pushBool(true);
					return S_OK;
				}
			}
		} else script->RuntimeError("HasItem: item name expected");

		stack->pushBool(false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateParticleEmitter
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateParticleEmitter") == 0) {
		stack->correctParams(3);
		bool FollowParent = stack->pop()->GetBool();
		int OffsetX = stack->pop()->GetInt();
		int OffsetY = stack->pop()->GetInt();

		CPartEmitter *Emitter = createParticleEmitter(FollowParent, OffsetX, OffsetY);
		if (Emitter) stack->pushNative(_partEmitter, true);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteParticleEmitter
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteParticleEmitter") == 0) {
		stack->correctParams(0);
		if (_partEmitter) {
			Game->UnregisterObject(_partEmitter);
			_partEmitter = NULL;
		}
		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddAttachment
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddAttachment") == 0) {
		stack->correctParams(4);
		const char *Filename = stack->pop()->GetString();
		bool PreDisplay = stack->pop()->GetBool(true);
		int OffsetX = stack->pop()->GetInt();
		int OffsetY = stack->pop()->GetInt();

		HRESULT res;
		CAdEntity *Ent = new CAdEntity(Game);
		if (FAILED(res = Ent->loadFile(Filename))) {
			delete Ent;
			Ent = NULL;
			script->RuntimeError("AddAttachment() failed loading entity '%s'", Filename);
			stack->pushBool(false);
		} else {
			Game->RegisterObject(Ent);

			Ent->_posX = OffsetX;
			Ent->_posY = OffsetY;
			Ent->_active = true;

			if (PreDisplay) _attachmentsPre.Add(Ent);
			else _attachmentsPost.Add(Ent);

			stack->pushBool(true);
		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveAttachment
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveAttachment") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();
		bool Found = false;
		if (Val->IsNative()) {
			CBScriptable *Obj = Val->GetNative();
			for (int i = 0; i < _attachmentsPre.GetSize(); i++) {
				if (_attachmentsPre[i] == Obj) {
					Found = true;
					Game->UnregisterObject(_attachmentsPre[i]);
					_attachmentsPre.RemoveAt(i);
					i--;
				}
			}
			for (int i = 0; i < _attachmentsPost.GetSize(); i++) {
				if (_attachmentsPost[i] == Obj) {
					Found = true;
					Game->UnregisterObject(_attachmentsPost[i]);
					_attachmentsPost.RemoveAt(i);
					i--;
				}
			}
		} else {
			const char *attachmentName = Val->GetString();
			for (int i = 0; i < _attachmentsPre.GetSize(); i++) {
				if (_attachmentsPre[i]->_name && scumm_stricmp(_attachmentsPre[i]->_name, attachmentName) == 0) {
					Found = true;
					Game->UnregisterObject(_attachmentsPre[i]);
					_attachmentsPre.RemoveAt(i);
					i--;
				}
			}
			for (int i = 0; i < _attachmentsPost.GetSize(); i++) {
				if (_attachmentsPost[i]->_name && scumm_stricmp(_attachmentsPost[i]->_name, attachmentName) == 0) {
					Found = true;
					Game->UnregisterObject(_attachmentsPost[i]);
					_attachmentsPost.RemoveAt(i);
					i--;
				}
			}
		}
		stack->pushBool(Found);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetAttachment
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetAttachment") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();

		CAdObject *Ret = NULL;
		if (Val->IsInt()) {
			int Index = Val->GetInt();
			int CurrIndex = 0;
			for (int i = 0; i < _attachmentsPre.GetSize(); i++) {
				if (CurrIndex == Index) Ret = _attachmentsPre[i];
				CurrIndex++;
			}
			for (int i = 0; i < _attachmentsPost.GetSize(); i++) {
				if (CurrIndex == Index) Ret = _attachmentsPost[i];
				CurrIndex++;
			}
		} else {
			const char *attachmentName = Val->GetString();
			for (int i = 0; i < _attachmentsPre.GetSize(); i++) {
				if (_attachmentsPre[i]->_name && scumm_stricmp(_attachmentsPre[i]->_name, attachmentName) == 0) {
					Ret = _attachmentsPre[i];
					break;
				}
			}
			if (!Ret) {
				for (int i = 0; i < _attachmentsPost.GetSize(); i++) {
					if (_attachmentsPost[i]->_name && scumm_stricmp(_attachmentsPost[i]->_name, attachmentName) == 0) {
						Ret = _attachmentsPre[i];
						break;
					}
				}
			}
		}

		if (Ret != NULL) stack->pushNative(Ret, true);
		else stack->pushNULL();

		return S_OK;
	}

	else return CBObject::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdObject::scGetProperty(const char *name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->SetString("object");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Active") == 0) {
		_scValue->SetBool(_active);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IgnoreItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IgnoreItems") == 0) {
		_scValue->SetBool(_ignoreItems);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SceneIndependent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SceneIndependent") == 0) {
		_scValue->SetBool(_sceneIndependent);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesWidth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesWidth") == 0) {
		_scValue->SetInt(_subtitlesWidth);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosRelative
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesPosRelative") == 0) {
		_scValue->SetBool(_subtitlesModRelative);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesPosX") == 0) {
		_scValue->SetInt(_subtitlesModX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesPosY") == 0) {
		_scValue->SetInt(_subtitlesModY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosXCenter
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesPosXCenter") == 0) {
		_scValue->SetBool(_subtitlesModXCenter);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumItems (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumItems") == 0) {
		_scValue->SetInt(getInventory()->_takenItems.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ParticleEmitter (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ParticleEmitter") == 0) {
		if (_partEmitter) _scValue->SetNative(_partEmitter, true);
		else _scValue->SetNULL();

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumAttachments (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumAttachments") == 0) {
		_scValue->SetInt(_attachmentsPre.GetSize() + _attachmentsPost.GetSize());
		return _scValue;
	}


	else return CBObject::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::scSetProperty(const char *name, CScValue *value) {

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Active") == 0) {
		_active = value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IgnoreItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IgnoreItems") == 0) {
		_ignoreItems = value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SceneIndependent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SceneIndependent") == 0) {
		_sceneIndependent = value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesWidth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesWidth") == 0) {
		_subtitlesWidth = value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosRelative
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesPosRelative") == 0) {
		_subtitlesModRelative = value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesPosX") == 0) {
		_subtitlesModX = value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesPosY") == 0) {
		_subtitlesModY = value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosXCenter
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SubtitlesPosXCenter") == 0) {
		_subtitlesModXCenter = value->GetBool();
		return S_OK;
	}

	else return CBObject::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CAdObject::scToString() {
	return "[ad object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::SetFont(const char *Filename) {
	if (_font) Game->_fontStorage->RemoveFont(_font);
	if (Filename) {
		_font = Game->_fontStorage->AddFont(Filename);
		return _font == NULL ? E_FAIL : S_OK;
	} else {
		_font = NULL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
int CAdObject::getHeight() {
	if (!_currentSprite) return 0;
	else {
		CBFrame *frame = _currentSprite->_frames[_currentSprite->_currentFrame];
		int ret = 0;
		for (int i = 0; i < frame->_subframes.GetSize(); i++) {
			ret = MAX(ret, frame->_subframes[i]->_hotspotY);
		}

		if (_zoomable) {
			float zoom = ((CAdGame *)Game)->_scene->getZoomAt(_posX, _posY);
			ret = (int)(ret * zoom / 100);
		}
		return ret;
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdObject::talk(const char *Text, const char *Sound, uint32 Duration, const char *Stances, TTextAlign Align) {
	if (!_sentence) _sentence = new CAdSentence(Game);
	if (!_sentence) return;

	if (_forcedTalkAnimName && _forcedTalkAnimUsed) {
		delete[] _forcedTalkAnimName;
		_forcedTalkAnimName = NULL;
		_forcedTalkAnimUsed = false;
	}

	delete(_sentence->_sound);
	_sentence->_sound = NULL;

	_sentence->setText(Text);
	Game->_stringTable->Expand(&_sentence->_text);
	_sentence->setStances(Stances);
	_sentence->_duration = Duration;
	_sentence->_align = Align;
	_sentence->_startTime = Game->_timer;
	_sentence->_currentStance = -1;
	_sentence->_font = _font == NULL ? Game->_systemFont : _font;
	_sentence->_freezable = _freezable;

	// try to locate speech file automatically
	bool DeleteSound = false;
	if (!Sound) {
		char *Key = Game->_stringTable->GetKey(Text);
		if (Key) {
			Sound = ((CAdGame *)Game)->FindSpeechFile(Key);
			delete [] Key;

			if (Sound) DeleteSound = true;
		}
	}

	// load sound and set duration appropriately
	if (Sound) {
		CBSound *snd = new CBSound(Game);
		if (snd && SUCCEEDED(snd->setSound(Sound, SOUND_SPEECH, true))) {
			_sentence->setSound(snd);
			if (_sentence->_duration <= 0) {
				uint32 Length = snd->getLength();
				if (Length != 0) _sentence->_duration = Length;
			}
		} else delete snd;
	}

	// set duration by text length
	if (_sentence->_duration <= 0) {// TODO: Avoid longs.
		_sentence->_duration = MAX((size_t)1000, Game->_subtitlesSpeed * strlen(_sentence->_text));
	}


	int x, y, width, height;

	x = _posX;
	y = _posY;

	if (!_sceneIndependent && _subtitlesModRelative) {
		x -= ((CAdGame *)Game)->_scene->getOffsetLeft();
		y -= ((CAdGame *)Game)->_scene->getOffsetTop();
	}


	if (_subtitlesWidth > 0) width = _subtitlesWidth;
	else {
		if ((x < Game->_renderer->_width / 4 || x > Game->_renderer->_width * 0.75) && !Game->_touchInterface) {
			width = MAX(Game->_renderer->_width / 4, MIN(x * 2, (Game->_renderer->_width - x) * 2));
		} else width = Game->_renderer->_width / 2;
	}

	height = _sentence->_font->getTextHeight((byte *)_sentence->_text, width);

	y = y - height - getHeight() - 5;
	if (_subtitlesModRelative) {
		x += _subtitlesModX;
		y += _subtitlesModY;
	} else {
		x = _subtitlesModX;
		y = _subtitlesModY;
	}
	if (_subtitlesModXCenter)
		x = x - width / 2;


	x = MIN(MAX(0, x), Game->_renderer->_width - width);
	y = MIN(MAX(0, y), Game->_renderer->_height - height);

	_sentence->_width = width;


	_sentence->_pos.x = x;
	_sentence->_pos.y = y;


	if (_subtitlesModRelative) {
		_sentence->_pos.x += ((CAdGame *)Game)->_scene->getOffsetLeft();
		_sentence->_pos.y += ((CAdGame *)Game)->_scene->getOffsetTop();
	}

	_sentence->_fixedPos = !_subtitlesModRelative;


	_sentence->setupTalkFile(Sound);

	_state = STATE_TALKING;

	if (DeleteSound) delete [] Sound;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::reset() {
	if (_state == STATE_PLAYING_ANIM && _animSprite != NULL) {
		delete _animSprite;
		_animSprite = NULL;
	} else if (_state == STATE_TALKING && _sentence) {
		_sentence->finish();
	}

	_state = _nextState = STATE_READY;

	Game->_scEngine->ResetObject(this);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::persist(CBPersistMgr *persistMgr) {
	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_active));
	persistMgr->transfer(TMEMBER(_blockRegion));
	persistMgr->transfer(TMEMBER(_currentBlockRegion));
	persistMgr->transfer(TMEMBER(_currentWptGroup));
	persistMgr->transfer(TMEMBER(_currentSprite));
	persistMgr->transfer(TMEMBER(_drawn));
	persistMgr->transfer(TMEMBER(_font));
	persistMgr->transfer(TMEMBER(_ignoreItems));
	persistMgr->transfer(TMEMBER_INT(_nextState));
	persistMgr->transfer(TMEMBER(_sentence));
	persistMgr->transfer(TMEMBER_INT(_state));
	persistMgr->transfer(TMEMBER(_animSprite));
	persistMgr->transfer(TMEMBER(_sceneIndependent));
	persistMgr->transfer(TMEMBER(_forcedTalkAnimName));
	persistMgr->transfer(TMEMBER(_forcedTalkAnimUsed));
	persistMgr->transfer(TMEMBER(_tempSprite2));
	persistMgr->transfer(TMEMBER_INT(_type));
	persistMgr->transfer(TMEMBER(_wptGroup));
	persistMgr->transfer(TMEMBER(_stickRegion));
	persistMgr->transfer(TMEMBER(_subtitlesModRelative));
	persistMgr->transfer(TMEMBER(_subtitlesModX));
	persistMgr->transfer(TMEMBER(_subtitlesModY));
	persistMgr->transfer(TMEMBER(_subtitlesModXCenter));
	persistMgr->transfer(TMEMBER(_subtitlesWidth));
	persistMgr->transfer(TMEMBER(_inventory));
	persistMgr->transfer(TMEMBER(_partEmitter));

	for (int i = 0; i < MAX_NUM_REGIONS; i++) persistMgr->transfer(TMEMBER(_currentRegions[i]));

	_attachmentsPre.persist(persistMgr);
	_attachmentsPost.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_registerAlias));

	persistMgr->transfer(TMEMBER(_partFollowParent));
	persistMgr->transfer(TMEMBER(_partOffsetX));
	persistMgr->transfer(TMEMBER(_partOffsetY));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::updateSounds() {
	if (_sentence && _sentence->_sound)
		updateOneSound(_sentence->_sound);

	return CBObject::updateSounds();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::resetSoundPan() {
	if (_sentence && _sentence->_sound) {
		_sentence->_sound->setPan(0.0f);
	}
	return CBObject::resetSoundPan();
}


//////////////////////////////////////////////////////////////////////////
bool CAdObject::getExtendedFlag(const char *FlagName) {
	if (!FlagName) return false;
	else if (strcmp(FlagName, "usable") == 0) return true;

	else return CBObject::getExtendedFlag(FlagName);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::saveAsText(CBDynBuffer *Buffer, int Indent) {
	if (_blockRegion) _blockRegion->saveAsText(Buffer, Indent + 2, "BLOCKED_REGION");
	if (_wptGroup) _wptGroup->saveAsText(Buffer, Indent + 2);

	CBBase::saveAsText(Buffer, Indent + 2);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::updateBlockRegion() {
	CAdGame *AdGame = (CAdGame *)Game;
	if (AdGame->_scene) {
		if (_blockRegion && _currentBlockRegion)
			_currentBlockRegion->Mimic(_blockRegion, _zoomable ? AdGame->_scene->getScaleAt(_posY) : 100.0f, _posX, _posY);

		if (_wptGroup && _currentWptGroup)
			_currentWptGroup->Mimic(_wptGroup, _zoomable ? AdGame->_scene->getScaleAt(_posY) : 100.0f, _posX, _posY);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CAdInventory *CAdObject::getInventory() {
	if (!_inventory) {
		_inventory = new CAdInventory(Game);
		((CAdGame *)Game)->RegisterInventory(_inventory);
	}
	return _inventory;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::afterMove() {
	CAdRegion *NewRegions[MAX_NUM_REGIONS];

	((CAdGame *)Game)->_scene->getRegionsAt(_posX, _posY, NewRegions, MAX_NUM_REGIONS);
	for (int i = 0; i < MAX_NUM_REGIONS; i++) {
		if (!NewRegions[i]) break;
		bool RegFound = false;
		for (int j = 0; j < MAX_NUM_REGIONS; j++) {
			if (_currentRegions[j] == NewRegions[i]) {
				_currentRegions[j] = NULL;
				RegFound = true;
				break;
			}
		}
		if (!RegFound) NewRegions[i]->applyEvent("ActorEntry");
	}

	for (int i = 0; i < MAX_NUM_REGIONS; i++) {
		if (_currentRegions[i] && Game->ValidObject(_currentRegions[i])) {
			_currentRegions[i]->applyEvent("ActorLeave");
		}
		_currentRegions[i] = NewRegions[i];
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::invalidateCurrRegions() {
	for (int i = 0; i < MAX_NUM_REGIONS; i++) _currentRegions[i] = NULL;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::GetScale(float *ScaleX, float *ScaleY) {
	if (_zoomable) {
		if (_scaleX >= 0 || _scaleY >= 0) {
			*ScaleX = _scaleX < 0 ? 100 : _scaleX;
			*ScaleY = _scaleY < 0 ? 100 : _scaleY;
		} else if (_scale >= 0) *ScaleX = *ScaleY = _scale;
		else *ScaleX = *ScaleY = ((CAdGame *)Game)->_scene->getZoomAt(_posX, _posY) + _relativeScale;
	} else {
		*ScaleX = *ScaleY = 100;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::updateSpriteAttachments() {
	for (int i = 0; i < _attachmentsPre.GetSize(); i++) {
		_attachmentsPre[i]->update();
	}
	for (int i = 0; i < _attachmentsPost.GetSize(); i++) {
		_attachmentsPost[i]->update();
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::displaySpriteAttachments(bool PreDisplay) {
	if (PreDisplay) {
		for (int i = 0; i < _attachmentsPre.GetSize(); i++) {
			displaySpriteAttachment(_attachmentsPre[i]);
		}
	} else {
		for (int i = 0; i < _attachmentsPost.GetSize(); i++) {
			displaySpriteAttachment(_attachmentsPost[i]);
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::displaySpriteAttachment(CAdObject *Attachment) {
	if (!Attachment->_active) return S_OK;

	float ScaleX, ScaleY;
	GetScale(&ScaleX, &ScaleY);

	int OrigX = Attachment->_posX;
	int OrigY = Attachment->_posY;

	// inherit position from owner
	Attachment->_posX = (int)(this->_posX + Attachment->_posX * ScaleX / 100.0f);
	Attachment->_posY = (int)(this->_posY + Attachment->_posY * ScaleY / 100.0f);

	// inherit other props
	Attachment->_alphaColor = this->_alphaColor;
	Attachment->_blendMode = this->_blendMode;

	Attachment->_scale = this->_scale;
	Attachment->_relativeScale = this->_relativeScale;
	Attachment->_scaleX = this->_scaleX;
	Attachment->_scaleY = this->_scaleY;

	Attachment->_rotate = this->_rotate;
	Attachment->_relativeRotate = this->_relativeRotate;
	Attachment->_rotateValid = this->_rotateValid;

	Attachment->_registerAlias = this;
	Attachment->_registrable = this->_registrable;

	HRESULT ret = Attachment->display();

	Attachment->_posX = OrigX;
	Attachment->_posY = OrigY;

	return ret;
}

//////////////////////////////////////////////////////////////////////////
CPartEmitter *CAdObject::createParticleEmitter(bool FollowParent, int OffsetX, int OffsetY) {
	_partFollowParent = FollowParent;
	_partOffsetX = OffsetX;
	_partOffsetY = OffsetY;

	if (!_partEmitter) {
		_partEmitter = new CPartEmitter(Game, this);
		if (_partEmitter) {
			Game->RegisterObject(_partEmitter);
		}
	}
	updatePartEmitter();
	return _partEmitter;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::updatePartEmitter() {
	if (!_partEmitter) return E_FAIL;

	if (_partFollowParent) {
		float ScaleX, ScaleY;
		GetScale(&ScaleX, &ScaleY);

		_partEmitter->_posX = (int)(_posX + (ScaleX / 100.0f) * _partOffsetX);
		_partEmitter->_posY = (int)(_posY + (ScaleY / 100.0f) * _partOffsetY);
	}
	return _partEmitter->update();
}

} // end of namespace WinterMute
