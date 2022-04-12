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

#ifndef MTROPOLIS_PLUGIN_STANDARD_DATA_H
#define MTROPOLIS_PLUGIN_STANDARD_DATA_H

#include "mtropolis/data.h"

namespace MTropolis {

namespace Data {

namespace Standard {

struct CursorModifier : public PlugInModifierData {
	uint16 unknown1;
	Event applyWhen;
	uint16 unknown2;
	Event removeWhen;
	uint16 unknown3;
	uint32 cursorID;
	uint8 unknown4[4];

protected:
	DataReadErrorCode load(const PlugInModifier &prefix, DataReader &reader) override;
};

struct STransCtModifier : public PlugInModifierData {
	uint16 unknown1;	// Type tag? (0x17)
	Event unknown2;		// Probably "apply when"
	uint16 unknown3;	// Type tag? (0x17)
	Event unknown4;		// Probably "remove when"
	uint16 unknown5;	// Type tag? (1)
	uint32 unknown6;
	uint16 unknown7;	// Type tag? (1)
	uint32 unknown8;
	uint16 unknown9;	// Type tag? (1)
	uint32 unknown10;
	uint16 unknown11;	// Type tag? (1)
	uint32 unknown12;
	uint16 unknown13;	// Type tag? (1)
	uint32 unknown14;
	uint16 unknown15;	// Type tag? (0x14)
	uint8 unknown16[2];

protected:
	DataReadErrorCode load(const PlugInModifier &prefix, DataReader &reader) override;
};

struct MediaCueMessengerModifier : public PlugInModifierData {
	enum MessageFlags {
		kMessageFlagImmediate = 0x1,
		kMessageFlagCascade = 0x2,
		kMessageFlagRelay = 0x3,
	};

	enum TriggerTiming {
		kTriggerTimingStart = 0,
		kTriggerTimingDuring = 1,
		kTriggerTimingEnd = 2,
	};

	PlugInTypeTaggedValue enableWhen;
	PlugInTypeTaggedValue disableWhen;
	PlugInTypeTaggedValue sendEvent;
	PlugInTypeTaggedValue nonStandardMessageFlags;	// int type, non-standard
	uint16 unknown1;
	uint32 destination;
	uint32 unknown2;
	PlugInTypeTaggedValue with;
	PlugInTypeTaggedValue executeAt;	// May validly be a label, variable, integer, or integer range
	PlugInTypeTaggedValue triggerTiming;	// int type

protected:
	DataReadErrorCode load(const PlugInModifier &prefix, DataReader &reader) override;
};

} // End of namespace Standard

} // End of namespace Data

} // End of namespace MTropolis

#endif
