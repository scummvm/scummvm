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

#ifndef ACCESS_NOCTROPOLIS_PLAYER_H
#define ACCESS_NOCTROPOLIS_PLAYER_H

#include "common/scummsys.h"
#include "access/player.h"

namespace Access {

namespace Noctropolis {

class NoctropolisEngine;

class NoctropolisPlayer : public Player {
private:
	NoctropolisEngine *_game;
	Direction _lastDirection;
	AnimationManager *_animManager;
	bool _isStil;

	void updatePlayerDirection();

	void calcManScale() override;
	void calcManScale1();

public:
	NoctropolisPlayer(AccessEngine *vm, bool isStil = false);
	~NoctropolisPlayer();

	void load() override;

	void loadAnimation(int fileNum, int subFile);
	void walk() override;

	void updateTimers() override;

	void synchronize(Common::Serializer &s) override;

	void setDirFromScript(byte newDir) override;
};

} // End of namespace Noctropolis

} // End of namespace Access

#endif /* ACCESS_NOCTROPOLIS_PLAYER_H */
