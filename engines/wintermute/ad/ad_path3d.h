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

#ifndef WINTERMUTE_AD_PATH3D_H
#define WINTERMUTE_AD_PATH3D_H

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/persistent.h"

#include "math/vector3d.h"

namespace Wintermute {

class AdPath3D : public BaseClass {
public:
	DXVector3 *getCurrent();
	DXVector3 *getNext();
	DXVector3 *getFirst();
	bool setReady(bool ready);
	void addPoint(float x, float y, float z);
	void addPoint(DXVector3 Point);
	void reset();
	AdPath3D(BaseGame *inGame);
	virtual ~AdPath3D();
	bool _ready;
	BaseArray<DXVector3 *> _points;
	int32 _currIndex;

	DECLARE_PERSISTENT(AdPath3D, BaseClass)
};

} // namespace Wintermute

#endif
