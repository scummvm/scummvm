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

#ifndef __WmeAdTalkNode_H__
#define __WmeAdTalkNode_H__

#include "persistent.h"
#include "BBase.h"

namespace WinterMute {
class CAdSpriteSet;
class CBSprite;
class CAdTalkNode : public CBBase {
public:
	char *m_SpriteSetFilename;
	CAdSpriteSet *m_SpriteSet;
	CBSprite *GetSprite(TDirection Dir);
	bool IsInTimeInterval(uint32 Time, TDirection Dir);
	HRESULT LoadSprite();
	DECLARE_PERSISTENT(CAdTalkNode, CBBase)

	CAdTalkNode(CBGame *inGame);
	virtual ~CAdTalkNode();
	HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);
	virtual HRESULT SaveAsText(CBDynBuffer *Buffer, int Indent = 0);
	char *m_SpriteFilename;
	CBSprite *m_Sprite;
	uint32 m_StartTime;
	uint32 m_EndTime;
	bool m_PlayToEnd;
	bool m_PreCache;
	char *m_Comment;

};

} // end of namespace WinterMute

#endif
