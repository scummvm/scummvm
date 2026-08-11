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

#include "agds/region.h"
#include "agds/resourceManager.h"
#include "common/algorithm.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"
#include <math.h>

namespace AGDS {

Region::Region(const Common::String &resourceName, Common::SeekableReadStream &stream) {
	auto size = stream.size();
	name = readString(stream);
	center.x = stream.readSint16LE();
	center.y = stream.readSint16LE();
	flags = stream.readUint16LE();
	debug("region %s at (%d,%d) %04x", name.c_str(), center.x, center.y, flags);
	while (stream.pos() + 2 <= size) {
		uint16 ext = stream.readUint16LE();
		if (ext)
			debug("extended entries %u", ext);

		PointsType points;
		while (ext--) {
			int16 a = stream.readSint16LE();
			int16 b = stream.readSint16LE();
			int16 c = stream.readUint16LE();
			if (c != -12851) // 0xcdcd
				debug("extended entry: %d %d %d", a, b, c);
			else
				debug("extended entry: %d %d", a, b);
			points.push_back(Common::Point(a, b));
		}
		regions.push_back(points);
	}
}

Region::Region(const Common::Rect rect) : flags(0) {
	PointsType points;
	points.push_back(Common::Point(rect.left, rect.top));
	points.push_back(Common::Point(rect.right, rect.top));
	points.push_back(Common::Point(rect.right, rect.bottom));
	points.push_back(Common::Point(rect.left, rect.bottom));
	regions.push_back(points);

	center.x = (rect.left + rect.right) / 2;
	center.y = (rect.top + rect.bottom) / 2;
}

Common::String Region::toString() const {
	Common::String str = Common::String::format("region(%d, %d, [", center.x, center.y);
	for (size_t i = 0; i < regions.size(); ++i) {
		if (i != 0)
			str += ", ";
		str += "Region {";
		const PointsType &points = regions[i];
		for (size_t j = 0; j < points.size(); ++j) {
			if (j != 0)
				str += ", ";
			str += Common::String::format("(%d, %d)", points[j].x, points[j].y);
		}
		str += "}";
	}
	str += "]";
	return str;
}

void Region::move(Common::Point rel) {
	if (rel.x == 0 && rel.y == 0)
		return;

	center += rel;
	for (uint i = 0; i < regions.size(); ++i) {
		PointsType &points = regions[i];
		for (uint j = 0; j < points.size(); ++j)
			points[j] += rel;
	}
}

Common::Point Region::topLeft() const {
	if (regions.empty())
		return Common::Point();

	Common::Point p = regions[0][0];
	for (uint i = 0; i < regions.size(); ++i) {
		const PointsType &points = regions[i];

		for (uint j = 0; j < points.size(); ++j) {
			Common::Point point = points[j];
			if (point.x < p.x)
				p.x = point.x;
			if (point.y < p.y)
				p.y = point.y;
		}
	}
	return p;
}

// FIXME: copied from wintermute/base_region.cpp

struct dPoint {
	double x, y;
};

bool Region::pointIn(Common::Point point) const {
	for (uint r = 0; r < regions.size(); ++r) {
		const PointsType &points = regions[r];
		uint32 size = points.size();
		if (size < 3) {
			continue;
		}

		int counter = 0;
		double xinters;
		dPoint p, p1, p2;

		p.x = (double)point.x;
		p.y = (double)point.y;

		p1.x = (double)points[0].x;
		p1.y = (double)points[0].y;

		for (uint32 i = 1; i <= size; i++) {
			p2.x = (double)points[i % size].x;
			p2.y = (double)points[i % size].y;

			if (p.y > MIN(p1.y, p2.y)) {
				if (p.y <= MAX(p1.y, p2.y)) {
					if (p.x <= MAX(p1.x, p2.x)) {
						if (p1.y != p2.y) {
							xinters = (p.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
							if (p1.x == p2.x || p.x <= xinters) {
								counter++;
							}
						}
					}
				}
			}
			p1 = p2;
		}

		if (counter % 2 == 0) {
			continue;
		} else {
			return true;
		}
	}
	return false;
}

// Walk-region geometry, reimplemented from the original engine.

static inline double distSq(Common::Point a, Common::Point b) {
	double dx = a.x - b.x, dy = a.y - b.y;
	return dx * dx + dy * dy;
}

static inline double cross2(double ax, double ay, double bx, double by) {
	return ax * by - ay * bx;
}

// Intersect edge a1->a2 with segment b1->b2.
// Returns 1 on a proper intersection, 0 when none, -2 when collinear.
static int segIntersect(Common::Point a1, Common::Point a2,
                        Common::Point b1, Common::Point b2,
                        double &tSegOut, double &tEdgeOut,
                        Common::Point &hit) {
	double rx = a2.x - a1.x, ry = a2.y - a1.y;
	double sx = b2.x - b1.x, sy = b2.y - b1.y;
	double denom = cross2(rx, ry, sx, sy);
	double qpx = b1.x - a1.x, qpy = b1.y - a1.y;
	if (denom == 0.0) {
		if (cross2(qpx, qpy, rx, ry) == 0.0)
			return -2;
		return 0;
	}
	double t = cross2(qpx, qpy, sx, sy) / denom; // along edge
	double u = cross2(qpx, qpy, rx, ry) / denom; // along segment
	if (t < 0.0 || t > 1.0 || u < 0.0 || u > 1.0)
		return 0;
	tEdgeOut = t;
	tSegOut = u;
	hit.x = (int16)(a1.x + t * rx);
	hit.y = (int16)(a1.y + t * ry);
	return 1;
}

int Region::walkPointInside(Common::Point p) const {
	if (p == center)
		return 1;

	for (uint r = 0; r < regions.size(); ++r) {
		const PointsType &poly = regions[r];
		uint n = poly.size();
		if (n < 3)
			continue;

		int crossings = 0, leftGrazes = 0, rightGrazes = 0;
		double rayDX = center.x - p.x, rayDY = center.y - p.y;

		for (uint i = 0; i < n; ++i) {
			Common::Point v0 = poly[i];
			Common::Point v1 = poly[(i + 1) % n];
			if (v1 == p)
				return 0; // exactly on a vertex

			double tSeg, tEdge;
			Common::Point hit;
			int res = segIntersect(v0, v1, p, center, tSeg, tEdge, hit);
			if (res == 0 || res == -2)
				continue;
			if (tEdge > 0.0 && tEdge < 1.0) {
				crossings++;
			} else {
				// vertex touch: classify by which side of the ray the
				// edge's other endpoint lies, to dedup below
				Common::Point other = (tEdge <= 0.0) ? v1 : v0;
				double side = cross2(rayDX, rayDY, other.x - p.x, other.y - p.y);
				crossings++;
				if (side < 0)
					leftGrazes++;
				else
					rightGrazes++;
			}
		}
		crossings -= MIN(leftGrazes, rightGrazes);
		if (crossings & 1)
			return -1;
	}
	return 1;
}

bool Region::snapSpiral(Common::Point &p, int maxRadius) const {
	if (walkPointInside(p) > 0)
		return true;
	for (int radius = 1; radius <= maxRadius; ++radius) {
		for (int dx = -radius; dx <= radius; ++dx) {
			Common::Point c(p.x + dx, p.y + radius);
			if (walkPointInside(c) > 0) { p = c; return true; }
			c = Common::Point(p.x + dx, p.y - radius);
			if (walkPointInside(c) > 0) { p = c; return true; }
		}
		for (int dy = -radius + 1; dy <= radius - 1; ++dy) {
			Common::Point c(p.x + radius, p.y + dy);
			if (walkPointInside(c) > 0) { p = c; return true; }
			c = Common::Point(p.x - radius, p.y + dy);
			if (walkPointInside(c) > 0) { p = c; return true; }
		}
	}
	return false;
}

bool Region::lineStepUntilInside(Common::Point &p, int dx, int dy) const {
	if (dx == 0 && dy == 0)
		return false;
	int adx = ABS(dx), ady = ABS(dy);
	int sx = dx > 0 ? 1 : -1, sy = dy > 0 ? 1 : -1;
	int x = p.x, y = p.y;
	int err = (adx > ady ? adx : -ady) / 2;
	for (int i = 0; i < 200; ++i) {
		int e = err;
		if (e > -adx) { err -= ady; x += sx; }
		if (e < ady)  { err += adx; y += sy; }
		if (walkPointInside(Common::Point(x, y)) > 0) {
			p = Common::Point(x, y);
			return true;
		}
	}
	return false;
}

bool Region::snapToNearestEdge(Common::Point &p, bool &onVertex) const {
	double best = 1e18;
	Common::Point bestPt;
	bool bestOnVertex = false;
	bool found = false;

	for (uint r = 0; r < regions.size(); ++r) {
		const PointsType &poly = regions[r];
		uint n = poly.size();
		for (uint i = 0; i < n; ++i) {
			Common::Point v0 = poly[i];
			Common::Point v1 = poly[(i + 1) % n];
			double ex = v1.x - v0.x, ey = v1.y - v0.y;
			double len2 = ex * ex + ey * ey;
			double t = len2 > 0 ? ((p.x - v0.x) * ex + (p.y - v0.y) * ey) / len2 : 0.0;
			if (t > 0.95) t = 1.0;
			if (t < 0.05) t = 0.0;
			Common::Point cand((int16)(v0.x + t * ex + 0.5), (int16)(v0.y + t * ey + 0.5));
			double d = distSq(p, cand);
			if (d < best) {
				best = d;
				bestPt = cand;
				bestOnVertex = (t == 0.0 || t == 1.0);
				found = true;
			}
		}
	}
	if (!found)
		return false;
	p = bestPt;
	onVertex = bestOnVertex;
	return true;
}

namespace {

struct WalkCrossing {
	Common::Point pt;
	double dist2;
	int poly;
	int edgeStart; // index of the edge's first vertex
	int hits;
};

// Walk a few pixels from 'hit' toward 'to'; a vertex graze that stays
// walkable is not a real boundary crossing.
static bool grazeIsWalkable(const Region &rgn, Common::Point hit, Common::Point to) {
	int dx = to.x - hit.x, dy = to.y - hit.y;
	if (dx == 0 && dy == 0)
		return true;
	double len = sqrt((double)dx * dx + (double)dy * dy);
	for (int i = 1; i <= 3; ++i) {
		Common::Point c((int16)(hit.x + dx * i / len + 0.5), (int16)(hit.y + dy * i / len + 0.5));
		if (rgn.walkPointInside(c) < 0)
			return false;
	}
	return true;
}

static void collectCrossings(const Region &rgn, Common::Point from, Common::Point to,
                             Common::Array<WalkCrossing> &out) {
	for (uint r = 0; r < rgn.regions.size(); ++r) {
		const Region::PointsType &poly = rgn.regions[r];
		uint n = poly.size();
		if (n < 3)
			continue;
		for (uint i = 0; i < n; ++i) {
			Common::Point v0 = poly[i];
			Common::Point v1 = poly[(i + 1) % n];
			double tSeg, tEdge;
			Common::Point hit;
			if (segIntersect(v0, v1, from, to, tSeg, tEdge, hit) != 1)
				continue;
			// vertex graze filter: hits within 2 px of an edge endpoint that
			// remain walkable are ignored
			if ((distSq(hit, v0) < 4.0 || distSq(hit, v1) < 4.0) && grazeIsWalkable(rgn, hit, to))
				continue;
			// endpoints resting on a boundary edge (frequent after an
			// earlier walk ended near a wall) are not real crossings either
			if ((distSq(hit, from) < 4.0 || distSq(hit, to) < 4.0) && grazeIsWalkable(rgn, hit, to))
				continue;

			bool merged = false;
			for (uint k = 0; k < out.size(); ++k) {
				if (out[k].pt == hit) {
					out[k].hits++;
					merged = true;
					break;
				}
			}
			if (merged)
				continue;

			WalkCrossing c;
			c.pt = hit;
			c.dist2 = distSq(hit, to);
			c.poly = r;
			c.edgeStart = i;
			c.hits = 1;
			// insert sorted ascending by distance to destination
			uint pos = 0;
			while (pos < out.size() && out[pos].dist2 < c.dist2)
				pos++;
			out.insert_at(pos, c);
		}
	}
}

static bool bresenhamInside(const Region &rgn, Common::Point a, Common::Point b) {
	int adx = ABS(b.x - a.x), ady = ABS(b.y - a.y);
	int sx = b.x > a.x ? 1 : -1, sy = b.y > a.y ? 1 : -1;
	int x = a.x, y = a.y;
	int err = (adx > ady ? adx : -ady) / 2;
	while (x != b.x || y != b.y) {
		int e = err;
		if (e > -adx) { err -= ady; x += sx; }
		if (e < ady)  { err += adx; y += sy; }
		if (rgn.walkPointInside(Common::Point(x, y)) < 0)
			return false;
	}
	return true;
}

static bool anyCrossing(const Region &rgn, Common::Point a, Common::Point b) {
	for (uint r = 0; r < rgn.regions.size(); ++r) {
		const Region::PointsType &poly = rgn.regions[r];
		uint n = poly.size();
		if (n < 3)
			continue;
		for (uint i = 0; i < n; ++i) {
			double tSeg, tEdge;
			Common::Point hit;
			if (segIntersect(poly[i], poly[(i + 1) % n], a, b, tSeg, tEdge, hit) != 1)
				continue;
			if ((distSq(hit, poly[i]) < 4.0 || distSq(hit, poly[(i + 1) % n]) < 4.0) &&
			    grazeIsWalkable(rgn, hit, b))
				continue;
			return true;
		}
	}
	return false;
}

static bool collinear(Common::Point a, Common::Point b, Common::Point c) {
	return (double)(b.x - a.x) * (c.y - a.y) == (double)(b.y - a.y) * (c.x - a.x);
}

static void straightenPath(const Region &rgn, Common::Array<Common::Point> &path) {
	bool changed = true;
	while (changed && path.size() > 2) {
		changed = false;
		for (uint i = 1; i + 1 < path.size(); ++i) {
			Common::Point a = path[i - 1], b = path[i], c = path[i + 1];
			bool drop = distSq(a, b) < 4.0 || distSq(b, c) < 4.0 || distSq(a, c) < 4.0 ||
			            collinear(a, b, c) ||
			            (!anyCrossing(rgn, a, c) && bresenhamInside(rgn, a, c));
			if (drop) {
				path.remove_at(i);
				changed = true;
				break;
			}
		}
	}
}

static const int kDirStep[8][2] = {
	{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}
};

static void nudgeCornersInside(const Region &rgn, Common::Array<Common::Point> &path) {
	for (uint i = 0; i < path.size(); ++i) {
		if (rgn.walkPointInside(path[i]) >= 1)
			continue;
		bool inside[8];
		int count = 0;
		for (int k = 0; k < 8; ++k) {
			inside[k] = rgn.walkPointInside(
			                Common::Point(path[i].x + kDirStep[k][0], path[i].y + kDirStep[k][1])) > 0;
			if (inside[k])
				count++;
		}
		if (!count) {
			warning("cannot shift path corner into walk region");
			continue;
		}
		// middle of the contiguous inside arc
		int start = 0;
		while (start < 8 && !(inside[start] && !inside[(start + 7) & 7]))
			start++;
		if (start == 8) { // all inside
			start = 0;
		}
		int len = 0;
		while (len < 8 && inside[(start + len) & 7])
			len++;
		int mid = (start + (len - 1) / 2) & 7;
		path[i].x += kDirStep[mid][0];
		path[i].y += kDirStep[mid][1];
	}
}

static double pathLength(const Common::Array<Common::Point> &path) {
	double len = 0;
	for (uint i = 1; i < path.size(); ++i)
		len += sqrt(distSq(path[i - 1], path[i]));
	return len;
}

static void detourAroundPolygon(const Region &rgn, const WalkCrossing &entry,
                                const WalkCrossing &exit,
                                Common::Array<Common::Point> &path) {
	const Region::PointsType &poly = rgn.regions[entry.poly];
	int n = poly.size();

	// forward (ascending vertex order): advance then push
	Common::Array<Common::Point> fwd;
	fwd.push_back(entry.pt);
	{
		int v = entry.edgeStart, steps = 0;
		do {
			v = (v + 1) % n;
			fwd.push_back(poly[v]);
		} while (v != exit.edgeStart && ++steps <= n);
	}
	fwd.push_back(exit.pt);
	straightenPath(rgn, fwd);
	double lenF = pathLength(fwd);

	// backward (descending vertex order): push then retreat
	Common::Array<Common::Point> bwd;
	bwd.push_back(entry.pt);
	{
		int v = entry.edgeStart, steps = 0;
		while (v != exit.edgeStart && steps <= n) {
			bwd.push_back(poly[v]);
			v = (v + n - 1) % n;
			steps++;
		}
	}
	bwd.push_back(exit.pt);
	straightenPath(rgn, bwd);
	double lenB = pathLength(bwd);

	const Common::Array<Common::Point> &chosen = (lenB <= lenF) ? bwd : fwd;
	for (uint i = 0; i < chosen.size(); ++i)
		path.push_back(chosen[i]);
}

} // anonymous namespace

bool Region::buildWalkPath(Common::Array<Common::Point> &path,
                           Common::Point from, Common::Point to) const {
	path.clear();
	if (walkPointInside(from) < 1 || walkPointInside(to) < 1)
		return false;
	path.push_back(from);

	Common::Array<WalkCrossing> crossings;
	collectCrossings(*this, from, to, crossings);

	if (crossings.size() & 1) {
		// odd count: degenerate touch, try to drop a duplicate hit
		int dup = -1, dupCount = 0;
		for (uint i = 0; i < crossings.size(); ++i) {
			if (crossings[i].hits == 2) {
				dupCount++;
				dup = i;
			}
		}
		if (dupCount == 1) {
			crossings.remove_at(dup);
		} else if (crossings.size() == 1 && (crossings[0].pt == from || crossings[0].pt == to)) {
			crossings.remove_at(0);
		} else {
			debug("buildWalkPath: %u crossings (odd), giving up", crossings.size());
			path.clear();
			return false;
		}
	}

	// crossings are sorted ascending by distance to the destination, so the
	// highest index is the first one hit walking from the start
	for (int i = (int)crossings.size() - 1; i >= 1; i -= 2)
		detourAroundPolygon(*this, crossings[i], crossings[i - 1], path);

	if (path.back() != to)
		path.push_back(to);

	straightenPath(*this, path);
	nudgeCornersInside(*this, path);
	return !path.empty();
}

} // namespace AGDS
