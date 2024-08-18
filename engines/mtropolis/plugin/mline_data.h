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

namespace MLine {

// Known mLine custom modifiers:
// * Launchme
// - URLLaunch: Launch/Open external files and applications
//
// * Database
// - Database (???)


struct MLineLauncherModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1String;
	PlugInTypeTaggedValue unknown2String;
	PlugInTypeTaggedValue unknown3String;
	PlugInTypeTaggedValue unknown4Int;
	PlugInTypeTaggedValue unknown5Int;
	PlugInTypeTaggedValue unknown6Bool;
	PlugInTypeTaggedValue unknown7Event;
	PlugInTypeTaggedValue unknown8Bool;
	PlugInTypeTaggedValue unknown9Bool;
	PlugInTypeTaggedValue unknown10Bool;
	PlugInTypeTaggedValue unknown11Bool;
	PlugInTypeTaggedValue unknown12Bool;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

} // End of namespace MLine

} // End of namespace Data

} // End of namespace MTropolis

#endif
