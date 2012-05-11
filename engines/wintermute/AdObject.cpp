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

#include "dcgf.h"
#include "engines/wintermute/AdGame.h"
#include "engines/wintermute/AdItem.h"
#include "engines/wintermute/AdObject.h"
#include "engines/wintermute/AdInventory.h"
#include "engines/wintermute/AdLayer.h"
#include "engines/wintermute/AdScene.h"
#include "engines/wintermute/AdSceneNode.h"
#include "engines/wintermute/AdSentence.h"
#include "engines/wintermute/AdWaypointGroup.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BFrame.h"
#include "engines/wintermute/BSound.h"
#include "engines/wintermute/BSurfaceStorage.h"
#include "engines/wintermute/BSubFrame.h"
#include "engines/wintermute/BFont.h"
#include "engines/wintermute/BFontStorage.h"
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/BStringTable.h"
#include "engines/wintermute/scriptables/ScEngine.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/scriptables/ScValue.h"
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
	SAFE_DELETE(_animSprite);
	SAFE_DELETE(_sentence);
	SAFE_DELETE_ARRAY(_forcedTalkAnimName);

	SAFE_DELETE(_blockRegion);
	SAFE_DELETE(_wptGroup);

	SAFE_DELETE(_currentBlockRegion);
	SAFE_DELETE(_currentWptGroup);


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
HRESULT CAdObject::PlayAnim(const char *Filename) {
	SAFE_DELETE(_animSprite);
	_animSprite = new CBSprite(Game, this);
	if (!_animSprite) {
		Game->LOG(0, "CAdObject::PlayAnim: error creating temp sprite (object:\"%s\" sprite:\"%s\")", _name, Filename);
		return E_FAIL;
	}
	HRESULT res = _animSprite->LoadFile(Filename);
	if (FAILED(res)) {
		Game->LOG(res, "CAdObject::PlayAnim: error loading temp sprite (object:\"%s\" sprite:\"%s\")", _name, Filename);
		SAFE_DELETE(_animSprite);
		return res;
	}
	_state = STATE_PLAYING_ANIM;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::Display() {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::Update() {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {

	//////////////////////////////////////////////////////////////////////////
	// PlayAnim / PlayAnimAsync
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "PlayAnim") == 0 || strcmp(Name, "PlayAnimAsync") == 0) {
		Stack->CorrectParams(1);
		if (FAILED(PlayAnim(Stack->Pop()->GetString()))) Stack->PushBool(false);
		else {
			if (strcmp(Name, "PlayAnimAsync") != 0) Script->WaitFor(this);
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Reset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Reset") == 0) {
		Stack->CorrectParams(0);
		Reset();
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsTalking
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsTalking") == 0) {
		Stack->CorrectParams(0);
		Stack->PushBool(_state == STATE_TALKING);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopTalk / StopTalking
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "StopTalk") == 0 || strcmp(Name, "StopTalking") == 0) {
		Stack->CorrectParams(0);
		if (_sentence) _sentence->Finish();
		if (_state == STATE_TALKING) {
			_state = _nextState;
			_nextState = STATE_READY;
			Stack->PushBool(true);
		} else Stack->PushBool(false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ForceTalkAnim
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ForceTalkAnim") == 0) {
		Stack->CorrectParams(1);
		const char *AnimName = Stack->Pop()->GetString();
		SAFE_DELETE_ARRAY(_forcedTalkAnimName);
		_forcedTalkAnimName = new char[strlen(AnimName) + 1];
		strcpy(_forcedTalkAnimName, AnimName);
		_forcedTalkAnimUsed = false;
		Stack->PushBool(true);
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// Talk / TalkAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Talk") == 0 || strcmp(Name, "TalkAsync") == 0) {
		Stack->CorrectParams(5);

		const char *Text    = Stack->Pop()->GetString();
		CScValue *SoundVal = Stack->Pop();
		int Duration  = Stack->Pop()->GetInt();
		CScValue *ValStances = Stack->Pop();

		const char *Stances = ValStances->IsNULL() ? NULL : ValStances->GetString();

		int Align;
		CScValue *val = Stack->Pop();
		if (val->IsNULL()) Align = TAL_CENTER;
		else Align = val->GetInt();

		Align = MIN(MAX(0, Align), NUM_TEXT_ALIGN - 1);

		const char *Sound = SoundVal->IsNULL() ? NULL : SoundVal->GetString();

		Talk(Text, Sound, Duration, Stances, (TTextAlign)Align);
		if (strcmp(Name, "TalkAsync") != 0) Script->WaitForExclusive(this);

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StickToRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "StickToRegion") == 0) {
		Stack->CorrectParams(1);

		CAdLayer *Main = ((CAdGame *)Game)->_scene->_mainLayer;
		bool RegFound = false;

		int i;
		CScValue *Val = Stack->Pop();
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
		Stack->PushBool(RegFound);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetFont") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		if (Val->IsNULL()) SetFont(NULL);
		else SetFont(Val->GetString());

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetFont") == 0) {
		Stack->CorrectParams(0);
		if (_font && _font->_filename) Stack->PushString(_font->_filename);
		else Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TakeItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TakeItem") == 0) {
		Stack->CorrectParams(2);

		if (!_inventory) {
			_inventory = new CAdInventory(Game);
			((CAdGame *)Game)->RegisterInventory(_inventory);
		}

		CScValue *val = Stack->Pop();
		if (!val->IsNULL()) {
			const char *ItemName = val->GetString();
			val = Stack->Pop();
			const char *InsertAfter = val->IsNULL() ? NULL : val->GetString();
			if (FAILED(_inventory->InsertItem(ItemName, InsertAfter))) Script->RuntimeError("Cannot add item '%s' to inventory", ItemName);
			else {
				// hide associated entities
				((CAdGame *)Game)->_scene->HandleItemAssociations(ItemName, false);
			}

		} else Script->RuntimeError("TakeItem: item name expected");

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DropItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DropItem") == 0) {
		Stack->CorrectParams(1);

		if (!_inventory) {
			_inventory = new CAdInventory(Game);
			((CAdGame *)Game)->RegisterInventory(_inventory);
		}

		CScValue *val = Stack->Pop();
		if (!val->IsNULL()) {
			if (FAILED(_inventory->RemoveItem(val->GetString()))) Script->RuntimeError("Cannot remove item '%s' from inventory", val->GetString());
			else {
				// show associated entities
				((CAdGame *)Game)->_scene->HandleItemAssociations(val->GetString(), true);
			}
		} else Script->RuntimeError("DropItem: item name expected");

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetItem") == 0) {
		Stack->CorrectParams(1);

		if (!_inventory) {
			_inventory = new CAdInventory(Game);
			((CAdGame *)Game)->RegisterInventory(_inventory);
		}

		CScValue *val = Stack->Pop();
		if (val->_type == VAL_STRING) {
			CAdItem *item = ((CAdGame *)Game)->GetItemByName(val->GetString());
			if (item) Stack->PushNative(item, true);
			else Stack->PushNULL();
		} else if (val->IsNULL() || val->GetInt() < 0 || val->GetInt() >= _inventory->_takenItems.GetSize())
			Stack->PushNULL();
		else
			Stack->PushNative(_inventory->_takenItems[val->GetInt()], true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HasItem") == 0) {
		Stack->CorrectParams(1);

		if (!_inventory) {
			_inventory = new CAdInventory(Game);
			((CAdGame *)Game)->RegisterInventory(_inventory);
		}

		CScValue *val = Stack->Pop();
		if (!val->IsNULL()) {
			for (int i = 0; i < _inventory->_takenItems.GetSize(); i++) {
				if (val->GetNative() == _inventory->_takenItems[i]) {
					Stack->PushBool(true);
					return S_OK;
				} else if (scumm_stricmp(val->GetString(), _inventory->_takenItems[i]->_name) == 0) {
					Stack->PushBool(true);
					return S_OK;
				}
			}
		} else Script->RuntimeError("HasItem: item name expected");

		Stack->PushBool(false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateParticleEmitter
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateParticleEmitter") == 0) {
		Stack->CorrectParams(3);
		bool FollowParent = Stack->Pop()->GetBool();
		int OffsetX = Stack->Pop()->GetInt();
		int OffsetY = Stack->Pop()->GetInt();

		CPartEmitter *Emitter = CreateParticleEmitter(FollowParent, OffsetX, OffsetY);
		if (Emitter) Stack->PushNative(_partEmitter, true);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteParticleEmitter
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteParticleEmitter") == 0) {
		Stack->CorrectParams(0);
		if (_partEmitter) {
			Game->UnregisterObject(_partEmitter);
			_partEmitter = NULL;
		}
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddAttachment
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddAttachment") == 0) {
		Stack->CorrectParams(4);
		const char *Filename = Stack->Pop()->GetString();
		bool PreDisplay = Stack->Pop()->GetBool(true);
		int OffsetX = Stack->Pop()->GetInt();
		int OffsetY = Stack->Pop()->GetInt();

		HRESULT res;
		CAdEntity *Ent = new CAdEntity(Game);
		if (FAILED(res = Ent->LoadFile(Filename))) {
			SAFE_DELETE(Ent);
			Script->RuntimeError("AddAttachment() failed loading entity '%s'", Filename);
			Stack->PushBool(false);
		} else {
			Game->RegisterObject(Ent);

			Ent->_posX = OffsetX;
			Ent->_posY = OffsetY;
			Ent->_active = true;

			if (PreDisplay) _attachmentsPre.Add(Ent);
			else _attachmentsPost.Add(Ent);

			Stack->PushBool(true);
		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveAttachment
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RemoveAttachment") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();
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
			const char *Name = Val->GetString();
			for (int i = 0; i < _attachmentsPre.GetSize(); i++) {
				if (_attachmentsPre[i]->_name && scumm_stricmp(_attachmentsPre[i]->_name, Name) == 0) {
					Found = true;
					Game->UnregisterObject(_attachmentsPre[i]);
					_attachmentsPre.RemoveAt(i);
					i--;
				}
			}
			for (int i = 0; i < _attachmentsPost.GetSize(); i++) {
				if (_attachmentsPost[i]->_name && scumm_stricmp(_attachmentsPost[i]->_name, Name) == 0) {
					Found = true;
					Game->UnregisterObject(_attachmentsPost[i]);
					_attachmentsPost.RemoveAt(i);
					i--;
				}
			}
		}
		Stack->PushBool(Found);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetAttachment
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetAttachment") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

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
			const char *Name = Val->GetString();
			for (int i = 0; i < _attachmentsPre.GetSize(); i++) {
				if (_attachmentsPre[i]->_name && scumm_stricmp(_attachmentsPre[i]->_name, Name) == 0) {
					Ret = _attachmentsPre[i];
					break;
				}
			}
			if (!Ret) {
				for (int i = 0; i < _attachmentsPost.GetSize(); i++) {
					if (_attachmentsPost[i]->_name && scumm_stricmp(_attachmentsPost[i]->_name, Name) == 0) {
						Ret = _attachmentsPre[i];
						break;
					}
				}
			}
		}

		if (Ret != NULL) Stack->PushNative(Ret, true);
		else Stack->PushNULL();

		return S_OK;
	}

	else return CBObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdObject::ScGetProperty(char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("object");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Active") == 0) {
		_scValue->SetBool(_active);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IgnoreItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IgnoreItems") == 0) {
		_scValue->SetBool(_ignoreItems);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SceneIndependent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SceneIndependent") == 0) {
		_scValue->SetBool(_sceneIndependent);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesWidth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesWidth") == 0) {
		_scValue->SetInt(_subtitlesWidth);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosRelative
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosRelative") == 0) {
		_scValue->SetBool(_subtitlesModRelative);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosX") == 0) {
		_scValue->SetInt(_subtitlesModX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosY") == 0) {
		_scValue->SetInt(_subtitlesModY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosXCenter
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosXCenter") == 0) {
		_scValue->SetBool(_subtitlesModXCenter);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumItems (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumItems") == 0) {
		_scValue->SetInt(GetInventory()->_takenItems.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ParticleEmitter (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ParticleEmitter") == 0) {
		if (_partEmitter) _scValue->SetNative(_partEmitter, true);
		else _scValue->SetNULL();

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumAttachments (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumAttachments") == 0) {
		_scValue->SetInt(_attachmentsPre.GetSize() + _attachmentsPost.GetSize());
		return _scValue;
	}


	else return CBObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::ScSetProperty(char *Name, CScValue *Value) {

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Active") == 0) {
		_active = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IgnoreItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IgnoreItems") == 0) {
		_ignoreItems = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SceneIndependent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SceneIndependent") == 0) {
		_sceneIndependent = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesWidth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesWidth") == 0) {
		_subtitlesWidth = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosRelative
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosRelative") == 0) {
		_subtitlesModRelative = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosX") == 0) {
		_subtitlesModX = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosY") == 0) {
		_subtitlesModY = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosXCenter
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosXCenter") == 0) {
		_subtitlesModXCenter = Value->GetBool();
		return S_OK;
	}

	else return CBObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CAdObject::ScToString() {
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
int CAdObject::GetHeight() {
	if (!_currentSprite) return 0;
	else {
		CBFrame *frame = _currentSprite->_frames[_currentSprite->_currentFrame];
		int ret = 0;
		for (int i = 0; i < frame->_subframes.GetSize(); i++) {
			ret = MAX(ret, frame->_subframes[i]->_hotspotY);
		}

		if (_zoomable) {
			float zoom = ((CAdGame *)Game)->_scene->GetZoomAt(_posX, _posY);
			ret = ret * zoom / 100;
		}
		return ret;
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdObject::Talk(const char *Text, const char *Sound, uint32 Duration, const char *Stances, TTextAlign Align) {
	if (!_sentence) _sentence = new CAdSentence(Game);
	if (!_sentence) return;

	if (_forcedTalkAnimName && _forcedTalkAnimUsed) {
		SAFE_DELETE_ARRAY(_forcedTalkAnimName);
		_forcedTalkAnimUsed = false;
	}

	SAFE_DELETE(_sentence->_sound);

	_sentence->SetText(Text);
	Game->_stringTable->Expand(&_sentence->_text);
	_sentence->SetStances(Stances);
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
		if (snd && SUCCEEDED(snd->SetSound(Sound, SOUND_SPEECH, true))) {
			_sentence->SetSound(snd);
			if (_sentence->_duration <= 0) {
				uint32 Length = snd->GetLength();
				if (Length != 0) _sentence->_duration = Length;
			}
		} else delete snd;
	}

	// set duration by text length
	if (_sentence->_duration <= 0) {// TODO: Avoid longs.
		_sentence->_duration = MAX((unsigned long)1000, Game->_subtitlesSpeed * strlen(_sentence->_text));
	}


	int x, y, width, height;

	x = _posX;
	y = _posY;

	if (!_sceneIndependent && _subtitlesModRelative) {
		x -= ((CAdGame *)Game)->_scene->GetOffsetLeft();
		y -= ((CAdGame *)Game)->_scene->GetOffsetTop();
	}


	if (_subtitlesWidth > 0) width = _subtitlesWidth;
	else {
		if ((x < Game->_renderer->_width / 4 || x > Game->_renderer->_width * 0.75) && !Game->_touchInterface) {
			width = MAX(Game->_renderer->_width / 4, MIN(x * 2, (Game->_renderer->_width - x) * 2));
		} else width = Game->_renderer->_width / 2;
	}

	height = _sentence->_font->GetTextHeight((byte  *)_sentence->_text, width);

	y = y - height - GetHeight() - 5;
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
		_sentence->_pos.x += ((CAdGame *)Game)->_scene->GetOffsetLeft();
		_sentence->_pos.y += ((CAdGame *)Game)->_scene->GetOffsetTop();
	}

	_sentence->_fixedPos = !_subtitlesModRelative;


	_sentence->SetupTalkFile(Sound);

	_state = STATE_TALKING;

	if (DeleteSound) delete [] Sound;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::Reset() {
	if (_state == STATE_PLAYING_ANIM && _animSprite != NULL) {
		SAFE_DELETE(_animSprite);
	} else if (_state == STATE_TALKING && _sentence) {
		_sentence->Finish();
	}

	_state = _nextState = STATE_READY;

	Game->_scEngine->ResetObject(this);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::Persist(CBPersistMgr *PersistMgr) {
	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_active));
	PersistMgr->Transfer(TMEMBER(_blockRegion));
	PersistMgr->Transfer(TMEMBER(_currentBlockRegion));
	PersistMgr->Transfer(TMEMBER(_currentWptGroup));
	PersistMgr->Transfer(TMEMBER(_currentSprite));
	PersistMgr->Transfer(TMEMBER(_drawn));
	PersistMgr->Transfer(TMEMBER(_font));
	PersistMgr->Transfer(TMEMBER(_ignoreItems));
	PersistMgr->Transfer(TMEMBER_INT(_nextState));
	PersistMgr->Transfer(TMEMBER(_sentence));
	PersistMgr->Transfer(TMEMBER_INT(_state));
	PersistMgr->Transfer(TMEMBER(_animSprite));
	PersistMgr->Transfer(TMEMBER(_sceneIndependent));
	PersistMgr->Transfer(TMEMBER(_forcedTalkAnimName));
	PersistMgr->Transfer(TMEMBER(_forcedTalkAnimUsed));
	PersistMgr->Transfer(TMEMBER(_tempSprite2));
	PersistMgr->Transfer(TMEMBER_INT(_type));
	PersistMgr->Transfer(TMEMBER(_wptGroup));
	PersistMgr->Transfer(TMEMBER(_stickRegion));
	PersistMgr->Transfer(TMEMBER(_subtitlesModRelative));
	PersistMgr->Transfer(TMEMBER(_subtitlesModX));
	PersistMgr->Transfer(TMEMBER(_subtitlesModY));
	PersistMgr->Transfer(TMEMBER(_subtitlesModXCenter));
	PersistMgr->Transfer(TMEMBER(_subtitlesWidth));
	PersistMgr->Transfer(TMEMBER(_inventory));
	PersistMgr->Transfer(TMEMBER(_partEmitter));

	for (int i = 0; i < MAX_NUM_REGIONS; i++) PersistMgr->Transfer(TMEMBER(_currentRegions[i]));

	_attachmentsPre.Persist(PersistMgr);
	_attachmentsPost.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(_registerAlias));

	PersistMgr->Transfer(TMEMBER(_partFollowParent));
	PersistMgr->Transfer(TMEMBER(_partOffsetX));
	PersistMgr->Transfer(TMEMBER(_partOffsetY));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::UpdateSounds() {
	if (_sentence && _sentence->_sound)
		UpdateOneSound(_sentence->_sound);

	return CBObject::UpdateSounds();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::ResetSoundPan() {
	if (_sentence && _sentence->_sound) {
		_sentence->_sound->SetPan(0.0f);
	}
	return CBObject::ResetSoundPan();
}


//////////////////////////////////////////////////////////////////////////
bool CAdObject::GetExtendedFlag(char *FlagName) {
	if (!FlagName) return false;
	else if (strcmp(FlagName, "usable") == 0) return true;

	else return CBObject::GetExtendedFlag(FlagName);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	if (_blockRegion) _blockRegion->SaveAsText(Buffer, Indent + 2, "BLOCKED_REGION");
	if (_wptGroup) _wptGroup->SaveAsText(Buffer, Indent + 2);

	CBBase::SaveAsText(Buffer, Indent + 2);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::UpdateBlockRegion() {
	CAdGame *AdGame = (CAdGame *)Game;
	if (AdGame->_scene) {
		if (_blockRegion && _currentBlockRegion)
			_currentBlockRegion->Mimic(_blockRegion, _zoomable ? AdGame->_scene->GetScaleAt(_posY) : 100.0f, _posX, _posY);

		if (_wptGroup && _currentWptGroup)
			_currentWptGroup->Mimic(_wptGroup, _zoomable ? AdGame->_scene->GetScaleAt(_posY) : 100.0f, _posX, _posY);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CAdInventory *CAdObject::GetInventory() {
	if (!_inventory) {
		_inventory = new CAdInventory(Game);
		((CAdGame *)Game)->RegisterInventory(_inventory);
	}
	return _inventory;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::AfterMove() {
	CAdRegion *NewRegions[MAX_NUM_REGIONS];

	((CAdGame *)Game)->_scene->GetRegionsAt(_posX, _posY, NewRegions, MAX_NUM_REGIONS);
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
		if (!RegFound) NewRegions[i]->ApplyEvent("ActorEntry");
	}

	for (int i = 0; i < MAX_NUM_REGIONS; i++) {
		if (_currentRegions[i] && Game->ValidObject(_currentRegions[i])) {
			_currentRegions[i]->ApplyEvent("ActorLeave");
		}
		_currentRegions[i] = NewRegions[i];
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::InvalidateCurrRegions() {
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
		else *ScaleX = *ScaleY = ((CAdGame *)Game)->_scene->GetZoomAt(_posX, _posY) + _relativeScale;
	} else {
		*ScaleX = *ScaleY = 100;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::UpdateSpriteAttachments() {
	for (int i = 0; i < _attachmentsPre.GetSize(); i++) {
		_attachmentsPre[i]->Update();
	}
	for (int i = 0; i < _attachmentsPost.GetSize(); i++) {
		_attachmentsPost[i]->Update();
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::DisplaySpriteAttachments(bool PreDisplay) {
	if (PreDisplay) {
		for (int i = 0; i < _attachmentsPre.GetSize(); i++) {
			DisplaySpriteAttachment(_attachmentsPre[i]);
		}
	} else {
		for (int i = 0; i < _attachmentsPost.GetSize(); i++) {
			DisplaySpriteAttachment(_attachmentsPost[i]);
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::DisplaySpriteAttachment(CAdObject *Attachment) {
	if (!Attachment->_active) return S_OK;

	float ScaleX, ScaleY;
	GetScale(&ScaleX, &ScaleY);

	int OrigX = Attachment->_posX;
	int OrigY = Attachment->_posY;

	// inherit position from owner
	Attachment->_posX = this->_posX + Attachment->_posX * ScaleX / 100.0f;
	Attachment->_posY = this->_posY + Attachment->_posY * ScaleY / 100.0f;

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

	HRESULT ret = Attachment->Display();

	Attachment->_posX = OrigX;
	Attachment->_posY = OrigY;

	return ret;
}

//////////////////////////////////////////////////////////////////////////
CPartEmitter *CAdObject::CreateParticleEmitter(bool FollowParent, int OffsetX, int OffsetY) {
	_partFollowParent = FollowParent;
	_partOffsetX = OffsetX;
	_partOffsetY = OffsetY;

	if (!_partEmitter) {
		_partEmitter = new CPartEmitter(Game, this);
		if (_partEmitter) {
			Game->RegisterObject(_partEmitter);
		}
	}
	UpdatePartEmitter();
	return _partEmitter;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::UpdatePartEmitter() {
	if (!_partEmitter) return E_FAIL;

	if (_partFollowParent) {
		float ScaleX, ScaleY;
		GetScale(&ScaleX, &ScaleY);

		_partEmitter->_posX = _posX + (ScaleX / 100.0f) * _partOffsetX;
		_partEmitter->_posY = _posY + (ScaleY / 100.0f) * _partOffsetY;
	}
	return _partEmitter->Update();
}

} // end of namespace WinterMute
