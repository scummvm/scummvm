/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BSURFACESTORAGE_H
#define WINTERMUTE_BSURFACESTORAGE_H


#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/Base/BBase.h"

namespace WinterMute {
class CBSurface;
class CBSurfaceStorage : public CBBase {
public:
	uint32 _lastCleanupTime;
	ERRORCODE initLoop();
	ERRORCODE sortSurfaces();
	static int surfaceSortCB(const void *arg1, const void *arg2);
	ERRORCODE cleanup(bool Warn = false);
	//DECLARE_PERSISTENT(CBSurfaceStorage, CBBase);

	ERRORCODE restoreAll();
	CBSurface *addSurface(const char *filename, bool defaultCK = true, byte ckRed = 0, byte ckGreen = 0, byte ckBlue = 0, int lifeTime = -1, bool keepLoaded = false);
	ERRORCODE removeSurface(CBSurface *surface);
	CBSurfaceStorage(CBGame *inGame);
	virtual ~CBSurfaceStorage();

	CBArray<CBSurface *, CBSurface *> _surfaces;
};

} // end of namespace WinterMute

#endif
