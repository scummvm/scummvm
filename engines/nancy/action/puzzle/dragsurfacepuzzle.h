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

#ifndef NANCY_ACTION_DRAGSURFACEPUZZLE_H
#define NANCY_ACTION_DRAGSURFACEPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionzone.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy17 cemetery map. A picture larger than the viewport, moved around by
// dragging it. Action zones on the picture change the cursor, and the scene when
// they are clicked. The part of the picture in view is kept under a name.
class DragSurfacePuzzle : public PuzzleRecord {
public:
	DragSurfacePuzzle() : PuzzleRecord(7) {}
	virtual ~DragSurfacePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "DragSurfacePuzzle"; }

	// A press that moves less than this is a click
	static const int kDragThreshold = 3;

	// Whether a zone is active, and has a position on the picture inside of it
	bool isInZone(const ActionZone &zone, const Common::Point &picturePos) const;
	void dragTo(const Common::Point &pos);
	void saveView();
	void redraw();

	// -- File data --
	Common::Rect _src;		// the part of the picture that is in view
	Common::Rect _dest;
	Common::String _saveName;
	uint16 _hoverCursor = 0;
	uint16 _dragCursor = 0;
	Common::Array<ActionZone> _zones;

	// -- Runtime --
	bool _isMouseDown = false;
	bool _hasDragged = false;
	Common::Point _pressPos;
	Common::Point _lastDragPos;
	SceneChangeDescription _zoneScene;
	bool _zoneSceneRequested = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_DRAGSURFACEPUZZLE_H
