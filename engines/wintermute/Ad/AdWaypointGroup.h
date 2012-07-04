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

#ifndef WINTERMUTE_ADWAYPOINTGROUP_H
#define WINTERMUTE_ADWAYPOINTGROUP_H

#include "engines/wintermute/Base/BObject.h"

namespace WinterMute {
class CBPoint;
class CAdWaypointGroup : public CBObject {
public:
	float _lastMimicScale;
	int _lastMimicX;
	int _lastMimicY;
	void cleanup();
	HRESULT Mimic(CAdWaypointGroup *Wpt, float Scale = 100.0f, int X = 0, int Y = 0);
	DECLARE_PERSISTENT(CAdWaypointGroup, CBObject)
	virtual HRESULT saveAsText(CBDynBuffer *buffer, int indent);
	bool _active;
	CAdWaypointGroup(CBGame *inGame);
	HRESULT loadFile(const char *filename);
	HRESULT loadBuffer(byte *buffer, bool complete = true);
	virtual ~CAdWaypointGroup();
	CBArray<CBPoint *, CBPoint *> _points;
	int _editorSelectedPoint;
	virtual CScValue *scGetProperty(const char *name);
	virtual HRESULT scSetProperty(const char *name, CScValue *value);
};

} // end of namespace WinterMute

#endif
