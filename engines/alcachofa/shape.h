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

#ifndef SHAPE_H
#define SHAPE_H

#include "common/stream.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/span.h"
#include "common/util.h"
#include "math/vector2d.h"

namespace Alcachofa {

struct Polygon {
	Common::Span<const Common::Point> _points;
};

struct PathFindingPolygon : Polygon {
	Common::Span<const int8> _pointValues;
	int8 _polygonValue;
};

struct FloorColorPolygon : Polygon {
	Common::Span<const uint32> _pointColors;
	Common::Span<const uint8> _pointWeights;
	int8 _polygonValue;
};

template<class TShape, typename TPolygon>
struct PolygonIterator {
	using difference_type = uint;
	using value_type = TPolygon;
	using my_type = PolygonIterator<TShape, TPolygon>;

	inline value_type operator*() const {
		return _shape.at(_index);
	}

	inline PolygonIterator<TShape, TPolygon> &operator++() {
		assert(_index < _shape.polygonCount());
		_index++;
		return *this;
	}

	inline PolygonIterator<TShape, TPolygon> &operator++(int) {
		assert(_index < _shape.polygonCount());
		auto tmp = *this;
		++*this;
		return tmp;
	}

	inline bool operator==(const my_type& it) const {
		return &this->_shape == &it._shape && this->_index == it._index;
	}

	inline bool operator!=(const my_type& it) const {
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

protected:
	uint addPolygon(uint maxCount);

	using PolygonRange = Common::Pair<uint, uint>;
	Common::Array<PolygonRange> _polygons;
	Common::Array<Common::Point> _points;
};

/**
 * @brief Path finding is based on the Shape class with the invariant that
 * every polygon is a convex quad.
 * Equal points of different quads link them together, for edges we add an
 * additional link point in the center of the edge.
 *
 * The resulting graph is processed using Floyd-Warshall to precalculate for
 * the actual path finding. Additionally we check whether a character can
 * walk straight through an edge instead of following the link points.
 *
 * None of this is implemented yet by the way ;)
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

private:
	Common::Array<int8> _pointValues;
	Common::Array<int8> _polygonValues;
};

class FloorColorShape final : public Shape {
public:
	using iterator = PolygonIterator<FloorColorShape, FloorColorPolygon>;
	static constexpr const uint kPointsPerPolygon = 4;

	FloorColorShape();
	FloorColorShape(Common::ReadStream &stream);

	inline iterator begin() const { return { *this, 0 }; }
	inline iterator end() const { return { *this, polygonCount() }; }

	FloorColorPolygon at(uint index) const;

private:
	Common::Array<uint32> _pointColors;
	Common::Array<uint8> _pointWeights;
	Common::Array<int8> _polygonValues;
};

}

#endif // SHAPE_H
