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
	Common::MultiMap<int, pyrodactyl::anim::Sprite *> obj_seq;

	// The file index which contains the fighting moves of all characters
	Common::Array<Common::String> anim_set;

	// The movement sets for sprites in levels
	Common::Array<pyrodactyl::ai::MovementSet> move_set;

	// These sprites are only for animated objects and cannot be interacted with
	Common::Array<pyrodactyl::anim::Sprite> background;

	// These sprites fly across the screen randomly
	Common::Array<pyrodactyl::anim::Sprite> fly;

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
	bool CollidingWithObject(pyrodactyl::event::Info &info, Common::String &id);
	bool CollidingWithLevel(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s);
	bool PlayerInCombat(pyrodactyl::event::Info &info);
	void BattleAlert(pyrodactyl::event::Info &info);

	void SortObjectsToDraw();
	void MoveObject(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s);

	void Think(pyrodactyl::event::Info &info, Common::Array<pyrodactyl::event::EventResult> &result,
			   Common::Array<pyrodactyl::event::EventSeqInfo> &end_seq, Common::String &id);

	void DrawObjects(pyrodactyl::event::Info &info);
	void SetCamera();

	bool LayerVisible(pyrodactyl::anim::Sprite *obj);

public:
	// The objects in the level, and the player character
	Common::Array<pyrodactyl::anim::Sprite> objects;

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
	Common::String preview_path;

	// A full rendered image of the level
	pyrodactyl::image::Image img;

	Level() : player_index(0) { reset(); }
	~Level() { reset(); }

	void reset();

	void Camera(int x, int y, int w, int h) {
		camera.x = x;
		camera.y = y;
		camera.w = w;
		camera.h = h;
	}
	Rect Camera() { return camera; }

	void PlayerStop() { objects[player_index].stop(); }

	const Common::String &PlayerID() { return objects[player_index].id(); }
	void PlayerID(const Common::String &ID, const int &X, const int &Y);

	void ShowMap(bool val) { showmap.normal = val; }
	bool ShowMap() { return showmap.current; }

	bool operator()(int i, int j);

	// This calculates the unlocked moves for each sprite in the level, and the visibility of objects
	void CalcProperties(pyrodactyl::event::Info &info);

	// Loading function
	void load(const Common::String &filename, pyrodactyl::event::Info &info, pyrodactyl::event::TriggerSet &game_over,
			  const int &player_x = -1, const int &player_y = -1);

	// One time load called first-time
	void LoadMoves(const Common::String &filename);
	void LoadConst(const Common::String &filename);

	// Used to see if a sprite collides with a rectangle
	void CalcTrigCollide(pyrodactyl::event::Info &info);

	// See if a player clicked on the sprite in contact
	bool ContainsClick(const Common::String &id, const Common::Event &Event);
#if 0
	bool ContainsClick(const Common::String &id, const SDL_Event &Event);
#endif

	// Get index of a sprite in the object array
	pyrodactyl::anim::Sprite *GetSprite(const Common::String &id);

	void HandleEvents(pyrodactyl::event::Info &info, const Common::Event &Event);
#if 0
	void HandleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event);
#endif
	LevelResult internalEvents(pyrodactyl::event::Info &info, Common::Array<pyrodactyl::event::EventResult> &result,
							   Common::Array<pyrodactyl::event::EventSeqInfo> &end_seq, bool EventInProgress);

	void PreDraw();
	void PreDrawObjects(Graphics::ManagedSurface *surf);
	void draw(pyrodactyl::event::Info &info);

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void loadState(rapidxml::xml_node<char> *node);

	void SetUI();
};
} // End of namespace level
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_LEVEL_H
