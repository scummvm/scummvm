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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// We place selector vocab name tables here for any game that doesn't have
// them. This includes the King's Quest IV Demo and LSL3 Demo.

#include "sci/engine/kernel.h"
#include "sci/engine/seg_manager.h"

namespace Sci {

struct SelectorRemap {
	SciVersion minVersion;
	SciVersion maxVersion;
	const char *name;
	uint32 slot;
};

static const char * const sci0Selectors[] = {
	           "y",          "x",         "view",      "loop",        "cel", //  0 -  4
	   "underBits",      "nsTop",       "nsLeft",  "nsBottom",    "nsRight", //  5 -  9
	       "lsTop",     "lsLeft",     "lsBottom",   "lsRight",     "signal", // 10 - 14
	 "illegalBits",      "brTop",       "brLeft",  "brBottom",    "brRight", // 15 - 19
	        "name",        "key",         "time",      "text",   "elements", // 20 - 25
	       "color",       "back",         "mode",     "style",      "state", // 25 - 29
	        "font",       "type",       "window",    "cursor",        "max", // 30 - 34
	        "mark",        "who",      "message",      "edit",       "play", // 35 - 39
	      "number",     "handle",       "client",        "dx",         "dy", // 40 - 44
	   "b-moveCnt",       "b-i1",         "b-i2",      "b-di",    "b-xAxis", // 45 - 49
	      "b-incr",      "xStep",        "yStep", "moveSpeed",  "canBeHere", // 50 - 54
	     "heading",      "mover",         "doit", "isBlocked",     "looper", // 55 - 59
	    "priority",  "modifiers",       "replay",    "setPri",         "at", // 60 - 64
	        "next",       "done",        "width",  "wordFail", "syntaxFail", // 65 - 69
	"semanticFail", "pragmaFail",         "said",   "claimed",      "value", // 70 - 74
	        "save",    "restore",        "title",    "button",       "icon", // 75 - 79
	        "draw",     "delete",            "z"                             // 80 - 82
};

static const char * const sci1Selectors[] = {
	  "parseLang",  "printLang", "subtitleLang",       "size",    "points", // 83 - 87
	    "palette",    "dataInc",       "handle",        "min",       "sec", // 88 - 92
	      "frame",        "vol",          "pri",    "perform",  "moveDone"  // 93 - 97
};

static const char * const sci11Selectors[] = {
	  "topString",      "flags",    "quitGame",     "restart",      "hide", // 98 - 102
	"scaleSignal",     "scaleX",      "scaleY",    "maxScale","vanishingX", // 103 - 107
	 "vanishingY"                                                           // 108
};

#ifdef ENABLE_SCI32
static const char * const sci2Selectors[] = {
	    "plane",           "x",           "y",            "z",     "scaleX", //  0 -  4
	   "scaleY",    "maxScale",    "priority",  "fixPriority",     "inLeft", //  5 -  9
	    "inTop",     "inRight",    "inBottom", "useInsetRect",       "view", // 10 - 14
	     "loop",         "cel",      "bitmap",       "nsLeft",      "nsTop", // 15 - 19
	  "nsRight",    "nsBottom",      "lsLeft",        "lsTop",    "lsRight", // 20 - 25
	 "lsBottom",      "signal", "illegalBits",       "brLeft",      "brTop", // 25 - 29
	  "brRight",    "brBottom",        "name",          "key",       "time", // 30 - 34
	     "text",    "elements",        "fore",         "back",       "mode", // 35 - 39
	    "style",       "state",        "font",         "type",     "window", // 40 - 44
	   "cursor",         "max",        "mark",          "who",    "message", // 45 - 49
	     "edit",        "play",      "number",      "nodePtr",     "client", // 50 - 54
	       "dx",          "dy",   "b-moveCnt",         "b-i1",       "b-i2", // 55 - 59
	     "b-di",     "b-xAxis",      "b-incr",        "xStep",      "yStep", // 60 - 64
	"moveSpeed",  "cantBeHere",     "heading",        "mover",       "doit", // 65 - 69
	"isBlocked",      "looper",   "modifiers",       "replay",     "setPri", // 70 - 74
	       "at",        "next",        "done",        "width", "pragmaFail", // 75 - 79
	  "claimed",       "value",        "save",      "restore",      "title", // 80 - 84
	   "button",        "icon",        "draw",       "delete",  "printLang", // 85 - 89
	     "size",      "points",     "palette",      "dataInc",     "handle", // 90 - 94
	      "min",         "sec",       "frame",          "vol",    "perform", // 95 - 99
	 "moveDone",   "topString",       "flags",     "quitGame",    "restart", // 100 - 104
	     "hide", "scaleSignal",  "vanishingX",   "vanishingY",    "picture", // 105 - 109
	     "resX",        "resY",   "coordType",         "data",       "skip", // 110 - 104
	   "center",         "all",        "show",     "textLeft",    "textTop", // 115 - 119
	"textRight",  "textBottom", "borderColor",    "titleFore",  "titleBack", // 120 - 124
	"titleFont",      "dimmed",    "frameOut",      "lastKey",  "magnifier", // 125 - 129
	 "magPower",    "mirrored",       "pitch",         "roll",        "yaw", // 130 - 134 
	     "left",       "right",         "top",       "bottom",   "numLines"  // 135 - 139
};
#endif

static const SelectorRemap sciSelectorRemap[] = {
	{    SCI_VERSION_0_EARLY,     SCI_VERSION_0_LATE,   "moveDone", 170 },
	{    SCI_VERSION_0_EARLY,     SCI_VERSION_0_LATE,     "points", 316 },
	{    SCI_VERSION_0_EARLY,     SCI_VERSION_0_LATE,      "flags", 368 },
	{    SCI_VERSION_1_EARLY,     SCI_VERSION_1_LATE,    "nodePtr",  44 },
	{     SCI_VERSION_1_LATE,     SCI_VERSION_1_LATE, "cantBeHere",  57 },
	{    SCI_VERSION_1_EARLY,     SCI_VERSION_1_LATE,  "topString", 101 },
	{    SCI_VERSION_1_EARLY,     SCI_VERSION_1_LATE,      "flags", 102 },
	// SCI1.1
	{        SCI_VERSION_1_1,        SCI_VERSION_1_1,    "nodePtr",  41 },
	{        SCI_VERSION_1_1,        SCI_VERSION_1_1, "cantBeHere",  54 },
	{        SCI_VERSION_1_1,        SCI_VERSION_2_1,     "-info-",4103 },
	{ SCI_VERSION_NONE,             SCI_VERSION_NONE,            0,   0 }
};

Common::StringArray Kernel::checkStaticSelectorNames() {
	Common::StringArray names;
	const int offset = (getSciVersion() < SCI_VERSION_1_1) ? 3 : 0;

#ifdef ENABLE_SCI32
	const int count = (getSciVersion() <= SCI_VERSION_1_1) ? ARRAYSIZE(sci0Selectors) + offset : ARRAYSIZE(sci2Selectors);
#else
	const int count = ARRAYSIZE(sci0Selectors) + offset;
#endif
	int countSci1 = ARRAYSIZE(sci1Selectors);
	int countSci11 = ARRAYSIZE(sci11Selectors);

	// Resize the list of selector names and fill in the SCI 0 names.
	names.resize(count);
	if (getSciVersion() <= SCI_VERSION_1_LATE) {
		// Fill selectors 0 - 2 for SCI0 - SCI1 late
		names[0] = "species";
		names[1] = "superClass";
		names[2] = "-info-";
	}

	if (getSciVersion() <= SCI_VERSION_1_1) {
		// SCI0 - SCI11
		for (int i = offset; i < count; i++)
			names[i] = sci0Selectors[i - offset];

		if (getSciVersion() > SCI_VERSION_01) {
			// Several new selectors were added in SCI 1 and later.
			names.resize(count + countSci1);
			for (int i = count; i < count + countSci1; i++)
				names[i] = sci1Selectors[i - count];
		}

		if (getSciVersion() >= SCI_VERSION_1_1) {
			// Several new selectors were added in SCI 1.1
			names.resize(count + countSci1 + countSci11);
			for (int i = count + countSci1; i < count + countSci1 + countSci11; i++)
				names[i] = sci11Selectors[i - count - countSci1];
		}

		// Now, we need to find out selectors which keep changing place...
		// We do that by dissecting game objects, and looking for selectors at
		// specified locations.

		// We need to initialize script 0 here, to make sure that it's always
		// located at segment 1.
		_segMan->instantiateScript(0);

		// The Actor class contains the init, xLast and yLast selectors, which
		// we reference directly. It's always in script 998, so we need to
		// explicitly load it here.
		if (_resMan->testResource(ResourceId(kResourceTypeScript, 998))) {
			_segMan->instantiateScript(998);

			const Object *actorClass = _segMan->getObject(_segMan->findObjectByName("Actor"));

			if (actorClass) {
				// The init selector is always the first function
				int initSelectorPos = actorClass->getFuncSelector(0);

				if (names.size() < (uint32)initSelectorPos + 2)
					names.resize((uint32)initSelectorPos + 2);

				names[initSelectorPos] = "init";
				// dispose comes right after init
				names[initSelectorPos + 1] = "dispose";

				if ((getSciVersion() >= SCI_VERSION_1_EGA)) {
					// Find the xLast and yLast selectors, used in kDoBresen

					// xLast and yLast always come between illegalBits and xStep
					int illegalBitsSelectorPos = actorClass->locateVarSelector(_segMan, 15 + offset);	// illegalBits
					int xStepSelectorPos = actorClass->locateVarSelector(_segMan, 51 + offset);	// xStep
					if (xStepSelectorPos - illegalBitsSelectorPos != 3) {
						error("illegalBits and xStep selectors aren't found in "
							  "known locations. illegalBits = %d, xStep = %d",
							  illegalBitsSelectorPos, xStepSelectorPos);
					}

					int xLastSelectorPos = actorClass->getVarSelector(illegalBitsSelectorPos + 1);
					int yLastSelectorPos = actorClass->getVarSelector(illegalBitsSelectorPos + 2);

					if (names.size() < (uint32)yLastSelectorPos + 1)
						names.resize((uint32)yLastSelectorPos + 1);

					names[xLastSelectorPos] = "xLast";
					names[yLastSelectorPos] = "yLast";
				}	// if ((getSciVersion() >= SCI_VERSION_1_EGA))
			}	// if (actorClass)

			_segMan->uninstantiateScript(998);
		}	// if (_resMan->testResource(ResourceId(kResourceTypeScript, 998)))

		if (_resMan->testResource(ResourceId(kResourceTypeScript, 981))) {
			// The SysWindow class contains the open selectors, which we
			// reference directly. It's always in script 981, so we need to
			// explicitly load it here
			_segMan->instantiateScript(981);

			const Object *sysWindowClass = _segMan->getObject(_segMan->findObjectByName("SysWindow"));

			if (sysWindowClass) {
				if (sysWindowClass->getMethodCount() < 2)
					error("The SysWindow class has less than 2 methods");

				// The open selector is always the second function
				int openSelectorPos = sysWindowClass->getFuncSelector(1);

				if (names.size() < (uint32)openSelectorPos + 1)
					names.resize((uint32)openSelectorPos + 1);

				names[openSelectorPos] = "open";
			}

			_segMan->uninstantiateScript(981);
		}	// if (_resMan->testResource(ResourceId(kResourceTypeScript, 981)))

		if (g_sci->getGameId() == GID_HOYLE4) {
			// The demo of Hoyle 4 is one of the few demos with lip syncing and no selector vocabulary.
			// This needs two selectors, "syncTime" and "syncCue", which keep changing positions in each
			// game. Usually, games with speech and lip sync have a selector vocabulary, so we don't need
			// to set these two selectors, but we need for Hoyle...
			if (names.size() < 276)
				names.resize(276);

			names[274] = "syncTime";
			names[275] = "syncCue";
		} else if (g_sci->getGameId() == GID_PEPPER) {
			// Same as above for the non-interactive demo of Pepper
			if (names.size() < 539)
				names.resize(539);

			names[263] = "syncTime";
			names[264] = "syncCue";
			names[538] = "startText";
		} else if (g_sci->getGameId() == GID_LAURABOW2) {
			// The floppy of version needs the changeState selector set to match up with the
			// CD version's workarounds.
			if (names.size() < 251)
				names.resize(251);

			names[144] = "changeState";
		} else if (g_sci->getGameId() == GID_CNICK_KQ) {
			if (names.size() < 447)
				names.resize(447);

			names[446] = "say";
		}

#ifdef ENABLE_SCI32
	} else {
		// SCI2+
		for (int i = 0; i < count; i++)
			names[i] = sci2Selectors[i];
#endif
	}

	for (const SelectorRemap *selectorRemap = sciSelectorRemap; selectorRemap->slot; ++selectorRemap) {
		if (getSciVersion() >= selectorRemap->minVersion && getSciVersion() <= selectorRemap->maxVersion) {
			const uint32 slot = selectorRemap->slot;
			if (slot >= names.size())
				names.resize(slot + 1);
			names[slot] = selectorRemap->name;
		}
	}

	return names;
}

} // End of namespace Sci
