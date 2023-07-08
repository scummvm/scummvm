
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
public:
	~Walker() override {}

	bool walk_load_walker_and_shadow_series() override;
	machine *walk_initialize_walker() override;
};

} // namespace Burger
} // namespace M4

#endif
