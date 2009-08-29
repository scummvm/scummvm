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
         "number",    "nodePtr",   "client",        "dx",         "dy", // 40 - 44
      "b-moveCnt",       "b-i1",     "b-i2",      "b-di",    "b-xAxis", // 45 - 49
         "b-incr",      "xStep",    "yStep", "moveSpeed",  "canBeHere", // 50 - 54
        "heading",      "mover",     "doit", "isBlocked",     "looper", // 55 - 59
       "priority",  "modifiers",   "replay",    "setPri",         "at", // 60 - 64
           "next",       "done",    "width",  "wordFail", "syntaxFail", // 65 - 69
   "semanticFail", "pragmaFail",     "said",   "claimed",      "value", // 70 - 74
           "save",    "restore",    "title",    "button",       "icon", // 75 - 79
           "draw",     "delete",        "z"                             // 80 - 82
};

void createFirstPart(Common::StringList &names, int offset, bool hasCantBeHere, bool hasNodePtr) {
	int count = ARRAYSIZE(selectorNamesFirstPart) + offset;
	int i;
	names.resize(count);

	for (i = 0; i < offset; i++)
		names[i] = "";

	for (i = offset; i < count; i++) {
		names[i] = selectorNamesFirstPart[i - offset];
		if (hasNodePtr && i == handleIndex + offset)
			names[i] = "nodePtr";
		if (hasCantBeHere && i == canBeHereIndex + offset)
			names[i] = "cantBeHere";
	}	
}

// Taken from King's Quest IV (Full Game)
// offset: 3, hascantbehere: false, hasNodePtr: false
static const SelectorRemap kq4_demo_selectors[] = {
	{      "init",  87 }, {   "dispose",  88 }, {      "size",  96 },
	{    "caller", 119 }, {       "cue", 121 }, {     "owner", 130 },
	{ "completed", 158 }, { "motionCue", 161 }, {    "cycler", 165 },
	{  "moveDone", 169 }, { "setTarget", 171 }
};

// Taken from Codename: Iceman (Full Game)
// offset: 3, hascantbehere: false, hasNodePtr: false
static const SelectorRemap iceman_demo_selectors[] = {
	{      "init",  87 }, {   "dispose",  88 }, {      "size",  96 },
	{    "caller", 119 }, {       "cue", 121 }, {     "owner", 130 },
	{ "completed", 159 }, { "motionCue", 162 }, {    "cycler", 164 },
	{  "moveDone", 170 }, { "setTarget", 171 }, {  "distance", 173 },
	{   "points",  316 }, {     "flags", 368 }
};

// Taken from EcoQuest 2 (Demo)
// offset: 0, hascantbehere: true, hasNodePtr: true
static const SelectorRemap christmas1992_selectors[] = {
	{    "parseLang",  83 }, {    "printLang",  84 }, { "subtitleLang",  85 },
	{         "size",  86 }, {       "points",  87 }, {      "palette",  88 },
	{      "dataInc",  89 }, {       "handle",  90 }, {          "min",  91 },
	{          "sec",  92 }, {        "frame",  93 }, {          "vol",  94 },
	{          "pri",  95 }, {     "moveDone",  97 }, {        "flags",  99 },
	{         "init", 110 }, {      "dispose", 111 }, {          "cue", 145 },
	{        "owner", 172 }, {    "completed", 250 }, {    "canBeHere", 450 },
	{       "cycler", 255 }, {     "distance", 264 }, {       "caller", 143 },
	{      "syncCue", 271 }, {     "syncTime", 270 }, {       "setVol", 178 }
};

// Taken from Leisure Suit Larry 1 VGA (Full Game)
// offset: 3, hascantbehere: true, hasNodePtr: true
static const SelectorRemap lsl1_demo_selectors[] = {
	{    "parseLang",  86 }, {    "printLang",  87 }, { "subtitleLang",  88 },
	{         "size",  89 }, {       "points",  90 }, {      "palette",  91 },
	{      "dataInc",  92 }, {       "handle",  93 }, {          "min",  94 },
	{          "sec",  95 }, {        "frame",  96 }, {          "vol",  97 },
	{          "pri",  98 }, {     "moveDone", 100 }, {        "flags", 102 },
	{         "init", 104 }, {      "dispose", 105 }, {       "caller", 134 },
	{          "cue", 136 }, {        "owner", 150 }, {       "setVol", 156 },
	{    "completed", 210 }, {    "motionCue", 213 }, {       "cycler", 215 },
	{    "setTarget", 221 }, {     "distance", 224 }, {    "canBeHere", 232 },
	{     "syncTime", 247 }, {      "syncCue", 248 }, { "egoMoveSpeed", 370 }
};

// Taken from Space Quest 1 VGA (Demo)
// offset: 3, hascantbehere: false, hasNodePtr: true
static const SelectorRemap lsl5_demo_selectors[] = {
	{    "parseLang",  86 }, {    "printLang",  87 }, { "subtitleLang",  88 },
	{         "size",  89 }, {       "points",  90 }, {      "palette",  91 },
	{      "dataInc",  92 }, {          "min",  94 }, {          "sec",  95 },
	{        "frame",  96 }, {          "vol",  97 }, {          "pri",  98 },
	{     "moveDone", 100 }, {         "init", 103 }, {      "dispose", 104 },
	{       "caller", 133 }, {          "cue", 135 }, {        "owner", 149 },
	{        "flags", 150 }, {    "completed", 207 }, {    "motionCue", 210 },
	{       "cycler", 212 }, {     "distance", 221 }, { "egoMoveSpeed", 357 }
};

// A macro for loading one of the above tables in the function below
#define USE_SELECTOR_TABLE(x) \
	for (uint32 i = 0; i < ARRAYSIZE(x); i++) { \
		if (x[i].slot >= names.size()) \
			names.resize(x[i].slot + 1); \
		names[x[i].slot] = x[i].name; \
	}

Common::StringList Kernel::checkStaticSelectorNames() {
	Common::StringList names;
	if (!g_engine)
		return names;

	Common::String gameID = ((SciEngine*)g_engine)->getGameID();

	if (gameID == "kq4sci") {
		createFirstPart(names, 3, false, false);
		USE_SELECTOR_TABLE(kq4_demo_selectors);
	} else if (gameID == "lsl3" || gameID == "iceman") { // identical, except iceman has "flags" 
		createFirstPart(names, 3, false, false);
		USE_SELECTOR_TABLE(iceman_demo_selectors);
	} else if (gameID == "christmas1992") {
		createFirstPart(names, 0, true, true);
		USE_SELECTOR_TABLE(christmas1992_selectors);
	} else if (gameID == "lsl1sci") {
		createFirstPart(names, 3, true, true);
		USE_SELECTOR_TABLE(lsl1_demo_selectors);
	} else if (gameID == "lsl5") {
		createFirstPart(names, 3, false, true);
		USE_SELECTOR_TABLE(lsl5_demo_selectors);
	}

	return names;
}
	
} // End of namespace Sci
