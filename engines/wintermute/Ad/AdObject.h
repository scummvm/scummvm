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

#ifndef WINTERMUTE_ADOBJECT_H
#define WINTERMUTE_ADOBJECT_H

#include "engines/wintermute/Ad/AdTypes.h"
#include "engines/wintermute/Base/PartEmitter.h"

namespace WinterMute {

class CAdWaypointGroup;
class CAdRegion;
class CAdSentence;
class CBFont;
class CBRegion;
class CAdInventory;

#define MAX_NUM_REGIONS 10

class CAdObject : public CBObject {
public:
	CPartEmitter *_partEmitter;
	virtual CPartEmitter *CreateParticleEmitter(bool FollowParent = false, int OffsetX = 0, int OffsetY = 0);
	virtual HRESULT UpdatePartEmitter();
	bool _partFollowParent;
	int _partOffsetX;
	int _partOffsetY;

	HRESULT InvalidateCurrRegions();
	bool _subtitlesModRelative;
	bool _subtitlesModXCenter;
	int _subtitlesModX;
	int _subtitlesModY;
	int _subtitlesWidth;
	CAdRegion *_stickRegion;
	bool _sceneIndependent;
	bool _ignoreItems;
	HRESULT UpdateBlockRegion();
	bool _forcedTalkAnimUsed;
	char *_forcedTalkAnimName;
	virtual bool GetExtendedFlag(const char *FlagName);
	virtual HRESULT ResetSoundPan();
	virtual HRESULT UpdateSounds();
	HRESULT Reset();
	DECLARE_PERSISTENT(CAdObject, CBObject)
	virtual void Talk(const char *Text, const char *Sound = NULL, uint32 Duration = 0, const char *Stances = NULL, TTextAlign Align = TAL_CENTER);
	virtual int GetHeight();
	CAdSentence *_sentence;
	HRESULT SetFont(const char *Filename);
	virtual HRESULT Update();
	virtual HRESULT Display();
	bool _drawn;
	bool _active;
	virtual HRESULT PlayAnim(const char *Filename);
	CBSprite *_animSprite;
	CBSprite *_currentSprite;
	TObjectState _state;
	TObjectState _nextState;
	TObjectType _type;
	CAdObject(CBGame *inGame);
	virtual ~CAdObject();
	CBFont *_font;
	CBSprite *_tempSprite2;
	CBRegion *_blockRegion;
	CAdWaypointGroup *_wptGroup;
	CBRegion *_currentBlockRegion;
	CAdWaypointGroup *_currentWptGroup;
	CAdInventory *GetInventory();

	virtual HRESULT saveAsText(CBDynBuffer *Buffer, int Indent);

	virtual HRESULT AfterMove();
	CAdRegion *_currentRegions[MAX_NUM_REGIONS];

	// scripting interface
	virtual CScValue *scGetProperty(const char *Name);
	virtual HRESULT scSetProperty(const char *Name, CScValue *Value);
	virtual HRESULT scCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	virtual const char *scToString();

	CBArray<CAdObject *, CAdObject *> _attachmentsPre;
	CBArray<CAdObject *, CAdObject *> _attachmentsPost;

	HRESULT UpdateSpriteAttachments();
	HRESULT DisplaySpriteAttachments(bool PreDisplay);
	CAdObject *_registerAlias;
private:
	HRESULT DisplaySpriteAttachment(CAdObject *Attachment);
	CAdInventory *_inventory;

protected:
	HRESULT GetScale(float *ScaleX, float *ScaleY);
};

} // end of namespace WinterMute

#endif
