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

#ifndef TESORO_CORE_RENDERQUEUE_H
#define TESORO_CORE_RENDERQUEUE_H

#include "common/array.h"

#include "tesoro/core/animimage.h"

namespace Tesoro {

/**
 * Holds what will be visible on the screen for the current frame,  kept as a list of horizontal spans.
 * Sprites are queued one image row at a time; a span that is nearer cuts the spans behind it,
 * Nothing is copied until the frame is composed.
 */
class RenderQueue {
public:
	// /** Where the pixels of a span come from: nothing (already drawn), a solid colour, or a pointer into an image. */
	enum { kNoPixels = 0, kSolid = 1 };

	/* a span is a horizontal segment of a row, with its depth, pixel source, and colour */
	struct Span {
		int next = 0;			// index of the next span in the row, 0 for the end
		uintptr_t pixels = 0;	// kNoPixels, kSolid, or the address of the first 16-bit pixel
		int16 x0 = 0, x1 = 0;	// first and last screen column
		uint16 depth = 0;
		uint16 colour = 0;
	};

	enum { kRows = 480, kWidth = 640, kLiteralNodes = 125, kMaskNodes = 50 };

	/** One screen row: the list head and the spans, taken from a small pool of fixed size. */
	class Row {
	public:
		/** A mask row holds shadow spans: its sources are not pixel pointers, so cutting a span never moves them. */
		explicit Row(uint nodes, bool mask = false);

		void clear();
		const Span &span(int i) const { return _nodes[i]; }
		int head() const { return _head; }

		/**
		 * Adds the span xs..xe at depth, cutting whatever is behind it and being cut by what is in front. drawn tells that
		 * the pixels were already written to the frame: a piece of the span cut off at its start then gets no pixels.
		 */
		void insert(int32 xs, int32 xe, uint16 depth, uintptr_t pixels, uint16 colour, bool drawn = false);

	private:
		friend class RenderQueue;

		int allocate();
		int next(int i) const { return i == 0 ? _head : _nodes[i].next; }
		void setNext(int i, int to) { if (i == 0) _head = to; else _nodes[i].next = to; }

		int _head = 0;
		uint _bump = 2;
		int _free = 0;
		bool _mask = false;
		Common::Array<Span> _nodes;
	};

	RenderQueue();

	/** Empties every row and the counters of the frame. */
	void reset();

	/**
	 * Queues an image with its top left corner at (x, y). Literal images and solid colour images go into the row
	 * lists; any other type is a shadow image, whose rows go into the mask lists with extra (the lowest and highest depth of
	 * the spans it darkens, in its two halves, or 0 for those behind it).
	 */
	void addSprite(int32 x, int32 y, uint16 depth, const AnimImage &image, uint32 extra);

	const Row &row(uint y) const { return _rows[y]; }

private:
	void queueRows(const byte *image, uint firstRow, uint count, uint y, int32 x, uint16 depth, bool clip, bool solid);

	Common::Array<Row> _rows;
	Common::Array<Row> _maskRows;
};

} // End of namespace Tesoro

#endif
