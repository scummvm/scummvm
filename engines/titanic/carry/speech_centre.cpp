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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "titanic/carry/speech_centre.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSpeechCentre, CBrain)
	ON_MESSAGE(PuzzleSolvedMsg)
	ON_MESSAGE(ChangeSeasonMsg)
	ON_MESSAGE(SpeechFallsFromTreeMsg)
	ON_MESSAGE(FrameMsg)
END_MESSAGE_MAP()

void CSpeechCentre::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field13C, indent);
	file->writeQuotedLine(_season, indent);
	file->writeNumberLine(_field14C, indent);

	CBrain::save(file, indent);
}

void CSpeechCentre::load(SimpleFile *file) {
	file->readNumber();
	_field13C = file->readNumber();
	_season = file->readString();
	_field14C = file->readNumber();

	CBrain::load(file);
}

bool CSpeechCentre::PuzzleSolvedMsg(CPuzzleSolvedMsg *msg) {
	if (_field13C == 1 && _season == "Autumn")
		_canTake = true;
	return true;
}

bool CSpeechCentre::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	_season = msg->_season;
	return true;
}

bool CSpeechCentre::SpeechFallsFromTreeMsg(CSpeechFallsFromTreeMsg *msg) {
	setVisible(true);
	dragMove(msg->_pos);
	_field14C = true;
	return true;
}

bool CSpeechCentre::FrameMsg(CFrameMsg *msg) {
	if (_field14C) {
		if (_bounds.top > 200)
			_field14C = false;

		makeDirty();
		_bounds.top += 3;
		makeDirty();
	}

	return true;
}

} // End of namespace Titanic
