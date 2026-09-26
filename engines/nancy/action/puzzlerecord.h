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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_PUZZLERECORD_H
#define NANCY_ACTION_PUZZLERECORD_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Common base for all puzzle action records. Holds the scene changes and
// give-up hotspot that nearly every puzzle has.
class PuzzleRecord : public RenderActionRecord {
public:
	PuzzleRecord(uint zOrder) : RenderActionRecord(zOrder) {}
	virtual ~PuzzleRecord() {}

	Common::String getRecordExtraInfo() const override;

protected:
	// Reads the Nancy 13+ count-prefixed give-up hotspot array into the exit fields,
	// keeping only the first record.
	void readExitHotspot(Common::SeekableReadStream &stream);

	bool isExitHotspotHovered(const NancyInput &input) const;
	void setExitCursor() const;

	// Shows the exit cursor and returns true while the mouse is over the give-up hotspot
	bool hoverExitHotspot(const NancyInput &input) const;

	// Scene the puzzle moves to when solved; its flag is set on solve
	SceneChangeWithFlag _solveScene;

	// Scene the puzzle moves to when left unsolved, e.g. via the give-up hotspot
	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	bool _exitCursorFromData = false; // otherwise the generic puzzle exit cursor is used
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PUZZLERECORD_H
