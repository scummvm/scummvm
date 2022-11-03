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

#ifndef ACCESS_AMAZON_PLAYER_H
#define ACCESS_AMAZON_PLAYER_H

#include "common/scummsys.h"
#include "access/player.h"

namespace Access {

namespace Amazon {

class AmazonEngine;

class AmazonPlayer : public Player {
private:
	AmazonEngine *_game;
public:
	AmazonPlayer(AccessEngine *vm);

	void load() override;
};

} // End of namespace Amazon

} // End of namespace Access

#endif /* ACCESS_AMAZON_PLAYER_H */
