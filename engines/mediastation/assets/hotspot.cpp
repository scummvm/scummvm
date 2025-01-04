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

#include "mediastation/assets/hotspot.h"

namespace MediaStation {

Operand Hotspot::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
	switch (methodId) {
	case kMouseActivateMethod: {
		assert(args.empty());
		warning("Hotspot::callMethod(): BuiltInFunction::mouseActivate is not implemented");
		return Operand();
	}

	case kMouseDeactivateMethod: {
		assert(args.empty());
		warning("Hotspot::callMethod(): BuiltInFunction::mouseDeactivate is not implemented");
		return Operand();
	}

	default: {
		error("Got unimplemented method ID %d", methodId);
	}
	}
}

} // End of namespace MediaStation
