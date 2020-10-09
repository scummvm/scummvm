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

#ifndef WINTERMUTE_LIGHT3D_H
#define WINTERMUTE_LIGHT3D_H

#include "common/memstream.h"
#include "engines/wintermute/base/base_persistence_manager.h"
#include "engines/wintermute/base/base_scriptable.h"
#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Wintermute {

class Light3D : public BaseScriptable {
public:
	bool persist(BasePersistenceManager *persistMgr);
	bool getViewMatrix(Math::Matrix4 *viewMatrix);
	Light3D(BaseGame *inGame);
	virtual ~Light3D();
	uint32 _diffuseColor;
	Math::Vector3d _position;
	Math::Vector3d _target;
	bool _isSpotlight;
	bool _active;
	float _falloff;

	float _distance;
	bool _isAvailable;

	bool setLight(int index = 0);
	bool loadFrom3DS(Common::MemoryReadStream &fileStream);
};

} // namespace Wintermute

#endif
