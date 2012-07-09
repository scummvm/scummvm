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

#ifndef WINTERMUTE_ADREGION_H
#define WINTERMUTE_ADREGION_H

#include "engines/wintermute/Base/BRegion.h"

namespace WinterMute {

class CAdRegion : public CBRegion {
public:
	DECLARE_PERSISTENT(CAdRegion, CBRegion)
	uint32 _alpha;
	float _zoom;
	bool _blocked;
	bool _decoration;
	CAdRegion(CBGame *inGame);
	virtual ~CAdRegion();
	ERRORCODE loadFile(const char *filename);
	ERRORCODE loadBuffer(byte *buffer, bool complete = true);
	virtual ERRORCODE saveAsText(CBDynBuffer *buffer, int indent);

	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual ERRORCODE scSetProperty(const char *name, CScValue *value);
	virtual ERRORCODE scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();
};

} // end of namespace WinterMute

#endif
