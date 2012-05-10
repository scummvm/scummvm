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


#include "BScriptHolder.h"
#include "persistent.h"

union SDL_Event;

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
	virtual HRESULT AfterMove();
	float _relativeRotate;
	bool _rotateValid;
	float _rotate;
	void SetSoundEvent(char *EventName);
	bool _rotatable;
	uint32 _alphaColor;
	float _scale;
	float _scaleX;
	float _scaleY;
	float _relativeScale;
	virtual bool IsReady();
	virtual bool GetExtendedFlag(char *FlagName);
	virtual HRESULT ResetSoundPan();
	virtual HRESULT UpdateSounds();
	HRESULT UpdateOneSound(CBSound *Sound);
	bool _autoSoundPanning;
	uint32 _sFXStart;
	int _sFXVolume;
	HRESULT SetSFXTime(uint32 Time);
	HRESULT SetSFXVolume(int Volume);
	HRESULT ResumeSFX();
	HRESULT PauseSFX();
	HRESULT StopSFX(bool DeleteSound = true);
	HRESULT PlaySFX(char *Filename, bool Looping = false, bool PlayNow = true, char *EventName = NULL, uint32 LoopStart = 0);
	CBSound *_sFX;

	TSFXType _sFXType;
	float _sFXParam1;
	float _sFXParam2;
	float _sFXParam3;
	float _sFXParam4;

	virtual bool HandleMouseWheel(int Delta);
	virtual HRESULT HandleMouse(TMouseEvent Event, TMouseButton Button);
	virtual bool HandleKeypress(SDL_Event *event);
	virtual int GetHeight();
	HRESULT SetCursor(char *Filename);
	HRESULT SetActiveCursor(char *Filename);
	HRESULT Cleanup();
	char *GetCaption(int Case = 1);
	void SetCaption(char *Caption, int Case = 1);
	bool _editorSelected;
	bool _editorAlwaysRegister;
	bool _editorOnly;
	bool _is3D;
	DECLARE_PERSISTENT(CBObject, CBScriptHolder)
	virtual HRESULT ShowCursor();
	CBSprite *_cursor;
	bool _sharedCursors;
	CBSprite *_activeCursor;
	virtual HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent);
	virtual HRESULT Listen(CBScriptHolder *param1, uint32 param2);
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
	virtual HRESULT Update()  {
		return E_FAIL;
	};
	virtual HRESULT Display() {
		return E_FAIL;
	};
	virtual HRESULT InvalidateDeviceObjects()  {
		return S_OK;
	};
	virtual HRESULT RestoreDeviceObjects()     {
		return S_OK;
	};
	bool _nonIntMouseEvents;


public:
	// scripting interface
	virtual CScValue *ScGetProperty(char *Name);
	virtual HRESULT ScSetProperty(char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);
	virtual char *ScToString();
};

} // end of namespace WinterMute

#endif
