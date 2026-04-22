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

#ifndef MADS_PHANTOM_PHANTOM_H
#define MADS_PHANTOM_PHANTOM_H

#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

class PhantomEngine : public MADSV2Engine {
private:
	static void global_object_sprite();
	static void stop_walker_basic();
	static void stop_walker_tricks();

public:
	PhantomEngine(OSystem *syst, const MADSGameDescription *gameDesc) :
		MADSV2Engine(syst, gameDesc) {}
	~PhantomEngine() override {}

	Common::Error run() override;

	void global_init_code() override;
	void section_music(int section_num) override;
	void global_section_constructor() override;
	void global_daemon_code() override;
	void global_pre_parser_code() override;
	void global_parser_code() override;
	void global_error_code() override;
	void global_room_init() override;
	void global_sound_driver() override;
};

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
