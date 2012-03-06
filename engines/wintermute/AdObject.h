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

#include "AdTypes.h"
#include "PartEmitter.h"

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
	CPartEmitter *m_PartEmitter;
	virtual CPartEmitter *CreateParticleEmitter(bool FollowParent = false, int OffsetX = 0, int OffsetY = 0);
	virtual HRESULT UpdatePartEmitter();
	bool m_PartFollowParent;
	int m_PartOffsetX;
	int m_PartOffsetY;

	HRESULT InvalidateCurrRegions();
	bool m_SubtitlesModRelative;
	bool m_SubtitlesModXCenter;
	int m_SubtitlesModX;
	int m_SubtitlesModY;
	int m_SubtitlesWidth;
	CAdRegion *m_StickRegion;
	bool m_SceneIndependent;
	bool m_IgnoreItems;
	HRESULT UpdateBlockRegion();
	bool m_ForcedTalkAnimUsed;
	char *m_ForcedTalkAnimName;
	virtual bool GetExtendedFlag(char *FlagName);
	virtual HRESULT ResetSoundPan();
	virtual HRESULT UpdateSounds();
	HRESULT Reset();
	DECLARE_PERSISTENT(CAdObject, CBObject)
	virtual void Talk(char *Text, char *Sound = NULL, uint32 Duration = 0, char *Stances = NULL, TTextAlign Align = TAL_CENTER);
	virtual int GetHeight();
	CAdSentence *m_Sentence;
	HRESULT SetFont(char *Filename);
	virtual HRESULT Update();
	virtual HRESULT Display();
	bool m_Drawn;
	bool m_Active;
	virtual HRESULT PlayAnim(char *Filename);
	CBSprite *m_AnimSprite;
	CBSprite *m_CurrentSprite;
	TObjectState m_State;
	TObjectState m_NextState;
	TObjectType m_Type;
	CAdObject(CBGame *inGame);
	virtual ~CAdObject();
	CBFont *m_Font;
	CBSprite *m_TempSprite2;
	CBRegion *m_BlockRegion;
	CAdWaypointGroup *m_WptGroup;
	CBRegion *m_CurrentBlockRegion;
	CAdWaypointGroup *m_CurrentWptGroup;
	CAdInventory *GetInventory();

	virtual HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent);

	virtual HRESULT AfterMove();
	CAdRegion *m_CurrentRegions[MAX_NUM_REGIONS];

	// scripting interface
	virtual CScValue *ScGetProperty(char *Name);
	virtual HRESULT ScSetProperty(char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);
	virtual char *ScToString();

	CBArray<CAdObject *, CAdObject *> m_AttachmentsPre;
	CBArray<CAdObject *, CAdObject *> m_AttachmentsPost;

	HRESULT UpdateSpriteAttachments();
	HRESULT DisplaySpriteAttachments(bool PreDisplay);
	CAdObject *m_RegisterAlias;
private:
	HRESULT DisplaySpriteAttachment(CAdObject *Attachment);
	CAdInventory *m_Inventory;

protected:
	HRESULT GetScale(float *ScaleX, float *ScaleY);
};

} // end of namespace WinterMute

#endif
