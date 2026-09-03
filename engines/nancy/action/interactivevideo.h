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

#ifndef NANCY_ACTION_INTERACTIVEVIDEO_H
#define NANCY_ACTION_INTERACTIVEVIDEO_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class ActionManager;
class PlaySecondaryMovie;

struct InteractiveHotspot {
	Common::Rect hotspot;
	int32 setID = 0;
	int16 flagID = -1;
	int16 cursorID = -1;
};

struct InteractiveFrame {
	uint16 frameID = 0;
	bool triggerOnNoHotspot = false;
	int16 noHSFlagID = -1;
	int16 noHSCursorID = -1;
	Common::Array<InteractiveHotspot> hotspots;
};

// Contents of an .iv file: the movie the hotspots belong to, and the list of
// movie frames that have hotspots on them.
struct InteractiveVideoData {
	Common::Path videoName;
	Common::Array<InteractiveFrame> frames;
};

void readInteractiveVideoFile(const Common::Path &filename, InteractiveVideoData &data);

class InteractiveVideo : public ActionRecord {
public:
	InteractiveVideo() {}
	virtual ~InteractiveVideo() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "InteractiveVideo"; }

	Common::Array<FlagDescription> _flags;
	Common::Array<int16> _cursors;

	InteractiveVideoData _ivData;

	// Pointer to a movie AR
	PlaySecondaryMovie *_movieAR = nullptr;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_INTERACTIVEVIDEO_H
