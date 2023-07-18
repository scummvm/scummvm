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
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_LEVEL_H
#define CRAB_LEVEL_H

#include "crab/event/GameEventInfo.h"
#include "crab/image/ImageManager.h"
#include "crab/ui/ParagraphData.h"
#include "crab/PathfindingGrid.h"
#include "crab/TMX/TMXMap.h"
#include "crab/animation/animset.h"
#include "crab/ui/button.h"
#include "crab/common_header.h"
#include "crab/animation/fightmove.h"
#include "crab/level/level_objects.h"
#include "crab/people/person.h"
#include "crab/animation/sprite.h"
#include "crab/level/talknotify.h"

namespace Crab {

namespace pyrodactyl {
namespace level {
class Level {
	// The .tmx file to import terrain from
	TMX::TMXMap _terrain;

	// The pathfinding grid for the level
	PathfindingGrid _pathfindingGrid;

	// The area we display to the player
	Rect _camera;

	// The player sprite location in the object list
	unsigned int _playerIndex;

	// The order in which to draw the sprites
	Common::MultiMap<int, pyrodactyl::anim::Sprite *> _objSeq;

	// The file index which contains the fighting moves of all characters
	Common::Array<Common::String> _animSet;

	// The movement sets for sprites in levels
	Common::Array<pyrodactyl::ai::MovementSet> _moveSet;

	// These sprites are only for animated objects and cannot be interacted with
	Common::Array<pyrodactyl::anim::Sprite> _background;

	// These sprites fly across the screen randomly
	Common::Array<pyrodactyl::anim::Sprite> _fly;

	// The id of the music track being played
	MusicInfo _music;

	// Is the world map accessible from this level?
	MapVis _showmap;

	// We disable exits when player is fighting enemies - this is used to check if we are in an exit area WHILE fighting enemies
	// If we are fighting inside exit area, the level switch is delayed until we walk out of exit and back in
	// to prevent instant level switch as soon as you beat all enemies
	bool _insideExit;

	bool _firstHit;

	// Default sprite parameters
	pyrodactyl::ai::SpriteConstant _scDefault;

	// Protected level functions
	bool collidingWithObject(pyrodactyl::event::Info &info, Common::String &id);
	bool collidingWithLevel(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s);
	bool playerInCombat(pyrodactyl::event::Info &info);
	void battleAlert(pyrodactyl::event::Info &info);

	void sortObjectsToDraw();
	void moveObject(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s);

	void think(pyrodactyl::event::Info &info, Common::Array<pyrodactyl::event::EventResult> &result,
			   Common::Array<pyrodactyl::event::EventSeqInfo> &endSeq, Common::String &id);

	void drawObjects(pyrodactyl::event::Info &info);
	void setCamera();

	bool layerVisible(pyrodactyl::anim::Sprite *obj);

public:
	// The objects in the level, and the player character
	Common::Array<pyrodactyl::anim::Sprite> _objects;

	// The notification text drawn if the player is able to talk to a sprite
	TalkNotify _talkNotify;

	// Used for drawing the destination marker for point and click movement
	PlayerDestMarker _destMarker;

	// The location of this level on the world map
	Vector2i _mapLoc;

	// The clip revealed by this level on the world map
	struct MapClip {
		// Which world map is this clip added to?
		int _id;

		// The clip itself
		Rect _rect;

		MapClip() {
			_id = 0;
		}
	} _mapClip;

	// Used to draw ambient dialog
	pyrodactyl::ui::ParagraphData _pop;

	// The path of the preview image
	Common::String _previewPath;

	// A full rendered image of the level
	pyrodactyl::image::Image _img;

	Level() : _playerIndex(0) {
		reset();
	}

	~Level() {
		reset();
	}

	void reset();

	void Camera(int x, int y, int w, int h) {
		_camera.x = x;
		_camera.y = y;
		_camera.w = w;
		_camera.h = h;
	}

	Rect camera() {
		return _camera;
	}

	void playerStop() {
		_objects[_playerIndex].stop();
	}

	const Common::String &playerId() {
		return _objects[_playerIndex].id();
	}

	void playerId(const Common::String &ID, const int &x, const int &y);

	void showMap(bool val) {
		_showmap._normal = val;
	}

	bool showMap() {
		return _showmap._current;
	}

	bool operator()(int i, int j);

	// This calculates the unlocked moves for each sprite in the level, and the visibility of objects
	void calcProperties(pyrodactyl::event::Info &info);

	// Loading function
	void load(const Common::String &filename, pyrodactyl::event::Info &info, pyrodactyl::event::TriggerSet &gameOver,
			  const int &playerX = -1, const int &playerY = -1);

	// One time load called first-time
	void loadMoves(const Common::String &filename);
	void loadConst(const Common::String &filename);

	// Used to see if a sprite collides with a rectangle
	void calcTrigCollide(pyrodactyl::event::Info &info);

	// See if a player clicked on the sprite in contact
	bool containsClick(const Common::String &id, const Common::Event &event);
#if 0
	bool ContainsClick(const Common::String &id, const SDL_Event &Event);
#endif

	// Get index of a sprite in the object array
	pyrodactyl::anim::Sprite *getSprite(const Common::String &id);

	void handleEvents(pyrodactyl::event::Info &info, const Common::Event &Event);
#if 0
	void handleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event);
#endif
	LevelResult internalEvents(pyrodactyl::event::Info &info, Common::Array<pyrodactyl::event::EventResult> &result,
							   Common::Array<pyrodactyl::event::EventSeqInfo> &endSeq, bool eventInProgress);

	void preDraw();
	void preDrawObjects(Graphics::ManagedSurface *surf);
	void draw(pyrodactyl::event::Info &info);

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void loadState(rapidxml::xml_node<char> *node);

	void setUI();
};
} // End of namespace level
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_LEVEL_H
