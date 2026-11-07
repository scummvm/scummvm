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

#ifndef TESORO_CORE_DISPLAY_H
#define TESORO_CORE_DISPLAY_H

#include "common/array.h"

#include "tesoro/core/renderqueue.h"

namespace Tesoro {

/**
 * The 640x480 screen of 16-bit pixels. Objects hand it their images during the frame; compose() then turns the
 * queued spans into pixels.
 */
class Display {
public:
	enum { kWidth = RenderQueue::kWidth, kHeight = RenderQueue::kRows };

	Display();

	/**
	 * Adds an image to the queue with its top left corner at (x, y), clipped to the screen.
	 * extra is passed on to the shadow blit and unused by the other image types.
	 */
	void queueImage(int32 x, int32 y, uint16 depth, const AnimImage &image, uint32 extra);

	/**
	 * Copies the queued spans of every row into the frame, left to right.
	 * A row must be covered from end to end (filled at least with the bg);
	 */
	void compose();

	/** Empties the queues for the next frame. The pixels of the frame stay until the next compose(). */
	void startFrame();

	const uint16 *frame() const { return &_frame[0]; }

	// Temp: a flat colour behind everything, so every row is covered.
	void fillBackground(uint16 colour) { _queue.fillBackground(colour); }

private:
	RenderQueue _queue;
	Common::Array<uint16> _frame;
};

} // End of namespace Tesoro

#endif
