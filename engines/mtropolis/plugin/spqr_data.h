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

#ifndef MTROPOLIS_PLUGIN_SPQR_DATA_H
#define MTROPOLIS_PLUGIN_SPQR_DATA_H

#include "mtropolis/data.h"

namespace MTropolis {

namespace Data {

namespace SPQR {

// Known SPQR - The Empire's Darkest Hour custom modifiers:
// fade - ???

struct FadeModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event; // Probably "Enable When"
	PlugInTypeTaggedValue unknown2Event; // Probably "Disable When"
	PlugInTypeTaggedValue unknown3Int;   // Int
	PlugInTypeTaggedValue unknown4Int;   // Int
	PlugInTypeTaggedValue unknown5Int;   // Int

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

} // End of namespace SPQR

} // End of namespace Data

} // End of namespace MTropolis

#endif
