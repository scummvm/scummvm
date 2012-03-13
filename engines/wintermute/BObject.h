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


//#include "SDL.h"
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
	TSpriteBlendMode m_BlendMode;
	virtual HRESULT AfterMove();
	float m_RelativeRotate;
	bool m_RotateValid;
	float m_Rotate;
	void SetSoundEvent(char *EventName);
	bool m_Rotatable;
	uint32 m_AlphaColor;
	float m_Scale;
	float m_ScaleX;
	float m_ScaleY;
	float m_RelativeScale;
	virtual bool IsReady();
	virtual bool GetExtendedFlag(char *FlagName);
	virtual HRESULT ResetSoundPan();
	virtual HRESULT UpdateSounds();
	HRESULT UpdateOneSound(CBSound *Sound);
	bool m_AutoSoundPanning;
	uint32 m_SFXStart;
	int m_SFXVolume;
	HRESULT SetSFXTime(uint32 Time);
	HRESULT SetSFXVolume(int Volume);
	HRESULT ResumeSFX();
	HRESULT PauseSFX();
	HRESULT StopSFX(bool DeleteSound = true);
	HRESULT PlaySFX(char *Filename, bool Looping = false, bool PlayNow = true, char *EventName = NULL, uint32 LoopStart = 0);
	CBSound *m_SFX;

	TSFXType m_SFXType;
	float m_SFXParam1;
	float m_SFXParam2;
	float m_SFXParam3;
	float m_SFXParam4;

	virtual bool HandleMouseWheel(int Delta);
	virtual HRESULT HandleMouse(TMouseEvent Event, TMouseButton Button);
	virtual bool HandleKeypress(SDL_Event *event);
	virtual int GetHeight();
	HRESULT SetCursor(char *Filename);
	HRESULT SetActiveCursor(char *Filename);
	HRESULT Cleanup();
	char *GetCaption(int Case = 1);
	void SetCaption(char *Caption, int Case = 1);
	bool m_EditorSelected;
	bool m_EditorAlwaysRegister;
	bool m_EditorOnly;
	bool m_Is3D;
	DECLARE_PERSISTENT(CBObject, CBScriptHolder)
	virtual HRESULT ShowCursor();
	CBSprite *m_Cursor;
	bool m_SharedCursors;
	CBSprite *m_ActiveCursor;
	virtual HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent);
	virtual HRESULT Listen(CBScriptHolder *param1, uint32 param2);
	bool m_Ready;
	bool m_Registrable;
	bool m_Zoomable;
	bool m_Shadowable;
	RECT m_Rect;
	bool m_RectSet;
	int m_ID;
	bool m_Movable;
	CBObject(CBGame *inGame);
	virtual ~CBObject();
	char *m_Caption[7];
	char *m_SoundEvent;
	int m_PosY;
	int m_PosX;
	bool m_SaveState;

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
	bool m_NonIntMouseEvents;


public:
	// scripting interface
	virtual CScValue *ScGetProperty(char *Name);
	virtual HRESULT ScSetProperty(char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);
	virtual char *ScToString();
};

} // end of namespace WinterMute

#endif
