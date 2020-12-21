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

#ifndef WORLD_ACTORS_U8AVATARMOVERPROCESS_H
#define WORLD_ACTORS_U8AVATARMOVERPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/world/actors/avatar_mover_process.h"

namespace Ultima {
namespace Ultima8 {

/**
 * Mover process that replicates the feel of U8 - moving, combat, jumps, etc.
 * Tries turning one quarter turn if movement is blocked.  Running temporarily
 * stops combat and plays some special movement.
 */
class U8AvatarMoverProcess : public AvatarMoverProcess {
public:
	U8AvatarMoverProcess();
	~U8AvatarMoverProcess();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	void tryAttack() override;

protected:
	void handleHangingMode() override;
	void handleCombatMode() override;
	void handleNormalMode() override;
	bool canAttack() override;

	void step(Animation::Sequence action, Direction direction, bool adjusted = false);
	void jump(Animation::Sequence action, Direction direction);

private:
	Animation::Sequence _lastHeadShakeAnim;

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
