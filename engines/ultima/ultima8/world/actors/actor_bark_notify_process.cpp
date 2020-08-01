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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/world/actors/actor_bark_notify_process.h"
#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ActorBarkNotifyProcess)

ActorBarkNotifyProcess::ActorBarkNotifyProcess()
	: GumpNotifyProcess() {
}

ActorBarkNotifyProcess::ActorBarkNotifyProcess(uint16 it)
	: GumpNotifyProcess(it) {
}

ActorBarkNotifyProcess::~ActorBarkNotifyProcess(void) {
}


void ActorBarkNotifyProcess::run() {
	Actor *a = getActor(_itemNum);
	if (!a) return;

	if (a->isDead() || !a->hasAnim(Animation::talk))
		return;

	bool doAnim = true;

	// if not standing or talking, don't do talk animation
	Animation::Sequence lastanim = a->getLastAnim();
	if (lastanim != Animation::stand && lastanim != Animation::talk)
		doAnim = false;
	else if (Kernel::get_instance()->getNumProcesses(_itemNum, ActorAnimProcess::ACTOR_ANIM_PROC_TYPE) > 0)
		// if busy, don't do talk animation
		doAnim = false;

	// wait a short while (1-2.5 seconds) before doing the next animation
	// (or even if not doing the animation)
	Process *delayproc = new DelayProcess(30 + (getRandom() % 45));
	ProcId delaypid = Kernel::get_instance()->addProcess(delayproc);

	if (doAnim)
		a->doAnim(Animation::talk, dir_current);

	waitFor(delaypid);
}

void ActorBarkNotifyProcess::saveData(Common::WriteStream *ws) {
	GumpNotifyProcess::saveData(ws);
}

bool ActorBarkNotifyProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!GumpNotifyProcess::loadData(rs, version)) return false;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
