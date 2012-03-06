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

#ifndef WINTERMUTE_BIMAGE_H
#define WINTERMUTE_BIMAGE_H


//#include "FreeImage.h"
#include "BBase.h"
#include "common/endian.h"

struct FIBITMAP;

namespace WinterMute {

class CBImage: CBBase {

public:
	CBImage(CBGame *inGame, FIBITMAP *bitmap = NULL);
	~CBImage();


	byte *CreateBMPBuffer(uint32 *BufferSize = NULL);
	HRESULT Resize(int NewWidth, int NewHeight);
	HRESULT SaveBMPFile(char *Filename);
	HRESULT CopyFrom(CBImage *OrigImage, int NewWidth = 0, int NewHeight = 0);

	FIBITMAP *GetBitmap() const {
		return m_Bitmap;
	}
private:
	FIBITMAP *m_Bitmap;
};

} // end of namespace WinterMute

#endif
