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
 * Holds what will be visible on the screen for the current frame, kept as a list of horizontal spans.
 * Sprites are queued one image row at a time; a span that is nearer cuts the spans behind it.
 * Nothing is copied until the frame is composed.
 *
 * For instance, for any given row, the image might be comprised of:
 *  -------------------------------------------------------------------------------------------------------------------
 * |    sprite A   |          sprite B        | sprite A |      sprite C      |        sprite A      |    sprite D     |
 *  -------------------------------------------------------------------------------------------------------------------
 */
class RenderQueue {
public:
    /** Where the pixels of a span come from: nothing (already drawn), a solid colour, or a pointer into an image. */
    enum { kNoPixels = 0, kSolid = 1 };
    enum { kRows = 480, kWidth = 640, kLiteralSpans = 125, kMaskSpans = 50 };

    /**
     * A span is a horizontal segment of a row, with its depth, pixel source, and color.
     * Each span has a pointer to the index of the next span in the row. Index 0 is the end of the list.
     * */
    struct Span {
        int next = 0;			// index of the next span in the row, 0 for the end
        uintptr_t pixels = 0;	// kNoPixels, kSolid, or the address of the first 16-bit pixel
        int16 x0 = 0, x1 = 0;	// first and last screen column
        uint16 depth = 0;
        uint16 colour = 0;
    };

    /** One of 480 horizontal lines of the screen. Each row holds a list of spans. */
    class Row {
    public:
        /** A mask row holds shadow spans: its sources are not pixel pointers. */
        explicit Row(uint spans, bool mask = false);

        void clear();
        const Span &span(int i) const { return _spans[i]; }
        int head() const { return _head; }

        /**
         * Adds the span xs..xe at depth, cutting whatever is behind it and being cut by what is in front.
         * If drawn is true, a piece of the span cut off at its start then gets no pixels.
         */
        void insert(int32 xs, int32 xe, uint16 depth, uintptr_t pixels, uint16 colour, bool drawn = false);

    private:
        friend class RenderQueue;

        int allocate();
        int next(int i) const { return i == 0 ? _head : _spans[i].next; }
        void setNext(int i, int to) { if (i == 0) _head = to; else _spans[i].next = to; }


        int _head = 0; // index of the first span in the row
        uint _bump = 2;	// next never-used slot: slot 0 means no span, and slot 1 is left unused as in the original
        int _free = 0;
        bool _mask = false;
        Common::Array<Span> _spans;
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

    /**
	 * Scaled images are instead rastered (640 pixels per row), scaled by scaleX and scaleY (16.16 fixed point, a
	 * negative scaleY turning it upside down).
     * Note that visible pieces are also entered in the row lists(thats the kNoPixels),
     * so that queued images behind it do not paint over it when the rows are composed.
     * Solid and shadow images are not painted:
	 * their scaled stretches only go into the row lists.
	 */
	void rasterScaledImage(uint16 *frame, int32 x, int32 y, uint16 depth, const AnimImage &image, int32 scaleX, int32 scaleY, uint32 extra);

    /**
	 * Puts the shadow spans of a row into its main row: where a shadow falls on a span of the row it is turned into a shadow
	 * span (x0 = -1, x1 = the number of pixels, colour = the shade table's page)
	 */
	void mergeMask(uint y, uint16 *frameRow, const byte *table);

	/** The shade of a pixel colour on a page of the shade table. */
	static uint16 shade(const byte *table, uint16 page, uint16 colour);

    const Row &row(uint y) const { return _rows[y]; }

    // Temporary: one solid span over every row, behind everything (depth 0xffff is the furthest).
    void fillBackground(uint16 colour);

private:
    void rasterScaledRows(uint16 *frame, const byte *image, uint y, uint count, int32 xFixed, int32 rowFixed, uint16 depth, bool clip, int32 scaleX, int32 rowStep);
    void queueRows(const byte *image, uint firstRow, uint count, uint y, int32 x, uint16 depth, bool clip, bool solid);

    Common::Array<Row> _rows;
    Common::Array<Row> _maskRows;
};

} // End of namespace Tesoro

#endif
