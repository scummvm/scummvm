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

#ifndef WINTERMUTE_BSUBFRAME_H
#define WINTERMUTE_BSUBFRAME_H


#include "engines/wintermute/Base/BBase.h"
#include "engines/wintermute/Base/BScriptable.h"

namespace WinterMute {
class CBObject;
class CBSurface;
class CBSubFrame : public CBScriptable {
public:
	bool _mirrorX;
	bool _mirrorY;
	bool _decoration;
	HRESULT setSurface(const char *Filename, bool default_ck = true, byte ck_red = 0, byte ck_green = 0, byte ck_blue = 0, int LifeTime = -1, bool KeepLoaded = false);
	HRESULT setSurfaceSimple();
	DECLARE_PERSISTENT(CBSubFrame, CBScriptable)
	void setDefaultRect();
	uint32 _transparent;
	HRESULT saveAsText(CBDynBuffer *buffer, int indent) { return saveAsText(buffer, indent, true); }
	HRESULT saveAsText(CBDynBuffer *buffer, int indent, bool complete);
	bool _editorSelected;
	CBSubFrame(CBGame *inGame);
	virtual ~CBSubFrame();
	HRESULT loadBuffer(byte  *Buffer, int LifeTime, bool KeepLoaded);
	HRESULT draw(int X, int Y, CBObject *Register = NULL, float ZoomX = 100, float ZoomY = 100, bool Precise = true, uint32 Alpha = 0xFFFFFFFF, float Rotate = 0.0f, TSpriteBlendMode BlendMode = BLEND_NORMAL);
	bool getBoundingRect(LPRECT Rect, int X, int Y, float ScaleX = 100, float ScaleY = 100);

	int _hotspotX;
	int _hotspotY;
	uint32 _alpha;
	RECT _rect;

	bool _cKDefault;
	byte _cKRed;
	byte _cKGreen;
	byte _cKBlue;
	int _lifeTime;
	bool _keepLoaded;
	char *_surfaceFilename;

	bool _2DOnly;
	bool _3DOnly;

	CBSurface *_surface;

	// scripting interface
	virtual CScValue *scGetProperty(const char *Name);
	virtual HRESULT scSetProperty(const char *Name, CScValue *Value);
	virtual HRESULT scCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	virtual const char *scToString();

};

} // end of namespace WinterMute

#endif
