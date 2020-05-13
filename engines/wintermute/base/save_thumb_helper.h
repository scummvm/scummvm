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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
#ifndef WINTERMUTE_BASE_SAVE_THUMB_HELPER_H
#define WINTERMUTE_BASE_SAVE_THUMB_HELPER_H

namespace Wintermute {

class BaseImage;
class BaseGame;

class SaveThumbHelper {
public:
	SaveThumbHelper(BaseGame *inGame);
	virtual ~SaveThumbHelper(void);
	bool storeThumbnail(bool doFlip = false);
	bool storeScummVMThumbNail(bool doFlip = false);

	BaseImage *_thumbnail;
	BaseImage *_scummVMThumb;
private:
	BaseImage *storeThumb(bool doFlip, int width, int height);
	BaseGame *_gameRef;
};

} // End of namespace Wintermute

#endif
