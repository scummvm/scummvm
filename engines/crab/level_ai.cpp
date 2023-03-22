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

#include "crab/level.h"

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
LevelResult Level::InternalEvents(Info &info, std::vector<EventResult> &result, std::vector<EventSeqInfo> &end_seq, bool EventInProgress) {
	LevelResult l_result;

	// input.InternalEvents();
	CalcTrigCollide(info);

	if (terrain.CollideWithExit(objects[player_index].BoundRect(), l_result)) {
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
		info.LastPerson(l_result.val);
	else if (CollidingWithObject(info, l_result.val))
		info.LastPerson(l_result.val);
	else
		info.LastPerson("");

	return l_result;
}

//------------------------------------------------------------------------
// Purpose: Let the AI think and animate the level
//------------------------------------------------------------------------
void Level::Think(Info &info, std::vector<EventResult> &result,
				  std::vector<EventSeqInfo> &end_seq, std::string &id) {
	int index = 0;
	for (auto i = objects.begin(); i != objects.end(); ++i, ++index) {
		// Only bother if the sprite is visible
		if (i->Visible()) {
			// TODO: Find a place where this should be initialized... (SZ)
			if (i->pathing.grid == NULL)
				i->pathing.initialize(&pathfindingGrid);

			// If a sprite is dead, nothing else matters
			if (info.State(i->ID()) == PST_DYING) {
				if (i->LastFrame()) {
					info.StatSet(i->ID(), STAT_HEALTH, 0);
					info.State(i->ID(), PST_KO);
					i->EffectImg(false);
					i->InputStop();
				}
			} else if (info.State(i->ID()) != PST_KO) {
				Rect boundRect = i->BoundRect();

				i->pathing.SetPosition(Vector2f((float)(boundRect.x + boundRect.w / 2), (float)boundRect.y + boundRect.h / 2));
				i->pathing.Update(0);

				// For the AI sprites
				if (index != player_index) {
					switch (info.State(i->ID())) {
					case PST_FIGHT: {
						// Only attack if the player is alive
						if (info.State(objects[player_index].ID()) < PST_KO)
							i->Attack(info, objects[player_index], sc_default);
					} break;
					case PST_FLEE:
						i->Flee(info, terrain.area_exit, sc_default);
						break;
					case PST_NORMAL:
						if (i->TakingDamage(objects[player_index], sc_default)) {
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
					if (info.State(objects[player_index].ID()) < PST_KO)
						objects[player_index].ExchangeDamage(info, *i, sc_default);
				} else {
					// For the player sprite
					Rect boundRect = i->BoundRect();

					i->pathing.SetPosition(Vector2f((float)(boundRect.x + boundRect.w / 2), (float)boundRect.y + boundRect.h / 2));
					i->pathing.Update(0);

					i->MoveToDestPathfinding(info, sc_default);
				}

				i->InternalEvents(info, PlayerID(), result, end_seq);
				MoveObject(info, *i);
			}

			i->Animate(info);
		}
	}

	// Background sprites don't move
	for (auto &i : background)
		i.Animate(PST_NORMAL);

	// Flier sprites fly across the screen from left to right or vice versa
	// The movement is semi-random
	for (auto &i : fly) {
		i.FlyAround(camera, sc_default);
		i.Animate(PST_NORMAL);
	}
}

//------------------------------------------------------------------------
// Purpose: Once a player provokes any sprite, call this function
//------------------------------------------------------------------------
void Level::BattleAlert(Info &info) {
	int index = 0;
	for (auto i = objects.begin(); i != objects.end(); ++i, ++index) {
		if (index != player_index && i->Visible() && info.State(i->ID()) != PST_KO) {
			switch (info.Type(i->ID())) {
			case PE_NEUTRAL:
			case PE_HOSTILE:
				info.State(i->ID(), PST_FIGHT);
				break;
			case PE_COWARD:
				info.State(i->ID(), PST_FLEE);
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
	s.Move(sc_default);

	// First check collision with objects and forbidden areas inside a level
	if (CollidingWithLevel(info, s))
		s.ResolveCollide();

	// Finally see if we are inside the overall level bounds
	if (!terrain.InsideWalk(s.BoundRect()))
		s.ResolveInside(terrain.AreaWalk());
}

//------------------------------------------------------------------------
// Purpose: If any other object is trying to kill you, you are in combat
//------------------------------------------------------------------------
bool Level::PlayerInCombat(Info &info) {
	int index = 0;
	for (auto i = objects.begin(); i != objects.end(); ++i, ++index)
		if (index != player_index && info.State(i->ID()) == PST_FIGHT && i->Visible())
			return true;

	return false;
}
