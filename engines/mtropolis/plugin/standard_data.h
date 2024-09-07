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
	CursorModifier();

	bool haveRemoveWhen;

	PlugInTypeTaggedValue applyWhen;
	PlugInTypeTaggedValue removeWhen;
	PlugInTypeTaggedValue cursorIDAsLabel;

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct STransCtModifier : public PlugInModifierData {
	PlugInTypeTaggedValue enableWhen;  // Event
	PlugInTypeTaggedValue disableWhen;	// Event
	PlugInTypeTaggedValue transitionType;	// int
	PlugInTypeTaggedValue transitionDirection;	// int
	PlugInTypeTaggedValue unknown1; // int, seems to always be 1
	PlugInTypeTaggedValue steps;	// int, seems to always be 32
	PlugInTypeTaggedValue duration;    // int, always observed as 60000
	PlugInTypeTaggedValue fullScreen; // bool

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct MediaCueMessengerModifier : public PlugInModifierData {
	enum MessageFlags {
		kMessageFlagImmediate = 0x1,
		kMessageFlagCascade = 0x2,
		kMessageFlagRelay = 0x4,
	};

	enum TriggerTiming {
		kTriggerTimingStart = 0,
		kTriggerTimingDuring = 1,
		kTriggerTimingEnd = 2,
	};

	MediaCueMessengerModifier();

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
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct ObjectReferenceVariableModifier : public PlugInModifierData {
	PlugInTypeTaggedValue setToSourceParentWhen;
	PlugInTypeTaggedValue unknown1;
	PlugInTypeTaggedValue objectPath;

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct ListVariableModifier : public PlugInModifierData {
	enum ContentsType {
		kContentsTypeInteger = 1,
		kContentsTypePoint = 2,
		kContentsTypeRange = 3,
		kContentsTypeFloat = 4,
		kContentsTypeString = 5,
		kContentsTypeObject = 6,
		kContentsTypeVector = 8,
		kContentsTypeBoolean = 9,
	};

	ListVariableModifier();
	~ListVariableModifier();

	uint16 unknown1;
	uint32 contentsType;
	uint8 unknown2[4];

	bool havePersistentData;
	uint32 numValues;
	PlugInTypeTaggedValue *values;

	bool persistentValuesGarbled;

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct SysInfoModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct PanningModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event; // Probably "Enable When"
	PlugInTypeTaggedValue unknown2Event; // Probably "Disable When"
	PlugInTypeTaggedValue unknown3Int;   // Int
	PlugInTypeTaggedValue unknown4Int;   // Int
	PlugInTypeTaggedValue unknown5Int;   // Int

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct FadeModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event; // Probably "Enable When"
	PlugInTypeTaggedValue unknown2Event; // Probably "Disable When"
	PlugInTypeTaggedValue unknown3Int;   // Int
	PlugInTypeTaggedValue unknown4Int;   // Int
	PlugInTypeTaggedValue unknown5Int;   // Int

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

struct NavigateModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct OpenTitleModifier : public PlugInModifierData {
	PlugInTypeTaggedValue executeWhen;
	PlugInTypeTaggedValue pathOrUrl;
	PlugInTypeTaggedValue addToReturnList;

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

} // End of namespace Standard

} // End of namespace Data

} // End of namespace MTropolis

#endif
