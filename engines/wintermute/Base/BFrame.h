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

#ifndef WINTERMUTE_BFRAME_H
#define WINTERMUTE_BFRAME_H

#include "engines/wintermute/Base/BScriptable.h"
#include "engines/wintermute/coll_templ.h"

namespace WinterMute {
class CBSound;
class CBSubFrame;
class CBObject;
class CScScript;
class CScStack;
class CBFrame: public CBScriptable {
public:
	bool _killSound;
	bool _keyframe;
	HRESULT OneTimeDisplay(CBObject *Owner, bool Muted = false);
	DECLARE_PERSISTENT(CBFrame, CBScriptable)
	CBSound *_sound;
	bool _editorExpanded;
	bool GetBoundingRect(LPRECT Rect, int X, int Y, float ScaleX = 100, float ScaleY = 100);
	HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent);
	int _moveY;
	int _moveX;
	uint32 _delay;
	CBArray<CBSubFrame *, CBSubFrame *> _subframes;
	HRESULT Draw(int X, int Y, CBObject *Register = NULL, float ZoomX = 100, float ZoomY = 100, bool Precise = true, uint32 Alpha = 0xFFFFFFFF, bool AllFrames = false, float Rotate = 0.0f, TSpriteBlendMode BlendMode = BLEND_NORMAL);
	HRESULT LoadBuffer(byte  *Buffer, int LifeTime, bool KeepLoaded);

	CBFrame(CBGame *inGame);
	virtual ~CBFrame();

	CBArray<const char *, const char *> _applyEvent;

	// scripting interface
	virtual CScValue *ScGetProperty(const char *Name);
	virtual HRESULT ScSetProperty(const char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	virtual const char *ScToString();

};

} // end of namespace WinterMute

#endif
