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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * $URL$
 * $Id$
 *
 */

#include "wage/wage.h"
#include "wage/entities.h"
#include "wage/design.h"

#include "common/stream.h"

namespace Wage {

void Designed::setDesignBounds(Common::Rect *bounds) {
	_designBounds = new Common::Rect(*bounds);
	_design->setBounds(bounds);
}

Scene::Scene(String name, byte *data, int dataSize) {
	_name = name;
	_design = new Design(data, dataSize);

	Common::MemoryReadStream in(data, dataSize);

	in.skip(in.readUint16BE() - 2); // Skip design.
	setDesignBounds(readRect(in));
	_worldY = in.readSint16BE();
	_worldX = in.readSint16BE();
	_blocked[Scene::NORTH] = (in.readByte() != 0);
	_blocked[Scene::SOUTH] = (in.readByte() != 0);
	_blocked[Scene::EAST] = (in.readByte() != 0);
	_blocked[Scene::WEST] = (in.readByte() != 0);
	_soundFrequency = in.readSint16BE();
	_soundType = in.readByte();
	in.readByte(); // unknown
	_messages[Scene::NORTH] = readPascalString(in);
	_messages[Scene::SOUTH] = readPascalString(in);
	_messages[Scene::EAST] = readPascalString(in);
	_messages[Scene::WEST] = readPascalString(in);
	_soundName = readPascalString(in);
}

Obj::Obj(String name, byte *data, int dataSize) : _currentOwner(NULL), _currentScene(NULL) {
	_name = name;
	_design = new Design(data, dataSize);

	Common::MemoryReadStream in(data, dataSize);

	in.skip(in.readSint16BE() - 2); // Skip design.
	setDesignBounds(readRect(in));

	int16 namePlural = in.readSint16BE();

	if (namePlural == 256)
		_namePlural = true; // TODO: other flags?
	else if (namePlural == 0)
		_namePlural = false;
	else
		error("Obj <%s> had weird namePlural set", name.c_str());

	if (in.readSint16BE() != 0)
		error("Obj <%s> had short set", name.c_str());

	if (in.readByte() != 0)
		error("Obj <%s> had byte set", name.c_str());

	_accuracy = in.readByte();
	_value = in.readByte();
	_type = in.readSByte();
	_damage = in.readByte();
	_attackType = in.readSByte();
	_numberOfUses = in.readSint16BE();
	int16 returnTo = in.readSint16BE();
	if (returnTo == 256) // TODO any other possibilities?
		_returnToRandomScene = true;
	else if (returnTo == 0)
		_returnToRandomScene = false;
	else
		error("Obj <%s> had weird returnTo set", name.c_str());

	_sceneOrOwner = readPascalString(in);
	_clickMessage = readPascalString(in);
	_operativeVerb = readPascalString(in);
	_failureMessage = readPascalString(in);
	_useMessage = readPascalString(in);
	_sound = readPascalString(in);
}

Chr::Chr(String name, byte *data, int dataSize) {
	_name = name;
	_design = new Design(data, dataSize);

	Common::MemoryReadStream in(data, dataSize);

	in.skip(in.readSint16BE() - 2); // Skip design.
	setDesignBounds(readRect(in));

	_physicalStrength = in.readByte();
	_physicalHp = in.readByte();
	_naturalArmor = in.readByte();
	_physicalAccuracy = in.readByte();

	_spiritualStength = in.readByte();
	_spiritialHp = in.readByte();
	_resistanceToMagic = in.readByte();
	_spiritualAccuracy = in.readByte();

	_runningSpeed = in.readByte();
	_rejectsOffers = in.readByte();
	_followsOpponent = in.readByte();

	in.readSByte(); // TODO: ???
	in.readSint32BE(); // TODO: ???

	_weaponDamage1 = in.readByte();
	_weaponDamage2 = in.readByte();

	in.readSByte(); // TODO: ???

	if (in.readSByte() == 1)
		_playerCharacter = true;
	_maximumCarriedObjects = in.readByte();
	_returnTo = in.readSByte();

	_winningWeapons = in.readByte();
	_winningMagic = in.readByte();
	_winningRun = in.readByte();
	_winningOffer = in.readByte();
	_losingWeapons = in.readByte();
	_losingMagic = in.readByte();
	_losingRun = in.readByte();
	_losingOffer = in.readByte();

	_gender = in.readSByte();
	if (in.readSByte() == 1)
		_nameProperNoun = true;

	_initialScene = readPascalString(in);
	_nativeWeapon1 = readPascalString(in);
	_operativeVerb1 = readPascalString(in);
	_nativeWeapon2 = readPascalString(in);
	_operativeVerb2 = readPascalString(in);

	_initialComment = readPascalString(in);
	_scoresHitComment = readPascalString(in);
	_receivesHitComment = readPascalString(in);
	_makesOfferComment = readPascalString(in);
	_rejectsOfferComment = readPascalString(in);
	_acceptsOfferComment = readPascalString(in);
	_dyingWords = readPascalString(in);

	_initialSound = readPascalString(in);
	_scoresHitSound = readPascalString(in);
	_receivesHitSound = readPascalString(in);
	_dyingSound = readPascalString(in);

	_weaponSound1 = readPascalString(in);
	_weaponSound2 = readPascalString(in);
}

} // End of namespace Wage
