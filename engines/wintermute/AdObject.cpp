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
	m_Type = OBJECT_NONE;
	m_State = m_NextState = STATE_NONE;

	m_Active = true;
	m_Drawn = false;

	m_CurrentSprite = NULL;
	m_AnimSprite = NULL;
	m_TempSprite2 = NULL;

	m_Font = NULL;

	m_Sentence = NULL;

	m_ForcedTalkAnimName = NULL;
	m_ForcedTalkAnimUsed = false;

	m_BlockRegion = NULL;
	m_WptGroup = NULL;

	m_CurrentBlockRegion = NULL;
	m_CurrentWptGroup = NULL;

	m_IgnoreItems = false;
	m_SceneIndependent = false;

	m_StickRegion = NULL;

	m_SubtitlesModRelative = true;
	m_SubtitlesModX = 0;
	m_SubtitlesModY = 0;
	m_SubtitlesWidth = 0;
	m_SubtitlesModXCenter = true;

	m_Inventory = NULL;

	for (int i = 0; i < MAX_NUM_REGIONS; i++) m_CurrentRegions[i] = NULL;

	m_PartEmitter = NULL;
	m_PartFollowParent = false;
	m_PartOffsetX = m_PartOffsetY = 0;

	m_RegisterAlias = this;
}


//////////////////////////////////////////////////////////////////////////
CAdObject::~CAdObject() {
	m_CurrentSprite = NULL; // reference only, don't delete
	SAFE_DELETE(m_AnimSprite);
	SAFE_DELETE(m_Sentence);
	SAFE_DELETE_ARRAY(m_ForcedTalkAnimName);

	SAFE_DELETE(m_BlockRegion);
	SAFE_DELETE(m_WptGroup);

	SAFE_DELETE(m_CurrentBlockRegion);
	SAFE_DELETE(m_CurrentWptGroup);


	m_TempSprite2 = NULL; // reference only
	m_StickRegion = NULL;

	if (m_Font) Game->m_FontStorage->RemoveFont(m_Font);

	if (m_Inventory) {
		((CAdGame *)Game)->UnregisterInventory(m_Inventory);
		m_Inventory = NULL;
	}

	if (m_PartEmitter)
		Game->UnregisterObject(m_PartEmitter);


	for (int i = 0; i < m_AttachmentsPre.GetSize(); i++) {
		Game->UnregisterObject(m_AttachmentsPre[i]);
	}
	m_AttachmentsPre.RemoveAll();

	for (int i = 0; i < m_AttachmentsPost.GetSize(); i++) {
		Game->UnregisterObject(m_AttachmentsPost[i]);
	}
	m_AttachmentsPost.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::PlayAnim(char *Filename) {
	SAFE_DELETE(m_AnimSprite);
	m_AnimSprite = new CBSprite(Game, this);
	if (!m_AnimSprite) {
		Game->LOG(0, "CAdObject::PlayAnim: error creating temp sprite (object:\"%s\" sprite:\"%s\")", m_Name, Filename);
		return E_FAIL;
	}
	HRESULT res = m_AnimSprite->LoadFile(Filename);
	if (FAILED(res)) {
		Game->LOG(res, "CAdObject::PlayAnim: error loading temp sprite (object:\"%s\" sprite:\"%s\")", m_Name, Filename);
		SAFE_DELETE(m_AnimSprite);
		return res;
	}
	m_State = STATE_PLAYING_ANIM;

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
		Stack->PushBool(m_State == STATE_TALKING);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopTalk / StopTalking
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "StopTalk") == 0 || strcmp(Name, "StopTalking") == 0) {
		Stack->CorrectParams(0);
		if (m_Sentence) m_Sentence->Finish();
		if (m_State == STATE_TALKING) {
			m_State = m_NextState;
			m_NextState = STATE_READY;
			Stack->PushBool(true);
		} else Stack->PushBool(false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ForceTalkAnim
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ForceTalkAnim") == 0) {
		Stack->CorrectParams(1);
		char *AnimName = Stack->Pop()->GetString();
		SAFE_DELETE_ARRAY(m_ForcedTalkAnimName);
		m_ForcedTalkAnimName = new char[strlen(AnimName) + 1];
		strcpy(m_ForcedTalkAnimName, AnimName);
		m_ForcedTalkAnimUsed = false;
		Stack->PushBool(true);
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// Talk / TalkAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Talk") == 0 || strcmp(Name, "TalkAsync") == 0) {
		Stack->CorrectParams(5);

		char *Text    = Stack->Pop()->GetString();
		CScValue *SoundVal = Stack->Pop();
		int Duration  = Stack->Pop()->GetInt();
		CScValue *ValStances = Stack->Pop();

		char *Stances = ValStances->IsNULL() ? NULL : ValStances->GetString();

		int Align;
		CScValue *val = Stack->Pop();
		if (val->IsNULL()) Align = TAL_CENTER;
		else Align = val->GetInt();

		Align = std::min(std::max(0, Align), NUM_TEXT_ALIGN - 1);

		char *Sound = SoundVal->IsNULL() ? NULL : SoundVal->GetString();

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

		CAdLayer *Main = ((CAdGame *)Game)->m_Scene->m_MainLayer;
		bool RegFound = false;

		int i;
		CScValue *Val = Stack->Pop();
		if (Val->IsNULL() || !Main) {
			m_StickRegion = NULL;
			RegFound = true;
		} else if (Val->IsString()) {
			char *RegionName = Val->GetString();
			for (i = 0; i < Main->m_Nodes.GetSize(); i++) {
				if (Main->m_Nodes[i]->m_Type == OBJECT_REGION && Main->m_Nodes[i]->m_Region->m_Name && scumm_stricmp(Main->m_Nodes[i]->m_Region->m_Name, RegionName) == 0) {
					m_StickRegion = Main->m_Nodes[i]->m_Region;
					RegFound = true;
					break;
				}
			}
		} else if (Val->IsNative()) {
			CBScriptable *Obj = Val->GetNative();

			for (i = 0; i < Main->m_Nodes.GetSize(); i++) {
				if (Main->m_Nodes[i]->m_Type == OBJECT_REGION && Main->m_Nodes[i]->m_Region == Obj) {
					m_StickRegion = Main->m_Nodes[i]->m_Region;
					RegFound = true;
					break;
				}
			}

		}

		if (!RegFound) m_StickRegion = NULL;
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
		if (m_Font && m_Font->m_Filename) Stack->PushString(m_Font->m_Filename);
		else Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TakeItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TakeItem") == 0) {
		Stack->CorrectParams(2);

		if (!m_Inventory) {
			m_Inventory = new CAdInventory(Game);
			((CAdGame *)Game)->RegisterInventory(m_Inventory);
		}

		CScValue *val = Stack->Pop();
		if (!val->IsNULL()) {
			char *ItemName = val->GetString();
			val = Stack->Pop();
			char *InsertAfter = val->IsNULL() ? NULL : val->GetString();
			if (FAILED(m_Inventory->InsertItem(ItemName, InsertAfter))) Script->RuntimeError("Cannot add item '%s' to inventory", ItemName);
			else {
				// hide associated entities
				((CAdGame *)Game)->m_Scene->HandleItemAssociations(ItemName, false);
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

		if (!m_Inventory) {
			m_Inventory = new CAdInventory(Game);
			((CAdGame *)Game)->RegisterInventory(m_Inventory);
		}

		CScValue *val = Stack->Pop();
		if (!val->IsNULL()) {
			if (FAILED(m_Inventory->RemoveItem(val->GetString()))) Script->RuntimeError("Cannot remove item '%s' from inventory", val->GetString());
			else {
				// show associated entities
				((CAdGame *)Game)->m_Scene->HandleItemAssociations(val->GetString(), true);
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

		if (!m_Inventory) {
			m_Inventory = new CAdInventory(Game);
			((CAdGame *)Game)->RegisterInventory(m_Inventory);
		}

		CScValue *val = Stack->Pop();
		if (val->m_Type == VAL_STRING) {
			CAdItem *item = ((CAdGame *)Game)->GetItemByName(val->GetString());
			if (item) Stack->PushNative(item, true);
			else Stack->PushNULL();
		} else if (val->IsNULL() || val->GetInt() < 0 || val->GetInt() >= m_Inventory->m_TakenItems.GetSize())
			Stack->PushNULL();
		else
			Stack->PushNative(m_Inventory->m_TakenItems[val->GetInt()], true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HasItem") == 0) {
		Stack->CorrectParams(1);

		if (!m_Inventory) {
			m_Inventory = new CAdInventory(Game);
			((CAdGame *)Game)->RegisterInventory(m_Inventory);
		}

		CScValue *val = Stack->Pop();
		if (!val->IsNULL()) {
			for (int i = 0; i < m_Inventory->m_TakenItems.GetSize(); i++) {
				if (val->GetNative() == m_Inventory->m_TakenItems[i]) {
					Stack->PushBool(true);
					return S_OK;
				} else if (scumm_stricmp(val->GetString(), m_Inventory->m_TakenItems[i]->m_Name) == 0) {
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
		if (Emitter) Stack->PushNative(m_PartEmitter, true);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteParticleEmitter
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteParticleEmitter") == 0) {
		Stack->CorrectParams(0);
		if (m_PartEmitter) {
			Game->UnregisterObject(m_PartEmitter);
			m_PartEmitter = NULL;
		}
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddAttachment
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddAttachment") == 0) {
		Stack->CorrectParams(4);
		char *Filename = Stack->Pop()->GetString();
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

			Ent->m_PosX = OffsetX;
			Ent->m_PosY = OffsetY;
			Ent->m_Active = true;

			if (PreDisplay) m_AttachmentsPre.Add(Ent);
			else m_AttachmentsPost.Add(Ent);

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
			for (int i = 0; i < m_AttachmentsPre.GetSize(); i++) {
				if (m_AttachmentsPre[i] == Obj) {
					Found = true;
					Game->UnregisterObject(m_AttachmentsPre[i]);
					m_AttachmentsPre.RemoveAt(i);
					i--;
				}
			}
			for (int i = 0; i < m_AttachmentsPost.GetSize(); i++) {
				if (m_AttachmentsPost[i] == Obj) {
					Found = true;
					Game->UnregisterObject(m_AttachmentsPost[i]);
					m_AttachmentsPost.RemoveAt(i);
					i--;
				}
			}
		} else {
			char *Name = Val->GetString();
			for (int i = 0; i < m_AttachmentsPre.GetSize(); i++) {
				if (m_AttachmentsPre[i]->m_Name && scumm_stricmp(m_AttachmentsPre[i]->m_Name, Name) == 0) {
					Found = true;
					Game->UnregisterObject(m_AttachmentsPre[i]);
					m_AttachmentsPre.RemoveAt(i);
					i--;
				}
			}
			for (int i = 0; i < m_AttachmentsPost.GetSize(); i++) {
				if (m_AttachmentsPost[i]->m_Name && scumm_stricmp(m_AttachmentsPost[i]->m_Name, Name) == 0) {
					Found = true;
					Game->UnregisterObject(m_AttachmentsPost[i]);
					m_AttachmentsPost.RemoveAt(i);
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
			for (int i = 0; i < m_AttachmentsPre.GetSize(); i++) {
				if (CurrIndex == Index) Ret = m_AttachmentsPre[i];
				CurrIndex++;
			}
			for (int i = 0; i < m_AttachmentsPost.GetSize(); i++) {
				if (CurrIndex == Index) Ret = m_AttachmentsPost[i];
				CurrIndex++;
			}
		} else {
			char *Name = Val->GetString();
			for (int i = 0; i < m_AttachmentsPre.GetSize(); i++) {
				if (m_AttachmentsPre[i]->m_Name && scumm_stricmp(m_AttachmentsPre[i]->m_Name, Name) == 0) {
					Ret = m_AttachmentsPre[i];
					break;
				}
			}
			if (!Ret) {
				for (int i = 0; i < m_AttachmentsPost.GetSize(); i++) {
					if (m_AttachmentsPost[i]->m_Name && scumm_stricmp(m_AttachmentsPost[i]->m_Name, Name) == 0) {
						Ret = m_AttachmentsPre[i];
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
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("object");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Active") == 0) {
		m_ScValue->SetBool(m_Active);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IgnoreItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IgnoreItems") == 0) {
		m_ScValue->SetBool(m_IgnoreItems);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SceneIndependent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SceneIndependent") == 0) {
		m_ScValue->SetBool(m_SceneIndependent);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesWidth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesWidth") == 0) {
		m_ScValue->SetInt(m_SubtitlesWidth);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosRelative
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosRelative") == 0) {
		m_ScValue->SetBool(m_SubtitlesModRelative);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosX") == 0) {
		m_ScValue->SetInt(m_SubtitlesModX);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosY") == 0) {
		m_ScValue->SetInt(m_SubtitlesModY);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosXCenter
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosXCenter") == 0) {
		m_ScValue->SetBool(m_SubtitlesModXCenter);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumItems (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumItems") == 0) {
		m_ScValue->SetInt(GetInventory()->m_TakenItems.GetSize());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ParticleEmitter (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ParticleEmitter") == 0) {
		if (m_PartEmitter) m_ScValue->SetNative(m_PartEmitter, true);
		else m_ScValue->SetNULL();

		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumAttachments (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumAttachments") == 0) {
		m_ScValue->SetInt(m_AttachmentsPre.GetSize() + m_AttachmentsPost.GetSize());
		return m_ScValue;
	}


	else return CBObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::ScSetProperty(char *Name, CScValue *Value) {

	//////////////////////////////////////////////////////////////////////////
	// Active
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Active") == 0) {
		m_Active = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IgnoreItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IgnoreItems") == 0) {
		m_IgnoreItems = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SceneIndependent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SceneIndependent") == 0) {
		m_SceneIndependent = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesWidth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesWidth") == 0) {
		m_SubtitlesWidth = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosRelative
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosRelative") == 0) {
		m_SubtitlesModRelative = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosX") == 0) {
		m_SubtitlesModX = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosY") == 0) {
		m_SubtitlesModY = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SubtitlesPosXCenter
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SubtitlesPosXCenter") == 0) {
		m_SubtitlesModXCenter = Value->GetBool();
		return S_OK;
	}

	else return CBObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CAdObject::ScToString() {
	return "[ad object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::SetFont(char *Filename) {
	if (m_Font) Game->m_FontStorage->RemoveFont(m_Font);
	if (Filename) {
		m_Font = Game->m_FontStorage->AddFont(Filename);
		return m_Font == NULL ? E_FAIL : S_OK;
	} else {
		m_Font = NULL;
		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
int CAdObject::GetHeight() {
	if (!m_CurrentSprite) return 0;
	else {
		CBFrame *frame = m_CurrentSprite->m_Frames[m_CurrentSprite->m_CurrentFrame];
		int ret = 0;
		for (int i = 0; i < frame->m_Subframes.GetSize(); i++) {
			ret = std::max(ret, frame->m_Subframes[i]->m_HotspotY);
		}

		if (m_Zoomable) {
			float zoom = ((CAdGame *)Game)->m_Scene->GetZoomAt(m_PosX, m_PosY);
			ret = ret * zoom / 100;
		}
		return ret;
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdObject::Talk(char *Text, char *Sound, uint32 Duration, char *Stances, TTextAlign Align) {
	if (!m_Sentence) m_Sentence = new CAdSentence(Game);
	if (!m_Sentence) return;

	if (m_ForcedTalkAnimName && m_ForcedTalkAnimUsed) {
		SAFE_DELETE_ARRAY(m_ForcedTalkAnimName);
		m_ForcedTalkAnimUsed = false;
	}

	SAFE_DELETE(m_Sentence->m_Sound);

	m_Sentence->SetText(Text);
	Game->m_StringTable->Expand(&m_Sentence->m_Text);
	m_Sentence->SetStances(Stances);
	m_Sentence->m_Duration = Duration;
	m_Sentence->m_Align = Align;
	m_Sentence->m_StartTime = Game->m_Timer;
	m_Sentence->m_CurrentStance = -1;
	m_Sentence->m_Font = m_Font == NULL ? Game->m_SystemFont : m_Font;
	m_Sentence->m_Freezable = m_Freezable;

	// try to locate speech file automatically
	bool DeleteSound = false;
	if (!Sound) {
		char *Key = Game->m_StringTable->GetKey(Text);
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
			m_Sentence->SetSound(snd);
			if (m_Sentence->m_Duration <= 0) {
				uint32 Length = snd->GetLength();
				if (Length != 0) m_Sentence->m_Duration = Length;
			}
		} else delete snd;
	}

	// set duration by text length
	if (m_Sentence->m_Duration <= 0) {// TODO: Avoid longs.
		m_Sentence->m_Duration = MAX((unsigned long)1000, Game->m_SubtitlesSpeed * strlen(m_Sentence->m_Text));
	}


	int x, y, width, height;

	x = m_PosX;
	y = m_PosY;

	if (!m_SceneIndependent && m_SubtitlesModRelative) {
		x -= ((CAdGame *)Game)->m_Scene->GetOffsetLeft();
		y -= ((CAdGame *)Game)->m_Scene->GetOffsetTop();
	}


	if (m_SubtitlesWidth > 0) width = m_SubtitlesWidth;
	else {
		if ((x < Game->m_Renderer->m_Width / 4 || x > Game->m_Renderer->m_Width * 0.75) && !Game->m_TouchInterface) {
			width = std::max(Game->m_Renderer->m_Width / 4, std::min(x * 2, (Game->m_Renderer->m_Width - x) * 2));
		} else width = Game->m_Renderer->m_Width / 2;
	}

	height = m_Sentence->m_Font->GetTextHeight((byte  *)m_Sentence->m_Text, width);

	y = y - height - GetHeight() - 5;
	if (m_SubtitlesModRelative) {
		x += m_SubtitlesModX;
		y += m_SubtitlesModY;
	} else {
		x = m_SubtitlesModX;
		y = m_SubtitlesModY;
	}
	if (m_SubtitlesModXCenter)
		x = x - width / 2;


	x = std::min(std::max(0, x), Game->m_Renderer->m_Width - width);
	y = std::min(std::max(0, y), Game->m_Renderer->m_Height - height);

	m_Sentence->m_Width = width;


	m_Sentence->m_Pos.x = x;
	m_Sentence->m_Pos.y = y;


	if (m_SubtitlesModRelative) {
		m_Sentence->m_Pos.x += ((CAdGame *)Game)->m_Scene->GetOffsetLeft();
		m_Sentence->m_Pos.y += ((CAdGame *)Game)->m_Scene->GetOffsetTop();
	}

	m_Sentence->m_FixedPos = !m_SubtitlesModRelative;


	m_Sentence->SetupTalkFile(Sound);

	m_State = STATE_TALKING;

	if (DeleteSound) delete [] Sound;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::Reset() {
	if (m_State == STATE_PLAYING_ANIM && m_AnimSprite != NULL) {
		SAFE_DELETE(m_AnimSprite);
	} else if (m_State == STATE_TALKING && m_Sentence) {
		m_Sentence->Finish();
	}

	m_State = m_NextState = STATE_READY;

	Game->m_ScEngine->ResetObject(this);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::Persist(CBPersistMgr *PersistMgr) {
	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_Active));
	PersistMgr->Transfer(TMEMBER(m_BlockRegion));
	PersistMgr->Transfer(TMEMBER(m_CurrentBlockRegion));
	PersistMgr->Transfer(TMEMBER(m_CurrentWptGroup));
	PersistMgr->Transfer(TMEMBER(m_CurrentSprite));
	PersistMgr->Transfer(TMEMBER(m_Drawn));
	PersistMgr->Transfer(TMEMBER(m_Font));
	PersistMgr->Transfer(TMEMBER(m_IgnoreItems));
	PersistMgr->Transfer(TMEMBER_INT(m_NextState));
	PersistMgr->Transfer(TMEMBER(m_Sentence));
	PersistMgr->Transfer(TMEMBER_INT(m_State));
	PersistMgr->Transfer(TMEMBER(m_AnimSprite));
	PersistMgr->Transfer(TMEMBER(m_SceneIndependent));
	PersistMgr->Transfer(TMEMBER(m_ForcedTalkAnimName));
	PersistMgr->Transfer(TMEMBER(m_ForcedTalkAnimUsed));
	PersistMgr->Transfer(TMEMBER(m_TempSprite2));
	PersistMgr->Transfer(TMEMBER_INT(m_Type));
	PersistMgr->Transfer(TMEMBER(m_WptGroup));
	PersistMgr->Transfer(TMEMBER(m_StickRegion));
	PersistMgr->Transfer(TMEMBER(m_SubtitlesModRelative));
	PersistMgr->Transfer(TMEMBER(m_SubtitlesModX));
	PersistMgr->Transfer(TMEMBER(m_SubtitlesModY));
	PersistMgr->Transfer(TMEMBER(m_SubtitlesModXCenter));
	PersistMgr->Transfer(TMEMBER(m_SubtitlesWidth));
	PersistMgr->Transfer(TMEMBER(m_Inventory));
	PersistMgr->Transfer(TMEMBER(m_PartEmitter));

	for (int i = 0; i < MAX_NUM_REGIONS; i++) PersistMgr->Transfer(TMEMBER(m_CurrentRegions[i]));

	m_AttachmentsPre.Persist(PersistMgr);
	m_AttachmentsPost.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_RegisterAlias));

	PersistMgr->Transfer(TMEMBER(m_PartFollowParent));
	PersistMgr->Transfer(TMEMBER(m_PartOffsetX));
	PersistMgr->Transfer(TMEMBER(m_PartOffsetY));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::UpdateSounds() {
	if (m_Sentence && m_Sentence->m_Sound)
		UpdateOneSound(m_Sentence->m_Sound);

	return CBObject::UpdateSounds();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::ResetSoundPan() {
	if (m_Sentence && m_Sentence->m_Sound) {
		m_Sentence->m_Sound->SetPan(0.0f);
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
	if (m_BlockRegion) m_BlockRegion->SaveAsText(Buffer, Indent + 2, "BLOCKED_REGION");
	if (m_WptGroup) m_WptGroup->SaveAsText(Buffer, Indent + 2);

	CBBase::SaveAsText(Buffer, Indent + 2);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::UpdateBlockRegion() {
	CAdGame *AdGame = (CAdGame *)Game;
	if (AdGame->m_Scene) {
		if (m_BlockRegion && m_CurrentBlockRegion)
			m_CurrentBlockRegion->Mimic(m_BlockRegion, m_Zoomable ? AdGame->m_Scene->GetScaleAt(m_PosY) : 100.0f, m_PosX, m_PosY);

		if (m_WptGroup && m_CurrentWptGroup)
			m_CurrentWptGroup->Mimic(m_WptGroup, m_Zoomable ? AdGame->m_Scene->GetScaleAt(m_PosY) : 100.0f, m_PosX, m_PosY);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CAdInventory *CAdObject::GetInventory() {
	if (!m_Inventory) {
		m_Inventory = new CAdInventory(Game);
		((CAdGame *)Game)->RegisterInventory(m_Inventory);
	}
	return m_Inventory;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::AfterMove() {
	CAdRegion *NewRegions[MAX_NUM_REGIONS];

	((CAdGame *)Game)->m_Scene->GetRegionsAt(m_PosX, m_PosY, NewRegions, MAX_NUM_REGIONS);
	for (int i = 0; i < MAX_NUM_REGIONS; i++) {
		if (!NewRegions[i]) break;
		bool RegFound = false;
		for (int j = 0; j < MAX_NUM_REGIONS; j++) {
			if (m_CurrentRegions[j] == NewRegions[i]) {
				m_CurrentRegions[j] = NULL;
				RegFound = true;
				break;
			}
		}
		if (!RegFound) NewRegions[i]->ApplyEvent("ActorEntry");
	}

	for (int i = 0; i < MAX_NUM_REGIONS; i++) {
		if (m_CurrentRegions[i] && Game->ValidObject(m_CurrentRegions[i])) {
			m_CurrentRegions[i]->ApplyEvent("ActorLeave");
		}
		m_CurrentRegions[i] = NewRegions[i];
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::InvalidateCurrRegions() {
	for (int i = 0; i < MAX_NUM_REGIONS; i++) m_CurrentRegions[i] = NULL;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::GetScale(float *ScaleX, float *ScaleY) {
	if (m_Zoomable) {
		if (m_ScaleX >= 0 || m_ScaleY >= 0) {
			*ScaleX = m_ScaleX < 0 ? 100 : m_ScaleX;
			*ScaleY = m_ScaleY < 0 ? 100 : m_ScaleY;
		} else if (m_Scale >= 0) *ScaleX = *ScaleY = m_Scale;
		else *ScaleX = *ScaleY = ((CAdGame *)Game)->m_Scene->GetZoomAt(m_PosX, m_PosY) + m_RelativeScale;
	} else {
		*ScaleX = *ScaleY = 100;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::UpdateSpriteAttachments() {
	for (int i = 0; i < m_AttachmentsPre.GetSize(); i++) {
		m_AttachmentsPre[i]->Update();
	}
	for (int i = 0; i < m_AttachmentsPost.GetSize(); i++) {
		m_AttachmentsPost[i]->Update();
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::DisplaySpriteAttachments(bool PreDisplay) {
	if (PreDisplay) {
		for (int i = 0; i < m_AttachmentsPre.GetSize(); i++) {
			DisplaySpriteAttachment(m_AttachmentsPre[i]);
		}
	} else {
		for (int i = 0; i < m_AttachmentsPost.GetSize(); i++) {
			DisplaySpriteAttachment(m_AttachmentsPost[i]);
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::DisplaySpriteAttachment(CAdObject *Attachment) {
	if (!Attachment->m_Active) return S_OK;

	float ScaleX, ScaleY;
	GetScale(&ScaleX, &ScaleY);

	int OrigX = Attachment->m_PosX;
	int OrigY = Attachment->m_PosY;

	// inherit position from owner
	Attachment->m_PosX = this->m_PosX + Attachment->m_PosX * ScaleX / 100.0f;
	Attachment->m_PosY = this->m_PosY + Attachment->m_PosY * ScaleY / 100.0f;

	// inherit other props
	Attachment->m_AlphaColor = this->m_AlphaColor;
	Attachment->m_BlendMode = this->m_BlendMode;

	Attachment->m_Scale = this->m_Scale;
	Attachment->m_RelativeScale = this->m_RelativeScale;
	Attachment->m_ScaleX = this->m_ScaleX;
	Attachment->m_ScaleY = this->m_ScaleY;

	Attachment->m_Rotate = this->m_Rotate;
	Attachment->m_RelativeRotate = this->m_RelativeRotate;
	Attachment->m_RotateValid = this->m_RotateValid;

	Attachment->m_RegisterAlias = this;
	Attachment->m_Registrable = this->m_Registrable;

	HRESULT ret = Attachment->Display();

	Attachment->m_PosX = OrigX;
	Attachment->m_PosY = OrigY;

	return ret;
}

//////////////////////////////////////////////////////////////////////////
CPartEmitter *CAdObject::CreateParticleEmitter(bool FollowParent, int OffsetX, int OffsetY) {
	m_PartFollowParent = FollowParent;
	m_PartOffsetX = OffsetX;
	m_PartOffsetY = OffsetY;

	if (!m_PartEmitter) {
		m_PartEmitter = new CPartEmitter(Game, this);
		if (m_PartEmitter) {
			Game->RegisterObject(m_PartEmitter);
		}
	}
	UpdatePartEmitter();
	return m_PartEmitter;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdObject::UpdatePartEmitter() {
	if (!m_PartEmitter) return E_FAIL;

	if (m_PartFollowParent) {
		float ScaleX, ScaleY;
		GetScale(&ScaleX, &ScaleY);

		m_PartEmitter->m_PosX = m_PosX + (ScaleX / 100.0f) * m_PartOffsetX;
		m_PartEmitter->m_PosY = m_PosY + (ScaleY / 100.0f) * m_PartOffsetY;
	}
	return m_PartEmitter->Update();
}

} // end of namespace WinterMute
