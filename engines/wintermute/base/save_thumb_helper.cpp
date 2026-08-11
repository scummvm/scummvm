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

#include "engines/wintermute/base/save_thumb_helper.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/dcgf.h"

#include "graphics/scaler.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
SaveThumbHelper::SaveThumbHelper(BaseGame *inGame) : _game(inGame) {
	_thumbnail = nullptr;
	_scummVMThumb = nullptr;
}

//////////////////////////////////////////////////////////////////////////
SaveThumbHelper::~SaveThumbHelper() {
	SAFE_DELETE(_thumbnail);
	SAFE_DELETE(_scummVMThumb);
}

BaseImage *SaveThumbHelper::storeThumb(bool doFlip, int width, int height) {
	if (_game->_thumbnailWidth > 0 && _game->_thumbnailHeight > 0) {
		if (doFlip) {
			// when using opengl on windows it seems to be necessary to do this twice
			// works normally for direct3d
			_game->displayContent(false);
			_game->_renderer->flip();

			_game->displayContent(false);
			_game->_renderer->flip();
		}

		// normal thumbnail
		return _game->_renderer->takeScreenshot(width, height);
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
bool SaveThumbHelper::storeThumbnail(bool doFlip) {
	SAFE_DELETE(_thumbnail);

	if (_game->_thumbnailWidth > 0 && _game->_thumbnailHeight > 0) {
		_thumbnail = storeThumb(doFlip, _game->_thumbnailWidth, _game->_thumbnailHeight);
		if (!_thumbnail) {
			return STATUS_FAILED;
		}
	}
	storeScummVMThumbNail();
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool SaveThumbHelper::storeScummVMThumbNail(bool doFlip) {
	SAFE_DELETE(_scummVMThumb);

	_scummVMThumb = storeThumb(doFlip, kThumbnailWidth, kThumbnailHeight2);
	if (!_scummVMThumb) {
		return STATUS_FAILED;
	}
	return STATUS_OK;
}

} // End of namespace Wintermute
