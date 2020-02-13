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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA_SHARED_CORE_PARTY_H
#define ULTIMA_SHARED_CORE_PARTY_H

#include "common/array.h"
#include "common/str.h"
#include "common/serializer.h"
#include "ultima/shared/core/character.h"

namespace Ultima {
namespace Shared {

/**
 * Base class for the player's party
 */
class Party {
protected:
	Common::Array<Character *> _characters;
public:
	~Party();

	/**
	 * Add a character to the party
	 */
	void add(Character *c);

	/**
	 * Casting operator for convenient access to the first character
	 */
	operator Character &() const { return *_characters.front(); }

	/**
	 * Casting operator for convenient access to the first character
	 */
	operator Character *() const { return _characters.front(); }

	/**
	 * Synchronize data
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Returns true if the party is dead
	 */
	bool isDead() const;

	/**
	 * Returns true if the party has no food
	 */
	bool isFoodless() const;
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
