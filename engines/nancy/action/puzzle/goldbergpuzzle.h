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

#ifndef NANCY_ACTION_GOLDBERGPUZZLE_H
#define NANCY_ACTION_GOLDBERGPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Nancy17. Looks at a board the build puzzle left behind, and sends a stream to
// the scene of the first arrangement the board matches.
class GoldbergPuzzle : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "GoldbergPuzzle"; }

	static const int16 kEmptyZone = -1;

	struct Slot {
		int16 zoneID = 0;
		int16 pieceID = kEmptyZone;
	};

	struct Arrangement {
		Common::Array<Slot> slots;
		Common::String sceneName;
	};

	// The index of the first arrangement the board matches, -1 if there is none
	int findArrangement() const;

	Common::String _saveKey;
	FlagDescription _matchFlag;
	FlagDescription _notFirstFlag;	// for every outcome but the first arrangement
	Common::String _streamName;
	Common::Array<Arrangement> _arrangements;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_GOLDBERGPUZZLE_H
