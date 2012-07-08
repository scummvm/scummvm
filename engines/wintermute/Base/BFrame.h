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
	HRESULT oneTimeDisplay(CBObject *owner, bool muted = false);
	DECLARE_PERSISTENT(CBFrame, CBScriptable)
	CBSound *_sound;
	bool _editorExpanded;
	bool getBoundingRect(LPRECT rect, int x, int y, float scaleX = 100, float scaleY = 100);
	HRESULT saveAsText(CBDynBuffer *buffer, int indent);
	int _moveY;
	int _moveX;
	uint32 _delay;
	CBArray<CBSubFrame *, CBSubFrame *> _subframes;
	HRESULT draw(int x, int y, CBObject *registerOwner = NULL, float zoomX = 100, float zoomY = 100, bool precise = true, uint32 Alpha = 0xFFFFFFFF, bool allFrames = false, float rotate = 0.0f, TSpriteBlendMode blendMode = BLEND_NORMAL);
	HRESULT loadBuffer(byte *buffer, int lifeTime, bool keepLoaded);

	CBFrame(CBGame *inGame);
	virtual ~CBFrame();

	CBArray<const char *, const char *> _applyEvent;

	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual HRESULT scSetProperty(const char *name, CScValue *value);
	virtual HRESULT scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();

};

} // end of namespace WinterMute

#endif
