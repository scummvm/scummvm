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

#ifndef TITANIC_GAME_MANAGER_H
#define TITANIC_GAME_MANAGER_H

#include "common/scummsys.h"
#include "titanic/game_state.h"
#include "titanic/input_handler.h"
#include "titanic/input_translator.h"
#include "titanic/simple_file.h"
#include "titanic/video_surface.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/sound/music_room.h"
#include "titanic/sound/sound.h"

namespace Titanic {

class CProjectItem;
class CGameView;

class CGameManagerListItem : public ListItem {
};

class CGameManagerList : public List<CGameManagerListItem> {
};

class CGameManager {
private:
	CProjectItem *_project;
	CGameView *_gameView;
	CGameState _gameState;
	CSound _sound;
	CInputHandler _inputHandler;
	CInputTranslator _inputTranslator;
	CMusicRoom _musicRoom;
	CTrueTalkManager _trueTalkManager;
	Common::Rect _bounds;
	CGameManagerList _list;
	int _field30;
	int _field34;
	int _field48;
	int _field4C;
	int _field50;
	int _field54;
	CVideoSurface *_videoSurface;
	int _tickCount;
public:
	CGameManager(CProjectItem *project, CGameView *gameView);
	~CGameManager();

	/**
	 * Load data from a save file
	 */
	void load(SimpleFile *file);

	/**
	 * Called after loading a game has finished
	 */
	void gameLoaded();
};

} // End of namespace Titanic

#endif /* TITANIC_GAME_MANAGER_H */
