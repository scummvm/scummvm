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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARTREK_AWAYMISSION_H
#define STARTREK_AWAYMISSION_H

// All variables here get cleared to 0 upon starting an away mission.
struct AwayMission {
	int16 mouseX;
	int16 mouseY;
	byte field1c;
	byte field1d;
	byte transitioningIntoRoom; // Set while beaming in or walking into a room. Disables control?
	bool redshirtDead;
	byte activeAction;
	byte activeObject;  // The item that is going to be used on something
	byte passiveObject; // The item that the active item is used on (or the item looked at, etc).

	// If this is true after calling room-specific RDF code, the game will continue to run
	// any "default" code for the event, if any.
	bool rdfStillDoDefaultAction;

	// Bits 0-3 correspond to the crewmen? A bit is set if they're not selectable with
	// the use action?
	byte field24;

	int8 field25[4]; // Sets an object's direction after they finish walking somewhere?

	// Demon Word: nonzero if a rude response was given to the prelate.
	int16 field29;

	byte field33;
	byte field36;
	byte field38;
	byte field3a;

	int16 missionScore;
};
// Size: 0x129 bytes

#endif
