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

#ifndef MTROPOLIS_PLUGIN_MTI_DATA_H
#define MTROPOLIS_PLUGIN_MTI_DATA_H

#include "mtropolis/data.h"

namespace MTropolis {

namespace Data {

namespace MTI {

// Known Muppet Treasure Island custom modifiers:
// Shanghai - ???

struct ShanghaiModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;  // Probably "Enable When"
	PlugInTypeTaggedValue unknown2VarRef; // VarRef (Probably tile set)

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct PrintModifier : public PlugInModifierData {
	PlugInTypeTaggedValue executeWhen;
	PlugInTypeTaggedValue unknown1Bool;
	PlugInTypeTaggedValue unknown2Bool;
	PlugInTypeTaggedValue unknown3Bool;
	PlugInTypeTaggedValue filePath;
	PlugInTypeTaggedValue unknown4Bool;

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

} // End of namespace MTI

} // End of namespace Data

} // End of namespace MTropolis

#endif
