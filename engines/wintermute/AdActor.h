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

#ifndef __WmeAdActor_H__
#define __WmeAdActor_H__


#include "dctypes.h"    // Added by ClassView
#include "AdTypes.h"    // Added by ClassView
#include "AdTalkHolder.h"
#include "BPoint.h" // Added by ClassView
#include "persistent.h"
#include "common/str.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace WinterMute {
class CAdSpriteSet;
class CAdPath;
class CAdActor : public CAdTalkHolder {
public:
	TDirection AngleToDirection(int Angle);
	DECLARE_PERSISTENT(CAdActor, CAdTalkHolder)
	virtual int GetHeight();
	CBSprite *GetTalkStance(char *Stance);
	virtual void GoTo(int X, int Y, TDirection AfterWalkDir = DI_NONE);
	CBPoint *m_TargetPoint;
	virtual HRESULT Update();
	virtual HRESULT Display();
	TDirection m_TargetDir;
	TDirection m_AfterWalkDir;
	virtual void TurnTo(TDirection dir);
	CAdPath *m_Path;
	CAdSpriteSet *m_WalkSprite;
	CAdSpriteSet *m_StandSprite;
	CAdSpriteSet *m_TurnLeftSprite;
	CAdSpriteSet *m_TurnRightSprite;
	CBArray<CAdSpriteSet *, CAdSpriteSet *> m_TalkSprites;
	CBArray<CAdSpriteSet *, CAdSpriteSet *> m_TalkSpritesEx;
	TDirection m_Dir;
	CAdActor(CBGame *inGame/*=NULL*/);
	virtual ~CAdActor();
	HRESULT LoadFile(char *Filename);
	HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);

	// new anim system
	char *m_TalkAnimName;
	char *m_IdleAnimName;
	char *m_WalkAnimName;
	char *m_TurnLeftAnimName;
	char *m_TurnRightAnimName;
	CBArray<CAdSpriteSet *, CAdSpriteSet *> m_Anims;
	virtual HRESULT PlayAnim(char *Filename);
	CAdSpriteSet *GetAnimByName(char *AnimName);

	// scripting interface
	virtual CScValue *ScGetProperty(char *Name);
	virtual HRESULT ScSetProperty(char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);
	virtual char *ScToString();

private:
	HRESULT SetDefaultAnimNames();
	CBSprite *GetTalkStanceOld(char *Stance);
	HRESULT MergeAnims(char *AnimsFilename);
	CBSprite *m_AnimSprite2;

	void InitLine(CBPoint StartPt, CBPoint EndPt);
	void GetNextStep();
	void FollowPath();
	double m_PFStepX;
	double m_PFStepY;
	double m_PFX;
	double m_PFY;
	int m_PFCount;
};

} // end of namespace WinterMute

#endif // __WmeAdActor_H__
