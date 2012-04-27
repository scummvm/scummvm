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

#ifndef WINTERMUTE_BFONTSTORAGE_H
#define WINTERMUTE_BFONTSTORAGE_H


#include "BBase.h"
#include "persistent.h"
#include "coll_templ.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace WinterMute {

class CBFont;

class CBFontStorage : public CBBase {
public:
	DECLARE_PERSISTENT(CBFontStorage, CBBase)
	HRESULT Cleanup(bool Warn = false);
	HRESULT RemoveFont(CBFont *Font);
	CBFont *AddFont(char *Filename);
	CBFontStorage(CBGame *inGame);
	virtual ~CBFontStorage();
	CBArray<CBFont *, CBFont *> _fonts;
	HRESULT InitLoop();

	FT_Library GetFTLibrary() const {
		return _fTLibrary;
	}

private:
	void InitFreeType();
	FT_Library _fTLibrary;
};

} // end of namespace WinterMute

#endif
