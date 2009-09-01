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

namespace Sci {
	
struct SelectorRemap {
	const char *name;
	uint32 slot;
};

const int handleIndex = 41;
const int canBeHereIndex = 54;

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

// Taken from Codename: Iceman (Full Game)
static const SelectorRemap sci0SelectorRemap[] = {
    {       "caller", 119 }, {          "cue", 121 }, {        "owner", 130 },
	{    "completed", 159 }, {    "motionCue", 162 }, {       "cycler", 164 },
	{     "moveDone", 170 }, {     "distance", 173 }, {    "setCursor", 254 },
	{      "points",  316 }, {        "flags", 368 }
};

// Taken from Leisure Suit Larry 1 VGA (Full Game)
static const SelectorRemap sci1SelectorRemap[] = {
	{        "flags", 102 }, {         "init", 104 }, {      "dispose", 105 },
	{       "caller", 134 }, {          "cue", 136 }, {        "owner", 150 },
	{       "setVol", 156 }, {    "setCursor", 183 }, {    "completed", 210 },
	{       "cycler", 215 }, {     "distance", 224 }, {    "canBeHere", 232 },
	{     "syncTime", 247 }, {      "syncCue", 248 }
};

// Taken from KQ6 floppy (Full Game)
static const SelectorRemap sci11SelectorRemap[] = {
	{        "flags",  99 }, {         "init", 110 }, {      "dispose", 111 },
	{       "caller", 143 }, {          "cue", 145 }, {        "owner", 166 },
	{       "setVol", 172 }, {    "setCursor", 197 }, {    "completed", 242 },
	{       "cycler", 247 }, {     "distance", 256 }, {    "canBeHere", 264 },
	{     "syncTime", 279 }, {      "syncCue", 280 }
};

Common::StringList Kernel::checkStaticSelectorNames(SciVersion version) {
	Common::StringList names;
	int offset = (version < SCI_VERSION_1_1) ? 3 : 0;
	int count = ARRAYSIZE(sci0Selectors) + offset;
	names.resize(count);
	const SelectorRemap *selectorRemap = sci0SelectorRemap;
	uint32 selectorRemapSize = 0;
	int i;

	for (i = 0; i < offset; i++)
		names[i].clear();

	for (i = offset; i < count; i++)
		names[i] = sci0Selectors[i - offset];

	if (version <= SCI_VERSION_01) {
		selectorRemap = sci0SelectorRemap;
		selectorRemapSize = ARRAYSIZE(sci0SelectorRemap);
	} else {
		int count2 = ARRAYSIZE(sci1Selectors);
		names[handleIndex + offset] = "nodePtr";
		names[canBeHereIndex + offset] = "cantBeHere";
		names.resize(count + count2);

		for (i = count; i < count + count2; i++)
			names[i] = sci1Selectors[i - count];

		if (version < SCI_VERSION_1_1) {
			selectorRemap = sci1SelectorRemap;
			selectorRemapSize = ARRAYSIZE(sci1SelectorRemap);
		} else {
			selectorRemap = sci11SelectorRemap;
			selectorRemapSize = ARRAYSIZE(sci11SelectorRemap);
		}
	}

	for (uint32 k = 0; k < selectorRemapSize; k++) {
		if (selectorRemap[k].slot >= names.size())
			names.resize(selectorRemap[k].slot + 1);
		names[selectorRemap[k].slot] = selectorRemap[k].name;
	}

	return names;
}
	
} // End of namespace Sci
