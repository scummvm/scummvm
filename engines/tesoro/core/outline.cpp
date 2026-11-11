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

#include "tesoro/core/outline.h"

#include "common/util.h"

namespace Tesoro {

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
