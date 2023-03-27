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

#ifndef SCI_VERSION_H
#define SCI_VERSION_H

#include "sci/detection.h"

namespace Sci {

/**
 * Convenience function to obtain the active SCI version. Inline
 * due to frequent calling.
 */
static inline SciVersion getSciVersion() {
	// FIXME: declared in sci/resource/resource.cpp
	extern SciVersion g_sciVersion;

	assert(g_sciVersion != SCI_VERSION_NONE);
	return g_sciVersion;
}

} // End of namespace Sci

#endif // SCI_VERSION_H
