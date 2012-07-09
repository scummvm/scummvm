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

#ifndef WINTERMUTE_ADSCENENODE_H
#define WINTERMUTE_ADSCENENODE_H


#include "engines/wintermute/Ad/AdTypes.h"    // Added by ClassView
#include "engines/wintermute/Ad/AdRegion.h"   // Added by ClassView
#include "engines/wintermute/Ad/AdEntity.h"

namespace WinterMute {

class CAdSceneNode : public CBObject {
public:
	DECLARE_PERSISTENT(CAdSceneNode, CBObject)
	ERRORCODE setRegion(CAdRegion *region);
	ERRORCODE setEntity(CAdEntity *entity);
	CAdEntity *_entity;
	CAdRegion *_region;
	TObjectType _type;
	CAdSceneNode(CBGame *inGame);
	virtual ~CAdSceneNode();

};

}

#endif
