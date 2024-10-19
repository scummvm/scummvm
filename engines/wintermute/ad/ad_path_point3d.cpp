/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/ad/ad_path_point3d.h"
#include "engines/wintermute/base/base_persistence_manager.h"
#include "engines/wintermute/base/gfx/xmath.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdPathPoint3D, false)

//////////////////////////////////////////////////////////////////////////
AdPathPoint3D::AdPathPoint3D() : BaseClass() {
	_pos = DXVector3(0.0f, 0.0f, 0.0f);
	_distance = 0.0f;

	_marked = false;
	_origin = nullptr;
}

//////////////////////////////////////////////////////////////////////////
AdPathPoint3D::AdPathPoint3D(DXVector3 pos, float dist) {
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

	persistMgr->transferFloat("x", &_pos._x);
	persistMgr->transferFloat("y", &_pos._y);
	persistMgr->transferFloat("z", &_pos._z);

	return true;
}

} // namespace Wintermute
