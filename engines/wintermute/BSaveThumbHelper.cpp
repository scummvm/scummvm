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

#include "dcgf.h"
#include "BSaveThumbHelper.h"
#include "BImage.h"
#include "BGame.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBSaveThumbHelper::CBSaveThumbHelper(CBGame *inGame): CBBase(inGame) {
	_thumbnail = NULL;
}

//////////////////////////////////////////////////////////////////////////
CBSaveThumbHelper::~CBSaveThumbHelper(void) {
	delete _thumbnail;
	_thumbnail = NULL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSaveThumbHelper::StoreThumbnail(bool DoFlip) {
	delete _thumbnail;
	_thumbnail = NULL;

	if (Game->_thumbnailWidth > 0 && Game->_thumbnailHeight > 0) {
		if (DoFlip) {
			// when using opengl on windows it seems to be necessary to do this twice
			// works normally for direct3d
			Game->DisplayContent(false);
			Game->_renderer->Flip();

			Game->DisplayContent(false);
			Game->_renderer->Flip();
		}

		CBImage *Screenshot = Game->_renderer->TakeScreenshot();
		if (!Screenshot) return E_FAIL;

		// normal thumbnail
		if (Game->_thumbnailWidth > 0 && Game->_thumbnailHeight > 0) {
			_thumbnail = new CBImage(Game);
			_thumbnail->CopyFrom(Screenshot, Game->_thumbnailWidth, Game->_thumbnailHeight);
		}


		delete Screenshot;
		Screenshot = NULL;
	}
	return S_OK;
}

} // end of namespace WinterMute
