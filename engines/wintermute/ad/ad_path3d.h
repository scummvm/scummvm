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

#ifndef WINTERMUTE_AD_PATH3D_H
#define WINTERMUTE_AD_PATH3D_H

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/persistent.h"
#include "math/vector3d.h"

namespace Wintermute {

class AdPath3D : public BaseClass {
public:
	Math::Vector3d *getCurrent();
	Math::Vector3d *getNext();
	Math::Vector3d *getFirst();
	bool setReady(bool ready);
	void addPoint(float x, float y, float z);
	void addPoint(Math::Vector3d Point);
	void reset();
	AdPath3D(BaseGame *inGame);
	virtual ~AdPath3D();
	bool _ready;
	BaseArray<Math::Vector3d *> _points;
	int32 _currIndex;

	DECLARE_PERSISTENT(AdPath3D, BaseClass)
};

} // namespace Wintermute

#endif
