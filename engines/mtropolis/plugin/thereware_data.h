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

#ifndef MTROPOLIS_PLUGIN_THEREWARE_DATA_H
#define MTROPOLIS_PLUGIN_THEREWARE_DATA_H

#include "mtropolis/data.h"

namespace MTropolis {

namespace Data {

namespace Thereware {

// Known Thereware custom modifiers:
// * Alpha Kit:
// - Alpha: Alpha channel support for graphics and mToons
// - AlphaMatic: Automatically creates alpha information on graphics
//
// * HotText Kit:
// - HotText: Hypertext and scrolling text
// - KeyScroll: Keyboard scrolling for HotText
//
// * Spinner Kit:
// - Spinner: Effect for spinning an element with depth/two-sidedness
// - TurnOver: Like spinner, but specifically for 180 degree spins
// - Flipper: reflect graphics elements around horizontal or vertical axis
// - Squish: Squishing and elasticity for images
//
// * Rotator Kit:
// - Rotator: Rotation of graphics and mToons
// - Tracker: Track mouse or elements as a rotation angle source
//
// * WrapAround Kit:
// - WrapAround: wraparound effect for graphics and mToons
// - EasyScroller: various detail controls for scrolling behavior
//
// * Quick Kit:
// - Flasher: Lets objects blink on the screen
// - Snapper: Creates an invisible grid that elements can snap onto when dragged
// - Conductor: Forward messages to all the children of an element
// - Randomizer: Use random numbers without Miniscript
//
// * Mercury Kit:
// - DoubleClick: Messages on doubleclick. Can also do the same as the Counter messenger.
// - MouseTrap: Blocks/filters messages for a certain time period.
// - RandomTimer: Allows Miniscript altercations to the Timer messenger
//   Same as FlexiTimer in FixIt Kit
// - Repeater: Timer messenger that adds a certain delay before looping 
// - Counter: Sends a message after a certain number of incoming messages
// - TimeLoop: Sends message a certain number of times
//
// * System Kit:
// - Platform: Indicates computer type and OS
//
// * FixIt Kit:
// - GoThere: Moves an element to an arbitrary point on the screen. 
//   Superceded by the Point Motion modifier in mTropolis 2.0.
// - FlexiTimer: Allows Miniscript altercations to the Timer messenger
//	 Same as RandomTimer in Mercury Kit
//
//
// While those Kits were available individually, for mTropolis 2.0
// they were also available as a complete bundle in a single plugin,
// the Thereware Pro Kit.


struct ConductorModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Null;
	PlugInTypeTaggedValue unknown2Null;
	PlugInTypeTaggedValue unknown3Int;
	PlugInTypeTaggedValue unknown4Null;
	PlugInTypeTaggedValue unknown5Null;
	PlugInTypeTaggedValue unknown6Null;
	PlugInTypeTaggedValue unknown7Event;
	PlugInTypeTaggedValue unknown8Null;
	PlugInTypeTaggedValue unknown9Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct AlphaMaticModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct RotatorModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Bool;
	PlugInTypeTaggedValue unknown4Int;
	PlugInTypeTaggedValue unknown5Float;
	PlugInTypeTaggedValue unknown6Bool;
	PlugInTypeTaggedValue unknown7Point;
	PlugInTypeTaggedValue unknown8Int;
	PlugInTypeTaggedValue unknown9Bool;
	PlugInTypeTaggedValue unknown10Int;
	PlugInTypeTaggedValue unknown11Event;
	PlugInTypeTaggedValue unknown12Label;
	PlugInTypeTaggedValue unknown13Null;
	PlugInTypeTaggedValue unknown14Int;
	PlugInTypeTaggedValue unknown15Point;
	PlugInTypeTaggedValue unknown16Point;
	PlugInTypeTaggedValue unknown17Point;
	PlugInTypeTaggedValue unknown18Bool;
	PlugInTypeTaggedValue unknown19Point;
	PlugInTypeTaggedValue unknown20Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct TrackerModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Label;
	PlugInTypeTaggedValue unknown4Int;
	PlugInTypeTaggedValue unknown5Int;
	PlugInTypeTaggedValue unknown6Label;
	PlugInTypeTaggedValue unknown7Bool;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};


struct DoubleClickModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1UniversalTime;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Event;
	PlugInTypeTaggedValue unknown4Null;
	PlugInTypeTaggedValue unknown5Label;
	PlugInTypeTaggedValue unknown6Int;
	PlugInTypeTaggedValue unknown7Int;

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct MouseTrapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1UniversalTime;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Event;
	PlugInTypeTaggedValue unknown4Event;
	PlugInTypeTaggedValue unknown5Null;
	PlugInTypeTaggedValue unknown6Label;
	PlugInTypeTaggedValue unknown7Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct WrapAroundModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Point;
	PlugInTypeTaggedValue unknown4Int;
	PlugInTypeTaggedValue unknown5Bool;
	PlugInTypeTaggedValue unknown6Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct EasyScrollerModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Int;
	PlugInTypeTaggedValue unknown4Int;
	PlugInTypeTaggedValue unknown5Label;
	PlugInTypeTaggedValue unknown6Int;
	PlugInTypeTaggedValue unknown7Int;
	PlugInTypeTaggedValue unknown8Int;
	PlugInTypeTaggedValue unknown9Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct TimeLoopModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Event;
	PlugInTypeTaggedValue unknown4Null;
	PlugInTypeTaggedValue unknown5Label;
	PlugInTypeTaggedValue unknown6Int;
	PlugInTypeTaggedValue unknown7Int;
	PlugInTypeTaggedValue unknown8UniversalTime;
	PlugInTypeTaggedValue unknown9Bool;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct GoThereModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Point;
	PlugInTypeTaggedValue unknown4UniversalTime;
	PlugInTypeTaggedValue unknown5Event;
	PlugInTypeTaggedValue unknown6Label;
	PlugInTypeTaggedValue unknown7Null;
	PlugInTypeTaggedValue unknown8Int;
	PlugInTypeTaggedValue unknown9Int;
	PlugInTypeTaggedValue unknown10Bool;
	PlugInTypeTaggedValue unknown11Bool;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct RandomizerModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Int;
	PlugInTypeTaggedValue unknown2Int;
	PlugInTypeTaggedValue unknown3Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

} // End of namespace Thereware

} // End of namespace Data

} // End of namespace MTropolis

#endif
