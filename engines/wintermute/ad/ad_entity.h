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

#ifndef WINTERMUTE_ADENTITY_H
#define WINTERMUTE_ADENTITY_H

#include "engines/wintermute/ad/ad_talk_holder.h"

namespace WinterMute {
class CVidTheoraPlayer;
class CAdEntity : public CAdTalkHolder {
public:
	CVidTheoraPlayer *_theora;
	bool setSprite(const char *filename);
	int _walkToX;
	int _walkToY;
	TDirection _walkToDir;
	void setItem(const char *itemName);
	char *_item;
	DECLARE_PERSISTENT(CAdEntity, CAdTalkHolder)
	void updatePosition();
	virtual int getHeight();
	CBRegion *_region;
	virtual bool saveAsText(CBDynBuffer *buffer, int indent);
	virtual bool update();
	virtual bool display();
	CAdEntity(CBGame *inGame);
	virtual ~CAdEntity();
	bool loadFile(const char *filename);
	bool loadBuffer(byte *buffer, bool complete = true);
	TEntityType _subtype;

	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual bool scSetProperty(const char *name, CScValue *value);
	virtual bool scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();

};

} // end of namespace WinterMute

#endif
