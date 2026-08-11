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

#ifndef ALCACHOFA_SHAPE_H
#define ALCACHOFA_SHAPE_H

#include "common/stream.h"
#include "common/array.h"
#include "common/stack.h"
#include "common/rect.h"
#include "common/span.h"
#include "common/util.h"
#include "math/vector2d.h"

#include "alcachofa/common.h"

namespace Alcachofa {

struct EdgeDistances {
	EdgeDistances(Common::Point edgeA, Common::Point edgeB, Common::Point query);

	float _edgeLength;
	float _onEdge;
	float _toEdge;
};

struct Polygon {
	uint _index;
	Common::Span<const Common::Point> _points;

	bool contains(Common::Point query) const;
	bool intersectsEdge(uint startPointI, Common::Point a, Common::Point b) const;
	EdgeDistances edgeDistances(uint startPointI, Common::Point query) const;
	Common::Point closestPointTo(Common::Point query, float &distanceSqr) const;
	inline Common::Point closestPointTo(Common::Point query) const {
		float dummy;
		return closestPointTo(query, dummy);
	}
	Common::Point midPoint() const;
};

struct PathFindingPolygon : Polygon {
	Common::Span<const uint8> _pointDepths;
	int8 _order;

	using SharedPoint = Common::Pair<uint, uint>;

	float depthAt(Common::Point query) const;
	uint findSharedPoints(const PathFindingPolygon &other, Common::Span<SharedPoint> sharedPoints) const;
};

struct FloorColorPolygon : Polygon {
	Common::Span<const Color> _pointColors;

	Color colorAt(Common::Point query) const;
};

template<class TShape, typename TPolygon>
struct PolygonIterator {
	using difference_type = uint;
	using value_type = TPolygon;
	using my_type = PolygonIterator<TShape, TPolygon>;

	inline value_type operator*() const {
		return _shape.at(_index);
	}

	inline my_type &operator++() {
		assert(_index < _shape.polygonCount());
		_index++;
		return *this;
	}

	inline my_type operator++(int) {
		assert(_index < _shape.polygonCount());
		auto tmp = *this;
		++*this;
		return tmp;
	}

	inline bool operator==(const my_type &it) const {
		return &this->_shape == &it._shape && this->_index == it._index;
	}

	inline bool operator!=(const my_type &it) const {
		return &this->_shape != &it._shape || this->_index != it._index;
	}

private:
	friend typename Common::remove_const_t<TShape>;
	PolygonIterator(const TShape &shape, uint index = 0)
		: _shape(shape)
		, _index(index) {
	}

	const TShape &_shape;
	uint _index;
};

class Shape {
public:
	using iterator = PolygonIterator<Shape, Polygon>;

	Shape();
	Shape(Common::ReadStream &stream);

	inline Common::Point firstPoint() const { return _points.empty() ? Common::Point() : _points[0]; }
	inline uint polygonCount() const { return _polygons.size(); }
	inline bool empty() const { return polygonCount() == 0; }
	inline iterator begin() const { return { *this, 0 }; }
	inline iterator end() const { return { *this, polygonCount() }; }

	Polygon at(uint index) const;
	int32 polygonContaining(Common::Point query) const;
	bool contains(Common::Point query) const;
	Common::Point closestPointTo(Common::Point query, int32 &polygonI) const;
	inline Common::Point closestPointTo(Common::Point query) const {
		int32 dummy;
		return closestPointTo(query, dummy);
	}
	void setAsRectangle(const Common::Rect &rect);

protected:
	uint addPolygon(uint maxCount);

	using PolygonRange = Common::Pair<uint, uint>;
	Common::Array<PolygonRange> _polygons;
	Common::Array<Common::Point> _points;
};

/**
 * @brief Path finding is based on the Shape class with the invariant that
 * every polygon is a convex polygon with at most four points.
 * Equal points of different quads link them together, for shared edges we
 * add an additional link point in the center of the edge.
 *
 * The resulting graph is processed using Floyd-Warshall to precalculate for
 * the actual path finding. Additionally we check whether a character can
 * walk straight through an edge instead of following the link points.
 */
class PathFindingShape final : public Shape {
public:
	using iterator = PolygonIterator<PathFindingShape, PathFindingPolygon>;
	static constexpr const uint kPointsPerPolygon = 4;

	PathFindingShape();
	PathFindingShape(Common::ReadStream &stream);

	inline iterator begin() const { return { *this, 0 }; }
	inline iterator end() const { return { *this, polygonCount() }; }

	PathFindingPolygon at(uint index) const;
	int8 orderAt(Common::Point query) const;
	float depthAt(Common::Point query) const;
	bool findPath(
		Common::Point from,
		Common::Point to,
		Common::Stack<Common::Point> &path) const;
	int32 edgeTarget(uint polygonI, uint pointI) const;
	bool findEvadeTarget(
		Common::Point centerTarget,
		float depthScale,
		float minDistSqr,
		Common::Point &evadeTarget) const;

private:
	using LinkIndex = Common::Pair<int32, int32>;

	void setupLinks();
	void setupLinkPoint(
		const PathFindingPolygon &outer,
		const PathFindingPolygon &inner,
		PathFindingPolygon::SharedPoint pointI);
	void setupLinkEdge(
		const PathFindingPolygon &outer,
		const PathFindingPolygon &inner,
		LinkIndex outerP, LinkIndex innerP);
	void initializeFloydWarshall();
	void calculateFloydWarshall();
	bool canGoStraightThrough(
		Common::Point from,
		Common::Point to,
		int32 fromContaining, int32 toContaining) const;
	void floydWarshallPath(
		Common::Point from,
		Common::Point to,
		int32 fromContaining, int32 toContaining,
		Common::Stack<Common::Point> &path) const;

	Common::Array<uint8> _pointDepths;
	Common::Array<int8> _polygonOrders;

	/**
	 * These are the edges in the graph, they are either points
	 * that are shared by two polygons or artificial points in
	 * the center of a shared edge
	 */
	Common::Array<Common::Point> _linkPoints;
	/**
	 * For each point of each polygon the index (or -1) to
	 * the corresponding link point. The second point is the
	 * index to the artifical center point
	 */
	struct LinkPolygonIndices {
		LinkPolygonIndices();
		LinkIndex _points[kPointsPerPolygon];
	};
	Common::Array<LinkPolygonIndices> _linkIndices;
	/**
	 * For the going-straight-through-edges check we need
	 * to know for each shared edge (defined by the starting point)
	 * into which quad we will walk.
	 */
	Common::Array<int32> _targetQuads;
	Common::Array<uint> _distanceMatrix; ///< for Floyd-Warshall
	Common::Array<int32> _previousTarget; ///< for Floyd-Warshall
};

using OptionalColor = Common::Pair<bool, Color>;
class FloorColorShape final : public Shape {
public:
	using iterator = PolygonIterator<FloorColorShape, FloorColorPolygon>;
	static constexpr const uint kPointsPerPolygon = 4;

	FloorColorShape();
	FloorColorShape(Common::ReadStream &stream);

	inline iterator begin() const { return { *this, 0 }; }
	inline iterator end() const { return { *this, polygonCount() }; }

	FloorColorPolygon at(uint index) const;
	OptionalColor colorAt(Common::Point query) const;

private:
	Common::Array<Color> _pointColors;
};

}

#endif // ALCACHOFA_SHAPE_H
