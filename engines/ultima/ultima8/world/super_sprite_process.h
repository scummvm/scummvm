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

#ifndef ULTIMA8_WORLD_SUPERSPRITEPROCESS_H
#define ULTIMA8_WORLD_SUPERSPRITEPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/point3.h"

namespace Ultima {
namespace Ultima8 {

//! Creates a damaging sprite like a rocket or something, for Crusader
class SuperSpriteProcess : public Process {
private:
	int _shape;
	struct Point3 _nowpt; 	//!< The current x,y,z position (1st pt in struct)
	struct Point3 _nextpt; 	//!< The next x,y,z position (2nd pt in struct)
	struct Point3 _pt3; 	//!< The ?? x,y,z position (3rd pt in struct)
	struct Point3 _startpt;	//!< The start x,y,z position (4th pt in struct)
	struct Point3 _pt5; 	//!< The ?? x,y,z position (5th pt in struct)
	struct Point3 _destpt;	//!< The destination x,y,z position

	uint16 _frame;
	uint16 _fireType;
	uint16 _damage;
	uint16 _source;
	uint16 _target;
	int32 _counter;

	uint16 _item0x77; //!< TODO: what is this?
	uint16 _spriteNo;

	float _xstep;
	float _ystep;
	float _zstep;

	bool _startedAsFiretype9;
	bool _expired;

public:
	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE()

	SuperSpriteProcess();

	//! SuperSpriteProcess Constructor
	//! \param shape The shape to use
	//! \param frame The initial/first frame of the sprite animation
	//! \param sx Start X coord of the sprite in the world
	//! \param sy Start Y coord of the sprite in the world
	//! \param sz Start Z coord of the sprite in the world
	//! \param dx Dest X coord of the sprite in the world
	//! \param dy Dest Y coord of the sprite in the world
	//! \param dz Dest Z coord of the sprite in the world
	//! \param inexact true if the destination is not exactly chosen
	//!
	SuperSpriteProcess(int shape, int frame, int sx, int sy, int sz,
					   int dx, int dy, int dz, uint16 firetype,
					   uint16 damage, uint16 source, uint16 target, bool inexact);

	//! The SuperSpriteProcess destructor
	~SuperSpriteProcess(void) override;

	//! The SpriteProcess run function
	void run() override;

	//! Move the sprite to a new location
	void move(int x, int y, int z);

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

protected:
	bool areaSearch();
	void advanceFrame();
	void hitAndFinish();
	void makeBulletSplash(const Point3 &pt);
	void destroyItemOrTerminate();
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
