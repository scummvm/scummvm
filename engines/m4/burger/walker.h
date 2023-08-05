
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

#ifndef M4_BURGER_WALKER_H
#define M4_BURGER_WALKER_H

#include "m4/adv_r/adv_walk.h"

namespace M4 {
namespace Burger {

class Walker : public M4::Walker {
private:
	const char *_name = nullptr;
	int _channel = 0;
	int _room = 0;
	int _vol = 0;
	int32 _trigger = 0;
	bool _flag = false;

	/**
	 * This is called when PLAYER walker code sends system message back to C (used by MAIN PLAYER WALKER ONLY)
	 */
	static void player_walker_callback(frac16 myMessage, machine *sender);

public:
	~Walker() override {}

	bool walk_load_walker_and_shadow_series() override;
	machine *walk_initialize_walker() override;

	void reset_walker_sprites();
	void wilbur_speech(const char *name, int trigger = -1, int room = -1, byte flags = 0,
		int vol = 256, int channel = 1);
	void wilbur_say();
	void wilburs_speech_finished();
};

extern void enable_player();
extern void wilbur_abduct(int trigger);

} // namespace Burger
} // namespace M4

#endif
