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

#ifndef CRAB_SPRITE_H
#define CRAB_SPRITE_H

#include "crab/image/ImageManager.h"
#include "crab/level/LevelExit.h"
#include "crab/music/MusicManager.h"
#include "crab/PathfindingAgent.h"
#include "crab/animation/PopUp.h"
#include "crab/Shape.h"
#include "crab/ai/SpriteConstant.h"
#include "crab/animation/animset.h"
#include "crab/collision.h"
#include "crab/common_header.h"
#include "crab/people/person.h"
#include "crab/ai/spriteai.h"
#include "crab/timer.h"

namespace Crab {

class PathfindingAgent;

namespace pyrodactyl {
namespace anim {

class Sprite {
protected:
	// Used to sync sprite to character
	Common::String _id;

	// The position of the sprite
	Vector2i _pos;

	// The velocity of the sprite, target velocity is our maximum velocity
	Vector2f _vel, _target;

	// The image of the sprite and it's dimensions
	ImageKey _image;
	Vector2i _imgSize;

	// Clip is the portion of the sprite map to be drawn
	Rect _clip;

	// The hit boxes of the character - v is vulnerable hit box, d is damage hit box
	Rect _boxV, _boxD;

	// The direction the sprite is facing
	Direction _dir;

	// The currently playing image effect
	ImageEffect _imgEff;

	// The complete animation set for the sprite
	AnimSet _animSet;

	// The conditions for sprite visibility
	pyrodactyl::event::TriggerSet _visible;

	// Current sprite combo and input for the sprite
	pyrodactyl::input::FightInput _input;

	// Have we done damage for this frame - used to avoid repeated damage for the same frame
	bool _damageDone;

	// Dialog shown without events
	PopUpCollection _popup;

protected:
	void resetFrame(const pyrodactyl::people::PersonState &pst);
	bool fightCollide(Rect hitbox, Rect enemy_bounds, Range &range, const pyrodactyl::ai::SpriteConstant &sc);
	bool damageValid(Sprite &s, const pyrodactyl::ai::SpriteConstant &sc);

	void clip(const Rect &rect) { _clip = rect; }
	void boxV(const Rect &rect) { _boxV = rect; }
	void boxD(const Rect &rect) { _boxD = rect; }

public:
	// The AI data for the sprite
	pyrodactyl::ai::SpriteAIData _aiData;
	PathfindingAgent _pathing;

	// The modifier applied to the sprite velocity
	Vector2f _velMod;

	// The layer associated with the sprite (used to show/hide sprite according to auto hide layers)
	int _layer;

	// Is the mouse hovering over this sprite?
	bool _hover;

	// The list of collisions currently taking place with the sprite
	Common::List<CollisionData> _collideData;

	Sprite();

	~Sprite() {
		_pathing.reset();
	}

	void visible(bool val) {
		_visible.Result(val);
	}

	bool visible() {
		return _visible.Result();
	}

	void calcProperties(pyrodactyl::event::Info &info);

	void x(int X) {
		_pos.x = X;
	}

	void y(int Y) {
		_pos.y = Y;
	}

	int x() {
		return _pos.x;
	}

	int y() {
		return _pos.y;
	}

	void walkPattern(const pyrodactyl::ai::MovementSet &set) {
		_aiData._walk = set;
	}

	void move(const pyrodactyl::ai::SpriteConstant &sc);

	// Resolve collisions for polygons we want to be outside of
	void resolveCollide();

	// Resolve collisions for the walk rectangle
	void resolveInside(Rect collider);

	void stop() {
		_vel.Set();
		_target.Set();
		_aiData._dest._active = false;
	}
	void inputStop() {
		_input.reset();
	}

	void xVel(const float &val) {
		_target.x = val * _velMod.x;
	}
	void yVel(const float &val) {
		_target.y = val * _velMod.y;
	}

	float xVel() {
		return _vel.x;
	}

	float yVel() {
		return _vel.y;
	}

	Vector2f vel() {
		return _vel;
	}

	const Common::String &id() {
		return _id;
	}

	int w() {
		return _clip.w;
	}

	int h() {
		return _clip.h;
	}

	const ImageKey &img() {
		return _image;
	}

	Rect dialogClip(const pyrodactyl::people::PersonState &state) {
		return _animSet._walk.dialogClip(state);
	}

	void dialogUpdateClip(const pyrodactyl::people::PersonState &state) {
		_animSet._walk.updateClip(state);
	}

	bool popupShow() {
		return _popup.show();
	}

	Rect boundRect();
	Rect boxV();
	Rect boxD();
	Rect posRect();
	Rect rangeRect(const Rect &bounds, const Range &range);
	Vector2i camFocus();

	double distSq(const Sprite &s);

	void effectImg(bool vis) {
		_imgEff._visible = vis;
	}

	bool lastFrame() {
		return _animSet._fight.lastFrame();
	}

	bool takingDamage(Sprite &sp, const pyrodactyl::ai::SpriteConstant &sc);
	void takeDamage(pyrodactyl::event::Info &info, Sprite &s);
	void exchangeDamage(pyrodactyl::event::Info &info, Sprite &s, const pyrodactyl::ai::SpriteConstant &sc);

	void load(rapidxml::xml_node<char> *node, Common::Array<Common::String> &animations);
	void internalEvents(pyrodactyl::event::Info &info, const Common::String &player_id,
						Common::Array<pyrodactyl::event::EventResult> &result, Common::Array<pyrodactyl::event::EventSeqInfo> &end_seq);

	void draw(pyrodactyl::event::Info &info, const Rect &camera);
	void drawPopup(pyrodactyl::ui::ParagraphData &pop, const Rect &camera);

	void walk(const bool &reset);
	void walk(const pyrodactyl::people::PersonState &pst);

	void updateFrame(const pyrodactyl::people::PersonState &pst, const bool &repeat = false);
	void assignFrame();

	void updateMove(const pyrodactyl::input::FightAnimationType &combo);
	void forceUpdateMove(const pyrodactyl::input::FightAnimationType &combo);

	void updateMove(const unsigned int &index);
	void forceUpdateMove(const unsigned int &index);

	// Set sprite destination
	void setDestPathfinding(const Vector2i &dest, bool reachable = true);

	// Used for sprite movement controlled by player input (usually the player sprite)
	void handleEvents(pyrodactyl::event::Info &info, const Rect &camera, const pyrodactyl::ai::SpriteConstant &sc, const Common::Event &Event);
#if 0
	void HandleEvents(pyrodactyl::event::Info &info, const Rect &camera, const pyrodactyl::ai::SpriteConstant &sc, const SDL_Event &Event);
#endif

	// This is for sprites with valid object ids
	void animate(pyrodactyl::event::Info &info);

	// This is for objects without valid ids - like <background> and <fly> sprites
	void animate(const pyrodactyl::people::PersonState &pst);

	// AI behavior routine for sprites attacking the player
	void attack(pyrodactyl::event::Info &info, Sprite &targetSp, const pyrodactyl::ai::SpriteConstant &sc);

	// AI behavior routine for sprites running away from the player
	// Requires every exit in the level be accessible
	void flee(pyrodactyl::event::Info &info, Common::Array<pyrodactyl::level::Exit> &areaExit, const pyrodactyl::ai::SpriteConstant &sc);

	// Used for sprites that fly across semi randomly on the screen
	void flyAround(const Rect &camera, const pyrodactyl::ai::SpriteConstant &sc);

	// Used for the player destination movement
	void moveToDest(pyrodactyl::event::Info &info, const pyrodactyl::ai::SpriteConstant &sc);
	void moveToDestPathfinding(pyrodactyl::event::Info &info, const pyrodactyl::ai::SpriteConstant &sc);

	// Used for AI movement - returns true if at the destination, false otherwise
	bool moveToLoc(Vector2i &dest, const float &vel, const pyrodactyl::ai::SpriteConstant &sc);
	bool moveToLocPathfinding(Vector2i &dest, const float &vel, const pyrodactyl::ai::SpriteConstant &sc);

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void loadState(rapidxml::xml_node<char> *node);
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_SPRITE_H
