/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/ad/ad_path_point3d.h"
#include "engines/wintermute/base/base_persistence_manager.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdPathPoint3D, false)

//////////////////////////////////////////////////////////////////////////
AdPathPoint3D::AdPathPoint3D() : BaseClass(),
                                 _origin(nullptr), _marked(false),
                                 _distance(0.0f), _pos(0.0f, 0.0f, 0.0f) {
}

//////////////////////////////////////////////////////////////////////////
AdPathPoint3D::AdPathPoint3D(Math::Vector3d pos, float dist) {
	_pos = pos;
	_distance = dist;

	_marked = false;
	_origin = nullptr;
}

//////////////////////////////////////////////////////////////////////////
AdPathPoint3D::~AdPathPoint3D() {
	_origin = nullptr;
}

//////////////////////////////////////////////////////////////////////////
bool AdPathPoint3D::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferFloat(TMEMBER(_distance));
	persistMgr->transferBool(TMEMBER(_marked));
	persistMgr->transferPtr(TMEMBER(_origin));

	persistMgr->transferFloat("x", &_pos.x());
	persistMgr->transferFloat("y", &_pos.y());
	persistMgr->transferFloat("z", &_pos.z());

	return true;
}

} // namespace Wintermute
