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

#ifndef WINTERMUTE_BASE_SURFACE_STORAGE_H
#define WINTERMUTE_BASE_SURFACE_STORAGE_H

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/coll_templ.h"

namespace Wintermute {
class BaseSurface;
class BaseSurfaceStorage : public BaseClass {
public:
	uint32 _lastCleanupTime;
	bool initLoop();
	bool sortSurfaces();
	static int surfaceSortCB(const void *arg1, const void *arg);
	bool cleanup(bool warn = false);
	//DECLARE_PERSISTENT(BaseSurfaceStorage, BaseClass);

	bool restoreAll();
	BaseSurface *addSurface(const Common::String &filename, bool defaultCK = true, byte ckRed = 0, byte ckGreen = 0, byte ckBlue = 0, int lifeTime = -1, bool keepLoaded = false);
	bool removeSurface(BaseSurface *surface);
	BaseSurfaceStorage(BaseGame *inGame);
	~BaseSurfaceStorage() override;

	BaseArray<BaseSurface *> _surfaces;
};

} // End of namespace Wintermute

#endif
