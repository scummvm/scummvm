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

#ifndef ASYLUM_PUZZLES_PUZZLE_H
#define ASYLUM_PUZZLES_PUZZLE_H

#include "common/rect.h"
#include "common/serializer.h"

#include "asylum/eventhandler.h"
#include "asylum/shared.h"

namespace Asylum {

class AsylumEngine;
class Screen;
class Sound;
class VideoPlayer;
class Cursor;
class GraphicResource;
struct GraphicQueueItem;

class Puzzle : public EventHandler, public Common::Serializable {
public:
	Puzzle(AsylumEngine *engine);
	virtual ~Puzzle() {};

	bool handleEvent(const AsylumEvent &evt);

	// Serializable
	virtual void saveLoadWithSerializer(Common::Serializer &s);

protected:
	AsylumEngine *_vm;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	virtual bool init(const AsylumEvent &evt) = 0;
	virtual bool update(const AsylumEvent &evt);
	virtual void updateScreen() {};
	virtual void updateCursor() {};
	virtual bool activate(const AsylumEvent &evt) { return true; }
	virtual bool key(const AsylumEvent &evt) { return true; }
	virtual bool mouseLeftUp(const AsylumEvent &evt) { return true; }
	virtual bool mouseLeftDown(const AsylumEvent &evt) { return true; }
	virtual bool mouseRightUp(const AsylumEvent &evt) { return true; }
	virtual bool mouseRightDown(const AsylumEvent &evt) { return true; }

	bool keyExit(const AsylumEvent &evt);
	void exitPuzzle();

	//////////////////////////////////////////////////////////////////////////
	// Hit test functions
	//////////////////////////////////////////////////////////////////////////
	bool hitTest(const int16 (*polygonPoint)[2], const Common::Point &point, uint32 index) const;
	bool hitTest(const int16 (*polygonPoint)[2], const Common::Point &point) const;
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLES_PUZZLE_H
