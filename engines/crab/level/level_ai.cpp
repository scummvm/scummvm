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

#include "crab/level/level.h"

namespace Crab {

using namespace TMX;
using namespace pyrodactyl::stat;
using namespace pyrodactyl::anim;
using namespace pyrodactyl::level;
using namespace pyrodactyl::image;
using namespace pyrodactyl::people;
using namespace pyrodactyl::input;
using namespace pyrodactyl::event;

//------------------------------------------------------------------------
// Purpose: Let the level tick along and notify if we go into an exit
//------------------------------------------------------------------------
LevelResult Level::internalEvents(Info &info, Common::Array<EventResult> &result, Common::Array<EventSeqInfo> &end_seq, bool EventInProgress) {
	LevelResult l_result;

	// input.internalEvents();
	CalcTrigCollide(info);

	if (terrain.CollideWithExit(objects[player_index].boundRect(), l_result)) {
		if (PlayerInCombat(info)) {
			inside_exit = true;
		} else if (inside_exit == false) {
			l_result.type = LR_LEVEL;
			return l_result;
		}
	} else {
		inside_exit = false;

		if (PlayerInCombat(info))
			showmap.current = false;
		else
			showmap.current = showmap.normal;
	}

	if (EventInProgress)
		PlayerStop();
	else
		Think(info, result, end_seq, l_result.val);

	if (l_result.val != "")
		info.lastPerson(l_result.val);
	else if (CollidingWithObject(info, l_result.val))
		info.lastPerson(l_result.val);
	else
		info.lastPerson("");

	return l_result;
}

//------------------------------------------------------------------------
// Purpose: Let the AI think and animate the level
//------------------------------------------------------------------------
void Level::Think(Info &info, Common::Array<EventResult> &result,
				  Common::Array<EventSeqInfo> &end_seq, Common::String &id) {
	unsigned int index = 0;
	for (auto i = objects.begin(); i != objects.end(); ++i, ++index) {
		// Only bother if the sprite is visible
		if (i->visible()) {
			// TODO: Find a place where this should be initialized... (SZ)
			if (i->_pathing.grid == NULL)
				i->_pathing.initialize(&pathfindingGrid);

			// If a sprite is dead, nothing else matters
			if (info.state(i->id()) == PST_DYING) {
				if (i->lastFrame()) {
					info.statSet(i->id(), STAT_HEALTH, 0);
					info.state(i->id(), PST_KO);
					i->effectImg(false);
					i->inputStop();
				}
			} else if (info.state(i->id()) != PST_KO) {
				Rect boundRect = i->boundRect();

				i->_pathing.SetPosition(Vector2f((float)(boundRect.x + boundRect.w / 2), (float)boundRect.y + boundRect.h / 2));
				i->_pathing.Update(0);

				// For the AI sprites
				if (index != player_index) {
					switch (info.state(i->id())) {
					case PST_FIGHT: {
						// Only attack if the player is alive
						if (info.state(objects[player_index].id()) < PST_KO)
							i->attack(info, objects[player_index], sc_default);
					} break;
					case PST_FLEE:
						i->flee(info, terrain.area_exit, sc_default);
						break;
					case PST_NORMAL:
						if (i->takingDamage(objects[player_index], sc_default)) {
							if (first_hit) {
								BattleAlert(info);
								first_hit = false;
							}
						}
						/*else
						TraversePath(*i);*/
						break;
					default:
						break;
					}

					// Only do this if the player is alive
					if (info.state(objects[player_index].id()) < PST_KO)
						objects[player_index].exchangeDamage(info, *i, sc_default);
				} else {
					// For the player sprite
					boundRect = i->boundRect();

					i->_pathing.SetPosition(Vector2f((float)(boundRect.x + boundRect.w / 2), (float)boundRect.y + boundRect.h / 2));
					i->_pathing.Update(0);

					i->moveToDestPathfinding(info, sc_default);
				}

				i->internalEvents(info, PlayerID(), result, end_seq);
				MoveObject(info, *i);
			}

			i->animate(info);
		}
	}

	// Background sprites don't move
	for (auto &i : background)
		i.animate(PST_NORMAL);

	// Flier sprites fly across the screen from left to right or vice versa
	// The movement is semi-random
	for (auto &i : fly) {
		i.flyAround(camera, sc_default);
		i.animate(PST_NORMAL);
	}
}

//------------------------------------------------------------------------
// Purpose: Once a player provokes any sprite, call this function
//------------------------------------------------------------------------
void Level::BattleAlert(Info &info) {
	unsigned int index = 0;
	for (auto i = objects.begin(); i != objects.end(); ++i, ++index) {
		if (index != player_index && i->visible() && info.state(i->id()) != PST_KO) {
			switch (info.type(i->id())) {
			case PE_NEUTRAL:
			case PE_HOSTILE:
				info.state(i->id(), PST_FIGHT);
				break;
			case PE_COWARD:
				info.state(i->id(), PST_FLEE);
				break;
			default:
				break;
			}
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Move a sprite according to its velocity
//------------------------------------------------------------------------
void Level::MoveObject(Info &info, pyrodactyl::anim::Sprite &s) {
	// Update x,y coordinates according to velocity
	s.move(sc_default);

	// First check collision with objects and forbidden areas inside a level
	if (CollidingWithLevel(info, s))
		s.resolveCollide();

	// Finally see if we are inside the overall level bounds
	if (!terrain.InsideWalk(s.boundRect()))
		s.resolveInside(terrain.AreaWalk());
}

//------------------------------------------------------------------------
// Purpose: If any other object is trying to kill you, you are in combat
//------------------------------------------------------------------------
bool Level::PlayerInCombat(Info &info) {
	unsigned int index = 0;
	for (auto i = objects.begin(); i != objects.end(); ++i, ++index)
		if (index != player_index && info.state(i->id()) == PST_FIGHT && i->visible())
			return true;

	return false;
}

} // End of namespace Crab
