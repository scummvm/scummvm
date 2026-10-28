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

#ifndef NANCY_SCRIPT_XSHEET_H
#define NANCY_SCRIPT_XSHEET_H

#include "common/array.h"
#include "common/str.h"

#include "engines/nancy/movieplayer.h"

#include "engines/nancy/script/records.h"

namespace Nancy {
namespace Script {

// A character in a conversation is put together from movies, one for each part
// of it (body, head). An XSheet lists the parts, where each goes in the
// viewport, and the frame of each part's movie to show at each of its own
// frames. A part without a frame number plays through by itself.
class XSheet {
public:
	~XSheet() { clear(); }

	// Shows the first frame
	bool load(const Common::String &name);
	void clear();

	bool isLoaded() const { return !_trees.empty(); }
	const Common::String &getName() const { return _name; }
	uint getNumFrames() const { return _numFrames; }
	uint32 getFramesPerSecond() const { return _framesPerSecond; }
	uint getFrame() const { return _frame; }

	void showFrame(uint frame);

	// The parts that play by themselves at the frame that shows
	void startFreeRunning();
	void updateFreeRunning();
	bool isFreeRunningDone() const;

	// Puts the parts that play by themselves on their last frame, so they stay
	// together with the rest
	void parkFreeRunning();

	void setVisible(bool isVisible);

	// The first part is drawn over the ones after it
	void setZ(double z, const SceneContext &context);

private:
	static const int32 kFreeRunning = -1;

	struct Tree {
		MoviePlayer player;
		RecordGraphic graphic;
		Common::Rect dest;
		Common::Array<int32> frames;	// the movie frame to show at each frame of the XSheet
		int32 shownFrame = -1;
	};

	bool isTreeFreeRunning(const Tree &tree) const;
	void showMovieFrame(Tree &tree, int32 movieFrame);
	void drawTree(Tree &tree, const Graphics::Surface &frame);

	Common::String _name;
	Common::Array<Tree *> _trees;
	uint _numFrames = 0;
	uint32 _framesPerSecond = 0;
	uint _frame = 0;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_XSHEET_H
