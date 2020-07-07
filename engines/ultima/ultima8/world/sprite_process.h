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

#ifndef ULTIMA8_WORLD_SPRITEPROCESS_H
#define ULTIMA8_WORLD_SPRITEPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/usecode/intrinsics.h"

namespace Ultima {
namespace Ultima8 {

//! Creates a Sprite. Animates it. Destroys it.
class SpriteProcess : public Process {
	int     _shape;
	int     _frame;
	int     _firstFrame;
	int     _lastFrame;
	int     _repeats;
	int     _delay;
	int     _x, _y, _z;
	int     _delayCounter;
	bool    _initialized;
public:
	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE()

	SpriteProcess();

	//! SpriteProcess Constructor
	//! \param shape The shape to use
	//! \param frame The initial/first frame of the sprite animation
	//! \param last_frame The last frame of the sprite animation
	//! \param repeats The number of times to play the sprite animation
	//! \param delay The number of runs to wait before incrementing the frame
	//! \param x X coord of the sprite in the world
	//! \param y Y coord of the sprite in the world
	//! \param z Z coord of the sprite in the world
	//! \param delayed_init if true, wait with initialization until first run
	SpriteProcess(int shape, int frame, int last_frame,
	              int repeats, int delay, int x, int y, int z,
	              bool delayed_init = false);

	//! The SpriteProcess destructor
	~SpriteProcess(void) override;

	//! Move the sprite to a new location
	void move(int x, int y, int z);

	//! The SpriteProcess run function
	void run() override;

	INTRINSIC(I_createSprite);
//	INTRINSIC(I_createSpriteEx);

protected:
	void init();

public:
	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
