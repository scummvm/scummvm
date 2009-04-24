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

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/scnplayer.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/inter.h"
#include "gob/videoplayer.h"

namespace Gob {

SCNPlayer::SCNPlayer(GobEngine *vm) : _vm(vm) {
}

SCNPlayer::~SCNPlayer() {
}

inline bool SCNPlayer::lineStartsWith(const Common::String &line, const char *start) {
	return (strstr(line.c_str(), start) == line.c_str());
}

bool SCNPlayer::play(const char *fileName) {
	debugC(1, kDebugSCN, "Playing SCN \"%s\"", fileName);

	// The video player needs some fake variables
	_vm->_inter->allocateVars(32);

	// Init the screen
	_vm->_draw->initScreen();
	_vm->_draw->_cursorIndex = -1;

	_vm->_util->longDelay(200); // Letting everything settle

	Common::File scn;

	if (!scn.open(fileName))
		return false;

	return play(scn);
}

bool SCNPlayer::play(Common::File &scn) {
	// Read labels
	LabelMap labels;
	if (!readLabels(scn, labels))
		return false;

	// Iterate over all lines
	while (!scn.err() && !scn.eos()) {
		Common::String line = scn.readLine();

		// Interpret
		if (line == "CLEAR") {
			clearScreen();
		} else if (lineStartsWith(line, "IMD_PRELOAD ")) {
			playVideo(line.c_str() + 12);
		} else if (lineStartsWith(line, "GOTO ")) {
			gotoLabel(scn, labels, line.c_str() + 5);
		}

		// Mind user input
		_vm->_util->processInput();
		if (_vm->shouldQuit())
			return true;
	}

	if (scn.err())
		return false;

	return true;
}

bool SCNPlayer::readLabels(Common::File &scn, LabelMap &labels) {
	debugC(1, kDebugSCN, "Reading SCN labels");

	int32 startPos = scn.pos();

	// Iterate over all lines
	while (!scn.err() && !scn.eos()) {
		Common::String line = scn.readLine();

		if (lineStartsWith(line, "LABEL ")) {
			// Label => Add to the hashmap
			labels.setVal(line.c_str() + 6, scn.pos());
			debugC(2, kDebugSCN, "Found label \"%s\" (%d)", line.c_str() + 6, scn.pos());
		}
	}

	if (scn.err())
		return false;

	// Seek back
	if (!scn.seek(startPos))
		return false;

	return true;
}

void SCNPlayer::gotoLabel(Common::File &scn, const LabelMap &labels, const char *label) {
	debugC(2, kDebugSCN, "Jumping to label \"%s\"", label);

	if (!labels.contains(label))
		return;

	scn.seek(labels.getVal(label));
}

void SCNPlayer::clearScreen() {
	debugC(1, kDebugSCN, "Clearing the screen");
	_vm->_video->clearScreen();
}

void SCNPlayer::playVideo(const char *fileName) {
	// Trimming spaces
	while (*fileName == ' ')
		fileName++;

	debugC(1, kDebugSCN, "Playing video \"%s\"", fileName);

	// Playing the video
	if (_vm->_vidPlayer->primaryOpen(fileName)) {
		_vm->_vidPlayer->primaryPlay();
		_vm->_vidPlayer->primaryClose();
	}
}

} // End of namespace Gob
