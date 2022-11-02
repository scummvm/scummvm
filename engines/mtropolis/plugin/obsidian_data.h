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

#ifndef MTROPOLIS_PLUGIN_OBSIDIAN_DATA_H
#define MTROPOLIS_PLUGIN_OBSIDIAN_DATA_H

#include "mtropolis/data.h"

namespace MTropolis {

namespace Data {

namespace Obsidian {

// Known Obsidian custom modifiers:
// Movement - Heat/water effects, "fly" behavior (?)
// rectshift - Heat/water/star effects (?)
// xorCheck - Inspiration realm canvas puzzle
// xorMod - Inspiration realm canvas puzzle
// WordMixer - Bureau realm WordMixer terminal
// Dictionary - Bureau realm file cabinet dictionary
// TextWork - Text manipulation operations

struct MovementModifier : public PlugInModifierData {
	PlugInTypeTaggedValue enableWhen;		// Event
	PlugInTypeTaggedValue disableWhen;		// Event
	PlugInTypeTaggedValue dest;				// Point
	PlugInTypeTaggedValue type;				// Bool, seems to always be "false"
	PlugInTypeTaggedValue unknown5Point;	// Point, always (0,0)
	PlugInTypeTaggedValue unknown6Int;		// Int, always 5
	PlugInTypeTaggedValue rate;				// Float
	PlugInTypeTaggedValue frequency;		// Int
	PlugInTypeTaggedValue triggerEvent;
	PlugInTypeTaggedValue unknown10Label;	// Label, always (5,108) which doesn't seem to correspond to anything
	PlugInTypeTaggedValue unknown11Null;	// Null, possibly message payload
	PlugInTypeTaggedValue unknown12Int;		// Int, always 3, possibly message flags

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct RectShiftModifier : public PlugInModifierData {
	PlugInTypeTaggedValue enableWhen; // Event, enable when
	PlugInTypeTaggedValue disableWhen; // Event, disable when
	PlugInTypeTaggedValue direction;	// Int, 4 = horizontal, 1 = vertical

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct TextWorkModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct DictionaryModifier : public PlugInModifierData {
	PlugInTypeTaggedValue str;
	PlugInTypeTaggedValue index;

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct WordMixerModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct XorModModifier : public PlugInModifierData {
	PlugInTypeTaggedValue enableWhen; // Probably "enable when"
	PlugInTypeTaggedValue disableWhen; // Probably "disable when"
	PlugInTypeTaggedValue shapeID;

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct XorCheckModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event; // Probably "enable when"
	PlugInTypeTaggedValue unknown2Event; // Probably "disable when"

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

} // End of namespace Obsidian

} // End of namespace Data

} // End of namespace MTropolis

#endif
