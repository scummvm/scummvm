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
#include "AdEntity.h"
#include "BParser.h"
#include "BDynBuffer.h"
#include "BActiveRect.h"
#include "BSurfaceStorage.h"
#include "ScValue.h"
#include "BGame.h"
#include "AdGame.h"
#include "AdScene.h"
#include "BSound.h"
#include "AdWaypointGroup.h"
#include "BFontStorage.h"
#include "BFont.h"
#include "AdSentence.h"
#include "ScScript.h"
#include "ScStack.h"
#include "BRegion.h"
#include "BSprite.h"
#include "BFileManager.h"
#include "PlatformSDL.h"
#include "utils.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdEntity, false)

//////////////////////////////////////////////////////////////////////////
CAdEntity::CAdEntity(CBGame *inGame): CAdTalkHolder(inGame) {
	m_Type = OBJECT_ENTITY;
	m_Subtype = ENTITY_NORMAL;
	m_Region = NULL;
	m_Item = NULL;

	m_WalkToX = m_WalkToY = 0;
	m_WalkToDir = DI_NONE;
}


//////////////////////////////////////////////////////////////////////////
CAdEntity::~CAdEntity() {
	Game->UnregisterObject(m_Region);

	delete[] m_Item;
	m_Item = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdEntity::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing ENTITY file '%s'", Filename);


	delete [] Buffer;

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
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::LoadBuffer(byte  *Buffer, bool Complete) {
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
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_ENTITY) {
			Game->LOG(0, "'ENTITY' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	CAdGame *AdGame = (CAdGame *)Game;
	CBSprite *spr = NULL;
	int ar = 0, ag = 0, ab = 0, alpha = 0;
	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_X:
			parser.ScanStr((char *)params, "%d", &m_PosX);
			break;

		case TOKEN_Y:
			parser.ScanStr((char *)params, "%d", &m_PosY);
			break;

		case TOKEN_SPRITE: {
			SAFE_DELETE(m_Sprite);
			spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			else m_Sprite = spr;
		}
		break;

		case TOKEN_TALK: {
			spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->LoadFile((char *)params, AdGame->m_TexTalkLifeTime))) cmd = PARSERR_GENERIC;
			else m_TalkSprites.Add(spr);
		}
		break;

		case TOKEN_TALK_SPECIAL: {
			spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->LoadFile((char *)params, AdGame->m_TexTalkLifeTime))) cmd = PARSERR_GENERIC;
			else m_TalkSpritesEx.Add(spr);
		}
		break;

		case TOKEN_NAME:
			SetName((char *)params);
			break;

		case TOKEN_ITEM:
			SetItem((char *)params);
			break;

		case TOKEN_CAPTION:
			SetCaption((char *)params);
			break;

		case TOKEN_FONT:
			SetFont((char *)params);
			break;

		case TOKEN_SCALABLE:
			parser.ScanStr((char *)params, "%b", &m_Zoomable);
			break;

		case TOKEN_SCALE: {
			int s;
			parser.ScanStr((char *)params, "%d", &s);
			m_Scale = (float)s;

		}
		break;

		case TOKEN_RELATIVE_SCALE: {
			int s;
			parser.ScanStr((char *)params, "%d", &s);
			m_RelativeScale = (float)s;

		}
		break;

		case TOKEN_ROTABLE:
		case TOKEN_ROTATABLE:
			parser.ScanStr((char *)params, "%b", &m_Rotatable);
			break;

		case TOKEN_REGISTRABLE:
		case TOKEN_INTERACTIVE:
			parser.ScanStr((char *)params, "%b", &m_Registrable);
			break;

		case TOKEN_SHADOWABLE:
		case TOKEN_COLORABLE:
			parser.ScanStr((char *)params, "%b", &m_Shadowable);
			break;

		case TOKEN_ACTIVE:
			parser.ScanStr((char *)params, "%b", &m_Active);
			break;

		case TOKEN_CURSOR:
			SAFE_DELETE(m_Cursor);
			m_Cursor = new CBSprite(Game);
			if (!m_Cursor || FAILED(m_Cursor->LoadFile((char *)params))) {
				SAFE_DELETE(m_Cursor);
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_EDITOR_SELECTED:
			parser.ScanStr((char *)params, "%b", &m_EditorSelected);
			break;

		case TOKEN_REGION: {
			if (m_Region) Game->UnregisterObject(m_Region);
			m_Region = NULL;
			CBRegion *rgn = new CBRegion(Game);
			if (!rgn || FAILED(rgn->LoadBuffer(params, false))) cmd = PARSERR_GENERIC;
			else {
				m_Region = rgn;
				Game->RegisterObject(m_Region);
			}
		}
		break;

		case TOKEN_BLOCKED_REGION: {
			SAFE_DELETE(m_BlockRegion);
			SAFE_DELETE(m_CurrentBlockRegion);
			CBRegion *rgn = new CBRegion(Game);
			CBRegion *crgn = new CBRegion(Game);
			if (!rgn || !crgn || FAILED(rgn->LoadBuffer(params, false))) {
				SAFE_DELETE(m_BlockRegion);
				SAFE_DELETE(m_CurrentBlockRegion);
				cmd = PARSERR_GENERIC;
			} else {
				m_BlockRegion = rgn;
				m_CurrentBlockRegion = crgn;
				m_CurrentBlockRegion->Mimic(m_BlockRegion);
			}
		}
		break;

		case TOKEN_WAYPOINTS: {
			SAFE_DELETE(m_WptGroup);
			SAFE_DELETE(m_CurrentWptGroup);
			CAdWaypointGroup *wpt = new CAdWaypointGroup(Game);
			CAdWaypointGroup *cwpt = new CAdWaypointGroup(Game);
			if (!wpt || !cwpt || FAILED(wpt->LoadBuffer(params, false))) {
				SAFE_DELETE(m_WptGroup);
				SAFE_DELETE(m_CurrentWptGroup);
				cmd = PARSERR_GENERIC;
			} else {
				m_WptGroup = wpt;
				m_CurrentWptGroup = cwpt;
				m_CurrentWptGroup->Mimic(m_WptGroup);
			}
		}
		break;

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_SUBTYPE: {
			if (scumm_stricmp((char *)params, "sound") == 0) {
				SAFE_DELETE(m_Sprite);
				if (Game->m_EditorMode) {
					spr = new CBSprite(Game, this);
					if (!spr || FAILED(spr->LoadFile("entity_sound.sprite"))) cmd = PARSERR_GENERIC;
					else m_Sprite = spr;
				}
				if (Game->m_EditorMode) m_EditorOnly = true;
				m_Zoomable = false;
				m_Rotatable = false;
				m_Registrable = Game->m_EditorMode;
				m_Shadowable = false;
				m_Subtype = ENTITY_SOUND;
			}
		}
		break;

		case TOKEN_SOUND:
			PlaySFX((char *)params, false, false);
			break;

		case TOKEN_SOUND_START_TIME:
			parser.ScanStr((char *)params, "%d", &m_SFXStart);
			break;

		case TOKEN_SOUND_VOLUME:
			parser.ScanStr((char *)params, "%d", &m_SFXVolume);
			break;

		case TOKEN_SOUND_PANNING:
			parser.ScanStr((char *)params, "%b", &m_AutoSoundPanning);
			break;

		case TOKEN_SAVE_STATE:
			parser.ScanStr((char *)params, "%b", &m_SaveState);
			break;

		case TOKEN_PROPERTY:
			ParseProperty(params, false);
			break;

		case TOKEN_IGNORE_ITEMS:
			parser.ScanStr((char *)params, "%b", &m_IgnoreItems);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.ScanStr((char *)params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.ScanStr((char *)params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;

		case TOKEN_WALK_TO_X:
			parser.ScanStr((char *)params, "%d", &m_WalkToX);
			break;

		case TOKEN_WALK_TO_Y:
			parser.ScanStr((char *)params, "%d", &m_WalkToY);
			break;

		case TOKEN_WALK_TO_DIR: {
			int i;
			parser.ScanStr((char *)params, "%d", &i);
			if (i < 0) i = 0;
			if (i >= NUM_DIRECTIONS) i = DI_NONE;
			m_WalkToDir = (TDirection)i;
		}
		break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in ENTITY definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading ENTITY definition");
		if (spr) delete spr;
		return E_FAIL;
	}

	if (m_Region && m_Sprite) {
		Game->LOG(0, "Warning: Entity '%s' has both sprite and region.", m_Name);
	}

	UpdatePosition();

	if (alpha != 0 && ar == 0 && ag == 0 && ab == 0) {
		ar = ag = ab = 255;
	}
	m_AlphaColor = DRGBA(ar, ag, ab, alpha);
	m_State = STATE_READY;

	if (m_Item && ((CAdGame *)Game)->IsItemTaken(m_Item)) m_Active = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::Display() {
	if (m_Active) {
		UpdateSounds();

		uint32 Alpha;
		if (m_AlphaColor != 0) Alpha = m_AlphaColor;
		else Alpha = m_Shadowable ? ((CAdGame *)Game)->m_Scene->GetAlphaAt(m_PosX, m_PosY) : 0xFFFFFFFF;

		float ScaleX, ScaleY;
		GetScale(&ScaleX, &ScaleY);

		float Rotate;
		if (m_Rotatable) {
			if (m_RotateValid) Rotate = m_Rotate;
			else Rotate = ((CAdGame *)Game)->m_Scene->GetRotationAt(m_PosX, m_PosY) + m_RelativeRotate;
		} else Rotate = 0.0f;


		bool Reg = m_Registrable;
		if (m_IgnoreItems && ((CAdGame *)Game)->m_SelectedItem) Reg = false;

		if (m_Region && (Reg || m_EditorAlwaysRegister)) {
			Game->m_Renderer->m_RectList.Add(new CBActiveRect(Game, m_RegisterAlias, m_Region, Game->m_OffsetX, Game->m_OffsetY));
		}

		DisplaySpriteAttachments(true);
		if (m_CurrentSprite) {
			m_CurrentSprite->Display(m_PosX,
			                         m_PosY,
			                         (Reg || m_EditorAlwaysRegister) ? m_RegisterAlias : NULL,
			                         ScaleX,
			                         ScaleY,
			                         Alpha,
			                         Rotate,
			                         m_BlendMode);
		}
		DisplaySpriteAttachments(false);

		if (m_PartEmitter) m_PartEmitter->Display(m_Region);

	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::Update() {
	m_CurrentSprite = NULL;

	if (m_State == STATE_READY && m_AnimSprite) {
		SAFE_DELETE(m_AnimSprite);
	}

	// finished playing animation?
	if (m_State == STATE_PLAYING_ANIM && m_AnimSprite != NULL && m_AnimSprite->m_Finished) {
		m_State = STATE_READY;
		m_CurrentSprite = m_AnimSprite;
	}

	if (m_Sentence && m_State != STATE_TALKING) m_Sentence->Finish();

	// default: stand animation
	if (!m_CurrentSprite) m_CurrentSprite = m_Sprite;

	switch (m_State) {
		//////////////////////////////////////////////////////////////////////////
	case STATE_PLAYING_ANIM:
		m_CurrentSprite = m_AnimSprite;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_READY:
		if (!m_AnimSprite)
			m_CurrentSprite = m_Sprite;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_TALKING: {
		m_Sentence->Update();
		if (m_Sentence->m_CurrentSprite) m_TempSprite2 = m_Sentence->m_CurrentSprite;

		bool TimeIsUp = (m_Sentence->m_Sound && m_Sentence->m_SoundStarted && (!m_Sentence->m_Sound->IsPlaying() && !m_Sentence->m_Sound->IsPaused())) || (!m_Sentence->m_Sound && m_Sentence->m_Duration <= Game->m_Timer - m_Sentence->m_StartTime);
		if (m_TempSprite2 == NULL || m_TempSprite2->m_Finished || (/*m_TempSprite2->m_Looping &&*/ TimeIsUp)) {
			if (TimeIsUp) {
				m_Sentence->Finish();
				m_TempSprite2 = NULL;
				m_State = STATE_READY;
			} else {
				m_TempSprite2 = GetTalkStance(m_Sentence->GetNextStance());
				if (m_TempSprite2) {
					m_TempSprite2->Reset();
					m_CurrentSprite = m_TempSprite2;
				}
				((CAdGame *)Game)->AddSentence(m_Sentence);
			}
		} else {
			m_CurrentSprite = m_TempSprite2;
			((CAdGame *)Game)->AddSentence(m_Sentence);
		}
	}
	break;
	}


	if (m_CurrentSprite) {
		m_CurrentSprite->GetCurrentFrame(m_Zoomable ? ((CAdGame *)Game)->m_Scene->GetZoomAt(m_PosX, m_PosY) : 100);
		if (m_CurrentSprite->m_Changed) {
			m_PosX += m_CurrentSprite->m_MoveX;
			m_PosY += m_CurrentSprite->m_MoveY;
		}
	}

	UpdateBlockRegion();
	m_Ready = (m_State == STATE_READY);


	UpdatePartEmitter();
	UpdateSpriteAttachments();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// StopSound
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "StopSound") == 0 && m_Subtype == ENTITY_SOUND) {
		Stack->CorrectParams(0);

		if (FAILED(StopSFX(false))) Stack->PushBool(false);
		else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PlayTheora") == 0) {
		Stack->CorrectParams(0);
		Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "StopTheora") == 0) {
		Stack->CorrectParams(0);
		Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsTheoraPlaying
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsTheoraPlaying") == 0) {
		Stack->CorrectParams(0);
		Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PauseTheora") == 0) {
		Stack->CorrectParams(0);
		Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResumeTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ResumeTheora") == 0) {
		Stack->CorrectParams(0);
		Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsTheoraPaused
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsTheoraPaused") == 0) {
		Stack->CorrectParams(0);
		Stack->PushBool(false);

		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// CreateRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateRegion") == 0) {
		Stack->CorrectParams(0);
		if (!m_Region) {
			m_Region = new CBRegion(Game);
			Game->RegisterObject(m_Region);
		}
		if (m_Region) Stack->PushNative(m_Region, true);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteRegion") == 0) {
		Stack->CorrectParams(0);
		if (m_Region) {
			Game->UnregisterObject(m_Region);
			m_Region = NULL;
			Stack->PushBool(true);
		} else Stack->PushBool(false);

		return S_OK;
	}

	else return CAdTalkHolder::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdEntity::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("entity");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Item
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Item") == 0) {
		if (m_Item) m_ScValue->SetString(m_Item);
		else m_ScValue->SetNULL();

		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Subtype (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Subtype") == 0) {
		if (m_Subtype == ENTITY_SOUND)
			m_ScValue->SetString("sound");
		else
			m_ScValue->SetString("normal");

		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WalkToX") == 0) {
		m_ScValue->SetInt(m_WalkToX);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WalkToY") == 0) {
		m_ScValue->SetInt(m_WalkToY);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToDirection
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WalkToDirection") == 0) {
		m_ScValue->SetInt((int)m_WalkToDir);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Region (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Region") == 0) {
		if (m_Region) m_ScValue->SetNative(m_Region, true);
		else m_ScValue->SetNULL();
		return m_ScValue;
	}

	else return CAdTalkHolder::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::ScSetProperty(char *Name, CScValue *Value) {

	//////////////////////////////////////////////////////////////////////////
	// Item
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Item") == 0) {
		SetItem(Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WalkToX") == 0) {
		m_WalkToX = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WalkToY") == 0) {
		m_WalkToY = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToDirection
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WalkToDirection") == 0) {
		int Dir = Value->GetInt();
		if (Dir >= 0 && Dir < NUM_DIRECTIONS) m_WalkToDir = (TDirection)Dir;
		return S_OK;
	}


	else return CAdTalkHolder::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CAdEntity::ScToString() {
	return "[entity object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "ENTITY {\n");
	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", m_Name);
	if (m_Subtype == ENTITY_SOUND)
		Buffer->PutTextIndent(Indent + 2, "SUBTYPE=\"SOUND\"\n");
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", GetCaption());
	Buffer->PutTextIndent(Indent + 2, "ACTIVE=%s\n", m_Active ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "X=%d\n", m_PosX);
	Buffer->PutTextIndent(Indent + 2, "Y=%d\n", m_PosY);
	Buffer->PutTextIndent(Indent + 2, "SCALABLE=%s\n", m_Zoomable ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "INTERACTIVE=%s\n", m_Registrable ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "COLORABLE=%s\n", m_Shadowable ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "EDITOR_SELECTED=%s\n", m_EditorSelected ? "TRUE" : "FALSE");
	if (m_IgnoreItems)
		Buffer->PutTextIndent(Indent + 2, "IGNORE_ITEMS=%s\n", m_IgnoreItems ? "TRUE" : "FALSE");
	if (m_Rotatable)
		Buffer->PutTextIndent(Indent + 2, "ROTATABLE=%s\n", m_Rotatable ? "TRUE" : "FALSE");

	if (!m_AutoSoundPanning)
		Buffer->PutTextIndent(Indent + 2, "SOUND_PANNING=%s\n", m_AutoSoundPanning ? "TRUE" : "FALSE");

	if (!m_SaveState)
		Buffer->PutTextIndent(Indent + 2, "SAVE_STATE=%s\n", m_SaveState ? "TRUE" : "FALSE");

	if (m_Item && m_Item[0] != '\0') Buffer->PutTextIndent(Indent + 2, "ITEM=\"%s\"\n", m_Item);

	Buffer->PutTextIndent(Indent + 2, "WALK_TO_X=%d\n", m_WalkToX);
	Buffer->PutTextIndent(Indent + 2, "WALK_TO_Y=%d\n", m_WalkToY);
	if (m_WalkToDir != DI_NONE)
		Buffer->PutTextIndent(Indent + 2, "WALK_TO_DIR=%d\n", (int)m_WalkToDir);

	int i;

	for (i = 0; i < m_Scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", m_Scripts[i]->m_Filename);
	}

	if (m_Subtype == ENTITY_NORMAL && m_Sprite && m_Sprite->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "SPRITE=\"%s\"\n", m_Sprite->m_Filename);

	if (m_Subtype == ENTITY_SOUND && m_SFX && m_SFX->m_SoundFilename) {
		Buffer->PutTextIndent(Indent + 2, "SOUND=\"%s\"\n", m_SFX->m_SoundFilename);
		Buffer->PutTextIndent(Indent + 2, "SOUND_START_TIME=%d\n", m_SFXStart);
		Buffer->PutTextIndent(Indent + 2, "SOUND_VOLUME=%d\n", m_SFXVolume);
	}


	if (D3DCOLGetR(m_AlphaColor) != 0 || D3DCOLGetG(m_AlphaColor) != 0 ||  D3DCOLGetB(m_AlphaColor) != 0)
		Buffer->PutTextIndent(Indent + 2, "ALPHA_COLOR { %d,%d,%d }\n", D3DCOLGetR(m_AlphaColor), D3DCOLGetG(m_AlphaColor), D3DCOLGetB(m_AlphaColor));

	if (D3DCOLGetA(m_AlphaColor) != 0)
		Buffer->PutTextIndent(Indent + 2, "ALPHA = %d\n", D3DCOLGetA(m_AlphaColor));

	if (m_Scale >= 0)
		Buffer->PutTextIndent(Indent + 2, "SCALE = %d\n", (int)m_Scale);

	if (m_RelativeScale != 0)
		Buffer->PutTextIndent(Indent + 2, "RELATIVE_SCALE = %d\n", (int)m_RelativeScale);

	if (m_Font && m_Font->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "FONT=\"%s\"\n", m_Font->m_Filename);

	if (m_Cursor && m_Cursor->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "CURSOR=\"%s\"\n", m_Cursor->m_Filename);

	CAdTalkHolder::SaveAsText(Buffer, Indent + 2);

	if (m_Region) m_Region->SaveAsText(Buffer, Indent + 2);

	if (m_ScProp) m_ScProp->SaveAsText(Buffer, Indent + 2);

	CAdObject::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CAdEntity::GetHeight() {
	if (m_Region && !m_Sprite) {
		return m_Region->m_Rect.bottom - m_Region->m_Rect.top;
	} else {
		if (m_CurrentSprite == NULL) m_CurrentSprite = m_Sprite;
		return CAdObject::GetHeight();
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdEntity::UpdatePosition() {
	if (m_Region && !m_Sprite) {
		m_PosX = m_Region->m_Rect.left + (m_Region->m_Rect.right - m_Region->m_Rect.left) / 2;
		m_PosY = m_Region->m_Rect.bottom;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::Persist(CBPersistMgr *PersistMgr) {
	CAdTalkHolder::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_Item));
	PersistMgr->Transfer(TMEMBER(m_Region));
	//PersistMgr->Transfer(TMEMBER(m_Sprite));
	PersistMgr->Transfer(TMEMBER_INT(m_Subtype));
	m_TalkSprites.Persist(PersistMgr);
	m_TalkSpritesEx.Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_WalkToX));
	PersistMgr->Transfer(TMEMBER(m_WalkToY));
	PersistMgr->Transfer(TMEMBER_INT(m_WalkToDir));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdEntity::SetItem(char *ItemName) {
	CBUtils::SetString(&m_Item, ItemName);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::SetSprite(char *Filename) {
	bool SetCurrent = false;
	if (m_CurrentSprite == m_Sprite) {
		m_CurrentSprite = NULL;
		SetCurrent = true;
	}

	SAFE_DELETE(m_Sprite);
	CBSprite *spr = new CBSprite(Game, this);
	if (!spr || FAILED(spr->LoadFile(Filename))) {
		SAFE_DELETE(m_Sprite);
		return E_FAIL;
	} else {
		m_Sprite = spr;
		m_CurrentSprite = m_Sprite;
		return S_OK;
	}
}

} // end of namespace WinterMute
