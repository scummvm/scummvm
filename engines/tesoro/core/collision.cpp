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

#include "tesoro/core/collision.h"

#include "common/util.h"

namespace Tesoro {

namespace {

/** A point along the movement as an exact fraction, 0 at the start and 1 at the end. The denominator is positive. */
struct Fraction {
	int64 numerator, denominator;
};

Fraction makeFraction(int64 numerator, int64 denominator) {
	return denominator < 0 ? Fraction{-numerator, -denominator} : Fraction{numerator, denominator};
}

bool lessThan(const Fraction &a, const Fraction &b) {
	return a.numerator * b.denominator < b.numerator * a.denominator;
}

} // End of anonymous namespace

int32 sweepRects(int32 ax1, int32 ay1, int32 ax2, int32 ay2, int32 bx1, int32 by1, int32 bx2, int32 by2, int32 dx, int32 dy) {
	// If they already overlap, return 0 immediately.
	if (ax1 <= bx2 && bx1 <= ax2 && ay2 <= by1 && by2 <= ay1)
		return 0;

	// On each axis the two overlap during a window of time. The rectangles touch when the windows of both axes are open
	// together, so the answer is the later of the two openings, as long as it comes before either window closes and
	// within the movement (0 to 1).
	const int32 from[2] = {ax1, ay2}, to[2] = {ax2, ay1};
	const int32 otherFrom[2] = {bx1, by2}, otherTo[2] = {bx2, by1};
	const int32 move[2] = {dx, dy};

	// mark when the window opens for each axis and when it closes
	Fraction opens = {0, 1}, closes = {1, 1};
	for (int axis = 0; axis < 2; ++axis) {
		if (move[axis] == 0) {
			// If theres no movement in this axis, then it either overlaps the whole time or never does.
			if (from[axis] > otherTo[axis] || otherFrom[axis] > to[axis])
				return -1;
			continue;
		}

		// The leading edge reaches the other rectangle, then the trailing edge leaves it.
		const bool forward = move[axis] > 0;
		const Fraction start = makeFraction(forward ? otherFrom[axis] - to[axis] : otherTo[axis] - from[axis], move[axis]);
		const Fraction end = makeFraction(forward ? otherTo[axis] - from[axis] : otherFrom[axis] - to[axis], move[axis]);
		if (lessThan(opens, start))
			opens = start;
		if (lessThan(end, closes))
			closes = end;
	}

	if (lessThan(closes, opens))
		return -1;

	// The original's fixed point: 16 bits of fraction, truncated.
	return (int32)((opens.numerator * 0x10000) / opens.denominator);
}

static inline uint32 pack(int x, int y) { return (uint32)(uint16)x | ((uint32)(uint16)y << 16); }

bool Outline::contains(int px, int py) const {
	const int edges = (int)count - 1;
	if (edges == 0)
		return false;

	// The y of the point before an edge's start, skipping points level with the start (a vertex crossing counts once).
	auto previousY = [this](int k, int startY) {
		int prev = y(k == 0 ? count - 2 : k - 1);
		if (prev == startY) {
			int j = k - 2;
			do {
				const int t = j < 0 ? count - 2 : j;
				j = t - 1;
				prev = y(t);
			} while (prev == startY);
		}
		return prev;
	};

	byte crossings = 0;
	for (int k = 0; k < edges; ++k) {
		const int x1 = x(k), y1 = y(k), x2 = x(k + 1), y2 = y(k + 1);
		if (y1 < y2) {
			if (y1 <= py && py <= y2) {
				if (py == y1) {
					if (x1 < px) {
						if (previousY(k, y1) < y1)
							++crossings;
					} else if (x1 == px) {
						return true;
					}
				} else if (py != y2) {
					const int at = ((py - y1) * (x2 - x1)) / (y2 - y1) + x1;
					if (at == px)
						return true;
					if (at < px)
						++crossings;
				}
			}
		} else if (y2 < y1) {
			if (y2 <= py && py <= y1) {
				if (py == y1) {
					if (px <= x1) {
						if (x1 == px)
							return true;
					} else if (previousY(k, y1) > y1) {
						++crossings;
					}
				} else if (py != y2) {
					const int at = ((py - y1) * (x2 - x1)) / (y2 - y1) + x1;
					if (at == px)
						return true;
					if (at < px)
						++crossings;
				}
			}
		} else if (y1 == py) {
			if (MIN(x1, x2) <= px && px <= MAX(x1, x2))
				return true;
		}
	}
	return (crossings & 1) != 0;
}

void Outline::clipColumns(int bx1, int bx2, int cx1, int cx2, Outline &out, uint32 *d) const {
	// The y of the line through points i and i + 1 at column at.
	auto cut = [this](int i, int at) {
		const int q = ((at - x(i)) * (y(i + 1) - y(i))) / (x(i + 1) - x(i));
		return (int)(int16)((int16)q + (int16)y(i));
	};

	int src = 0, di = 0, i = 0;
	if (bx1 < cx1) {
		int next = x(1);
		while (next < cx1) {
			++i;
			next = x(i + 1);
		}
		d[di++] = pack(cx1, cut(i, cx1));
		src = i;
	}

	if (cx2 < bx2) {
		int next = x(src + 1);
		while (next < cx2) {
			d[di++] = points[src];
			next = x(src + 2);
			++src;
		}
		d[di] = points[src];
		out.split = (uint16)(di + 1);
		d[di + 1] = pack(cx2, cut(src, cx2));
		int q = src + 1;
		next = x(src + 2);
		while (cx2 < next) {
			next = x(q + 2);
			++q;
		}
		d[di + 2] = pack(cx2, cut(q, cx2));
		di += 3;
		src = q + 1;
	} else {
		for (uint32 c = (uint32)split - (uint32)i; c != 0; --c)
			d[di++] = points[src++];
		out.split = (uint16)(di - 1);
	}

	if (cx1 <= bx1) {
		for (int c = (int)count - src; c != 0; --c)
			d[di++] = points[src++];
		out.count = (uint16)di;
		out.points = d;
		return;
	}

	int next = x(src + 1);
	while (cx1 < next) {
		d[di++] = points[src];
		next = x(src + 2);
		++src;
	}
	d[di] = points[src];
	d[di + 1] = pack(cx1, cut(src, cx1));
	out.count = (uint16)(di + 2);
	out.points = d;
}

bool Outline::overlaps(const Outline &o) const {
	const int n1 = count, m1 = split, n2 = o.count, m2 = o.split;
	int j = 0, i = 0;
	bool f6 = false, f7 = false, f8 = false;

	// Chain 1 of the other outline against the first chain of this one, left to right.
	if (m1 != 0) {
		do {
			if (m2 < i)
				break;
			const int bx = o.x(i);
			if (x(j + 1) < bx) {
				++j;
			} else {
				const int ax = x(j);
				if (bx < ax) {
					++i;
				} else if (bx == ax) {
					if (y(j) < o.y(i)) {
						if (f6) return true;
						++i;
						f8 = true;
						f7 = true;
					} else {
						if (f7) return true;
						++i;
						f6 = true;
					}
				} else {
					const int p = (o.y(i) - y(j)) * (x(j + 1) - ax);
					const int q = (y(j + 1) - y(j)) * (bx - ax);
					if (p <= q) {
						if (f8) return true;
						f6 = true;
					} else {
						if (f6) return true;
						f8 = true;
					}
					++i;
					f7 = f8;
				}
			}
		} while (j < m1);
	}

	// The other's second chain, walked back from where this one's first chain ended.
	i = m2 + 1;
	const int start = i;
	if (j >= 0) {
		int k = i;
		do {
			if (n2 <= k)
				break;
			const int ax = x(j), bx = o.x(k);
			if (bx < ax) {
				--j;
			} else if (x(j + 1) < bx) {
				++k;
			} else if (bx == ax) {
				if (y(j) < o.y(k)) {
					if (f6) return true;
					f7 = true;
					++k;
					f8 = true;
				} else {
					if (f7) return true;
					++k;
					f6 = true;
				}
			} else {
				const int p = (o.y(k) - y(j)) * (x(j + 1) - ax);
				const int q = (y(j + 1) - y(j)) * (bx - ax);
				if (p <= q) {
					if (f8) return true;
					f6 = true;
					f7 = f8;
				} else {
					if (f6) return true;
					f7 = true;
				}
				++k;
				f8 = f7;
			}
		} while (j >= 0);
	}

	// The same from the other end: this outline's second chain against the other's first.
	int ii = 0;
	j = n1 - 1;
	if (m1 < j) {
		do {
			if (m2 < ii)
				break;
			const int bx = o.x(ii);
			if (x(j - 1) < bx) {
				--j;
			} else {
				const int ax = x(j);
				if (bx < ax) {
					++ii;
				} else if (bx == ax) {
					if (o.y(ii) < y(j)) {
						if (f7) return true;
						++ii;
						f6 = true;
					} else {
						if (f6) return true;
						++ii;
						f8 = true;
						f7 = true;
					}
				} else {
					const int p = (o.y(ii) - y(j)) * (x(j - 1) - ax);
					const int q = (y(j - 1) - y(j)) * (bx - ax);
					if (p < q) {
						if (f8) return true;
						f6 = true;
					} else {
						if (f6) return true;
						f8 = true;
					}
					++ii;
					f7 = f8;
				}
			}
		} while (m1 < j);
	}

	if (j < n1) {
		int k = start;
		while (k < n2) {
			const int ax = x(j), bx = o.x(k);
			if (bx < ax) {
				++j;
			} else if (x(j + 1) < bx) {
				++k;
			} else if (bx == ax) {
				if (o.y(k) < y(j)) {
					if (f7) return true;
					++k;
					f6 = true;
				} else {
					if (f6) return true;
					f7 = true;
					++k;
					f8 = true;
				}
			} else {
				const int p = (x(j - 1) - ax) * (o.y(k) - y(j));
				const int q = (y(j - 1) - y(j)) * (bx - ax);
				if (p < q) {
					if (f8) return true;
					f6 = true;
					f7 = f8;
				} else {
					if (f6) return true;
					f7 = true;
				}
				++k;
				f8 = f7;
			}
			if (n1 <= j)
				return false;
		}
	}
	return false;
}

} // End of namespace Tesoro
