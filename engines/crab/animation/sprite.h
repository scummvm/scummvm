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
	Common::String id;

	// The position of the sprite
	Vector2i pos;

	// The velocity of the sprite, target velocity is our maximum velocity
	Vector2f vel, target;

	// The image of the sprite and it's dimensions
	ImageKey image;
	Vector2i img_size;

	// Clip is the portion of the sprite map to be drawn
	Rect clip;

	// The hit boxes of the character - v is vulnerable hit box, d is damage hit box
	Rect box_v, box_d;

	// The direction the sprite is facing
	Direction dir;

	// The currently playing image effect
	ImageEffect img_eff;

	// The complete animation set for the sprite
	AnimSet anim_set;

	// The conditions for sprite visibility
	pyrodactyl::event::TriggerSet visible;

	// Current sprite combo and input for the sprite
	pyrodactyl::input::FightInput input;

	// Have we done damage for this frame - used to avoid repeated damage for the same frame
	bool damage_done;

	// Dialog shown without events
	PopUpCollection popup;

protected:
	void ResetFrame(const pyrodactyl::people::PersonState &pst);
	bool FightCollide(Rect hitbox, Rect enemy_bounds, Range &range, const pyrodactyl::ai::SpriteConstant &sc);
	bool DamageValid(Sprite &s, const pyrodactyl::ai::SpriteConstant &sc);

	void Clip(const Rect &rect) { clip = rect; }
	void BoxV(const Rect &rect) { box_v = rect; }
	void BoxD(const Rect &rect) { box_d = rect; }

public:
	// The AI data for the sprite
	pyrodactyl::ai::SpriteAIData ai_data;
	PathfindingAgent pathing;

	// The modifier applied to the sprite velocity
	Vector2f vel_mod;

	// The layer associated with the sprite (used to show/hide sprite according to auto hide layers)
	int layer;

	// Is the mouse hovering over this sprite?
	bool hover;

	// The list of collisions currently taking place with the sprite
	Common::List<CollisionData> collide_data;

	Sprite();
	~Sprite() {}

	void Visible(bool val) { visible.Result(val); }
	bool Visible() { return visible.Result(); }

	void CalcProperties(pyrodactyl::event::Info &info);

	void X(int X) { pos.x = X; }
	void Y(int Y) { pos.y = Y; }

	int X() { return pos.x; }
	int Y() { return pos.y; }

	void WalkPattern(const pyrodactyl::ai::MovementSet &set) { ai_data.walk = set; }

	void Move(const pyrodactyl::ai::SpriteConstant &sc);

	// Resolve collisions for polygons we want to be outside of
	void ResolveCollide();

	// Resolve collisions for the walk rectangle
	void ResolveInside(Rect collider);

	void Stop() {
		vel.Set();
		target.Set();
		ai_data.dest.active = false;
	}
	void InputStop() { input.Reset(); }

	void XVel(const float &val) { target.x = val * vel_mod.x; }
	void YVel(const float &val) { target.y = val * vel_mod.y; }

	float XVel() { return vel.x; }
	float YVel() { return vel.y; }
	Vector2f Vel() { return vel; }

	const Common::String &ID() { return id; }

	int W() { return clip.w; }
	int H() { return clip.h; }

	const ImageKey &Img() { return image; }
	Rect DialogClip(const pyrodactyl::people::PersonState &state) { return anim_set.walk.DialogClip(state); }
	void DialogUpdateClip(const pyrodactyl::people::PersonState &state) { anim_set.walk.UpdateClip(state); }

	bool PopupShow() { return popup.Show(); }

	Rect BoundRect();
	Rect BoxV();
	Rect BoxD();
	Rect PosRect();
	Rect RangeRect(const Rect &bounds, const Range &range);
	Vector2i CamFocus();

	double DistSq(const Sprite &s);
	void EffectImg(bool vis) { img_eff.visible = vis; }
	bool LastFrame() { return anim_set.fight.LastFrame(); }

	bool TakingDamage(Sprite &sp, const pyrodactyl::ai::SpriteConstant &sc);
	void TakeDamage(pyrodactyl::event::Info &info, Sprite &s);
	void ExchangeDamage(pyrodactyl::event::Info &info, Sprite &s, const pyrodactyl::ai::SpriteConstant &sc);

	void Load(rapidxml::xml_node<char> *node, Common::Array<Common::String> &animations);
	void InternalEvents(pyrodactyl::event::Info &info, const Common::String &player_id,
						Common::Array<pyrodactyl::event::EventResult> &result, Common::Array<pyrodactyl::event::EventSeqInfo> &end_seq);

	void Draw(pyrodactyl::event::Info &info, const Rect &camera);
	void DrawPopup(pyrodactyl::ui::ParagraphData &pop, const Rect &camera);

	void Walk(const bool &reset);
	void Walk(const pyrodactyl::people::PersonState &pst);

	void UpdateFrame(const pyrodactyl::people::PersonState &pst, const bool &repeat = false);
	void AssignFrame();

	void UpdateMove(const pyrodactyl::input::FightAnimationType &combo);
	void ForceUpdateMove(const pyrodactyl::input::FightAnimationType &combo);

	void UpdateMove(const unsigned int &index);
	void ForceUpdateMove(const unsigned int &index);

	// Set sprite destination
	void SetDestPathfinding(const Vector2i &dest, bool reachable = true);

	// Used for sprite movement controlled by player input (usually the player sprite)
	void HandleEvents(pyrodactyl::event::Info &info, const Rect &camera, const pyrodactyl::ai::SpriteConstant &sc, const Common::Event &Event);
#if 0
	void HandleEvents(pyrodactyl::event::Info &info, const Rect &camera, const pyrodactyl::ai::SpriteConstant &sc, const SDL_Event &Event);
#endif

	// This is for sprites with valid object ids
	void Animate(pyrodactyl::event::Info &info);

	// This is for objects without valid ids - like <background> and <fly> sprites
	void Animate(const pyrodactyl::people::PersonState &pst);

	// AI behavior routine for sprites attacking the player
	void Attack(pyrodactyl::event::Info &info, Sprite &target_sp, const pyrodactyl::ai::SpriteConstant &sc);

	// AI behavior routine for sprites running away from the player
	// Requires every exit in the level be accessible
	void Flee(pyrodactyl::event::Info &info, Common::Array<pyrodactyl::level::Exit> &area_exit, const pyrodactyl::ai::SpriteConstant &sc);

	// Used for sprites that fly across semi randomly on the screen
	void FlyAround(const Rect &camera, const pyrodactyl::ai::SpriteConstant &sc);

	// Used for the player destination movement
	void MoveToDest(pyrodactyl::event::Info &info, const pyrodactyl::ai::SpriteConstant &sc);
	void MoveToDestPathfinding(pyrodactyl::event::Info &info, const pyrodactyl::ai::SpriteConstant &sc);

	// Used for AI movement - returns true if at the destination, false otherwise
	bool MoveToLoc(Vector2i &dest, const float &vel, const pyrodactyl::ai::SpriteConstant &sc);
	bool MoveToLocPathfinding(Vector2i &dest, const float &vel, const pyrodactyl::ai::SpriteConstant &sc);

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void LoadState(rapidxml::xml_node<char> *node);
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_SPRITE_H
