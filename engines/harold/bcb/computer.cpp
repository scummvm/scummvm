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
 * along with this source distribution. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/surface.h"

#include <math.h>

#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"

#include "common/textconsole.h"

namespace BCB {

static const int kComputerCompositeTransitionLeft = 36;
static const int kComputerCompositeTransitionTop = 36;
static const int kComputerCompositeTransitionRight = 606;
static const int kComputerCompositeTransitionBottom = 444;

bool BCBEngine::dispatchComputerDetailControls() {
	error("BCB: dispatchComputerDetailControls is not implemented");
}

bool BCBEngine::dispatchComputerMainControls() {
	error("BCB: dispatchComputerMainControls is not implemented");
}

} // End of namespace BCB
