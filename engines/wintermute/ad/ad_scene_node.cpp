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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

#include "engines/wintermute/ad/ad_scene_node.h"
#include "engines/wintermute/base/base_game.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdSceneNode, false)

//////////////////////////////////////////////////////////////////////////
AdSceneNode::AdSceneNode(BaseGame *inGame) : BaseObject(inGame) {
	_type = OBJECT_NONE;
	_region = nullptr;
	_entity = nullptr;
}


//////////////////////////////////////////////////////////////////////////
AdSceneNode::~AdSceneNode() {
	_gameRef->unregisterObject(_region);
	_region = nullptr;

	_gameRef->unregisterObject(_entity);
	_entity = nullptr;
}


//////////////////////////////////////////////////////////////////////////
bool AdSceneNode::setEntity(AdEntity *entity) {
	_type = OBJECT_ENTITY;
	_entity = entity;
	return _gameRef->registerObject(entity);
}


//////////////////////////////////////////////////////////////////////////
bool AdSceneNode::setRegion(AdRegion *region) {
	_type = OBJECT_REGION;
	_region = region;
	return _gameRef->registerObject(region);
}


//////////////////////////////////////////////////////////////////////////
bool AdSceneNode::persist(BasePersistenceManager *persistMgr) {

	BaseObject::persist(persistMgr);

	persistMgr->transferPtr(TMEMBER_PTR(_entity));
	persistMgr->transferPtr(TMEMBER_PTR(_region));
	persistMgr->transferSint32(TMEMBER_INT(_type));

	return STATUS_OK;
}

} // End of namespace Wintermute
