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

#include "tesoro/core/renderqueue.h"

#include "common/endian.h"
#include "common/textconsole.h"

namespace Tesoro {

RenderQueue::Row::Row(uint spans, bool mask) {
	_mask = mask;
	_spans.resize(spans);
}

void RenderQueue::Row::clear() {
	_head = 0;
	_bump = 2;
	_free = 0;
}

int RenderQueue::Row::allocate() {
	if (_free) {
		const int node = _free;
		_free = _spans[node].next;
		return node;
	}

	if (_bump >= _spans.size()) {
		// The original does not check and overwrites the next row.
		warning("RenderQueue: row is full");
		return -1;
	}
	return _bump++;
}

void RenderQueue::Row::insert(int32 xs, int32 xe, uint16 depth, uintptr_t pixels, uint16 colour, bool drawn) {
	int pred = 0;
	int cur = _head;
	bool hidden = false;
	int16 startX;

	for (;;) {
		startX = (int16)xs;
		if (!cur)
			break;

		Span &node = _spans[cur];
		const int32 nx0 = node.x0;
		const int32 nx1 = node.x1;

		if (xs <= nx1) {
			if (xe < nx0)
				break;

			if (depth < node.depth) {
				// The new span is nearer: it cuts the one it overlaps.
				if (nx0 < xs) {
					if (xe < nx1) {
						// It lies inside the old span, which is split around it.
						const int right = allocate();
						if (right < 0)
							return;
						Span &old = _spans[cur];
						_spans[right] = old;
						if (_mask || old.pixels < 2) {
							_spans[right].x0 = (int16)(xe + 1);
						} else {
							const int32 skipped = xe - nx0 + 1;
							_spans[right].x0 = (int16)(_spans[right].x0 + skipped);
							_spans[right].pixels += skipped * 2;
						}
						old.x1 = (int16)(startX - 1);
						old.next = right;
						pred = cur;
						break;
					}
					node.x1 = (int16)(startX - 1);
				} else {
					if (xe < nx1) {
						// It covers the front of the old span.
						if (_mask || node.pixels < 2) {
							node.x0 = (int16)(xe + 1);
						} else {
							const int32 skipped = xe - nx0 + 1;
							node.x0 = (int16)(node.x0 + skipped);
							node.pixels += skipped * 2;
						}
						break;
					}

					// It covers the whole old span, which goes back to the pool.
					setNext(pred, node.next);
					node.next = _free;
					_free = cur;
					cur = pred;
				}
			} else {
				// The old span is in front: the new one only shows where it does not reach.
				if (xs < nx0) {
					if (xe <= nx1) {
						xe = nx0 - 1;
						break;
					}

					const int left = allocate();
					if (left < 0)
						return;
					_spans[left].next = cur;
					_spans[left].x1 = (int16)(nx0 - 1);
					_spans[left].x0 = startX;
					_spans[left].pixels = drawn ? 0 : pixels;
					_spans[left].colour = colour;
					_spans[left].depth = depth;
					setNext(pred, left);
				} else if (xe <= nx1) {
					hidden = true;
					break;
				}

				const int32 skipped = nx1 - xs + 1;
				xs += skipped;
				if (!_mask && pixels >= 2)
					pixels += skipped * 2;
			}
		}

		pred = cur;
		cur = next(cur);
	}

	if (hidden)
		return;

	const int added = allocate();
	if (added < 0)
		return;
	_spans[added].next = next(pred);
	_spans[added].x0 = startX;
	_spans[added].x1 = (int16)xe;
	_spans[added].pixels = pixels;
	_spans[added].colour = colour;
	_spans[added].depth = depth;
	setNext(pred, added);
}

RenderQueue::RenderQueue() {
	for (uint i = 0; i < kRows; ++i)
		_rows.push_back(Row(kLiteralSpans));
	for (uint i = 0; i < kRows; ++i)
		_maskRows.push_back(Row(kMaskSpans, true));
}

void RenderQueue::reset() {
	for (uint i = 0; i < kRows; ++i) {
		_rows[i].clear();
		_maskRows[i].clear();
	}
}

void RenderQueue::addSprite(int32 x, int32 y, uint16 depth, const AnimImage &image, uint32 extra) {
	const byte *data = image.data();
	if (!data)
		return;

	const int32 right = READ_LE_UINT16(data + 8) - 1 + x;
	int32 bottom = READ_LE_UINT16(data + 10) - 1 + y;
	if (!(x < kWidth && right >= 0 && y < kRows && bottom >= 0))
		return;

	uint skipped = 0;
	if (y < 0) {
		skipped = -y;
		y = 0;
	}
	if (bottom > kRows - 1)
		bottom = kRows - 1;
	const uint count = bottom - y + 1;
	const bool clip = x < 0 || right > kWidth - 1;

	const uint type = READ_LE_UINT16(data + 12);
	if (type == 1 || type == 3)
		queueRows(data, skipped, count, y, x, depth, clip, type == 3);
	// else
	// 	queueMaskRows(data, skipped, count, y, x, depth, clip, extra);
}

void RenderQueue::fillBackground(uint16 colour) {
	// for each row insert a solid span covering the entire width at the furthest depth
	for (uint y = 0; y < kRows; ++y)
		_rows[y].insert(0, kWidth - 1, 0xffff, kSolid, colour);
}

uint16 RenderQueue::shade(const byte *table, uint16 page, uint16 colour) {
	const uint index = ((colour >> 1) & 0x38e) | ((colour >> 9) & 0x70);
	return READ_LE_UINT16(table + page * 256 + index);
}

void RenderQueue::mergeMask(uint y, uint16 *frameRow, const byte *table) {
	Row &main = _rows[y];
	Row &mask = _maskRows[y];
	int mainPred = 0, maskPrev = 0;

	while (mask.next(maskPrev)) {
		const int s = mask.next(maskPrev);
		const Span shadow = mask._spans[s];
		int m = main.next(mainPred);
		const int32 sx0 = shadow.x0, sx1 = shadow.x1;

		// The first span of the row that reaches the shadow's start.
		while (m && main._spans[m].x1 < sx0) {
			mainPred = m;
			m = main._spans[m].next;
		}
		if (!m) {
			warning("RenderQueue: a shadow span finds no span to fall on");
			return;
		}

		Span &under = main._spans[m];
		const int32 mx0 = under.x0, mx1 = under.x1;

		// Whether the shadow misses this span: its depth window does not hold it, or (without one) it is in front of the shadow.
		bool misses;
		if ((uint32)shadow.pixels != 0) {
			const uint16 low = (uint16)shadow.pixels, high = (uint16)((uint32)shadow.pixels >> 16);
			misses = under.depth < low || under.depth > high;
		} else {
			misses = shadow.depth > under.depth;
		}

		if (misses) {
			if (sx1 <= mx1) {
				// The shadow is dropped; its node goes back to the pool.
				mask.setNext(0, shadow.next);
				mask._spans[s].next = mask._free;
				mask._free = s;
				maskPrev = 0;
			} else {
				mask._spans[s].x0 = (int16)(mx1 + 1);
				mainPred = m;
			}
			continue;
		}

		// Darkens a span in place (a solid colour), or turns it into a shadow span over its source.
		const uint16 page = shadow.colour;
		const uintptr_t source = under.pixels;
		const bool literal = source > 1, solid = source == 1;

		if (sx0 == mx0) {
			if (sx1 > mx1 || sx1 == mx1) {
				// The span is shadowed from end to end.
				const int32 width = mx1 - mx0 + 1;
				if (sx1 > mx1)
					mask._spans[s].x0 = (int16)(mx1 + 1);
				else
					maskPrev = s;
				mainPred = m;
				if (solid) {
					under.colour = shade(table, page, under.colour);
				} else {
					under.x0 = (int16)-1;
					under.x1 = (int16)width;
					under.colour = page;
					if (!literal)
						under.pixels = (uintptr_t)(frameRow + mx0);
				}
				continue;
			}

			// The shadow covers the front of the span, which keeps the rest.
			const int front = main.allocate();
			if (front < 0)
				return;
			Span &part = main._spans[front];
			part = shadow;
			part.next = m;
			main.setNext(mainPred, front);
			mainPred = front;
			mask.setNext(0, shadow.next);
			maskPrev = 0;
			const int32 count = sx1 - sx0 + 1;
			main._spans[m].x0 = (int16)(sx1 + 1);
			if (literal) {
				part.x0 = (int16)-1;
				part.x1 = (int16)count;
				part.pixels = source;
				main._spans[m].pixels = source + count * 2;
			} else if (solid) {
				part.pixels = 1;
				part.colour = shade(table, page, main._spans[m].colour);
			} else {
				part.x0 = (int16)-1;
				part.x1 = (int16)count;
				part.pixels = (uintptr_t)(frameRow + sx0);
			}
			continue;
		}

		if (sx1 > mx1) {
			// The shadow starts inside the span and goes on past its end: the span is cut and its tail shadowed.
			const int tail = main.allocate();
			if (tail < 0)
				return;
			Span &part = main._spans[tail];
			const int32 count = mx1 - sx0 + 1;
			part.colour = page;
			part.depth = shadow.depth;
			part.next = main._spans[m].next;
			main._spans[m].x1 = (int16)(sx0 - 1);
			main._spans[m].next = tail;
			mask._spans[s].x0 = (int16)(mx1 + 1);
			mainPred = tail;
			if (literal) {
				part.x0 = (int16)-1;
				part.x1 = (int16)count;
				part.pixels = source + (sx0 - mx0) * 2;
			} else if (solid) {
				part.x1 = (int16)mx1;
				part.x0 = (int16)sx0;
				part.pixels = 1;
				part.colour = shade(table, page, main._spans[m].colour);
			} else {
				part.x0 = (int16)-1;
				part.x1 = (int16)count;
				part.pixels = (uintptr_t)(frameRow + sx0);
			}
			continue;
		}

		// The shadow ends inside the span or with it: the span is cut, the shadow follows it, and the rest (if any) after that.
		const int mid = main.allocate();
		if (mid < 0)
			return;
		int rest = 0;
		if (sx1 < mx1) {
			rest = main.allocate();
			if (rest < 0)
				return;
		}
		Span &part = main._spans[mid];
		part = shadow;
		const int32 count = sx1 - sx0 + 1;
		if (rest) {
			Span &after = main._spans[rest];
			after.x0 = (int16)(sx1 + 1);
			after.x1 = (int16)mx1;
			after.colour = main._spans[m].colour;
			after.depth = main._spans[m].depth;
			after.next = main._spans[m].next;
			part.next = rest;
		} else {
			part.next = main._spans[m].next;
		}
		main._spans[m].next = mid;
		main._spans[m].x1 = (int16)(sx0 - 1);
		mainPred = mid;
		mask.setNext(0, shadow.next);
		maskPrev = 0;
		if (literal) {
			part.x0 = (int16)-1;
			part.x1 = (int16)count;
			part.pixels = source + (sx0 - mx0) * 2;
			if (rest)
				main._spans[rest].pixels = source + (sx1 - mx0) * 2 + 2;
		} else if (solid) {
			part.pixels = 1;
			part.colour = shade(table, page, main._spans[m].colour);
			if (rest)
				main._spans[rest].pixels = 1;
		} else {
			part.x0 = (int16)-1;
			part.x1 = (int16)count;
			part.pixels = (uintptr_t)(frameRow + sx0);
			if (rest)
				main._spans[rest].pixels = 0;
		}
	}
}

void RenderQueue::queueRows(const byte *image, uint firstRow, uint count, uint y, int32 x, uint16 depth, bool clip, bool solid) {
	for (uint r = 0; r < count; ++r) {
		const byte *entry = image + 0x10 + 8 * (firstRow + r);
		const byte *item = image + READ_LE_UINT32(entry);
		uint items = READ_LE_UINT16(entry + 4);
		Row &row = _rows[y + r];
		int32 base = x;

		while (items) {
			--items;
			uintptr_t pixels = solid ? (uintptr_t)kSolid : (uintptr_t)(image + READ_LE_UINT32(item));
			const uint16 colour = solid ? READ_LE_UINT16(item) : 0;
			int32 xs = base + READ_LE_UINT16(item + 4);
			base = xs + READ_LE_UINT16(item + 6);
			item += 8;
			int32 xe = base - 1;

			if (clip) {
				if (xe < 0)
					continue;
				if (xs < 0) {
					if (!solid)
						pixels += -xs * 2;
					xs = 0;
				}
				if (xs > kWidth - 1)
					break;
				if (xe > kWidth - 1)
					xe = kWidth - 1;
			}

			row.insert(xs, xe, depth, pixels, colour);
		}
	}
}

// 32-bit wrapping arithmetic like the original's.
static int32 wrapMul(int32 a, int32 b) { return (int32)((uint32)a * (uint32)b); }

void RenderQueue::rasterScaledImage(uint16 *frame, int32 x, int32 y, uint16 depth, const AnimImage &image, int32 scaleX, int32 scaleY, uint32 extra) {
	const byte *data = image.data();
	if (!data)
		return;

	const uint32 width = READ_LE_UINT16(data + 8);
	const uint32 height = READ_LE_UINT16(data + 10);
	const int32 right = (wrapMul(width - 1, scaleX) >> 16) + x;
	int32 bottom = (wrapMul(height - 1, scaleY) >> 16) + y;
	int32 rows = bottom - y;
	const int32 xFixed = (int32)((uint32)x << 16);
	int32 rowFixed, rowStep;

	if (rows < 0) {
		// Upside down: the original walks the image from its last row.
		rows = -rows;
		rowStep = -(int32)(0xffffffffu / (uint32)-scaleY);
		y = bottom < 0 ? 0 : bottom;
		rowFixed = -wrapMul((bottom + rows) - y, rowStep);
		bottom += rows;
	} else {
		rowStep = (int32)(0xffffffffu / (uint32)scaleY);
		if (y < 0) {
			rowFixed = -wrapMul(rowStep, y);
			y = 0;
		} else {
			rowFixed = 0;
		}
	}

	if (bottom > kRows - 1)
		bottom = kRows - 1;
	const int32 count = (bottom - y) + 1;

	if (!(x <= right && x < kWidth && right >= 0 && y < kRows && bottom >= 0))
		return;

	const uint type = READ_LE_UINT16(data + 12);
	const bool clip = x < 0 || right > kWidth - 1;
	if (type == 1)
		rasterScaledRows(frame, data, y, count, xFixed, rowFixed, depth, clip, scaleX, rowStep);
}

void RenderQueue::rasterScaledRows(uint16 *frame, const byte *image, uint y, uint count, int32 xFixed, int32 rowFixed, uint16 depth, bool clip, int32 scaleX, int32 rowStep) {
	if ((int16)count == 0)
		return;

	if ((scaleX >> 8) == 0) {
		warning("RenderQueue: image scaled to nothing");
		return;
	}
	const int32 inverseX = 0x1000000 / (scaleX >> 8);
	int32 x = xFixed >> 16;

	for (uint r = 0; r < (uint16)count; ++r, ++y) {
		Row &row = _rows[y];
		uint16 *out = frame + y * kWidth;
		const int32 sourceRow = rowFixed >> 16;
		rowFixed += rowStep;

		const byte *entry = image + 0x10 + 8 * sourceRow;
		const byte *item = image + READ_LE_UINT32(entry);
		uint items = READ_LE_UINT16(entry + 4);
		int32 sourceX = 0;

		while (items) {
			--items;
			const byte *pixels = image + READ_LE_UINT32(item);
			const uint32 start = (uint32)wrapMul(sourceX + READ_LE_UINT16(item + 4), scaleX);
			sourceX += READ_LE_UINT16(item + 4) + READ_LE_UINT16(item + 6);
			item += 8;
			int32 xs = ((int32)start >> 16) + x;
			int32 xe = ((wrapMul(sourceX, scaleX) - 0x7fff) >> 16) + x;
			int32 offset = -(int32)(((start >> 8) & 0xff) * (uint32)(inverseX >> 8));
			if (offset != 0) {
				++xs;
				offset += inverseX;
			}

			if (clip) {
				if (xe < 0)
					continue;
				if (xs < 0) {
					offset -= wrapMul(xs, inverseX);
					xs = 0;
				}
				if (xs > kWidth - 1)
					break;
				if (xe > kWidth - 1)
					xe = kWidth - 1;
			}

			if (xs > xe)
				continue;

			// Paint the pixels now ...
			uint32 position = (uint32)offset;
			uint16 *dst = out + xs;
			for (uint n = (uint16)(xe - xs + 1); n != 0; --n) {
				*dst++ = READ_LE_UINT16(pixels + 2 * (position >> 16));
				position += inverseX;
			}

			// ... and enter the span without pixels, unless the image is not scaled.
			const uintptr_t source = scaleX == 0x10000 ? (uintptr_t)(pixels + (offset >> 16) * 2) : 0;
			row.insert(xs, xe, depth, source, 0, true);
		}
	}
}
} // End of namespace Tesoro
