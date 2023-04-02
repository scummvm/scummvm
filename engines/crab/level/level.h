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
	TMX::TMXMap terrain;

	// The pathfinding grid for the level
	PathfindingGrid pathfindingGrid;

	// The area we display to the player
	Rect camera;

	// The player sprite location in the object list
	unsigned int player_index;

	// The order in which to draw the sprites
	std::multimap<int, pyrodactyl::anim::Sprite *> obj_seq;

	// The file index which contains the fighting moves of all characters
	std::vector<std::string> anim_set;

	// The movement sets for sprites in levels
	std::vector<pyrodactyl::ai::MovementSet> move_set;

	// These sprites are only for animated objects and cannot be interacted with
	std::vector<pyrodactyl::anim::Sprite> background;

	// These sprites fly across the screen randomly
	std::vector<pyrodactyl::anim::Sprite> fly;

	// The id of the music track being played
	MusicInfo music;

	// Is the world map accessible from this level?
	MapVis showmap;

	// We disable exits when player is fighting enemies - this is used to check if we are in an exit area WHILE fighting enemies
	// If we are fighting inside exit area, the level switch is delayed until we walk out of exit and back in
	// to prevent instant level switch as soon as you beat all enemies
	bool inside_exit;

	bool first_hit;

	// Default sprite parameters
	pyrodactyl::ai::SpriteConstant sc_default;

	// Protected level functions
	bool CollidingWithObject(pyrodactyl::event::Info &info, std::string &id);
	bool CollidingWithLevel(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s);
	bool PlayerInCombat(pyrodactyl::event::Info &info);
	void BattleAlert(pyrodactyl::event::Info &info);

	void SortObjectsToDraw();
	void MoveObject(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s);

	void Think(pyrodactyl::event::Info &info, std::vector<pyrodactyl::event::EventResult> &result,
			   std::vector<pyrodactyl::event::EventSeqInfo> &end_seq, std::string &id);

	void DrawObjects(pyrodactyl::event::Info &info);
	void SetCamera();

	bool LayerVisible(pyrodactyl::anim::Sprite *obj);

public:
	// The objects in the level, and the player character
	std::vector<pyrodactyl::anim::Sprite> objects;

	// The notification text drawn if the player is able to talk to a sprite
	TalkNotify talk_notify;

	// Used for drawing the destination marker for point and click movement
	PlayerDestMarker dest_marker;

	// The location of this level on the world map
	Vector2i map_loc;

	// The clip revealed by this level on the world map
	struct MapClip {
		// Which world map is this clip added to?
		int id;

		// The clip itself
		Rect rect;

		MapClip() { id = 0; }
	} map_clip;

	// Used to draw ambient dialog
	pyrodactyl::ui::ParagraphData pop;

	// The path of the preview image
	std::string preview_path;

	Level() { Reset(); }
	~Level() {}

	void Reset();

	void Camera(int x, int y, int w, int h) {
		camera.x = x;
		camera.y = y;
		camera.w = w;
		camera.h = h;
	}
	Rect Camera() { return camera; }

	void PlayerStop() { objects[player_index].Stop(); }

	const std::string &PlayerID() { return objects[player_index].ID(); }
	void PlayerID(const std::string &ID, const int &X, const int &Y);

	void ShowMap(bool val) { showmap.normal = val; }
	bool ShowMap() { return showmap.current; }

	bool operator()(int i, int j);

	// This calculates the unlocked moves for each sprite in the level, and the visibility of objects
	void CalcProperties(pyrodactyl::event::Info &info);

	// Loading function
	void Load(const std::string &filename, pyrodactyl::event::Info &info, pyrodactyl::event::TriggerSet &game_over,
			  const int &player_x = -1, const int &player_y = -1);

	// One time load called first-time
	void LoadMoves(const std::string &filename);
	void LoadConst(const std::string &filename);

	// Used to see if a sprite collides with a rectangle
	void CalcTrigCollide(pyrodactyl::event::Info &info);

#if 0
	// See if a player clicked on the sprite in contact
	bool ContainsClick(const std::string &id, const SDL_Event &Event);
#endif

	// Get index of a sprite in the object array
	pyrodactyl::anim::Sprite *GetSprite(const std::string &id);

#if 0
	void HandleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event);
#endif
	LevelResult InternalEvents(pyrodactyl::event::Info &info, std::vector<pyrodactyl::event::EventResult> &result,
							   std::vector<pyrodactyl::event::EventSeqInfo> &end_seq, bool EventInProgress);

	void Draw(pyrodactyl::event::Info &info);

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void LoadState(rapidxml::xml_node<char> *node);

	void SetUI();
};
} // End of namespace level
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_LEVEL_H
