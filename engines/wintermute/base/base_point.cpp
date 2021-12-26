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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_point.h"
#include "engines/wintermute/base/base_persistence_manager.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(BasePoint, false)

//////////////////////////////////////////////////////////////////////////
BasePoint::BasePoint() {
	x = y = 0;
}


//////////////////////////////////////////////////////////////////////////
BasePoint::~BasePoint() {

}


//////////////////////////////////////////////////////////////////////////
BasePoint::BasePoint(int initX, int initY) {
	x = initX;
	y = initY;
}

//////////////////////////////////////////////////////////////////////////
bool BasePoint::persist(BasePersistenceManager *persistMgr) {

	persistMgr->transferSint32(TMEMBER(x));
	persistMgr->transferSint32(TMEMBER(y));

	return STATUS_OK;
}

} // End of namespace Wintermute
