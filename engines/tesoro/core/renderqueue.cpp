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

RenderQueue::Row::Row(uint nodes, bool mask) {
	_mask = mask;
	_nodes.resize(nodes);
}

void RenderQueue::Row::clear() {
	_head = 0;
	_bump = 2;
	_free = 0;
}

int RenderQueue::Row::allocate() {
	if (_free) {
		const int node = _free;
		_free = _nodes[node].next;
		return node;
	}

	if (_bump >= _nodes.size()) {
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

		Span &node = _nodes[cur];
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
						Span &old = _nodes[cur];
						_nodes[right] = old;
						if (_mask || old.pixels < 2) {
							_nodes[right].x0 = (int16)(xe + 1);
						} else {
							const int32 skipped = xe - nx0 + 1;
							_nodes[right].x0 = (int16)(_nodes[right].x0 + skipped);
							_nodes[right].pixels += skipped * 2;
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
					_nodes[left].next = cur;
					_nodes[left].x1 = (int16)(nx0 - 1);
					_nodes[left].x0 = startX;
					_nodes[left].pixels = drawn ? 0 : pixels;
					_nodes[left].colour = colour;
					_nodes[left].depth = depth;
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
	_nodes[added].next = next(pred);
	_nodes[added].x0 = startX;
	_nodes[added].x1 = (int16)xe;
	_nodes[added].pixels = pixels;
	_nodes[added].colour = colour;
	_nodes[added].depth = depth;
	setNext(pred, added);
}

RenderQueue::RenderQueue() {
	for (uint i = 0; i < kRows; ++i)
		_rows.push_back(Row(kLiteralNodes));
	for (uint i = 0; i < kRows; ++i)
		_maskRows.push_back(Row(kMaskNodes, true));
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

} // End of namespace Tesoro
