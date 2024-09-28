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
#ifndef DARKSEED_ANIMATION_H
#define DARKSEED_ANIMATION_H

#include "darkseed/objects.h"
#include "darkseed/player.h"
namespace Darkseed {

class Animation {
private:
	Player *_player = nullptr;
	Objects &_objectVar;

public:
	int _animIndexTbl[30];
	int _spriteAnimCountdownTimer[30];

	bool _isPlayingAnimation_maybe = false;
	uint16 _otherNspAnimationType_maybe = 0;

	bool _scaleSequence = false;
	bool _objRestarted = false;
	bool _frameAdvanced = false;

	int _nsp_sprite_scaling_y_position = 0;

	void updateAnimation();

	void advanceAnimationFrame(int nspAminIdx);

	void dcopanim();
	void sargoanim();
	void keeperanim();
	void stuffPlayer();
	void libanim(bool pickingUpReservedBook);

public:
	explicit Animation(Player *player, Objects &objectVar) : _player(player), _objectVar(objectVar) {}
	void setupOtherNspAnimation(int nspAnimIdx, int animId);
};

} // End of namespace Darkseed

#endif // DARKSEED_ANIMATION_H
