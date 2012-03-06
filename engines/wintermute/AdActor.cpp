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
#include "engines/wintermute/dctypes.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/BObject.h"
#include "engines/wintermute/AdActor.h"
#include "engines/wintermute/AdGame.h"
#include "engines/wintermute/AdScene.h"
#include "engines/wintermute/AdEntity.h"
#include "engines/wintermute/AdSpriteSet.h"
#include "engines/wintermute/AdWaypointGroup.h"
#include "engines/wintermute/AdPath.h"
#include "engines/wintermute/AdSentence.h"
#include "engines/wintermute/BObject.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/BSound.h"
#include "engines/wintermute/BRegion.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/utils.h"
#include "engines/wintermute/PlatformSDL.h"
#include <math.h>

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdActor, false)


//////////////////////////////////////////////////////////////////////////
CAdActor::CAdActor(CBGame *inGame): CAdTalkHolder(inGame) {
	m_Path = new CAdPath(Game);

	m_Type = OBJECT_ACTOR;
	m_Dir = DI_LEFT;

	m_WalkSprite = NULL;
	m_StandSprite = NULL;
	m_TurnLeftSprite = NULL;
	m_TurnRightSprite = NULL;

	m_TargetPoint = new CBPoint;
	m_AfterWalkDir = DI_NONE;

	m_AnimSprite2 = NULL;

	SetDefaultAnimNames();
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::SetDefaultAnimNames() {
	m_TalkAnimName = NULL;
	CBUtils::SetString(&m_TalkAnimName, "talk");

	m_IdleAnimName = NULL;
	CBUtils::SetString(&m_IdleAnimName, "idle");

	m_WalkAnimName = NULL;
	CBUtils::SetString(&m_WalkAnimName, "walk");

	m_TurnLeftAnimName = NULL;
	CBUtils::SetString(&m_TurnLeftAnimName, "turnleft");

	m_TurnRightAnimName = NULL;
	CBUtils::SetString(&m_TurnRightAnimName, "turnright");

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CAdActor::~CAdActor() {
	delete m_Path;
	delete m_TargetPoint;
	m_Path = NULL;
	m_TargetPoint = NULL;

	delete m_WalkSprite;
	delete m_StandSprite;
	delete m_TurnLeftSprite;
	delete m_TurnRightSprite;
	m_WalkSprite = NULL;
	m_StandSprite = NULL;
	m_TurnLeftSprite = NULL;
	m_TurnRightSprite = NULL;

	m_AnimSprite2 = NULL; // ref only

	for (int i = 0; i < m_TalkSprites.GetSize(); i++) {
		delete m_TalkSprites[i];
	}
	m_TalkSprites.RemoveAll();

	for (int i = 0; i < m_TalkSpritesEx.GetSize(); i++) {
		delete m_TalkSpritesEx[i];
	}
	m_TalkSpritesEx.RemoveAll();


	delete[] m_TalkAnimName;
	delete[] m_IdleAnimName;
	delete[] m_WalkAnimName;
	delete[] m_TurnLeftAnimName;
	delete[] m_TurnRightAnimName;
	m_TalkAnimName = NULL;
	m_IdleAnimName = NULL;
	m_WalkAnimName = NULL;
	m_TurnLeftAnimName = NULL;
	m_TurnRightAnimName = NULL;

	for (int i = 0; i < m_Anims.GetSize(); i++) {
		delete m_Anims[i];
		m_Anims[i] = NULL;
	}
	m_Anims.RemoveAll();

}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdActor::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing ACTOR file '%s'", Filename);


	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(ACTOR)
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
TOKEN_DEF(WALK)
TOKEN_DEF(STAND)
TOKEN_DEF(TALK_SPECIAL)
TOKEN_DEF(TALK)
TOKEN_DEF(TURN_LEFT)
TOKEN_DEF(TURN_RIGHT)
TOKEN_DEF(EVENTS)
TOKEN_DEF(FONT)
TOKEN_DEF(CURSOR)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(SOUND_VOLUME)
TOKEN_DEF(SOUND_PANNING)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(BLOCKED_REGION)
TOKEN_DEF(WAYPOINTS)
TOKEN_DEF(IGNORE_ITEMS)
TOKEN_DEF(ROTABLE)
TOKEN_DEF(ROTATABLE)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(SCALE)
TOKEN_DEF(RELATIVE_SCALE)
TOKEN_DEF(ALPHA)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(ANIMATION)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ACTOR)
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
	TOKEN_TABLE(WALK)
	TOKEN_TABLE(STAND)
	TOKEN_TABLE(TALK_SPECIAL)
	TOKEN_TABLE(TALK)
	TOKEN_TABLE(TURN_LEFT)
	TOKEN_TABLE(TURN_RIGHT)
	TOKEN_TABLE(EVENTS)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(SOUND_VOLUME)
	TOKEN_TABLE(SOUND_PANNING)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(BLOCKED_REGION)
	TOKEN_TABLE(WAYPOINTS)
	TOKEN_TABLE(IGNORE_ITEMS)
	TOKEN_TABLE(ROTABLE)
	TOKEN_TABLE(ROTATABLE)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(SCALE)
	TOKEN_TABLE(RELATIVE_SCALE)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(ANIMATION)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_ACTOR) {
			Game->LOG(0, "'ACTOR' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	CAdGame *AdGame = (CAdGame *)Game;
	CAdSpriteSet *spr = NULL;
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

		case TOKEN_NAME:
			SetName((char *)params);
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

		case TOKEN_WALK:
			delete m_WalkSprite;
			m_WalkSprite = NULL;
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->LoadBuffer(params, true, AdGame->m_TexWalkLifeTime, CACHE_HALF))) cmd = PARSERR_GENERIC;
			else m_WalkSprite = spr;
			break;

		case TOKEN_TALK:
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->LoadBuffer(params, true, AdGame->m_TexTalkLifeTime))) cmd = PARSERR_GENERIC;
			else m_TalkSprites.Add(spr);
			break;

		case TOKEN_TALK_SPECIAL:
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->LoadBuffer(params, true, AdGame->m_TexTalkLifeTime))) cmd = PARSERR_GENERIC;
			else m_TalkSpritesEx.Add(spr);
			break;

		case TOKEN_STAND:
			delete m_StandSprite;
			m_StandSprite = NULL;
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->LoadBuffer(params, true, AdGame->m_TexStandLifeTime))) cmd = PARSERR_GENERIC;
			else m_StandSprite = spr;
			break;

		case TOKEN_TURN_LEFT:
			delete m_TurnLeftSprite;
			m_TurnLeftSprite = NULL;
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->LoadBuffer(params, true))) cmd = PARSERR_GENERIC;
			else m_TurnLeftSprite = spr;
			break;

		case TOKEN_TURN_RIGHT:
			delete m_TurnRightSprite;
			m_TurnRightSprite = NULL;
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->LoadBuffer(params, true))) cmd = PARSERR_GENERIC;
			else m_TurnRightSprite = spr;
			break;

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_CURSOR:
			delete m_Cursor;
			m_Cursor = new CBSprite(Game);
			if (!m_Cursor || FAILED(m_Cursor->LoadFile((char *)params))) {
				delete m_Cursor;
				m_Cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_SOUND_VOLUME:
			parser.ScanStr((char *)params, "%d", &m_SFXVolume);
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

		case TOKEN_SOUND_PANNING:
			parser.ScanStr((char *)params, "%b", &m_AutoSoundPanning);
			break;

		case TOKEN_PROPERTY:
			ParseProperty(params, false);
			break;

		case TOKEN_BLOCKED_REGION: {
			delete m_BlockRegion;
			delete m_CurrentBlockRegion;
			m_BlockRegion = NULL;
			m_CurrentBlockRegion = NULL;
			CBRegion *rgn = new CBRegion(Game);
			CBRegion *crgn = new CBRegion(Game);
			if (!rgn || !crgn || FAILED(rgn->LoadBuffer(params, false))) {
				delete m_BlockRegion;
				delete m_CurrentBlockRegion;
				m_BlockRegion = NULL;
				m_CurrentBlockRegion = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				m_BlockRegion = rgn;
				m_CurrentBlockRegion = crgn;
				m_CurrentBlockRegion->Mimic(m_BlockRegion);
			}
		}
		break;

		case TOKEN_WAYPOINTS: {
			delete m_WptGroup;
			delete m_CurrentWptGroup;
			m_WptGroup = NULL;
			m_CurrentWptGroup = NULL;
			CAdWaypointGroup *wpt = new CAdWaypointGroup(Game);
			CAdWaypointGroup *cwpt = new CAdWaypointGroup(Game);
			if (!wpt || !cwpt || FAILED(wpt->LoadBuffer(params, false))) {
				delete m_WptGroup;
				delete m_CurrentWptGroup;
				m_WptGroup = NULL;
				m_CurrentWptGroup = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				m_WptGroup = wpt;
				m_CurrentWptGroup = cwpt;
				m_CurrentWptGroup->Mimic(m_WptGroup);
			}
		}
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

		case TOKEN_ANIMATION: {
			CAdSpriteSet *Anim = new CAdSpriteSet(Game, this);
			if (!Anim || FAILED(Anim->LoadBuffer(params, false))) cmd = PARSERR_GENERIC;
			else m_Anims.Add(Anim);
		}
		break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in ACTOR definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		if (spr) delete spr;
		Game->LOG(0, "Error loading ACTOR definition");
		return E_FAIL;
	}

	if (alpha != 0 && ar == 0 && ag == 0 && ab == 0) {
		ar = ag = ab = 255;
	}
	m_AlphaColor = DRGBA(ar, ag, ab, alpha);
	m_State = m_NextState = STATE_READY;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdActor::TurnTo(TDirection dir) {
	int delta1, delta2, delta3, delta;

	delta1 = dir - m_Dir;
	delta2 = dir + NUM_DIRECTIONS - m_Dir;
	delta3 = dir - NUM_DIRECTIONS - m_Dir;

	delta1 = (abs(delta1) <= abs(delta2)) ? delta1 : delta2;
	delta = (abs(delta1) <= abs(delta3)) ? delta1 : delta3;

	// already there?
	if (abs(delta) < 2) {
		m_Dir = dir;
		m_State = m_NextState;
		m_NextState = STATE_READY;
		return;
	}

	m_TargetDir = dir;
	m_State = delta < 0 ? STATE_TURNING_LEFT : STATE_TURNING_RIGHT;

	m_TempSprite2 = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdActor::GoTo(int X, int Y, TDirection AfterWalkDir) {
	m_AfterWalkDir = AfterWalkDir;
	if (X == m_TargetPoint->x && Y == m_TargetPoint->y && m_State == STATE_FOLLOWING_PATH) return;

	m_Path->Reset();
	m_Path->SetReady(false);

	m_TargetPoint->x = X;
	m_TargetPoint->y = Y;

	((CAdGame *)Game)->m_Scene->CorrectTargetPoint(m_PosX, m_PosY, &m_TargetPoint->x, &m_TargetPoint->y, true, this);

	m_State = STATE_SEARCHING_PATH;

}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::Display() {
	if (m_Active) UpdateSounds();

	uint32 Alpha;
	if (m_AlphaColor != 0) Alpha = m_AlphaColor;
	else Alpha = m_Shadowable ? ((CAdGame *)Game)->m_Scene->GetAlphaAt(m_PosX, m_PosY, true) : 0xFFFFFFFF;

	float ScaleX, ScaleY;
	GetScale(&ScaleX, &ScaleY);


	float Rotate;
	if (m_Rotatable) {
		if (m_RotateValid) Rotate = m_Rotate;
		else Rotate = ((CAdGame *)Game)->m_Scene->GetRotationAt(m_PosX, m_PosY) + m_RelativeRotate;
	} else Rotate = 0.0f;

	if (m_Active) DisplaySpriteAttachments(true);

	if (m_CurrentSprite && m_Active) {
		bool Reg = m_Registrable;
		if (m_IgnoreItems && ((CAdGame *)Game)->m_SelectedItem) Reg = false;

		m_CurrentSprite->Display(m_PosX,
		                         m_PosY,
		                         Reg ? m_RegisterAlias : NULL,
		                         ScaleX,
		                         ScaleY,
		                         Alpha,
		                         Rotate,
		                         m_BlendMode);

	}

	if (m_Active) DisplaySpriteAttachments(false);
	if (m_Active && m_PartEmitter) m_PartEmitter->Display();


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::Update() {
	m_CurrentSprite = NULL;

	if (m_State == STATE_READY) {
		if (m_AnimSprite) {
			delete m_AnimSprite;
			m_AnimSprite = NULL;
		}
		if (m_AnimSprite2) {
			m_AnimSprite2 = NULL;
		}
	}

	// finished playing animation?
	if (m_State == STATE_PLAYING_ANIM && m_AnimSprite != NULL && m_AnimSprite->m_Finished) {
		m_State = m_NextState;
		m_NextState = STATE_READY;
		m_CurrentSprite = m_AnimSprite;
	}

	if (m_State == STATE_PLAYING_ANIM_SET && m_AnimSprite2 != NULL && m_AnimSprite2->m_Finished) {
		m_State = m_NextState;
		m_NextState = STATE_READY;
		m_CurrentSprite = m_AnimSprite2;
	}

	if (m_Sentence && m_State != STATE_TALKING) m_Sentence->Finish();

	// default: stand animation
	if (!m_CurrentSprite) {
		if (m_Sprite) m_CurrentSprite = m_Sprite;
		else {
			if (m_StandSprite) {
				m_CurrentSprite = m_StandSprite->GetSprite(m_Dir);
			} else {
				CAdSpriteSet *Anim = GetAnimByName(m_IdleAnimName);
				if (Anim) m_CurrentSprite = Anim->GetSprite(m_Dir);
			}
		}
	}

	bool already_moved = false;

	switch (m_State) {
		//////////////////////////////////////////////////////////////////////////
	case STATE_PLAYING_ANIM:
		m_CurrentSprite = m_AnimSprite;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_PLAYING_ANIM_SET:
		m_CurrentSprite = m_AnimSprite2;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_TURNING_LEFT:
		if (m_TempSprite2 == NULL || m_TempSprite2->m_Finished) {
			if (m_Dir > 0) m_Dir = (TDirection)(m_Dir - 1);
			else m_Dir = (TDirection)(NUM_DIRECTIONS - 1);

			if (m_Dir == m_TargetDir) {
				m_TempSprite2 = NULL;
				m_State = m_NextState;
				m_NextState = STATE_READY;
			} else {
				if (m_TurnLeftSprite) {
					m_TempSprite2 = m_TurnLeftSprite->GetSprite(m_Dir);
				} else {
					CAdSpriteSet *Anim = GetAnimByName(m_TurnLeftAnimName);
					if (Anim) m_TempSprite2 = Anim->GetSprite(m_Dir);
				}

				if (m_TempSprite2) {
					m_TempSprite2->Reset();
					if (m_TempSprite2->m_Looping) m_TempSprite2->m_Looping = false;
				}
				m_CurrentSprite = m_TempSprite2;
			}
		} else m_CurrentSprite = m_TempSprite2;
		break;


		//////////////////////////////////////////////////////////////////////////
	case STATE_TURNING_RIGHT:
		if (m_TempSprite2 == NULL || m_TempSprite2->m_Finished) {
			m_Dir = (TDirection)(m_Dir + 1);

			if ((int)m_Dir >= (int)NUM_DIRECTIONS) m_Dir = (TDirection)(0);

			if (m_Dir == m_TargetDir) {
				m_TempSprite2 = NULL;
				m_State = m_NextState;
				m_NextState = STATE_READY;
			} else {
				if (m_TurnRightSprite) {
					m_TempSprite2 = m_TurnRightSprite->GetSprite(m_Dir);
				} else {
					CAdSpriteSet *Anim = GetAnimByName(m_TurnRightAnimName);
					if (Anim) m_TempSprite2 = Anim->GetSprite(m_Dir);
				}

				if (m_TempSprite2) {
					m_TempSprite2->Reset();
					if (m_TempSprite2->m_Looping) m_TempSprite2->m_Looping = false;
				}
				m_CurrentSprite = m_TempSprite2;
			}
		} else m_CurrentSprite = m_TempSprite2;
		break;


		//////////////////////////////////////////////////////////////////////////
	case STATE_SEARCHING_PATH:
		// keep asking scene for the path
		if (((CAdGame *)Game)->m_Scene->GetPath(CBPoint(m_PosX, m_PosY), *m_TargetPoint, m_Path, this))
			m_State = STATE_WAITING_PATH;
		break;


		//////////////////////////////////////////////////////////////////////////
	case STATE_WAITING_PATH:
		// wait until the scene finished the path
		if (m_Path->m_Ready) FollowPath();
		break;


		//////////////////////////////////////////////////////////////////////////
	case STATE_FOLLOWING_PATH:
		GetNextStep();
		already_moved = true;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_TALKING: {
		m_Sentence->Update(m_Dir);
		if (m_Sentence->m_CurrentSprite) m_TempSprite2 = m_Sentence->m_CurrentSprite;

		bool TimeIsUp = (m_Sentence->m_Sound && m_Sentence->m_SoundStarted && (!m_Sentence->m_Sound->IsPlaying() && !m_Sentence->m_Sound->IsPaused())) || (!m_Sentence->m_Sound && m_Sentence->m_Duration <= Game->m_Timer - m_Sentence->m_StartTime);
		if (m_TempSprite2 == NULL || m_TempSprite2->m_Finished || (/*m_TempSprite2->m_Looping &&*/ TimeIsUp)) {
			if (TimeIsUp) {
				m_Sentence->Finish();
				m_TempSprite2 = NULL;
				m_State = m_NextState;
				m_NextState = STATE_READY;
			} else {
				m_TempSprite2 = GetTalkStance(m_Sentence->GetNextStance());
				if (m_TempSprite2) {
					m_TempSprite2->Reset();
					m_CurrentSprite = m_TempSprite2;
					((CAdGame *)Game)->AddSentence(m_Sentence);
				}
			}
		} else {
			m_CurrentSprite = m_TempSprite2;
			((CAdGame *)Game)->AddSentence(m_Sentence);
		}
	}
	break;

	//////////////////////////////////////////////////////////////////////////
	case STATE_READY:
		if (!m_AnimSprite && !m_AnimSprite2) {
			if (m_Sprite) m_CurrentSprite = m_Sprite;
			else {
				if (m_StandSprite) {
					m_CurrentSprite = m_StandSprite->GetSprite(m_Dir);
				} else {
					CAdSpriteSet *Anim = GetAnimByName(m_IdleAnimName);
					if (Anim) m_CurrentSprite = Anim->GetSprite(m_Dir);
				}
			}
		}
		break;
	}


	if (m_CurrentSprite && !already_moved) {
		m_CurrentSprite->GetCurrentFrame(m_Zoomable ? ((CAdGame *)Game)->m_Scene->GetZoomAt(m_PosX, m_PosY) : 100, m_Zoomable ? ((CAdGame *)Game)->m_Scene->GetZoomAt(m_PosX, m_PosY) : 100);
		if (m_CurrentSprite->m_Changed) {
			m_PosX += m_CurrentSprite->m_MoveX;
			m_PosY += m_CurrentSprite->m_MoveY;
			AfterMove();
		}
	}

	//Game->QuickMessageForm("%s", m_CurrentSprite->m_Filename);

	UpdateBlockRegion();
	m_Ready = (m_State == STATE_READY);

	UpdatePartEmitter();
	UpdateSpriteAttachments();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdActor::FollowPath() {
	// skip current position
	m_Path->GetFirst();
	while (m_Path->GetCurrent() != NULL) {
		if (m_Path->GetCurrent()->x != m_PosX || m_Path->GetCurrent()->y != m_PosY) break;
		m_Path->GetNext();
	}

	// are there points to follow?
	if (m_Path->GetCurrent() != NULL) {
		m_State = STATE_FOLLOWING_PATH;;
		InitLine(CBPoint(m_PosX, m_PosY), *m_Path->GetCurrent());
	} else {
		if (m_AfterWalkDir != DI_NONE) TurnTo(m_AfterWalkDir);
		else m_State = STATE_READY;
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdActor::GetNextStep() {
	if (m_WalkSprite) {
		m_CurrentSprite = m_WalkSprite->GetSprite(m_Dir);
	} else {
		CAdSpriteSet *Anim = GetAnimByName(m_WalkAnimName);
		if (Anim) m_CurrentSprite = Anim->GetSprite(m_Dir);
	}

	if (!m_CurrentSprite) return;

	m_CurrentSprite->GetCurrentFrame(m_Zoomable ? ((CAdGame *)Game)->m_Scene->GetZoomAt(m_PosX, m_PosY) : 100, m_Zoomable ? ((CAdGame *)Game)->m_Scene->GetZoomAt(m_PosX, m_PosY) : 100);
	if (!m_CurrentSprite->m_Changed) return;


	int MaxStepX, MaxStepY;
	MaxStepX = abs(m_CurrentSprite->m_MoveX);
	MaxStepY = abs(m_CurrentSprite->m_MoveY);

	MaxStepX = std::max(MaxStepX, MaxStepY);
	MaxStepX = std::max(MaxStepX, 1);

	while (m_PFCount > 0 && MaxStepX >= 0) {
		m_PFX += m_PFStepX;
		m_PFY += m_PFStepY;

		m_PFCount--;
		MaxStepX--;
	}

	if (((CAdGame *)Game)->m_Scene->IsBlockedAt(m_PFX, m_PFY, true, this)) {
		if (m_PFCount == 0) {
			m_State = m_NextState;
			m_NextState = STATE_READY;
			return;
		}
		GoTo(m_TargetPoint->x, m_TargetPoint->y);
		return;
	}


	m_PosX = (int)m_PFX;
	m_PosY = (int)m_PFY;

	AfterMove();


	if (m_PFCount == 0) {
		if (m_Path->GetNext() == NULL) {
			m_PosX = m_TargetPoint->x;
			m_PosY = m_TargetPoint->y;

			m_Path->Reset();
			if (m_AfterWalkDir != DI_NONE) TurnTo(m_AfterWalkDir);
			else {
				m_State = m_NextState;
				m_NextState = STATE_READY;
			}
		} else InitLine(CBPoint(m_PosX, m_PosY), *m_Path->GetCurrent());
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdActor::InitLine(CBPoint StartPt, CBPoint EndPt) {
	m_PFCount = std::max((abs(EndPt.x - StartPt.x)) , (abs(EndPt.y - StartPt.y)));

	m_PFStepX = (double)(EndPt.x - StartPt.x) / m_PFCount;
	m_PFStepY = (double)(EndPt.y - StartPt.y) / m_PFCount;

	m_PFX = StartPt.x;
	m_PFY = StartPt.y;

	int angle = (int)(atan2((double)(EndPt.y - StartPt.y), (double)(EndPt.x - StartPt.x)) * (180 / 3.14));

	m_NextState = STATE_FOLLOWING_PATH;

	TurnTo(AngleToDirection(angle));
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// GoTo / GoToAsync
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GoTo") == 0 || strcmp(Name, "GoToAsync") == 0) {
		Stack->CorrectParams(2);
		int X = Stack->Pop()->GetInt();
		int Y = Stack->Pop()->GetInt();
		GoTo(X, Y);
		if (strcmp(Name, "GoToAsync") != 0) Script->WaitForExclusive(this);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoToObject / GoToObjectAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GoToObject") == 0 || strcmp(Name, "GoToObjectAsync") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();
		if (!Val->IsNative()) {
			Script->RuntimeError("actor.%s method accepts an entity refrence only", Name);
			Stack->PushNULL();
			return S_OK;
		}
		CAdObject *Obj = (CAdObject *)Val->GetNative();
		if (!Obj || Obj->m_Type != OBJECT_ENTITY) {
			Script->RuntimeError("actor.%s method accepts an entity refrence only", Name);
			Stack->PushNULL();
			return S_OK;
		}
		CAdEntity *Ent = (CAdEntity *)Obj;
		if (Ent->m_WalkToX == 0 && Ent->m_WalkToY == 0) GoTo(Ent->m_PosX, Ent->m_PosY);
		else GoTo(Ent->m_WalkToX, Ent->m_WalkToY, Ent->m_WalkToDir);
		if (strcmp(Name, "GoToObjectAsync") != 0) Script->WaitForExclusive(this);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnTo / TurnToAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TurnTo") == 0 || strcmp(Name, "TurnToAsync") == 0) {
		Stack->CorrectParams(1);
		int dir;
		CScValue *val = Stack->Pop();

		// turn to object?
		if (val->IsNative() && Game->ValidObject((CBObject *)val->GetNative())) {
			CBObject *obj = (CBObject *)val->GetNative();
			int angle = (int)(atan2((double)(obj->m_PosY - m_PosY), (double)(obj->m_PosX - m_PosX)) * (180 / 3.14));
			dir = (int)AngleToDirection(angle);
		}
		// otherwise turn to direction
		else dir = val->GetInt();

		if (dir >= 0 && dir < NUM_DIRECTIONS) {
			TurnTo((TDirection)dir);
			if (strcmp(Name, "TurnToAsync") != 0) Script->WaitForExclusive(this);
		}
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsWalking
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsWalking") == 0) {
		Stack->CorrectParams(0);
		Stack->PushBool(m_State == STATE_FOLLOWING_PATH);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MergeAnims
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MergeAnims") == 0) {
		Stack->CorrectParams(1);
		Stack->PushBool(SUCCEEDED(MergeAnims(Stack->Pop()->GetString())));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadAnim
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "UnloadAnim") == 0) {
		Stack->CorrectParams(1);
		char *AnimName = Stack->Pop()->GetString();

		bool Found = false;
		for (int i = 0; i < m_Anims.GetSize(); i++) {
			if (scumm_stricmp(m_Anims[i]->m_Name, AnimName) == 0) {
				// invalidate sprites in use
				if (m_Anims[i]->ContainsSprite(m_TempSprite2)) m_TempSprite2 = NULL;
				if (m_Anims[i]->ContainsSprite(m_CurrentSprite)) m_CurrentSprite = NULL;
				if (m_Anims[i]->ContainsSprite(m_AnimSprite2)) m_AnimSprite2 = NULL;

				delete m_Anims[i];
				m_Anims[i] = NULL;
				m_Anims.RemoveAt(i);
				i--;
				Found = true;
			}
		}
		Stack->PushBool(Found);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasAnim
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HasAnim") == 0) {
		Stack->CorrectParams(1);
		char *AnimName = Stack->Pop()->GetString();
		Stack->PushBool(GetAnimByName(AnimName) != NULL);
		return S_OK;
	}

	else return CAdTalkHolder::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdActor::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Direction
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Direction") == 0) {
		m_ScValue->SetInt(m_Dir);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("actor");
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// TalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TalkAnimName") == 0) {
		m_ScValue->SetString(m_TalkAnimName);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WalkAnimName") == 0) {
		m_ScValue->SetString(m_WalkAnimName);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IdleAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IdleAnimName") == 0) {
		m_ScValue->SetString(m_IdleAnimName);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnLeftAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TurnLeftAnimName") == 0) {
		m_ScValue->SetString(m_TurnLeftAnimName);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnRightAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TurnRightAnimName") == 0) {
		m_ScValue->SetString(m_TurnRightAnimName);
		return m_ScValue;
	}

	else return CAdTalkHolder::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Direction
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Direction") == 0) {
		int dir = Value->GetInt();
		if (dir >= 0 && dir < NUM_DIRECTIONS) m_Dir = (TDirection)dir;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TalkAnimName") == 0) {
		if (Value->IsNULL()) CBUtils::SetString(&m_TalkAnimName, "talk");
		else CBUtils::SetString(&m_TalkAnimName, Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WalkAnimName") == 0) {
		if (Value->IsNULL()) CBUtils::SetString(&m_WalkAnimName, "walk");
		else CBUtils::SetString(&m_WalkAnimName, Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IdleAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IdleAnimName") == 0) {
		if (Value->IsNULL()) CBUtils::SetString(&m_IdleAnimName, "idle");
		else CBUtils::SetString(&m_IdleAnimName, Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnLeftAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TurnLeftAnimName") == 0) {
		if (Value->IsNULL()) CBUtils::SetString(&m_TurnLeftAnimName, "turnleft");
		else CBUtils::SetString(&m_TurnLeftAnimName, Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnRightAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TurnRightAnimName") == 0) {
		if (Value->IsNULL()) CBUtils::SetString(&m_TurnRightAnimName, "turnright");
		else CBUtils::SetString(&m_TurnRightAnimName, Value->GetString());
		return S_OK;
	}

	else return CAdTalkHolder::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CAdActor::ScToString() {
	return "[actor object]";
}


//////////////////////////////////////////////////////////////////////////
CBSprite *CAdActor::GetTalkStance(char *Stance) {
	// forced stance?
	if (m_ForcedTalkAnimName && !m_ForcedTalkAnimUsed) {
		m_ForcedTalkAnimUsed = true;
		delete m_AnimSprite;
		m_AnimSprite = new CBSprite(Game, this);
		if (m_AnimSprite) {
			HRESULT res = m_AnimSprite->LoadFile(m_ForcedTalkAnimName);
			if (FAILED(res)) {
				Game->LOG(res, "CAdActor::GetTalkStance: error loading talk sprite (object:\"%s\" sprite:\"%s\")", m_Name, m_ForcedTalkAnimName);
				delete m_AnimSprite;
				m_AnimSprite = NULL;
			} else return m_AnimSprite;
		}
	}

	// old way
	if (m_TalkSprites.GetSize() > 0 || m_TalkSpritesEx.GetSize() > 0)
		return GetTalkStanceOld(Stance);

	// new way
	CBSprite *Ret = NULL;

	// do we have an animation with this name?
	CAdSpriteSet *Anim = GetAnimByName(Stance);
	if (Anim) Ret = Anim->GetSprite(m_Dir);

	// not - get a random talk
	if (!Ret) {
		CBArray<CAdSpriteSet *, CAdSpriteSet *> TalkAnims;
		for (int i = 0; i < m_Anims.GetSize(); i++) {
			if (scumm_stricmp(m_Anims[i]->m_Name, m_TalkAnimName) == 0)
				TalkAnims.Add(m_Anims[i]);
		}

		if (TalkAnims.GetSize() > 0) {
			int rnd = rand() % TalkAnims.GetSize();
			Ret = TalkAnims[rnd]->GetSprite(m_Dir);
		} else {
			if (m_StandSprite) Ret = m_StandSprite->GetSprite(m_Dir);
			else {
				Anim = GetAnimByName(m_IdleAnimName);
				if (Anim) Ret = Anim->GetSprite(m_Dir);
			}
		}
	}
	return Ret;
}

//////////////////////////////////////////////////////////////////////////
CBSprite *CAdActor::GetTalkStanceOld(char *Stance) {
	CBSprite *ret = NULL;

	if (Stance != NULL) {
		// search special stances
		for (int i = 0; i < m_TalkSpritesEx.GetSize(); i++) {
			if (scumm_stricmp(m_TalkSpritesEx[i]->m_Name, Stance) == 0) {
				ret = m_TalkSpritesEx[i]->GetSprite(m_Dir);
				break;
			}
		}
		if (ret == NULL) {
			// search generic stances
			for (int i = 0; i < m_TalkSprites.GetSize(); i++) {
				if (scumm_stricmp(m_TalkSprites[i]->m_Name, Stance) == 0) {
					ret = m_TalkSprites[i]->GetSprite(m_Dir);
					break;
				}
			}
		}
	}

	// not a valid stance? get a random one
	if (ret == NULL) {
		if (m_TalkSprites.GetSize() < 1) ret = m_StandSprite->GetSprite(m_Dir);
		else {
			// TODO: remember last
			int rnd = rand() % m_TalkSprites.GetSize();
			ret = m_TalkSprites[rnd]->GetSprite(m_Dir);
		}
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::Persist(CBPersistMgr *PersistMgr) {
	CAdTalkHolder::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER_INT(m_Dir));
	PersistMgr->Transfer(TMEMBER(m_Path));
	PersistMgr->Transfer(TMEMBER(m_PFCount));
	PersistMgr->Transfer(TMEMBER(m_PFStepX));
	PersistMgr->Transfer(TMEMBER(m_PFStepY));
	PersistMgr->Transfer(TMEMBER(m_PFX));
	PersistMgr->Transfer(TMEMBER(m_PFY));
	PersistMgr->Transfer(TMEMBER(m_StandSprite));
	m_TalkSprites.Persist(PersistMgr);
	m_TalkSpritesEx.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER_INT(m_TargetDir));
	PersistMgr->Transfer(TMEMBER_INT(m_AfterWalkDir));
	PersistMgr->Transfer(TMEMBER(m_TargetPoint));
	PersistMgr->Transfer(TMEMBER(m_TurnLeftSprite));
	PersistMgr->Transfer(TMEMBER(m_TurnRightSprite));
	PersistMgr->Transfer(TMEMBER(m_WalkSprite));

	PersistMgr->Transfer(TMEMBER(m_AnimSprite2));
	PersistMgr->Transfer(TMEMBER(m_TalkAnimName));
	PersistMgr->Transfer(TMEMBER(m_IdleAnimName));
	PersistMgr->Transfer(TMEMBER(m_WalkAnimName));
	PersistMgr->Transfer(TMEMBER(m_TurnLeftAnimName));
	PersistMgr->Transfer(TMEMBER(m_TurnRightAnimName));

	m_Anims.Persist(PersistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
TDirection CAdActor::AngleToDirection(int Angle) {
	TDirection ret = DI_DOWN;;

	if (Angle > -112 && Angle <= -67) ret = DI_UP;
	else if (Angle > -67  && Angle <= -22)  ret = DI_UPRIGHT;
	else if (Angle > -22  && Angle <= 22)   ret = DI_RIGHT;
	else if (Angle > 22   && Angle <= 67)   ret = DI_DOWNRIGHT;
	else if (Angle > 67   && Angle <= 112)  ret = DI_DOWN;
	else if (Angle > 112  && Angle <= 157)  ret = DI_DOWNLEFT;
	else if ((Angle > 157 && Angle <= 180) || (Angle >= -180 && Angle <= -157)) ret = DI_LEFT;
	else if (Angle > -157 && Angle <= -112) ret = DI_UPLEFT;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
int CAdActor::GetHeight() {
	// if no current sprite is set, set some
	if (m_CurrentSprite == NULL) {
		if (m_StandSprite) m_CurrentSprite = m_StandSprite->GetSprite(m_Dir);
		else {
			CAdSpriteSet *Anim = GetAnimByName(m_IdleAnimName);
			if (Anim) m_CurrentSprite = Anim->GetSprite(m_Dir);
		}
	}
	// and get height
	return CAdTalkHolder::GetHeight();
}


//////////////////////////////////////////////////////////////////////////
CAdSpriteSet *CAdActor::GetAnimByName(char *AnimName) {
	if (!AnimName) return NULL;

	for (int i = 0; i < m_Anims.GetSize(); i++) {
		if (scumm_stricmp(m_Anims[i]->m_Name, AnimName) == 0) return m_Anims[i];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::MergeAnims(char *AnimsFilename) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ANIMATION)
	TOKEN_TABLE_END


	byte *FileBuffer = Game->m_FileManager->ReadWholeFile(AnimsFilename);
	if (FileBuffer == NULL) {
		Game->LOG(0, "CAdActor::MergeAnims failed for file '%s'", AnimsFilename);
		return E_FAIL;
	}

	byte *Buffer = FileBuffer;
	byte *params;
	int cmd;
	CBParser parser(Game);

	HRESULT Ret = S_OK;

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_ANIMATION: {
			CAdSpriteSet *Anim = new CAdSpriteSet(Game, this);
			if (!Anim || FAILED(Anim->LoadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				Ret = E_FAIL;
			} else m_Anims.Add(Anim);
		}
		break;
		}
	}
	delete [] FileBuffer;
	return Ret;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::PlayAnim(char *Filename) {
	// if we have an anim with this name, use it
	CAdSpriteSet *Anim = GetAnimByName(Filename);
	if (Anim) {
		m_AnimSprite2 = Anim->GetSprite(m_Dir);
		if (m_AnimSprite2) {
			m_AnimSprite2->Reset();
			m_State = STATE_PLAYING_ANIM_SET;
			return S_OK;
		}
	}
	// otherwise call the standard handler
	return CAdTalkHolder::PlayAnim(Filename);
}

} // end of namespace WinterMute
