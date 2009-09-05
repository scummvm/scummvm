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
 * $URL$
 * $Id$
 */

#include "teenagent/dialog.h"
#include "teenagent/resources.h"
#include "teenagent/scene.h"

namespace TeenAgent {

void Dialog::show(Scene * scene, uint16 addr, uint16 animation, uint16 actor_animation) {
	debug(0, "Dialog::show(%04x, %u)", addr, animation);
	Resources * res = Resources::instance();
	int n = 0;
	Common::String message;
	byte color = 0xd1;
	
	while (n < 4) {
		byte c = res->eseg.get_byte(addr++);
		switch(c) {
		case 0:
			++n;
			if (n == 3) {
				color = color == 0xd1? 0xd0: 0xd1;
				//debug(0, "changing color", message);
			}
			continue;
		case 0xff:
			{
				SceneEvent e(SceneEvent::WaitForAnimation);
				scene->push(e);
			}
			++n;
			continue;
		default:
			if (n > 1) {
				if (!message.empty()) {
					if (animation != 0) {
						SceneEvent e(SceneEvent::PlayAnimation);
						e.animation = animation;
						e.color = 0x84; //4th slot, async animation
						scene->push(e);
					}
					if (actor_animation != 0) {
						SceneEvent e(SceneEvent::PlayAnimation);
						e.animation = actor_animation;
						e.color = 0x80; //1st slot, async animation
						scene->push(e);
					}
					SceneEvent e(SceneEvent::Message);
					e.message = message;
					e.color = color;
					scene->push(e);
				}
				message = (const char *)Resources::instance()->eseg.ptr(addr - 1);
			} else if (n == 1) {
				message += '\n';
				message += (const char *)Resources::instance()->eseg.ptr(addr - 1);
			} else if (n == 0 && message.empty()) {
				message = (const char *)Resources::instance()->eseg.ptr(addr - 1);
			}
			n = 0;
		}
	}
	if (!message.empty()) {
		if (animation != 0) {
			SceneEvent e(SceneEvent::PlayAnimation);
			e.animation = animation;
			e.color = 0x83; //3rd slot, async animation
			scene->push(e);
		} //copy paste ninja was here
		SceneEvent e(SceneEvent::Message);
		e.message = message;
		e.color = color;
		scene->push(e);
	}
}

uint16 Dialog::pop(Scene *scene, uint16 addr, uint16 animation, uint16 actor_animation) {
	debug(0, "Dialog::pop(%04x, %u)", addr, animation);
	Resources * res = Resources::instance();
	uint16 next;
	do {
		next = res->dseg.get_word(addr);
		addr += 2;
	} while (next == 0);
	uint16 next2 = res->dseg.get_word(addr);
	if (next2 != 0xffff)
		res->dseg.set_word(addr - 2, 0);
	show(scene, next, animation, actor_animation);
	return next;
}

} // End of namespace TeenAgent
