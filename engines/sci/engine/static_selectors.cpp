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

static const char * const selectorNamesFirstPart[] = {
              "y",          "x",     "view",      "loop",        "cel", //  0 -  4
      "underBits",      "nsTop",   "nsLeft",  "nsBottom",    "nsRight", //  5 -  9
          "lsTop",     "lsLeft", "lsBottom",   "lsRight",     "signal", // 10 - 14
    "illegalBits",      "brTop",   "brLeft",  "brBottom",    "brRight", // 15 - 19
           "name",        "key",     "time",      "text",   "elements", // 20 - 25
          "color",       "back",     "mode",     "style",      "state", // 25 - 29
           "font",       "type",   "window",    "cursor",        "max", // 30 - 34
           "mark",        "who",  "message",      "edit",       "play", // 35 - 39
         "number",     "handle",   "client",        "dx",         "dy", // 40 - 44
      "b-moveCnt",       "b-i1",     "b-i2",      "b-di",    "b-xAxis", // 45 - 49
         "b-incr",      "xStep",    "yStep", "moveSpeed",  "canBeHere", // 50 - 54
        "heading",      "mover",     "doit", "isBlocked",     "looper", // 55 - 59
       "priority",  "modifiers",   "replay",    "setPri",         "at", // 60 - 64
           "next",       "done",    "width",  "wordFail", "syntaxFail", // 65 - 69
   "semanticFail", "pragmaFail",     "said",   "claimed",      "value", // 70 - 74
           "save",    "restore",    "title",    "button",       "icon", // 75 - 79
           "draw",     "delete",        "z"                             // 80 - 82
};

// Taken from Codename: Iceman (Full Game)
static const SelectorRemap sci0_selectors[] = {
	{      "init",  87 }, {   "dispose",  88 }, {      "size",  96 },
	{    "caller", 119 }, {       "cue", 121 }, {     "owner", 130 },
	{ "completed", 159 }, { "motionCue", 162 }, {    "cycler", 164 },
	{  "moveDone", 170 }, {  "distance", 173 },	{ "setCursor", 254 },
	{   "points",  316 }, {     "flags", 368 }
};

// Taken from Leisure Suit Larry 1 VGA (Full Game)
static const SelectorRemap sci1_selectors[] = {
	{    "parseLang",  86 }, {    "printLang",  87 }, { "subtitleLang",  88 },
	{         "size",  89 }, {       "points",  90 }, {      "palette",  91 },
	{      "dataInc",  92 }, {       "handle",  93 }, {          "min",  94 },
	{          "sec",  95 }, {        "frame",  96 }, {          "vol",  97 },
	{          "pri",  98 }, {     "moveDone", 100 }, {        "flags", 102 },
	{         "init", 104 }, {      "dispose", 105 }, {       "caller", 134 },
	{          "cue", 136 }, {        "owner", 150 }, {       "setVol", 156 },
	{    "setCursor", 183 }, {    "completed", 210 }, {       "cycler", 215 },
	{     "distance", 224 }, {    "canBeHere", 232 }, {     "syncTime", 247 },
	{      "syncCue", 248 }
};

// Taken from KQ6 floppy (Full Game)
static const SelectorRemap sci11_selectors[] = {
	{    "parseLang",  83 }, {    "printLang",  84 }, { "subtitleLang",  85 },
	{         "size",  86 }, {       "points",  87 }, {      "palette",  88 },
	{      "dataInc",  89 }, {       "handle",  90 }, {          "min",  91 },
	{          "sec",  92 }, {        "frame",  93 }, {          "vol",  94 },
	{          "pri",  95 }, {     "moveDone",  97 }, {        "flags",  99 },
	{         "init", 110 }, {      "dispose", 111 }, {       "caller", 143 },
	{          "cue", 145 }, {        "owner", 166 }, {       "setVol", 172 },
	{    "setCursor", 197 }, {    "completed", 242 }, {       "cycler", 247 },
	{     "distance", 256 }, {    "canBeHere", 264 }, {     "syncTime", 279 },
	{      "syncCue", 280 }
};

// A macro for loading one of the above tables in the function below
#define USE_SELECTOR_TABLE(x) \


Common::StringList Kernel::checkStaticSelectorNames(SciVersion version) {
	Common::StringList names;
	int offset = (version < SCI_VERSION_1_1) ? 3 : 0;
	int count = ARRAYSIZE(selectorNamesFirstPart) + offset;
	names.resize(count);
	const SelectorRemap *selectors = sci0_selectors;

	for (int j = 0; j < offset; j++)
		names[j].clear();

	for (int i = offset; i < count; i++) {
		names[i] = selectorNamesFirstPart[i - offset];
		if (version >= SCI_VERSION_1_EGA && i == handleIndex + offset)
			names[i] = "nodePtr";
		if (version >= SCI_VERSION_1_EGA && i == canBeHereIndex + offset)
			names[i] = "cantBeHere";
	}	

	if (version <= SCI_VERSION_01)
		selectors = sci0_selectors;
	else if (version >= SCI_VERSION_1_EGA && version <= SCI_VERSION_1_LATE)
		selectors = sci1_selectors;
	else
		selectors = sci11_selectors;

	for (uint32 k = 0; k < ARRAYSIZE(selectors); k++) {
		if (selectors[k].slot >= names.size()) \
			names.resize(selectors[k].slot + 1); \
		names[selectors[k].slot] = selectors[k].name; \
	}

	return names;
}
	
} // End of namespace Sci
