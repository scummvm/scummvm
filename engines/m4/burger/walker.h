
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
	int _wilburPoof = -1;

	/**
	 * This is called when PLAYER walker code sends system message back to C (used by MAIN PLAYER WALKER ONLY)
	 */
	static void player_walker_callback(frac16 myMessage, machine *sender);

	/**
	 * Says speech randomly
	 */
	void speech_random(int count, int trigger, const char *name1,
		const char *name2, const char *name3 = nullptr,
		const char *name4 = nullptr, const char *name5 = nullptr,
		const char *name6 = nullptr, const char *name7 = nullptr,
		const char *name8 = nullptr, const char *name9 = nullptr);

public:
	~Walker() override {}

	bool walk_load_walker_and_shadow_series() override;
	machine *walk_initialize_walker() override;

	void reset_walker_sprites();
	void wilbur_speech(const char *name, int trigger = -1, int room = -1, byte flags = 0,
		int vol = 256, int channel = 1);
	void wilbur_say();
	void wilburs_speech_finished();
	void wilbur_teleported();

	/**
	 * Takes an array of string constants in the form of "object", "lookline", "takeline", "useline".
	 * The array has to end with four NULLs: NULL, NULL, NULL, NULL
	 * Use NULL in place of a lookline, takeline, or useline if you don't want to this function
	 * to handle it
	 */
	bool wilbur_said(const char *list[][4]);

	void wilbur_speech_random(const char *name1, const char *name2, int trigger = -1) {
		speech_random(2, trigger, name1, name2);
	}
	void wilbur_speech_random(const char *name1, const char *name2,
			const char *name3, int trigger = -1) {
		speech_random(3, trigger, name1, name2, name3);
	}
	void wilbur_speech_random(const char *name1, const char *name2,
		const char *name3, const char *name4, int trigger = -1) {
		speech_random(4, trigger, name1, name2, name3, name4);
	}
	void wilbur_speech_random(const char *name1, const char *name2,
		const char *name3, const char *name4, const char *name5, int trigger = -1) {
		speech_random(5, trigger, name1, name2, name3, name4, name5);
	}
	void wilbur_speech_random(const char *name1, const char *name2,
		const char *name3, const char *name4, const char *name5,
		const char *name6, int trigger = -1) {
		speech_random(6, trigger, name1, name2, name3, name4, name5, name6);
	}
	void wilbur_speech_random(const char *name1, const char *name2,
		const char *name3, const char *name4, const char *name5,
		const char *name6, const char *name7, int trigger = -1) {
		speech_random(7, trigger, name1, name2, name3, name4, name5, name6, name7);
	}

	bool wilbur_parser(const char **list);

	void wilbur_poof();
	void wilbur_unpoof();
};

extern void enable_player();
extern void wilbur_abduct(int trigger);

} // namespace Burger
} // namespace M4

#endif
