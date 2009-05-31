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

 // Console module header file

#ifndef SCI_CONSOLE_H
#define SCI_CONSOLE_H

#include "gui/debugger.h"

namespace Sci {

class SciEngine;

class Console : public GUI::Debugger {
public:
	Console(SciEngine *vm);
	virtual ~Console();

private:
	bool cmdGetVersion(int argc, const char **argv);
//	bool cmdClasses(int argc, const char **argv);	// TODO
	bool cmdOpcodes(int argc, const char **argv);
	bool cmdSelectors(int argc, const char **argv);
	bool cmdKernelNames(int argc, const char **argv);
	bool cmdSuffixes(int argc, const char **argv);
	bool cmdHexDump(int argc, const char **argv);
	bool cmdDissectScript(int argc, const char **argv);
	bool cmdRoomNumber(int argc, const char **argv);
	bool cmdResourceSize(int argc, const char **argv);
	bool cmdResourceTypes(int argc, const char **argv);
	bool cmdSci0Palette(int argc, const char **argv);
	bool cmdHexgrep(int argc, const char **argv);
	bool cmdList(int argc, const char **argv);
	bool cmdClearScreen(int argc, const char **argv);
	bool cmdRedrawScreen(int argc, const char **argv);
	bool cmdSaveGame(int argc, const char **argv);
	bool cmdRestoreGame(int argc, const char **argv);
	bool cmdRestartGame(int argc, const char **argv);
	bool cmdClassTable(int argc, const char **argv);
	bool cmdSentenceFragments(int argc, const char **argv);
	bool cmdParserNodes(int argc, const char **argv);
	bool cmdParserWords(int argc, const char **argv);
	bool cmdDrawPic(int argc, const char **argv);
	bool cmdDrawRect(int argc, const char **argv);
	bool cmdFillScreen(int argc, const char **argv);
	bool cmdCurrentPort(int argc, const char **argv);
	bool cmdPrintPort(int argc, const char **argv);
	bool cmdParseGrammar(int argc, const char **argv);
	bool cmdVisualState(int argc, const char **argv);
	bool cmdDynamicViews(int argc, const char **argv);
	bool cmdDroppedViews(int argc, const char **argv);
	bool cmdStatusBarColors(int argc, const char **argv);
	bool cmdSimulateKey(int argc, const char **argv);
	bool cmdTrackMouse(int argc, const char **argv);
	bool cmdPrintSegmentTable(int argc, const char **argv);
	bool cmdSegmentInfo(int argc, const char **argv);
	bool cmdKillSegment(int argc, const char **argv);
	bool cmdShowMap(int argc, const char **argv);
	bool cmdSongLib(int argc, const char **argv);
	bool cmdInvokeGC(int argc, const char **argv);
	bool cmdGCObjects(int argc, const char **argv);
	bool cmdExit(int argc, const char **argv);

	bool segmentInfo(int nr);
	void printList(List *l);

private:
	SciEngine *_vm;
};

} // End of namespace Sci

#endif
