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

#ifndef VCRUISE_RUNTIME_H
#define VCRUISE_RUNTIME_H

class OSystem;

namespace Graphics {

struct WinCursorGroup;

} // End of namespace Graphics

namespace VCruise {

enum GameState {
	kGameStateBoot,
	kGameStateCinematic,
	kGameStateQuit,
};

class Runtime {
public:
	explicit Runtime(OSystem *system);
	virtual ~Runtime();

	void loadCursors(const char *exeName);

	bool runFrame();
	void drawFrame();

private:
	void bootGame();

	Common::Array<Common::SharedPtr<Graphics::WinCursorGroup> > _cursors;		// Cursors indexed as CURSOR_CUR_##
	Common::Array<Common::SharedPtr<Graphics::WinCursorGroup> > _cursorsShort;	// Cursors indexed as CURSOR_#

	OSystem *_system;
	uint _roomNumber;
	GameState _gameState;
};

} // End of namespace VCruise

#endif
