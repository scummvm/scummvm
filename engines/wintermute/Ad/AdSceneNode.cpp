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

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Ad/AdSceneNode.h"
#include "engines/wintermute/Base/BGame.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdSceneNode, false)

//////////////////////////////////////////////////////////////////////////
CAdSceneNode::CAdSceneNode(CBGame *inGame): CBObject(inGame) {
	_type = OBJECT_NONE;
	_region = NULL;
	_entity = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdSceneNode::~CAdSceneNode() {
	_gameRef->unregisterObject(_region);
	_region = NULL;

	_gameRef->unregisterObject(_entity);
	_entity = NULL;
}


//////////////////////////////////////////////////////////////////////////
bool CAdSceneNode::setEntity(CAdEntity *entity) {
	_type = OBJECT_ENTITY;
	_entity = entity;
	return _gameRef->registerObject(entity);
}


//////////////////////////////////////////////////////////////////////////
bool CAdSceneNode::setRegion(CAdRegion *region) {
	_type = OBJECT_REGION;
	_region = region;
	return _gameRef->registerObject(region);
}


//////////////////////////////////////////////////////////////////////////
bool CAdSceneNode::persist(CBPersistMgr *persistMgr) {

	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_entity));
	persistMgr->transfer(TMEMBER(_region));
	persistMgr->transfer(TMEMBER_INT(_type));

	return STATUS_OK;
}

} // end of namespace WinterMute
