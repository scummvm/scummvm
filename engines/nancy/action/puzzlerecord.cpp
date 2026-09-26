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

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

static void appendSceneInfo(Common::String &info, const char *name, const SceneChangeWithFlag &scene) {
	if (scene._sceneChange.sceneID == kNoScene) {
		return;
	}

	if (!info.empty()) {
		info += ", ";
	}

	info += Common::String::format("%s scene %u", name, scene._sceneChange.sceneID);

	if (scene._flag.label != kEvNoEvent) {
		info += Common::String::format(" (flag %d, %s -> %s)", scene._flag.label,
			g_nancy->getEventFlagName(scene._flag.label).c_str(),
			scene._flag.flag == g_nancy->_true ? "true" : "false");
	}
}

Common::String PuzzleRecord::getRecordExtraInfo() const {
	Common::String info;
	appendSceneInfo(info, "Solve", _solveScene);
	appendSceneInfo(info, "Exit", _exitScene);
	return info;
}

void PuzzleRecord::readExitHotspot(Common::SeekableReadStream &stream) {
	Nancy::readExitHotspot(stream, _exitHotspot, _exitCursorType, _exitScene._sceneChange, _exitScene._flag);
	_exitCursorFromData = true;
}

bool PuzzleRecord::isExitHotspotHovered(const NancyInput &input) const {
	return !_exitHotspot.isEmpty() &&
		NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos);
}

void PuzzleRecord::setExitCursor() const {
	if (_exitCursorFromData) {
		// Zone cursors use the idle sprite in Nancy13, and the hotspot sprite from Nancy14 on
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_exitCursorType, true,
										g_nancy->getGameType() >= kGameTypeNancy14);
	} else {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
	}
}

bool PuzzleRecord::hoverExitHotspot(const NancyInput &input) const {
	if (!isExitHotspotHovered(input)) {
		return false;
	}

	setExitCursor();
	return true;
}

} // End of namespace Action
} // End of namespace Nancy
