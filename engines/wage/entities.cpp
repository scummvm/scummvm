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
 */

#include "wage/wage.h"
#include "wage/entities.h"
#include "wage/design.h"

#include "common/memstream.h"

namespace Wage {

void Designed::setDesignBounds(Common::Rect *bounds) {
	_designBounds = new Common::Rect(*bounds);
	_design->setBounds(bounds);
}

Scene::Scene(String name, Common::SeekableReadStream *data) {
	_name = name;
	_design = new Design(data);

	setDesignBounds(readRect(*data));
	_worldY = data->readSint16BE();
	_worldX = data->readSint16BE();
	_blocked[Scene::NORTH] = (data->readByte() != 0);
	_blocked[Scene::SOUTH] = (data->readByte() != 0);
	_blocked[Scene::EAST] = (data->readByte() != 0);
	_blocked[Scene::WEST] = (data->readByte() != 0);
	_soundFrequency = data->readSint16BE();
	_soundType = data->readByte();
	data->readByte(); // unknown
	_messages[Scene::NORTH] = readPascalString(*data);
	_messages[Scene::SOUTH] = readPascalString(*data);
	_messages[Scene::EAST] = readPascalString(*data);
	_messages[Scene::WEST] = readPascalString(*data);
	_soundName = readPascalString(*data);
}

Obj::Obj(String name, Common::SeekableReadStream *data) : _currentOwner(NULL), _currentScene(NULL) {
	_name = name;
	_design = new Design(data);

	setDesignBounds(readRect(*data));

	int16 namePlural = data->readSint16BE();

	if (namePlural == 256)
		_namePlural = true; // TODO: other flags?
	else if (namePlural == 0)
		_namePlural = false;
	else
		error("Obj <%s> had weird namePlural set", name.c_str());

	if (data->readSint16BE() != 0)
		error("Obj <%s> had short set", name.c_str());

	if (data->readByte() != 0)
		error("Obj <%s> had byte set", name.c_str());

	_accuracy = data->readByte();
	_value = data->readByte();
	_type = data->readSByte();
	_damage = data->readByte();
	_attackType = data->readSByte();
	_numberOfUses = data->readSint16BE();
	int16 returnTo = data->readSint16BE();
	if (returnTo == 256) // TODO any other possibilities?
		_returnToRandomScene = true;
	else if (returnTo == 0)
		_returnToRandomScene = false;
	else
		error("Obj <%s> had weird returnTo set", name.c_str());

	_sceneOrOwner = readPascalString(*data);
	_clickMessage = readPascalString(*data);
	_operativeVerb = readPascalString(*data);
	_failureMessage = readPascalString(*data);
	_useMessage = readPascalString(*data);
	_sound = readPascalString(*data);
}

Chr::Chr(String name, Common::SeekableReadStream *data) {
	_name = name;
	_design = new Design(data);

	setDesignBounds(readRect(*data));

	_physicalStrength = data->readByte();
	_physicalHp = data->readByte();
	_naturalArmor = data->readByte();
	_physicalAccuracy = data->readByte();

	_spiritualStength = data->readByte();
	_spiritialHp = data->readByte();
	_resistanceToMagic = data->readByte();
	_spiritualAccuracy = data->readByte();

	_runningSpeed = data->readByte();
	_rejectsOffers = data->readByte();
	_followsOpponent = data->readByte();

	data->readSByte(); // TODO: ???
	data->readSint32BE(); // TODO: ???

	_weaponDamage1 = data->readByte();
	_weaponDamage2 = data->readByte();

	data->readSByte(); // TODO: ???

	if (data->readSByte() == 1)
		_playerCharacter = true;
	_maximumCarriedObjects = data->readByte();
	_returnTo = data->readSByte();

	_winningWeapons = data->readByte();
	_winningMagic = data->readByte();
	_winningRun = data->readByte();
	_winningOffer = data->readByte();
	_losingWeapons = data->readByte();
	_losingMagic = data->readByte();
	_losingRun = data->readByte();
	_losingOffer = data->readByte();

	_gender = data->readSByte();
	if (data->readSByte() == 1)
		_nameProperNoun = true;

	_initialScene = readPascalString(*data);
	_nativeWeapon1 = readPascalString(*data);
	_operativeVerb1 = readPascalString(*data);
	_nativeWeapon2 = readPascalString(*data);
	_operativeVerb2 = readPascalString(*data);

	_initialComment = readPascalString(*data);
	_scoresHitComment = readPascalString(*data);
	_receivesHitComment = readPascalString(*data);
	_makesOfferComment = readPascalString(*data);
	_rejectsOfferComment = readPascalString(*data);
	_acceptsOfferComment = readPascalString(*data);
	_dyingWords = readPascalString(*data);

	_initialSound = readPascalString(*data);
	_scoresHitSound = readPascalString(*data);
	_receivesHitSound = readPascalString(*data);
	_dyingSound = readPascalString(*data);

	_weaponSound1 = readPascalString(*data);
	_weaponSound2 = readPascalString(*data);
}

} // End of namespace Wage
