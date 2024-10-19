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

#include "engines/wintermute/ad/ad_path3d.h"
#include "engines/wintermute/base/base_persistence_manager.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdPath3D, false)

//////////////////////////////////////////////////////////////////////////
AdPath3D::AdPath3D(BaseGame *inGame) : BaseClass(inGame) {
	_currIndex = -1;
	_ready = false;
}

//////////////////////////////////////////////////////////////////////////
AdPath3D::~AdPath3D() {
	reset();
}

//////////////////////////////////////////////////////////////////////////
void AdPath3D::reset() {
	for (uint i = 0; i < _points.size(); i++) {
		delete _points[i];
	}

	_points.clear();
	_currIndex = -1;
	_ready = false;
}

//////////////////////////////////////////////////////////////////////////
void AdPath3D::addPoint(DXVector3 point) {
	_points.add(new DXVector3(point));
}

//////////////////////////////////////////////////////////////////////////
void AdPath3D::addPoint(float x, float y, float z) {
	_points.add(new DXVector3(x, y, z));
}

//////////////////////////////////////////////////////////////////////////
bool AdPath3D::setReady(bool ready) {
	bool orig = _ready;
	_ready = ready;

	return orig;
}

//////////////////////////////////////////////////////////////////////////
DXVector3 *AdPath3D::getFirst() {
	if (_points.size() > 0) {
		_currIndex = 0;
		return _points[_currIndex];
	} else {
		return nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
DXVector3 *AdPath3D::getNext() {
	_currIndex++;
	if (static_cast<uint>(_currIndex) < _points.size()) {
		return _points[_currIndex];
	} else {
		return nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
DXVector3 *AdPath3D::getCurrent() {
	if (_currIndex >= 0 && static_cast<uint>(_currIndex) < _points.size()) {
		return _points[_currIndex];
	} else {
		return nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdPath3D::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferPtr(TMEMBER(_gameRef));

	persistMgr->transferSint32(TMEMBER(_currIndex));
	persistMgr->transferBool(TMEMBER(_ready));

	if (persistMgr->getIsSaving()) {
		int32 j = _points.size();
		persistMgr->transferSint32("ArraySize", &j);
		for (int i = 0; i < j; i++) {
			persistMgr->transferFloat("x", &_points[i]->_x);
			persistMgr->transferFloat("y", &_points[i]->_y);
			persistMgr->transferFloat("z", &_points[i]->_z);
		}
	} else {
		int32 j = 0;
		persistMgr->transferSint32("ArraySize", &j);
		for (int i = 0; i < j; i++) {
			float x, y, z;
			persistMgr->transferFloat("x", &x);
			persistMgr->transferFloat("y", &y);
			persistMgr->transferFloat("z", &z);
			addPoint(x, y, z);
		}
	}

	return true;
}

} // namespace Wintermute
