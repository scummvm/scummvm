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

#ifndef WINTERMUTE_BOBJECT_H
#define WINTERMUTE_BOBJECT_H


#include "engines/wintermute/Base/BScriptHolder.h"
#include "engines/wintermute/persistent.h"
#include "common/events.h"

namespace WinterMute {

class CBSprite;
class CBSound;
class CBSurface;
class CBScriptHolder;
class CScValue;
class CScStack;
class CScScript;
class CBObject : public CBScriptHolder {
public:
	TSpriteBlendMode _blendMode;
	virtual HRESULT afterMove();
	float _relativeRotate;
	bool _rotateValid;
	float _rotate;
	void setSoundEvent(const char *eventName);
	bool _rotatable;
	uint32 _alphaColor;
	float _scale;
	float _scaleX;
	float _scaleY;
	float _relativeScale;
	virtual bool isReady();
	virtual bool getExtendedFlag(const char *flagName);
	virtual HRESULT resetSoundPan();
	virtual HRESULT updateSounds();
	HRESULT updateOneSound(CBSound *sound);
	bool _autoSoundPanning;
	uint32 _sFXStart;
	int _sFXVolume;
	HRESULT setSFXTime(uint32 time);
	HRESULT setSFXVolume(int volume);
	HRESULT resumeSFX();
	HRESULT pauseSFX();
	HRESULT stopSFX(bool deleteSound = true);
	HRESULT playSFX(const char *filename, bool looping = false, bool playNow = true, const char *eventName = NULL, uint32 loopStart = 0);
	CBSound *_sFX;

	TSFXType _sFXType;
	float _sFXParam1;
	float _sFXParam2;
	float _sFXParam3;
	float _sFXParam4;

	virtual bool handleMouseWheel(int delta);
	virtual HRESULT handleMouse(TMouseEvent event, TMouseButton button);
	virtual bool handleKeypress(Common::Event *event, bool printable = false);
	virtual int getHeight();
	HRESULT setCursor(const char *filename);
	HRESULT setActiveCursor(const char *filename);
	HRESULT cleanup();
	char *getCaption(int caseVal = 1);
	void setCaption(const char *caption, int caseVal = 1);
	bool _editorSelected;
	bool _editorAlwaysRegister;
	bool _editorOnly;
	bool _is3D;
	DECLARE_PERSISTENT(CBObject, CBScriptHolder)
	virtual HRESULT showCursor();
	CBSprite *_cursor;
	bool _sharedCursors;
	CBSprite *_activeCursor;
	virtual HRESULT saveAsText(CBDynBuffer *buffer, int indent);
	virtual HRESULT listen(CBScriptHolder *param1, uint32 param2);
	bool _ready;
	bool _registrable;
	bool _zoomable;
	bool _shadowable;
	RECT _rect;
	bool _rectSet;
	int _iD;
	bool _movable;
	CBObject(CBGame *inGame);
	virtual ~CBObject();
	char *_caption[7];
	char *_soundEvent;
	int _posY;
	int _posX;
	bool _saveState;

	// base
	virtual HRESULT update()  {
		return E_FAIL;
	};
	virtual HRESULT display() {
		return E_FAIL;
	};
	virtual HRESULT invalidateDeviceObjects()  {
		return S_OK;
	};
	virtual HRESULT restoreDeviceObjects()     {
		return S_OK;
	};
	bool _nonIntMouseEvents;


public:
	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual HRESULT scSetProperty(const char *name, CScValue *value);
	virtual HRESULT scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();
};

} // end of namespace WinterMute

#endif
