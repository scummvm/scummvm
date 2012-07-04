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
	virtual CPartEmitter *createParticleEmitter(bool followParent = false, int offsetX = 0, int offsetY = 0);
	virtual HRESULT updatePartEmitter();
	bool _partFollowParent;
	int _partOffsetX;
	int _partOffsetY;

	HRESULT invalidateCurrRegions();
	bool _subtitlesModRelative;
	bool _subtitlesModXCenter;
	int _subtitlesModX;
	int _subtitlesModY;
	int _subtitlesWidth;
	CAdRegion *_stickRegion;
	bool _sceneIndependent;
	bool _ignoreItems;
	HRESULT updateBlockRegion();
	bool _forcedTalkAnimUsed;
	char *_forcedTalkAnimName;
	virtual bool getExtendedFlag(const char *flagName);
	virtual HRESULT resetSoundPan();
	virtual HRESULT updateSounds();
	HRESULT reset();
	DECLARE_PERSISTENT(CAdObject, CBObject)
	virtual void talk(const char *text, const char *sound = NULL, uint32 duration = 0, const char *stances = NULL, TTextAlign align = TAL_CENTER);
	virtual int getHeight();
	CAdSentence *_sentence;
	HRESULT SetFont(const char *filename);
	virtual HRESULT update();
	virtual HRESULT display();
	bool _drawn;
	bool _active;
	virtual HRESULT playAnim(const char *filename);
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
	CAdInventory *getInventory();

	virtual HRESULT saveAsText(CBDynBuffer *buffer, int indent);

	virtual HRESULT afterMove();
	CAdRegion *_currentRegions[MAX_NUM_REGIONS];

	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual HRESULT scSetProperty(const char *name, CScValue *value);
	virtual HRESULT scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();

	CBArray<CAdObject *, CAdObject *> _attachmentsPre;
	CBArray<CAdObject *, CAdObject *> _attachmentsPost;

	HRESULT updateSpriteAttachments();
	HRESULT displaySpriteAttachments(bool preDisplay);
	CAdObject *_registerAlias;
private:
	HRESULT displaySpriteAttachment(CAdObject *attachment);
	CAdInventory *_inventory;

protected:
	HRESULT getScale(float *scaleX, float *scaleY);
};

} // end of namespace WinterMute

#endif
