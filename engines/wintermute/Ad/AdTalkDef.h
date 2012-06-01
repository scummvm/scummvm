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

#ifndef WINTERMUTE_ADTALKDEF_H
#define WINTERMUTE_ADTALKDEF_H

#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/BObject.h"

namespace WinterMute {
class CAdTalkNode;
class CAdSpriteSet;
class CAdTalkDef : public CBObject {
public:
	char *_defaultSpriteSetFilename;
	CAdSpriteSet *_defaultSpriteSet;
	CBSprite *GetDefaultSprite(TDirection Dir);
	HRESULT LoadDefaultSprite();
	DECLARE_PERSISTENT(CAdTalkDef, CBObject)

	CAdTalkDef(CBGame *inGame);
	virtual ~CAdTalkDef();
	HRESULT LoadFile(const char *Filename);
	HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);
	CBArray<CAdTalkNode *, CAdTalkNode *> _nodes;
	char *_defaultSpriteFilename;
	CBSprite *_defaultSprite;
	virtual HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent = 0);
};

} // end of namespace WinterMute

#endif
