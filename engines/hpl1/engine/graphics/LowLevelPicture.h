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
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_LOWLEVEL_PICTURE_H
#define HPL_LOWLEVEL_PICTURE_H

#include "common/system.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class LowLevelPicture {
public:
	LowLevelPicture(const tString &type) : _type(type) {}
	virtual ~LowLevelPicture() = default;

	tString getType() { return _type; }

	uint32 getHeight() const { return _height; }
	uint32 getWidth() const { return _width; }

	virtual uint32 getBpp() const = 0;

	virtual bool hasAlpha() = 0;

	void setPath(const tString &path) { _path = path; }
	tString getPath() { return _path; }
	tString getFileName() const { return cString::GetFileName(_path); }

protected:
	uint32 _height;
	uint32 _width;

private:
	tString _type;
	tString _path;
};

} // namespace hpl

#endif // HPL_LOWLEVEL_PICTURE_H
