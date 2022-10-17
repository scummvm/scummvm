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

#ifndef TETRAEDGE_GAME_GALLERY_MENU_H
#define TETRAEDGE_GAME_GALLERY_MENU_H

#include "common/array.h"
#include "tetraedge/te/te_music.h"
#include "tetraedge/te/te_lua_gui.h"

namespace Tetraedge {

class GalleryMenu : public TeLuaGUI {
public:
	GalleryMenu();

	struct GalleryBtnObject {
		bool onValidated();

		Common::String _audioPath;
		Common::String _moviePath;
		GalleryMenu *_owner;
	};

	void enter();
	void leave();

	bool onQuitButton();
	bool onLockVideoButtonValidated();
	bool onSkipVideoButtonValidated();
	bool onVideoFinished();
	TeMusic &music();

private:
	TeMusic _music;
	Common::Array<GalleryBtnObject *> _btnObjects;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_GALLERY_MENU_H
