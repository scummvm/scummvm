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

#include "common/system.h"
#include "common/timer.h"

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/cga.h"
#include "chamber/script.h"
#include "chamber/resdata.h"

namespace Chamber {


void AnimateGauss(unsigned char *target) {
	unsigned char *sprite;
	unsigned char phase = Rand() % 4;
	if (phase == script_byte_vars.gauss_phase)
		phase = (phase + 1) % 4;
	script_byte_vars.gauss_phase = phase;
	sprite = gauss_data + 8 + phase * (8 * 30);
	CGA_Blit(sprite, 8, 8, 30, target, 80); /*draw to 0:4*/
}

void timerCallback(void *refCon) {
	script_byte_vars.timer_ticks++;
	if (!script_byte_vars.game_paused) {
		script_word_vars.timer_ticks2 = Swap16(Swap16(script_word_vars.timer_ticks2) + 1);
#if 1
		AnimateGauss(frontbuffer);
#endif
	}
}

void InitTimer(void) {
	g_system->getTimerManager()->installTimerProc(&timerCallback, 1000000, NULL, "mainTimer");
}

void UninitTimer(void) {
	g_system->getTimerManager()->removeTimerProc(&timerCallback);
}

} // End of namespace Chamber
