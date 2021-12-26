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

#ifndef SCI_ENGINE_HOYLE5POKER_H
#define SCI_ENGINE_HOYLE5POKER_H

#include "common/serializer.h"
#include "common/str.h"
#include "sci/engine/object.h"
#include "sci/engine/vm.h"
#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/util.h"

namespace Sci {

reg_t hoyle5PokerEngine(SciArray *data);

} // End of namespace Sci

#endif // SCI_ENGINE_HOYLE5POKER_H
