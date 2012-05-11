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


#include "coll_templ.h"
#include "BBase.h"

namespace WinterMute {
class CBSurface;
class CBSurfaceStorage : public CBBase {
public:
	uint32 _lastCleanupTime;
	HRESULT InitLoop();
	HRESULT SortSurfaces();
	static int SurfaceSortCB(const void *arg1, const void *arg2);
	HRESULT Cleanup(bool Warn = false);
	//DECLARE_PERSISTENT(CBSurfaceStorage, CBBase);

	HRESULT RestoreAll();
	CBSurface *AddSurface(const char *Filename, bool default_ck = true, byte ck_red = 0, byte ck_green = 0, byte ck_blue = 0, int LifeTime = -1, bool KeepLoaded = false);
	HRESULT RemoveSurface(CBSurface *surface);
	CBSurfaceStorage(CBGame *inGame);
	virtual ~CBSurfaceStorage();

	CBArray<CBSurface *, CBSurface *> _surfaces;
};

} // end of namespace WinterMute

#endif
