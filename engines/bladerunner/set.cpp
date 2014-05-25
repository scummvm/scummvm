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

#include "bladerunner/set.h"

#include "bladerunner/bladerunner.h"

#include "common/ptr.h"
#include "common/str.h"
#include "common/stream.h"

namespace BladeRunner {

#define kSet0 0x53657430

bool Set::open(const Common::String &name) {
	Common::ScopedPtr<Common::SeekableReadStream> s(_vm->getResourceStream(name));

	uint32 sig = s->readUint32BE();
	if (sig != kSet0)
		return false;

	return true;
}

} // End of namespace BladeRunner
