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

#ifndef WINTERMUTE_3D_EFFECT_H
#define WINTERMUTE_3D_EFFECT_H

#include "engines/wintermute/base/base_named_object.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
class Effect3D : public BaseClass {
public:
	Effect3D(BaseGame *inGame);
	~Effect3D();

	bool createFromFile(const Common::String &filename);
	uint32 getEffectHash() { return _effectHash; }
	bool invalidateDeviceObjects();
	bool restoreDeviceObjects();
	const char *getFileName() { return _filename.c_str(); }

private:
	Common::String _filename;
	uint32 _effectHash;
};

} // namespace Wintermute

#endif
