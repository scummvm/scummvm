#include <cxxtest/TestSuite.h>

#include "math/line2d.h"

class Line2dTestSuite : public CxxTest::TestSuite {
	static void assertVec(const Math::Vector2d &v, float x, float y) {
		TS_ASSERT_DELTA(v.getX(), x, 0.0001);
		TS_ASSERT_DELTA(v.getY(), y, 0.0001);
	}

	static float dot(const Math::Vector2d &a, const Math::Vector2d &b) {
		return a.getX() * b.getX() + a.getY() * b.getY();
	}

public:
	// The line built from a direction and a point must go through that point
	// and run along that direction.
	void test_constructorContainsPoint() {
		Math::Line2d l(Math::Vector2d(1, 2), Math::Vector2d(3, 4));

		TS_ASSERT(l.containsPoint(Math::Vector2d(3, 4)));
		TS_ASSERT(l.containsPoint(Math::Vector2d(4, 6)));
		TS_ASSERT(l.containsPoint(Math::Vector2d(1, 0)));
		TS_ASSERT(l.containsPoint(Math::Vector2d(-1, -4)));

		// Off the line along the normal
		TS_ASSERT(!l.containsPoint(Math::Vector2d(5, 3)));
		TS_ASSERT(!l.containsPoint(Math::Vector2d(0, 0)));
		TS_ASSERT(!l.containsPoint(Math::Vector2d(3, 5)));
	}

	void test_constructorNegativeDirection() {
		Math::Line2d l(Math::Vector2d(3, -5), Math::Vector2d(-2, 7));

		TS_ASSERT(l.containsPoint(Math::Vector2d(-2, 7)));
		TS_ASSERT(l.containsPoint(Math::Vector2d(1, 2)));
		TS_ASSERT(l.containsPoint(Math::Vector2d(-5, 12)));
		TS_ASSERT(!l.containsPoint(Math::Vector2d(1, 7)));
	}

	void test_axisAlignedLines() {
		Math::Line2d horizontal(Math::Vector2d(1, 0), Math::Vector2d(0, 2));
		TS_ASSERT(horizontal.containsPoint(Math::Vector2d(10, 2)));
		TS_ASSERT(horizontal.containsPoint(Math::Vector2d(-10, 2)));
		TS_ASSERT(!horizontal.containsPoint(Math::Vector2d(0, 0)));

		Math::Line2d vertical(Math::Vector2d(0, 1), Math::Vector2d(3, 0));
		TS_ASSERT(vertical.containsPoint(Math::Vector2d(3, 10)));
		TS_ASSERT(vertical.containsPoint(Math::Vector2d(3, -10)));
		TS_ASSERT(!vertical.containsPoint(Math::Vector2d(0, 0)));
	}

	// getDirection() must give back the direction the line was built with.
	void test_getDirection() {
		assertVec(Math::Line2d(Math::Vector2d(1, 2), Math::Vector2d(3, 4)).getDirection(), 1, 2);
		assertVec(Math::Line2d(Math::Vector2d(3, -5), Math::Vector2d(0, 0)).getDirection(), 3, -5);
		assertVec(Math::Line2d(Math::Vector2d(1, 0), Math::Vector2d(0, 2)).getDirection(), 1, 0);
		assertVec(Math::Line2d(Math::Vector2d(0, 1), Math::Vector2d(3, 0)).getDirection(), 0, 1);
		assertVec(Math::Line2d(Math::Vector2d(-4, -1), Math::Vector2d(9, 9)).getDirection(), -4, -1);
	}

	void test_getDistanceTo() {
		Math::Vector2d inter;

		Math::Line2d horizontal(Math::Vector2d(1, 0), Math::Vector2d(0, 2));
		TS_ASSERT_DELTA(horizontal.getDistanceTo(Math::Vector2d(5, 7), &inter), 5.0f, 0.0001);
		assertVec(inter, 5, 2);
		TS_ASSERT_DELTA(horizontal.getDistanceTo(Math::Vector2d(5, -1), &inter), 3.0f, 0.0001);
		assertVec(inter, 5, 2);

		Math::Line2d vertical(Math::Vector2d(0, 1), Math::Vector2d(3, 0));
		TS_ASSERT_DELTA(vertical.getDistanceTo(Math::Vector2d(-1, 4), &inter), 4.0f, 0.0001);
		assertVec(inter, 3, 4);

		Math::Line2d diagonal(Math::Vector2d(1, 1), Math::Vector2d(0, 0));
		TS_ASSERT_DELTA(diagonal.getDistanceTo(Math::Vector2d(0, 2), &inter), sqrtf(2.0f), 0.0001);
		assertVec(inter, 1, 1);

		// A point on the line is at distance 0 and is its own projection
		TS_ASSERT_DELTA(diagonal.getDistanceTo(Math::Vector2d(3, 3), &inter), 0.0f, 0.0001);
		assertVec(inter, 3, 3);

		// Without an intersection output
		TS_ASSERT_DELTA(diagonal.getDistanceTo(Math::Vector2d(2, 0), nullptr), sqrtf(2.0f), 0.0001);

		// Line not through the origin, with a non-unit direction
		Math::Line2d l(Math::Vector2d(1, 2), Math::Vector2d(3, 4));
		// (5, 3) is (3, 4) + (2, -1), and (2, -1) is perpendicular to (1, 2)
		TS_ASSERT_DELTA(l.getDistanceTo(Math::Vector2d(5, 3), &inter), sqrtf(5.0f), 0.0001);
		assertVec(inter, 3, 4);
	}

	void test_getPerpendicular() {
		Math::Line2d l(Math::Vector2d(1, 2), Math::Vector2d(0, 0));
		Math::Line2d perp = l.getPerpendicular(Math::Vector2d(5, 0));

		TS_ASSERT(perp.containsPoint(Math::Vector2d(5, 0)));
		TS_ASSERT_DELTA(dot(perp.getDirection(), l.getDirection()), 0.0f, 0.0001);

		// The perpendicular meets the line at the projection of (5, 0)
		Math::Vector2d inter;
		TS_ASSERT(l.intersectsLine(perp, &inter));
		assertVec(inter, 1, 2);
		TS_ASSERT(perp.intersectsLine(l, &inter));
		assertVec(inter, 1, 2);
	}

	void test_intersectsLine() {
		Math::Line2d a(Math::Vector2d(1, 1), Math::Vector2d(0, 0));
		Math::Line2d b(Math::Vector2d(1, -1), Math::Vector2d(0, 2));

		Math::Vector2d inter;
		TS_ASSERT(a.intersectsLine(b, &inter));
		assertVec(inter, 1, 1);
		TS_ASSERT(b.intersectsLine(a, &inter));
		assertVec(inter, 1, 1);
		TS_ASSERT(a.intersectsLine(b, nullptr));

		Math::Line2d h(Math::Vector2d(1, 0), Math::Vector2d(0, 2));
		Math::Line2d v(Math::Vector2d(0, 1), Math::Vector2d(3, 0));
		TS_ASSERT(h.intersectsLine(v, &inter));
		assertVec(inter, 3, 2);

		Math::Line2d c(Math::Vector2d(1, 2), Math::Vector2d(3, 4));
		Math::Line2d d(Math::Vector2d(3, -1), Math::Vector2d(-1, 3));
		TS_ASSERT(c.intersectsLine(d, &inter));
		assertVec(inter, 2, 2);
	}

	void test_intersectsLineParallel() {
		Math::Line2d a(Math::Vector2d(1, 1), Math::Vector2d(0, 0));
		Math::Line2d parallel(Math::Vector2d(2, 2), Math::Vector2d(0, 1));
		Math::Line2d antiParallel(Math::Vector2d(-1, -1), Math::Vector2d(5, 0));

		Math::Vector2d inter(7, 8);
		TS_ASSERT(!a.intersectsLine(parallel, &inter));
		TS_ASSERT(!a.intersectsLine(antiParallel, &inter));
		// The output is left alone when there is no intersection
		assertVec(inter, 7, 8);
	}

	void test_segmentGetLine() {
		Math::Segment2d s(Math::Vector2d(1, 1), Math::Vector2d(4, 5));
		Math::Line2d l = s.getLine();

		TS_ASSERT(l.containsPoint(s.begin()));
		TS_ASSERT(l.containsPoint(s.end()));
		TS_ASSERT(l.containsPoint(s.middle()));
		TS_ASSERT(l.containsPoint(Math::Vector2d(7, 9)));
		TS_ASSERT(!l.containsPoint(Math::Vector2d(4, 1)));
		assertVec(l.getDirection(), 3, 4);

		// Reversed segment: same line, opposite direction
		Math::Line2d r = Math::Segment2d(Math::Vector2d(4, 5), Math::Vector2d(1, 1)).getLine();
		TS_ASSERT(r.containsPoint(Math::Vector2d(1, 1)));
		TS_ASSERT(r.containsPoint(Math::Vector2d(4, 5)));
		assertVec(r.getDirection(), -3, -4);
	}

	void test_segmentContainsPoint() {
		Math::Segment2d s(Math::Vector2d(1, 1), Math::Vector2d(4, 5));

		TS_ASSERT(s.containsPoint(Math::Vector2d(1, 1)));
		TS_ASSERT(s.containsPoint(Math::Vector2d(4, 5)));
		TS_ASSERT(s.containsPoint(Math::Vector2d(2.5f, 3)));
		// On the line, but past the end
		TS_ASSERT(!s.containsPoint(Math::Vector2d(7, 9)));
		TS_ASSERT(!s.containsPoint(Math::Vector2d(-2, -3)));
		// Inside the bounding box, but off the line
		TS_ASSERT(!s.containsPoint(Math::Vector2d(2, 4)));
		TS_ASSERT(!s.containsPoint(Math::Vector2d(3, 1)));
	}

	void test_segmentIntersectsLine() {
		Math::Segment2d s(Math::Vector2d(0, 0), Math::Vector2d(4, 4));
		Math::Vector2d inter;

		Math::Line2d crossing(Math::Vector2d(1, -1), Math::Vector2d(0, 2));
		TS_ASSERT(s.intersectsLine(crossing, &inter));
		assertVec(inter, 1, 1);

		// The lines meet at (10, 10), outside the segment
		Math::Line2d missing(Math::Vector2d(1, -1), Math::Vector2d(0, 20));
		TS_ASSERT(!s.intersectsLine(missing, &inter));

		Math::Line2d parallel(Math::Vector2d(1, 1), Math::Vector2d(0, 1));
		TS_ASSERT(!s.intersectsLine(parallel, &inter));
	}

	void test_segmentGetPerpendicular() {
		Math::Segment2d s(Math::Vector2d(0, 0), Math::Vector2d(4, 0));
		Math::Line2d perp = s.getPerpendicular(Math::Vector2d(1, 3));

		TS_ASSERT(perp.containsPoint(Math::Vector2d(1, 3)));
		TS_ASSERT(perp.containsPoint(Math::Vector2d(1, -5)));
		TS_ASSERT(!perp.containsPoint(Math::Vector2d(2, 3)));
		TS_ASSERT_DELTA(perp.getDirection().getX(), 0.0f, 0.0001);

		Math::Vector2d inter;
		TS_ASSERT(s.intersectsLine(perp, &inter));
		assertVec(inter, 1, 0);
	}

	// The pattern Grim's Actor::handleCollisionWith uses to push an actor out
	// of a rotated box (the forklift): take the perpendicular of an edge
	// through the actor, find where it meets the edge, and move along it.
	void test_collisionPushOut() {
		const Math::Vector2d center(0, 0);
		const Math::Vector2d circle(1.5f, 1.5f);
		const float radius = 1.0f;

		Math::Segment2d edge(Math::Vector2d(2, 0), Math::Vector2d(0, 2));
		Math::Line2d perpendicular = edge.getPerpendicular(circle);
		TS_ASSERT(perpendicular.containsPoint(circle));

		Math::Vector2d p;
		TS_ASSERT(edge.intersectsLine(perpendicular, &p));
		assertVec(p, 1, 1);

		Math::Vector2d direction = perpendicular.getDirection();
		direction.normalize();
		// The push direction is the edge normal
		TS_ASSERT_DELTA(dot(direction, edge.getLine().getDirection()), 0.0f, 0.0001);

		Math::Vector2d point1(p - direction * radius);
		Math::Vector2d point2(p + direction * radius);
		Math::Vector2d point = center.getDistanceTo(point1) < center.getDistanceTo(point2) ? point2 : point1;

		const float h = 1.0f + sqrtf(0.5f);
		assertVec(point, h, h);
		TS_ASSERT_DELTA(edge.getLine().getDistanceTo(point, nullptr), radius, 0.0001);
	}
};
