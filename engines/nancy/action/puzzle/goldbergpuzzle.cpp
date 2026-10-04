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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"

#include "engines/nancy/puzzledata.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/puzzle/goldbergpuzzle.h"

namespace Nancy {
namespace Action {

void GoldbergPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _saveKey);
	_matchFlag.label = stream.readSint16LE();
	_matchFlag.flag = stream.readByte();
	_notFirstFlag.label = stream.readSint16LE();
	_notFirstFlag.flag = stream.readByte();
	readFilename(stream, _streamName);

	_arrangements.resize(stream.readUint16LE());
	for (Arrangement &arrangement : _arrangements) {
		arrangement.slots.resize(stream.readUint16LE());
		for (Slot &slot : arrangement.slots) {
			slot.zoneID = stream.readSint16LE();
			slot.pieceID = stream.readSint16LE();
		}

		readFilename(stream, arrangement.sceneName);
	}
}

int GoldbergPuzzle::findArrangement() const {
	const BuildBoardData *boards = (const BuildBoardData *)NancySceneState.getPuzzleData(BuildBoardData::getTag());
	if (!boards || !boards->boards.contains(_saveKey)) {
		return -1;
	}

	const BuildBoardData::Board &board = boards->boards[_saveKey];

	for (uint i = 0; i < _arrangements.size(); ++i) {
		bool matches = true;

		for (const Slot &slot : _arrangements[i].slots) {
			if (board.getPieceInZone(slot.zoneID) != slot.pieceID) {
				matches = false;
				break;
			}
		}

		if (matches) {
			return i;
		}
	}

	return -1;
}

void GoldbergPuzzle::execute() {
	int index = findArrangement();

	if (index != -1) {
		NancySceneState.setEventFlag(_matchFlag);
		NancySceneState.getStreams().changeScene(_streamName, _arrangements[index].sceneName, 0);
	}

	if (index != 0) {
		NancySceneState.setEventFlag(_notFirstFlag);
	}

	_isDone = true;
}

} // End of namespace Action
} // End of namespace Nancy
