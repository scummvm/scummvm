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

#include "crab/animation/sprite.h"

namespace Crab {

using namespace pyrodactyl::ai;
using namespace pyrodactyl::anim;
using namespace pyrodactyl::input;
using namespace pyrodactyl::event;

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
Sprite::Sprite() : _imgSize(1, 1), _velMod(1.0f, 1.0f) {
	_dir = DIRECTION_LEFT;
	_layer = -1;
	_image = 0;
	visible(true);
	_damageDone = false;
	_hover = false;
	// _pathing.SetSprite(this);
}

//------------------------------------------------------------------------
// Purpose: Load sprite from XML and animations from the index of all animation files
//------------------------------------------------------------------------
void Sprite::load(rapidxml::xml_node<char> *node, Common::Array<Common::String> &animations) {
	using namespace pyrodactyl::image;

	if (nodeValid(node)) {
		loadStr(_id, "id", node);

		// The amount by which the sprite x,y should be multiplied
		int multiply = 1;
		loadNum(multiply, "multiply", node, false);

		loadNum(_pos.x, "x", node);
		loadNum(_pos.y, "y", node);

		_pos.x *= multiply;
		_pos.y *= multiply;

		loadImgKey(_image, "img", node);
		loadNum(_layer, "layer", node, false);

		Image dat;
		g_engine->_imageManager->GetTexture(_image, dat);
		_imgSize.x = dat.W();
		_imgSize.y = dat.H();

		unsigned int index = 0;
		loadNum(index, "moveset", node);

		if (index < animations.size())
			_animSet.load(animations[index]);

		_animSet._fight.listAttackMoves(_aiData._fight._attack);

		loadDirection(_dir, node);
		_clip = _animSet._walk.clip(_dir);
		_boxV = _animSet._walk.boxV(_dir);

		if (nodeValid("visible", node, false))
			_visible.load(node->first_node("visible"));

		if (nodeValid("movement", node, false))
			_aiData._walk.load(node->first_node("movement"));

		if (nodeValid("popup", node, false))
			_popup.load(node->first_node("popup"));
	}
}

//------------------------------------------------------------------------
// Purpose: Move along x and y axis
//------------------------------------------------------------------------
void Sprite::move(const SpriteConstant &sc) {
	if (_target.x == 0.0f && (_vel.x > -sc._tweening && _vel.x < sc._tweening))
		_vel.x = 0.0f;
	else {
		_vel.x += (_target.x - _vel.x) * sc._tweening;
		_pos.x += _vel.x;
	}

	if (_target.y == 0.0f && (_vel.y > -sc._tweening && _vel.y < sc._tweening))
		_vel.y = 0.0f;
	else {
		_vel.y += (_target.y - _vel.y) * sc._tweening;
		_pos.y += _vel.y;
	}
}

//------------------------------------------------------------------------
// Purpose: Resolve collision with shapes excluding the level bounding rectangle
//------------------------------------------------------------------------
void Sprite::resolveCollide() {
	// NOTE: we don't check i->intersect here because we do that in the level functions
	for (auto i = _collideData.begin(); i != _collideData.end(); ++i) {
		Rect bounds = boundRect();
		if (i->type == SHAPE_POLYGON) {
			_pos.x -= i->data.x;
			_pos.y -= i->data.y;
		} else {
			Direction d = bounds.ResolveY(i->data);
			if (d == DIRECTION_UP)
				_pos.y -= i->data.y + i->data.h - _animSet._bounds.y + _animSet.anchorY(_dir) + 1;
			else if (d == DIRECTION_DOWN)
				_pos.y -= i->data.y - bounds.h - _animSet._bounds.y + _animSet.anchorY(_dir) - 1;

			d = bounds.ResolveX(i->data);
			if (d == DIRECTION_LEFT)
				_pos.x -= i->data.x + i->data.w - _animSet._bounds.x + _animSet.anchorX(_dir) + 1;
			else if (d == DIRECTION_RIGHT)
				_pos.x -= i->data.x - bounds.w - _animSet._bounds.x + _animSet.anchorX(_dir) - 1;
		}
	}

	// Clear the collision data
	_collideData.clear();
}

//------------------------------------------------------------------------
// Purpose: Resolve collision with the level bounding rectangle
//------------------------------------------------------------------------
void Sprite::resolveInside(Rect collider) {
	Rect bounds = boundRect();
	Direction d = bounds.ResolveX(collider);

	if (d == DIRECTION_RIGHT)
		_pos.x = collider.x - _animSet._bounds.x + _animSet.anchorX(_dir) + 1;
	else if (d == DIRECTION_LEFT)
		_pos.x = collider.x + collider.w - _animSet._bounds.x - bounds.w + _animSet.anchorX(_dir) - 1;

	bounds = boundRect();
	d = bounds.ResolveY(collider);

	if (d == DIRECTION_DOWN)
		_pos.y = collider.y - _animSet._bounds.y + _animSet.anchorY(_dir) + 1;
	else if (d == DIRECTION_UP)
		_pos.y = collider.y + collider.h - _animSet._bounds.y - bounds.h + _animSet.anchorY(_dir) - 1;
}

//------------------------------------------------------------------------
// Purpose: Various rectangles of the sprite
// Note: Every sprite in the game uses bottom center coordinates:
// draw_x = x - clip.w/2, draw_y = y - clip.h
//------------------------------------------------------------------------

// Used for walking and level geometry collision
Rect Sprite::boundRect() {
	Rect rect;
	rect.x = _pos.x + _animSet._bounds.x - _animSet.anchorX(_dir);
	rect.y = _pos.y + _animSet._bounds.y - _animSet.anchorY(_dir);
	rect.w = _animSet._bounds.w;
	rect.h = _animSet._bounds.h;
	return rect;
}

// Used for fighting
Rect Sprite::boxV() {
	Rect rect;
	rect.x = _pos.x + _boxV.x;
	rect.y = _pos.y + _boxV.y;
	rect.w = _boxV.w;
	rect.h = _boxV.h;
	return rect;
}

// Used for fighting
Rect Sprite::boxD() {
	Rect rect;
	rect.x = _pos.x + _boxD.x;
	rect.y = _pos.y + _boxD.y;
	rect.w = _boxD.w;
	rect.h = _boxD.h;
	return rect;
}

// Used for drawing object notifications over stuff
Rect Sprite::posRect() {
	Rect rect;
	rect.x = _pos.x - _animSet.anchorX(_dir);
	rect.y = _pos.y - _animSet.anchorY(_dir);
	rect.w = _clip.w;
	rect.h = _clip.h;
	return rect;
}

// The range rectangle is relative to the bounding rectangle, and needs to be updated according to the actual position
// Used for enemy sprite to find their required spot to attack the player
Rect Sprite::rangeRect(const Rect &bounds, const Range &range) {
	Rect rect;
	rect.x = bounds.x + range._val[_dir].x;
	rect.y = bounds.y + range._val[_dir].y;
	rect.w = range._val[_dir].w;
	rect.h = range._val[_dir].h;

	return rect;
}

// Used for focusing the camera on the sprite
Vector2i Sprite::camFocus() {
	Vector2i v;
	v.x = _pos.x + _animSet._focus.x;
	v.y = _pos.y + _animSet._focus.y;

	return v;
}

//------------------------------------------------------------------------
// Purpose: Draw the sprite
//------------------------------------------------------------------------
void Sprite::draw(pyrodactyl::event::Info &info, const Rect &camera) {
	using namespace pyrodactyl::image;
	using namespace pyrodactyl::text;

	int x = _pos.x - camera.x - _animSet.anchorX(_dir), y = _pos.y - camera.y - _animSet.anchorY(_dir);

	// Draw the shadow image relative to the bottom center of the sprite
	ShadowOffset sh = _animSet.shadow(_dir);
	if (sh._valid) {
		// Draw using custom offset
		g_engine->_imageManager->draw(x + _clip.w / 2 - _animSet._shadow._size.x + sh.x,
						   y + _clip.h - _animSet._shadow._size.y + sh.y,
						   _animSet._shadow._img);
	} else {
		// Draw using default offset
		g_engine->_imageManager->draw(x + _clip.w / 2 - _animSet._shadow._size.x + _animSet._shadow._offset.x,
						   y + _clip.h - _animSet._shadow._size.y + _animSet._shadow._offset.y,
						   _animSet._shadow._img);
	}

	g_engine->_imageManager->draw(x, y, _image, &_clip, _animSet.flip(_dir));
	_imgEff.draw(x, y);

	if (GameDebug) {
		// Nice boxes for the frames and boxV, boxD
		Rect bounds = boundRect(), vul = boxV(), dmg = boxD(), debugpos = posRect();
		bounds.draw(-camera.x, -camera.y);
		debugpos.draw(-camera.x, -camera.y, 255, 255, 255);
		dmg.draw(-camera.x, -camera.y, 255, 0, 0);
		vul.draw(-camera.x, -camera.y, 0, 0, 255);

		FightMove fm;
		if (_animSet._fight.nextMove(fm)) {
			Rect actual_range;
			actual_range.x = bounds.x + fm._ai._range._val[_dir].x;
			actual_range.y = bounds.y + fm._ai._range._val[_dir].y;
			actual_range.w = fm._ai._range._val[_dir].w;
			actual_range.h = fm._ai._range._val[_dir].h;

			actual_range.draw(-camera.x, -camera.y, 255, 0, 255);
		}

		/*int health = 0;
		info.StatGet(id, pyrodactyl::stat::STAT_HEALTH, health);
		g_engine->_textManager->draw(x + 60.0f, y - 100.0f, NumberToString(health), 0);

		pyrodactyl::people::PersonState state = info.State(id);
		g_engine->_textManager->draw(x, y - 60.0f, NumberToString(state), 0);

		g_engine->_textManager->draw(x + 120.0f, y - 60.0f, NumberToString(_aiData.dest.y), 0);*/

		if (_pathing.m_vSolution.size() > 0) {
			for (auto iter = _pathing.m_vSolution.begin(); iter != _pathing.m_vSolution.end(); ++iter) {
				bool nextToWall = false;
				for (auto neighbor = (*iter)->neighborNodes.begin(); neighbor != (*iter)->neighborNodes.end(); ++neighbor) {
					if ((*neighbor)->GetMovementCost() < 0 || (*neighbor)->GetMovementCost() > 1) {
						nextToWall = true;
						break;
					}
				}

				if (nextToWall)
					(*iter)->GetRect().draw(-camera.x, -camera.y, 0, 0, 0, 254);
				else
					(*iter)->GetRect().draw(-camera.x, -camera.y, 200, 200, 0, 254);
			}

			_pathing.m_pGoalTile->GetRect().draw(-camera.x, -camera.y, 0, 0, 200, 254);
			_pathing.m_pStartTile->GetRect().draw(-camera.x, -camera.y, 0, 200, 0, 254);

			// Draw adjacencies to the goal tile.
			/*	for(auto neighbor = _pathing.m_pGoalTile->neighborNodes.begin(); neighbor != _pathing.m_pGoalTile->neighborNodes.end(); ++neighbor)
				{
				(*neighbor)->GetRect().draw(-camera.x, -camera.y, 200, 0, 0, 254);
				}*/
		}
		Rect destinationRect = Rect((int)_pathing.destination.x - 5,
									(int)_pathing.destination.y - 5,
									10,
									10);
		destinationRect.draw(-camera.x, -camera.y, 0, 200, 0, 254);
	}
}

void Sprite::drawPopup(pyrodactyl::ui::ParagraphData &pop, const Rect &camera) {
	// This is different from draw because we draw the popup centered over the head
	int x = _pos.x - camera.x - _animSet.anchorX(_dir) + (_animSet._bounds.w / 2);
	int y = _pos.y - camera.y - _animSet.anchorY(_dir);

	_popup.draw(x, y, pop, camera);
}

//------------------------------------------------------------------------
// Purpose: Handle the movement in a level for the player only
//------------------------------------------------------------------------
void Sprite::handleEvents(Info &info, const Rect &camera, const SpriteConstant &sc, const Common::Event &Event) {

	int num = 0;
	info.statGet(_id, pyrodactyl::stat::STAT_SPEED, num);
	++num;
	float player_speed = static_cast<float>(num);

	// This is for Diablo style hold-mouse-button-in-direction-of-movement
	// This is only used if - point and click movement isn't being used, cursor is not inside the hud, the cursor is a normal cursor and the mouse is pressed
	if (!_aiData._dest._active && !g_engine->_mouse->inside_hud && !g_engine->_mouse->hover && g_engine->_mouse->Pressed()) {
		// To find where the click is w.r.t sprite, we need to see where it is being drawn
		int x = _pos.x - camera.x - _animSet.anchorX(_dir), y = _pos.y - camera.y - _animSet.anchorY(_dir);

		// Just use the bound rectangle dimensions
		Rect b = boundRect();
		int w = b.w, h = b.h;

		// X axis
		if (g_engine->_mouse->motion.x > x + w)
			xVel(player_speed * sc._walkVelMod.x);
		else if (g_engine->_mouse->motion.x < x)
			xVel(-player_speed * sc._walkVelMod.x);
		else
			xVel(0.0f);

		// Y axis
		if (g_engine->_mouse->motion.y > y + h)
			yVel(player_speed * sc._walkVelMod.y);
		else if (g_engine->_mouse->motion.y < y)
			yVel(-player_speed * sc._walkVelMod.y);
		else
			yVel(0.0f);
	} else // Keyboard movement
	{
		// Disable destination as soon as player presses a direction key
		// X axis
		if (g_engine->_inputManager->State(IG_LEFT)) {
			_aiData._dest._active = false;
			xVel(-player_speed * sc._walkVelMod.x);
		} else if (g_engine->_inputManager->State(IG_RIGHT)) {
			_aiData._dest._active = false;
			xVel(player_speed * sc._walkVelMod.x);
		} else if (!_aiData._dest._active)
			xVel(0.0f);

		// Y axis
		if (g_engine->_inputManager->State(IG_UP)) {
			_aiData._dest._active = false;
			yVel(-player_speed * sc._walkVelMod.y);
		} else if (g_engine->_inputManager->State(IG_DOWN)) {
			_aiData._dest._active = false;
			yVel(player_speed * sc._walkVelMod.y);
		} else if (!_aiData._dest._active)
			yVel(0.0f);

	}

	updateMove(_input.handleEvents(Event));

	// This is to prevent one frame of drawing with incorrect parameters
	animate(info);
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle the movement in a level for the player only
//------------------------------------------------------------------------
void Sprite::handleEvents(Info &info, const Rect &camera, const SpriteConstant &sc, const SDL_Event &Event) {
	int num = 0;
	info.StatGet(id, pyrodactyl::stat::STAT_SPEED, num);
	++num;
	float player_speed = static_cast<float>(num);

	// This is for Diablo style hold-mouse-button-in-direction-of-movement
	// This is only used if - point and click movement isn't being used, cursor is not inside the hud, the cursor is a normal cursor and the mouse is pressed
	if (!_aiData._dest._active && !g_engine->_mouse->inside_hud && !g_engine->_mouse->hover && g_engine->_mouse->Pressed()) {
		// To find where the click is w.r.t sprite, we need to see where it is being drawn
		int x = _pos.x - camera.x - _animSet.anchorX(_dir), y = _pos.y - camera.y - _animSet.anchorY(_dir);

		// Just use the bound rectangle dimensions
		Rect b = boundRect();
		int w = b.w, h = b.h;

		// X axis
		if (g_engine->_mouse->motion.x > x + w)
			xVel(player_speed * sc._walkVelMod.x);
		else if (g_engine->_mouse->motion.x < x)
			xVel(-player_speed * sc._walkVelMod.x);
		else
			xVel(0.0f);

		// Y axis
		if (g_engine->_mouse->motion.y > y + h)
			yVel(player_speed * sc._walkVelMod.y);
		else if (g_engine->_mouse->motion.y < y)
			yVel(-player_speed * sc._walkVelMod.y);
		else
			yVel(0.0f);
	} else // Keyboard movement
	{
		// Disable destination as soon as player presses a direction key
		// X axis
		if (g_engine->_inputManager->State(IG_LEFT)) {
			_aiData._dest._active = false;
			xVel(-player_speed * sc._walkVelMod.x);
		} else if (g_engine->_inputManager->State(IG_RIGHT)) {
			_aiData._dest._active = false;
			xVel(player_speed * sc._walkVelMod.x);
		} else if (!_aiData._dest._active)
			xVel(0.0f);

		// Y axis
		if (g_engine->_inputManager->State(IG_UP)) {
			_aiData._dest._active = false;
			yVel(-player_speed * sc._walkVelMod.y);
		} else if (g_engine->_inputManager->State(IG_DOWN)) {
			_aiData._dest._active = false;
			yVel(player_speed * sc._walkVelMod.y);
		} else if (!_aiData._dest._active)
			yVel(0.0f);
	}

	UpdateMove(input.handleEvents(Event));

	// This is to prevent one frame of drawing with incorrect parameters
	Animate(info);
}
#endif

//------------------------------------------------------------------------
// Purpose: Set destination for sprite movement
//------------------------------------------------------------------------
void Sprite::setDestPathfinding(const Vector2i &dest, bool reachable) {
	_aiData.dest(dest, true);
	_pathing.SetDestination(dest, reachable);
}

//------------------------------------------------------------------------
// Purpose: Walking animation
//------------------------------------------------------------------------
void Sprite::walk(const pyrodactyl::people::PersonState &pst) {
	_imgEff._visible = false;

	bool firstX = true;

	if (_aiData._dest._active) {
		Rect b = boundRect();
		if (_pos.x - _aiData._dest.x > -b.w && _pos.x - _aiData._dest.x < b.w)
			firstX = false;
	}

	bool reset = _animSet._walk.type(_vel, _dir, pst, firstX);

	if (reset)
		_animSet._walk.resetClip(_dir);

	walk(reset);
}

void Sprite::walk(const bool &reset) {
	if (_animSet._walk.updateClip(_dir, reset)) {
		_clip = _animSet._walk.clip(_dir);
		_boxV = _animSet._walk.boxV(_dir);
	}
}

//------------------------------------------------------------------------
// Purpose: Decide which animation to play
//------------------------------------------------------------------------
void Sprite::animate(Info &info) {
	if (_input.Idle())
		walk(info.state(_id));
	else
		updateFrame(info.state(_id));
}

void Sprite::animate(const pyrodactyl::people::PersonState &pst) {
	walk(pst);
}

//------------------------------------------------------------------------
// Purpose: We need to find if the vulnerable area of this sprite collides
// with hitbox (usually the damage area of another sprite)
//------------------------------------------------------------------------
bool Sprite::fightCollide(Rect hitbox, Rect enemyBounds, Range &range, const SpriteConstant &sc) {
	Rect bounds = boundRect();

	if (range._valid) {
		Rect actualRange = rangeRect(bounds, range);

		// The second part is a sanity check so the stray hitbox of a sprite 1000 pixels below does not cause damage
		if (hitbox.Collide(actualRange) && abs(bounds.y + bounds.h - enemyBounds.y - enemyBounds.h) < sc._planeW)
			return true;
	} else {
		if (hitbox.Collide(bounds) && abs(bounds.y + bounds.h - enemyBounds.y - enemyBounds.h) < sc._planeW)
			return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Update the frame info of the sprite
//------------------------------------------------------------------------
void Sprite::updateFrame(const pyrodactyl::people::PersonState &pst, const bool &repeat) {
	FrameUpdateResult res = _animSet._fight.updateFrame(_dir);
	if (res == FUR_SUCCESS) {
		assignFrame();
	} else if (res == FUR_FAIL) {
		_damageDone = false;
		stop();

		if (repeat == false)
			resetFrame(pst);
		else
			_animSet._fight.frameIndex(0);
	}
}

void Sprite::assignFrame() {
	FightAnimFrame faf;
	if (_animSet._fight.curFrame(faf, _dir)) {
		_clip = faf._clip;
		boxV(faf._boxV);
		boxD(faf._boxD);

		_pos.x += faf._delta.x;
		_pos.y += faf._delta.y;

		_input.state = faf._state;
	}
}

//------------------------------------------------------------------------
// Purpose: Update the move info of the player sprite
//------------------------------------------------------------------------
void Sprite::updateMove(const FightAnimationType &combo) {
	if (combo != FA_IDLE) {
		if (_input.Idle())
			forceUpdateMove(combo);
		else {
			FightAnimFrame faf;
			if (_animSet._fight.curFrame(faf, _dir))
				if (faf._branch)
					forceUpdateMove(combo);
		}
	}
}

void Sprite::forceUpdateMove(const FightAnimationType &combo) {
	unsigned int index = _animSet._fight.findMove(combo, _input.state);
	forceUpdateMove(index);
}

//------------------------------------------------------------------------
// Purpose: Update the move info of the AI or player sprite
//------------------------------------------------------------------------
void Sprite::updateMove(const unsigned int &index) {
	if (_input.Idle())
		forceUpdateMove(index);
}

void Sprite::forceUpdateMove(const unsigned int &index) {
	if (_animSet._fight.forceUpdate(index, _input, _dir)) {
		// This sets the sprite input to the current move input
		_animSet._fight.curCombo(_input);

		stop();
		assignFrame();
	} else
		_input.reset();
}
//------------------------------------------------------------------------
// Purpose: Reset the frame info of the sprite
//------------------------------------------------------------------------
void Sprite::resetFrame(const pyrodactyl::people::PersonState &pst) {
	_input.reset();
	walk(true);
	_animSet._fight.reset();

	_boxD.w = 0;
	_boxD.h = 0;
}

//------------------------------------------------------------------------
// Purpose: Check if both sprites in the same plane
//------------------------------------------------------------------------
bool Sprite::damageValid(Sprite &s, const SpriteConstant &sc) {
	// Get the y coordinates where these sprites are standing
	float Y = _pos.y + _clip.h, SY = s._pos.y + s._clip.h;

	if (abs(Y - SY) < sc._planeW)
		return true;

	return false;
}

//------------------------------------------------------------------------
// Purpose: Checks about dealing damage to sprite
//------------------------------------------------------------------------
void Sprite::calcProperties(Info &info) {
	_visible.evaluate(info);
	_animSet._fight.evaluate(info);
}

//------------------------------------------------------------------------
// Purpose: Checks about dealing damage to sprite
//------------------------------------------------------------------------
bool Sprite::takingDamage(Sprite &sp, const SpriteConstant &sc) {
	if (damageValid(sp, sc))
		if (boxV().w > 0 && boxV().h > 0 && sp.boxD().w > 0 && sp.boxD().h > 0)
			if (boxV().Collide(sp.boxD()))
				return true;

	/*Common::String words = NumberToString(BoxV().x) + " " + NumberToString(BoxV().y)+ " " + NumberToString(BoxV().w)
	+ " " + NumberToString(BoxV().h) + "\n" + NumberToString(sp.BoxD().x) + " " + NumberToString(sp.BoxD().y)
	+ " " + NumberToString(sp.BoxD().w) + " " + NumberToString(sp.BoxD().h) + "\n";
	fprintf(stdout,words.c_str());*/

	return false;
}

//------------------------------------------------------------------------
// Purpose: We know we are taking damage, now is the time to update stats
//------------------------------------------------------------------------
void Sprite::takeDamage(Info &info, Sprite &s) {
	using namespace pyrodactyl::stat;
	using namespace pyrodactyl::music;

	FightMove f;
	if (s._animSet._fight.curMove(f) && info.personValid(s.id()) && info.personValid(_id)) {
		int dmg = -1 * (f._eff._dmg + info.personGet(s.id()).stat.val[STAT_ATTACK].cur - info.personGet(_id).stat.val[STAT_DEFENSE].cur);
		if (dmg >= 0)
			dmg = -1;

		info.statChange(_id, STAT_HEALTH, dmg);

		int health = 1;
		info.statGet(_id, STAT_HEALTH, health);

		// Play death animation if dead, hurt animation otherwise
		if (health <= 0 && f._eff._death != -1)
			forceUpdateMove(f._eff._death);
		else if (f._eff._hurt != -1)
			forceUpdateMove(f._eff._hurt);

		g_engine->_musicManager->PlayEffect(f._eff._hit, 0);
		_imgEff = f._eff._img;
	}

	stop();
	s._damageDone = true;
}

//------------------------------------------------------------------------
// Purpose: We have 2 sprites, *this and s.
// Check damage between s.boxV and this->boxD and vice versa
//------------------------------------------------------------------------
void Sprite::exchangeDamage(Info &info, Sprite &s, const SpriteConstant &sc) {
	using namespace pyrodactyl::people;
	using namespace pyrodactyl::stat;

	// This object is taking damage from s
	if (!s._damageDone && takingDamage(s, sc))
		takeDamage(info, s);

	// Is the other sprite taking damage from this sprite?
	if (!_damageDone && s.takingDamage(*this, sc))
		s.takeDamage(info, *this);

	// We change the animation to dying in order to give time to the death animation to play out
	int num = 0;
	info.statGet(s.id(), STAT_HEALTH, num);
	if (num <= 0) {
		info.state(s.id(), PST_DYING);
		info.statChange(s.id(), STAT_HEALTH, 1);
	}

	info.statGet(_id, STAT_HEALTH, num);
	if (num <= 0) {
		info.state(_id, PST_DYING);
		info.statChange(_id, STAT_HEALTH, 1);
	}
}

//------------------------------------------------------------------------
// Purpose: Update status of ambient dialog via popup object
//------------------------------------------------------------------------
void Sprite::internalEvents(Info &info, const Common::String &player_id, Common::Array<EventResult> &result, Common::Array<EventSeqInfo> &end_seq) {
	_popup.internalEvents(info, player_id, result, end_seq);
}

//------------------------------------------------------------------------
// Purpose: Save all sprite positions to save file
//------------------------------------------------------------------------
void Sprite::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	root->append_attribute(doc.allocate_attribute("id", _id.c_str()));
	root->append_attribute(doc.allocate_attribute("x", gStrPool->Get(_pos.x)));
	root->append_attribute(doc.allocate_attribute("y", gStrPool->Get(_pos.y)));
}

//------------------------------------------------------------------------
// Purpose: Load all sprite positions from save file
//------------------------------------------------------------------------
void Sprite::loadState(rapidxml::xml_node<char> *node) {
	loadNum(_pos.x, "x", node);
	loadNum(_pos.y, "y", node);
}

} // End of namespace Crab
