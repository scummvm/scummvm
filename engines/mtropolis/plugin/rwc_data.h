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

#ifndef MTROPOLIS_PLUGIN_MLINE_DATA_H
#define MTROPOLIS_PLUGIN_MLINE_DATA_H

#include "mtropolis/data.h"

namespace MTropolis {

namespace Data {

namespace RWC {

// Known Real Wild Child! Australian Rock Music 1950s-90s custom modifiers:
// - Invalidate
// - Ripple
// - Spraycan
// - ThighBlaster


struct ThighBlasterModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

} // End of namespace RWC

} // End of namespace Data

} // End of namespace MTropolis

#endif
