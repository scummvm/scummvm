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
#include "engines/wintermute/Base/BPoint.h" // Added by ClassView
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
	TDirection angleToDirection(int angle);
	DECLARE_PERSISTENT(CAdActor, CAdTalkHolder)
	virtual int getHeight();
	CBSprite *getTalkStance(const char *stance);
	virtual void goTo(int x, int y, TDirection afterWalkDir = DI_NONE);
	CBPoint *_targetPoint;
	virtual HRESULT update();
	virtual HRESULT display();
	TDirection _targetDir;
	TDirection _afterWalkDir;
	virtual void turnTo(TDirection dir);
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
	HRESULT loadFile(const char *filename);
	HRESULT loadBuffer(byte *buffer, bool complete = true);

	// new anim system
	Common::String _talkAnimName;
	Common::String _idleAnimName;
	Common::String _walkAnimName;
	Common::String _turnLeftAnimName;
	Common::String _turnRightAnimName;
	CBArray<CAdSpriteSet *, CAdSpriteSet *> _anims;
	virtual HRESULT playAnim(const char *filename);
	CAdSpriteSet *getAnimByName(const Common::String &animName);

	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual HRESULT scSetProperty(const char *name, CScValue *value);
	virtual HRESULT scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();

private:
	HRESULT setDefaultAnimNames();
	CBSprite *getTalkStanceOld(const char *stance);
	HRESULT mergeAnims(const char *animsFilename);
	CBSprite *_animSprite2;

	void initLine(CBPoint startPt, CBPoint endPt);
	void getNextStep();
	void followPath();
	double _pFStepX;
	double _pFStepY;
	double _pFX;
	double _pFY;
	int _pFCount;
};

} // end of namespace WinterMute

#endif // WINTERMUTE_ADACTOR_H
