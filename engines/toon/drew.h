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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, MojoTouch has
 * exclusively licensed this code on March 23th, 2024, to be used in
 * closed-source products.
 * Therefore, any contributions (commits) to it will also be dual-licensed.
 *
 */

#ifndef TOON_DREW_H
#define TOON_DREW_H

#include "toon/character.h"

namespace Toon {

class ToonEngine;

class CharacterDrew : public Character {
public:
	CharacterDrew(ToonEngine *vm);
	~CharacterDrew() override;
	bool setupPalette() override;
	void playStandingAnim() override;
	void setPosition(int16 x, int16 y) override;
	void resetScale() override;
	void update(int32 timeIncrement) override;
	void playWalkAnim(int32 start, int32 end) override;
	int32 getRandomIdleAnim() override;
protected:
	int32 _currentScale;
};

} // End of namespace Toon

#endif
