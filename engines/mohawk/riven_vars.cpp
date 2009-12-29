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

#include "common/str.h"

#include "mohawk/riven.h"

namespace Mohawk {

// The Riven variable system is complex. The scripts of each stack give a number, but the number has to be matched 
// to a variable name defined in NAME resource 4.

static const char *variableNames[] = {
	// aspit
	"aatrusbook",
	"aatruspage",
	"acathbook",
	"acathpage",
	"acathstate",
	"adoit",
	"adomecombo",
	"agehn",
	"ainventory",
	"aova",
	"apower",
	"araw",
	"atemp",
	"atrap",
	"atrapbook",
	"auservolume",
	"azip",
	
	// bspit
	"bbacklock",
	"bbait",
	"bbigbridge",
	"bbirds",
	"bblrarm",
	"bblrdoor",
	"bblrgrt",
	"bblrsw",
	"bblrvalve",
	"bblrwtr",
	"bbook",
	"bbrlever",
	"bcavedoor",
	"bcombo",
	"bcpipegr",
	"bcratergg",
	"bdome",
	"bdrwr",
	"bfans",
	"bfmdoor",
	"bidvlv",
	"blab",
	"blabbackdr",
	"blabbook",
	"blabeye",
	"blabfrontdr",
	"blabpage",
	"blever",
	"bfrontlock",
	"bheat",
	"bmagcar",
	"bpipdr",
	"bprs",
	"bstove",
	"btrap",
	"bvalve",
	"bvise",
	"bytram",
	"bytramtime",
	"bytrap",
	"bytrapped",
	
	// gspit
	"gbook",
	"gcathtime",
	"gcathstate",
	"gcombo",
	"gdome",
	"gemagcar",
	"gimagecurr",
	"gimagemax",
	"gimagerot",
	"glkbtns",
	"glkbridge",
	"glkelev",
	"glview",
	"glviewmpos",
	"glviewpos",
	"gnmagrot",
	"gnmagcar",
	"gpinup",
	"gpinpos",
	"gpinsmpos",
	"grview",
	"grviewmpos",
	"grviewpos",
	"gscribe",
	"gscribetime",
	"gsubelev",
	"gsubdr",
	"gupmoov",
	"gwhark",
	"gwharktime",
	
	// jspit
	"jwmagcar",
	"jbeetle",
	"jbeetlepool",
	"jbook",
	"jbridge1",
	"jbridge2",
	"jbridge3",
	"jbridge4",
	"jbridge5",
	"jccb",
	"jcombo",
	"jcrg",
	"jdome",
	"jdrain",
	"jgallows",
	"jgate",
	"jgirl",
	"jiconcorrectorder",
	"jiconorder",
	"jicons",
	"jladder",
	"jleftpos",
	"jpeek",
	"jplaybeetle",
	"jprebel",
	"jprisondr",
	"jprisonsecdr",
	"jrbook",
	"jrightpos",
	"jsouthpathdr",
	"jschooldr",
	"jsub",
	"jsubdir",
	"jsubhatch",
	"jsubsw",
	"jsunners",
	"jsunnertime",
	"jthronedr",
	"jtunneldr",
	"jtunnellamps",
	"jvillagepeople",
	"jwarning",
	"jwharkpos",
	"jwharkram",
	"jwmouth",
	"jwmagcar",
	"jymagcar",
	
	// ospit
	"oambient",
	"obutton",
	"ocage",
	"odeskbook",
	"ogehnpage",
	"omusicplayer",
	"ostanddrawer",
	"ostove",
	
	// pspit
	"pbook",
	"pcage",
	"pcathcheck",
	"pcathstate",
	"pcathtime",
	"pcombo",
	"pcorrectorder",
	"pdome",
	"pelevcombo",
	"pleftpos",
	"prightpos",
	"ptemp",
	"pwharkpos",
	
	// rspit
	"rrebel",
	"rrebelview",
	"rrichard",
	"rvillagetime",
	
	// tspit
	"tbars",
	"tbeetle",
	"tblue",
	"tbook",
	"tbookvalve",
	"tcage",
	"tcombo",
	"tcorrectorder",
	"tcovercombo",
	"tdl",
	"tdome",
	"tdomeelev",
	"tdomeelevbtn",
	"tgatebrhandle",
	"tgatebridge",
	"tgatestate",
	"tgreen",
	"tgridoor",
	"tgrodoor",
	"tgrmdoor",
	"tguard",
	"timagedoor",
	"tmagcar",
	"torange",
	"tred",
	"tsecdoor",
	"tsubbridge",
	"ttelecover",
	"ttelehandle",
	"ttelepin",
	"ttelescope",
	"ttelevalve",
	"ttemple",
	"ttempledoor",
	"ttunneldoor",
	"tviewer",
	"tviolet",
	"twabrvalve",
	"twaffle",
	"tyellow",
	
	// Miscellaneous
	"elevbtn1",
	"elevbtn2",
	"elevbtn3",
	"domecheck",
	"transitionsenabled",
	"transitionmode",
	"waterenabled",
	"rivenambients",
	"stackvarsinitialized",
	"doingsetupscreens",
	"all_book",
	"playerhasbook",
	"returnstackid",
	"returncardid",
	"newpos",
	"themarble",
	"currentstackid",
	"currentcardid"
};

uint32 *MohawkEngine_Riven::getLocalVar(uint32 index) {
	return matchVarToString(getName(VariableNames, index));
}

uint32 MohawkEngine_Riven::getGlobalVar(uint32 index) {
	return _vars[index];
}

Common::String MohawkEngine_Riven::getGlobalVarName(uint32 index) {
	return Common::String(variableNames[index]);
}

uint32 *MohawkEngine_Riven::matchVarToString(Common::String varName) {
	return matchVarToString(varName.c_str());
}

uint32 *MohawkEngine_Riven::matchVarToString(const char *varName) {
	for (uint32 i = 0; i < _varCount; i++)
		if (!scumm_stricmp(varName, variableNames[i]))
			return &_vars[i];
	error ("Unknown variable: \'%s\'", varName);
	return NULL;
}

void MohawkEngine_Riven::initVars() {
	_varCount = ARRAYSIZE(variableNames);

	_vars = new uint32[_varCount];
	
	// Temporary:
	for (uint32 i = 0; i < _varCount; i++)
		_vars[i] = 0;
	
	// Init Variables to their correct starting state.
	*matchVarToString("ttelescope") = 5;
	*matchVarToString("tgatestate") = 1;
	*matchVarToString("jbridge1") = 1;
	*matchVarToString("jbridge4") = 1;
	*matchVarToString("jgallows") = 1;
	*matchVarToString("jiconcorrectorder") = 12068577;
	*matchVarToString("bblrvalve") = 1;
	*matchVarToString("bblrwtr") = 1;
	*matchVarToString("bfans") = 1;
	*matchVarToString("bytrap") = 2;
	*matchVarToString("aatruspage") = 1;
	*matchVarToString("acathpage") = 1;
	*matchVarToString("bheat") = 1;
	*matchVarToString("waterenabled") = 1;
	*matchVarToString("ogehnpage") = 1;
}

} // End of namespace Mohawk
