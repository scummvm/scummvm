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

#include "dcgf.h"
#include "AdSceneNode.h"
#include "BGame.h"

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
	Game->UnregisterObject(_region);
	_region = NULL;

	Game->UnregisterObject(_entity);
	_entity = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSceneNode::SetEntity(CAdEntity *Entity) {
	_type = OBJECT_ENTITY;
	_entity = Entity;
	return Game->RegisterObject(Entity);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSceneNode::SetRegion(CAdRegion *Region) {
	_type = OBJECT_REGION;
	_region = Region;
	return Game->RegisterObject(Region);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSceneNode::Persist(CBPersistMgr *PersistMgr) {

	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_entity));
	PersistMgr->Transfer(TMEMBER(_region));
	PersistMgr->Transfer(TMEMBER_INT(_type));

	return S_OK;
}

} // end of namespace WinterMute
