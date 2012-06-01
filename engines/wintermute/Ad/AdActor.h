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

#ifndef WINTERMUTE_ADACTOR_H
#define WINTERMUTE_ADACTOR_H


#include "engines/wintermute/dctypes.h"    // Added by ClassView
#include "engines/wintermute/Ad/AdTypes.h"    // Added by ClassView
#include "engines/wintermute/Ad/AdTalkHolder.h"
#include "engines/wintermute/BPoint.h" // Added by ClassView
#include "engines/wintermute/persistent.h"
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
	CBSprite *GetTalkStance(const char *Stance);
	virtual void GoTo(int X, int Y, TDirection AfterWalkDir = DI_NONE);
	CBPoint *_targetPoint;
	virtual HRESULT Update();
	virtual HRESULT Display();
	TDirection _targetDir;
	TDirection _afterWalkDir;
	virtual void TurnTo(TDirection dir);
	CAdPath *_path;
	CAdSpriteSet *_walkSprite;
	CAdSpriteSet *_standSprite;
	CAdSpriteSet *_turnLeftSprite;
	CAdSpriteSet *_turnRightSprite;
	CBArray<CAdSpriteSet *, CAdSpriteSet *> _talkSprites;
	CBArray<CAdSpriteSet *, CAdSpriteSet *> _talkSpritesEx;
	TDirection _dir;
	CAdActor(CBGame *inGame/*=NULL*/);
	virtual ~CAdActor();
	HRESULT LoadFile(const char *Filename);
	HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);

	// new anim system
	char *_talkAnimName;
	char *_idleAnimName;
	char *_walkAnimName;
	char *_turnLeftAnimName;
	char *_turnRightAnimName;
	CBArray<CAdSpriteSet *, CAdSpriteSet *> _anims;
	virtual HRESULT PlayAnim(const char *Filename);
	CAdSpriteSet *GetAnimByName(const char *AnimName);

	// scripting interface
	virtual CScValue *ScGetProperty(const char *Name);
	virtual HRESULT ScSetProperty(const char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	virtual const char *ScToString();

private:
	HRESULT SetDefaultAnimNames();
	CBSprite *GetTalkStanceOld(const char *Stance);
	HRESULT MergeAnims(const char *AnimsFilename);
	CBSprite *_animSprite2;

	void InitLine(CBPoint StartPt, CBPoint EndPt);
	void GetNextStep();
	void FollowPath();
	double _pFStepX;
	double _pFStepY;
	double _pFX;
	double _pFY;
	int _pFCount;
};

} // end of namespace WinterMute

#endif // WINTERMUTE_ADACTOR_H
