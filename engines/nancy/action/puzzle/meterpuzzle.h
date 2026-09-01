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

#ifndef NANCY_ACTION_METERPUZZLE_H
#define NANCY_ACTION_METERPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/movieplayer.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// A meter/gauge display, new in Nancy14 (AR 179): shows one frame of a bar
// animation. In modes 1/2 the value is a shared value table entry, driven by
// BlockingPuzzle (AR 180). In Nancy14, it handles the health meters for the
// final fight.
class MeterPuzzle : public RenderActionRecord {
public:
	MeterPuzzle() : RenderActionRecord(7) {}
	virtual ~MeterPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "MeterPuzzle"; }

	void redraw();
	int32 sampleValue() const;	// the tracked value (a value-table entry in modes 1/2)
	int computeFrame() const;	// the frame for the current value

	// -- File data --
	int16 _mode = 0;			// 0x00 - mode (0/1/2)
	int16 _modeParam = 0;		// 0x02
	int32 _modeValue = 0;		// 0x04

	Common::Path _animName;		// 0x08 - the bar animation
	uint16 _videoFormat = 0;	// 0x29
	int16 _value = 0;			// 0x2b - fallback value
	int16 _firstFrame = 0;		// 0x2d
	int16 _lastFrame = 0;		// 0x2f
	Common::Rect _srcRect;		// 0x31
	Common::Rect _destRect;		// 0x41 - draw position

	// -- Runtime state --
	MoviePlayer _animation;
	int16 _displayedValue = -1;	// last drawn frame
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_METERPUZZLE_H
