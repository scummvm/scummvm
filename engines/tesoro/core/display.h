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
#include "graphics/screen.h"

#include "tesoro/core/renderqueue.h"
#include "tesoro/core/textqueue.h"

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
	 * Queues an image scaled by scaleX and scaleY (16.16 fixed point, 0x10000 unchanged, negative scaleY upside down)
	 * Used for the pointer and for characters. Drawn when the frame is composed. Images
	 * deeper than the layer depth (larger depth values) are put on the frame before the queued rows are composed, the
	 * others after, on top; the layer depth is reset to 0 every frame, which puts everything with a depth of 0 in front.
	 */
	void queueImageScaled(int32 x, int32 y, uint16 depth, const AnimImage &image, int32 scaleX, int32 scaleY, uint32 extra);

	/**
	 * Composes the queued spans into the frame.
	 * A row must be covered from end to end (filled at least with the bg);
	 */
	void compose();

	/** Empties the queues for the next frame. The pixels of the frame stay until the next compose(). */
	void startFrame();

	const uint16 *frame() const { return &_frame[0]; }

	/** The table the shadows darken through, for the inventory shadow. */
	void setShadeTable(const byte *table);

	TextQueue &texts() { return _texts; }

	/** The font bank (see TextQueue). */
	void setFont(const AnimBank *bank) { _texts.setFont(bank); }

	void queueText(const byte *text, uint32 colour, int32 centreX, int32 y, int32 maxWidth, uint16 layerDepth, bool centred, int32 lineHeight) {
		_layerDepth = _texts.add(text, colour, centreX, y, maxWidth, layerDepth, centred, lineHeight);
	}

	// Temp: a flat colour behind everything, so every row is covered.
	void fillBackground(uint16 colour) { _queue.fillBackground(colour); }

	/**
	 * Fades the last composed frame to black in the given number of steps (the brightness goes down in equal steps,
	 * reaching black on the last), showing each step on the screen and waiting delayMs between steps. Leaves the black frame in the buffer.
	 */
	void fadeOut(int steps, int delayMs, Graphics::Screen *screen);

private:
	/** An image waiting for the compose. */
	struct Scaled {
		const AnimImage *image;
		int32 x, y;
		uint16 depth;
		int32 scaleX, scaleY;
		uint32 extra;
	};
	enum { kMaxScaled = 50 };

	RenderQueue _queue;
	Common::Array<byte> _shades;
	TextQueue _texts;
	Common::Array<Scaled> _scaled;
	uint16 _layerDepth = 0;
	Common::Array<uint16> _frame;
};

} // End of namespace Tesoro

#endif
