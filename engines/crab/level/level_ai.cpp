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
LevelResult Level::internalEvents(Info &info, Common::Array<EventResult> &result, Common::Array<EventSeqInfo> &endSeq, bool eventInProgress) {
	LevelResult lResult;

	// input.internalEvents();
	calcTrigCollide(info);

	if (_terrain.collideWithExit(_objects[_playerIndex].boundRect(), lResult)) {
		if (playerInCombat(info)) {
			_insideExit = true;
		} else if (_insideExit == false) {
			lResult._type = LR_LEVEL;
			return lResult;
		}
	} else {
		_insideExit = false;

		if (playerInCombat(info))
			_showmap._current = false;
		else
			_showmap._current = _showmap._normal;
	}

	if (eventInProgress)
		playerStop();
	else
		think(info, result, endSeq, lResult._val);

	if (lResult._val != "")
		info.lastPerson(lResult._val);
	else if (collidingWithObject(info, lResult._val))
		info.lastPerson(lResult._val);
	else
		info.lastPerson("");

	return lResult;
}

//------------------------------------------------------------------------
// Purpose: Let the AI think and animate the level
//------------------------------------------------------------------------
void Level::think(pyrodactyl::event::Info &info, Common::Array<pyrodactyl::event::EventResult> &result,
				  Common::Array<pyrodactyl::event::EventSeqInfo> &endSeq, Common::String &id) {
	unsigned int index = 0;
	for (auto i = _objects.begin(); i != _objects.end(); ++i, ++index) {
		// Only bother if the sprite is visible
		if (i->visible()) {
			// TODO: Find a place where this should be initialized... (SZ)
			if (i->_pathing._grid == NULL)
				i->_pathing.initialize(&_pathfindingGrid);

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

				i->_pathing.setPosition(Vector2f((float)(boundRect.x + boundRect.w / 2), (float)boundRect.y + boundRect.h / 2));
				i->_pathing.update(0);

				// For the AI sprites
				if (index != _playerIndex) {
					switch (info.state(i->id())) {
					case PST_FIGHT: {
						// Only attack if the player is alive
						if (info.state(_objects[_playerIndex].id()) < PST_KO)
							i->attack(info, _objects[_playerIndex], _scDefault);
					} break;
					case PST_FLEE:
						i->flee(info, _terrain._areaExit, _scDefault);
						break;
					case PST_NORMAL:
						if (i->takingDamage(_objects[_playerIndex], _scDefault)) {
							if (_firstHit) {
								battleAlert(info);
								_firstHit = false;
							}
						}
						/*else
						TraversePath(*i);*/
						break;
					default:
						break;
					}

					// Only do this if the player is alive
					if (info.state(_objects[_playerIndex].id()) < PST_KO)
						_objects[_playerIndex].exchangeDamage(info, *i, _scDefault);
				} else {
					// For the player sprite
					boundRect = i->boundRect();

					i->_pathing.setPosition(Vector2f((float)(boundRect.x + boundRect.w / 2), (float)boundRect.y + boundRect.h / 2));
					i->_pathing.update(0);

					i->moveToDestPathfinding(info, _scDefault);
				}

				i->internalEvents(info, playerId(), result, endSeq);
				moveObject(info, *i);
			}

			i->animate(info);
		}
	}

	// Background sprites don't move
	for (auto &i : _background)
		i.animate(PST_NORMAL);

	// Flier sprites fly across the screen from left to right or vice versa
	// The movement is semi-random
	for (auto &i : _fly) {
		i.flyAround(_camera, _scDefault);
		i.animate(PST_NORMAL);
	}
}

//------------------------------------------------------------------------
// Purpose: Once a player provokes any sprite, call this function
//------------------------------------------------------------------------
void Level::battleAlert(pyrodactyl::event::Info &info) {
	unsigned int index = 0;
	for (auto i = _objects.begin(); i != _objects.end(); ++i, ++index) {
		if (index != _playerIndex && i->visible() && info.state(i->id()) != PST_KO) {
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
void Level::moveObject(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s) {
	// Update x,y coordinates according to velocity
	s.move(_scDefault);

	// First check collision with objects and forbidden areas inside a level
	if (collidingWithLevel(info, s))
		s.resolveCollide();

	// Finally see if we are inside the overall level bounds
	if (!_terrain.insideWalk(s.boundRect()))
		s.resolveInside(_terrain.areaWalk());
}

//------------------------------------------------------------------------
// Purpose: If any other object is trying to kill you, you are in combat
//------------------------------------------------------------------------
bool Level::playerInCombat(pyrodactyl::event::Info &info) {
	unsigned int index = 0;
	for (auto i = _objects.begin(); i != _objects.end(); ++i, ++index)
		if (index != _playerIndex && info.state(i->id()) == PST_FIGHT && i->visible())
			return true;

	return false;
}

} // End of namespace Crab
