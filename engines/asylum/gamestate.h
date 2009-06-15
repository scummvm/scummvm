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
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_GameState_H_
#define ASYLUM_GameState_H_

#include "common/events.h"

#include "asylum/asylum.h"
#include "asylum/resourcepack.h"
#include "asylum/graphics.h"
#include "asylum/text.h"
#include "asylum/scene_resource.h"

namespace Asylum {

class Screen;
class Sound;
class SceneResource;
class Text;

class GameState {
public:
    GameState(Screen *screen, Sound *sound, uint8 sceneIdx);
	~GameState();

    void handleEvent(Common::Event *event, bool doUpdate);

	void enterScene();

private:
    Screen *_screen;
    Sound *_sound;
	Common::Event *_ev;

    Text *_text;
    SceneResource *_scene;
    ResourcePack *_resPack;
	ResourcePack *_musPack;
    GraphicResource *_bgResource;
	GraphicResource *_cursorResource;

    uint8 _sceneIdx;
    int _mouseX;
	int _mouseY;
    bool _leftClick;
	int _curMouseCursor;
	int _cursorStep;

	void update();
	void updateCursor();

	enum GameCursors {
		kCursorUpArrow = 1,
		kCursorUpLeftArrow = 2
		// TODO: add the rest
	};

}; // end of class GameState

} // end of namespace Asylum

#endif
