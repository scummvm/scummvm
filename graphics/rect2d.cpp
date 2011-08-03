
#include "common/scummsys.h"

#include "graphics/rect2d.h"

namespace Graphics {

Vector2d::Vector2d() :
	_x(0), _y(0) {

}

Vector2d::Vector2d(float x, float y) :
	_x(x), _y(y) {

}

Vector2d::Vector2d(const Vector2d &vec) :
	_x(vec._x), _y(vec._y) {

}

Vector2d &Vector2d::operator=(const Vector2d &vec) {
	_x = vec._x;
	_y = vec._y;
	return *this;
}

Vector2d &Vector2d::operator+(const Vector2d &vec) {
	_x += vec._x;
	_y += vec._y;
	return *this;
}

Vector2d &Vector2d::operator-(const Vector2d &vec) {
	_x -= vec._x;
	_y -= vec._y;
	return *this;
}

void Vector2d::rotateAround(const Vector2d &point, float angle) {
	_x -= point._x;
	_y -= point._y;
	float a = angle * LOCAL_PI / 180.0;

	float x = _x * cos(a) - _y * sin(a);
	_y = _x * sin(a) + _y * cos(a);
	_x = x;

	_x += point._x;
	_y += point._y;
}

float Vector2d::getAngle() const {
	const float mag = sqrt(_x * _x + _y * _y);
	float a = _x / mag;
	float b = _y / mag;
	float yaw;

	// find the angle on the upper half of the unit circle
	yaw = acos(a) * (180.0f / LOCAL_PI);
	if (b < 0.0f)
		// adjust for the lower half of the unit circle
		return 360.0f - yaw;
	else
		// no adjustment, angle is on the upper half
		return yaw;
}


Rect2d::Rect2d() {

}

Rect2d::Rect2d(const Vector2d &topLeft, const Vector2d &topRight,
               const Vector2d &bottomLeft, const Vector2d &bottomRight) :
	_topLeft(topLeft), _topRight(topRight),
	_bottomLeft(bottomLeft), _bottomRight(bottomRight) {

}

void Rect2d::rotateAround(const Vector2d &point, float angle) {
	_topLeft.rotateAround(point, angle);
	_topRight.rotateAround(point, angle);
	_bottomLeft.rotateAround(point, angle);
	_bottomRight.rotateAround(point, angle);
}

void Rect2d::rotateAroundCenter(float angle) {
	Vector2d center = getCenter();
	rotateAround(center, angle);
}

bool Rect2d::intersectsRect(const Rect2d &rect) const {

}

bool Rect2d::intersectsCircle(const Vector2d &center, float radius) const {
	Vector2d c = getCenter();
	float w = getWidth();
	float h = getHeight();

	float angle = (_topRight - _topLeft).getAngle();

	if (angle < 0.1 && angle > -0.1) {
		Vector2d circleDistance(fabs(center._x - c._x), fabs(center._y - c._y));

		if (circleDistance._x > (w / 2.f + radius)) {
			return false;
		}
		if (circleDistance._y > (h / 2.f + radius)) {
			return false;
		}

		if (circleDistance._x <= (w / 2.f)) {
			return true;
		}
		if (circleDistance._y <= (h / 2.f)) {
			return true;
		}

		float cornerDistance_sq = pow(circleDistance._x - w / 2.f, 2.f) +
								pow(circleDistance._y - h / 2.f, 2.f);

		return (cornerDistance_sq <= radius * radius);
	} else { //The rectangle was rotated
		Rect2d r(_topLeft, _topRight, _bottomLeft, _bottomRight);
		r.rotateAroundCenter(-angle);
		Vector2d circle(center);
		circle.rotateAround(r.getCenter(), -angle);
		return r.intersectsCircle(circle, radius);
	}
}

Vector2d Rect2d::getCenter() const {
	Vector2d sum;
	sum._x = _topLeft._x + _topRight._x + _bottomLeft._x + _bottomRight._x;
	sum._x /= 4;
	sum._y = _topLeft._y + _topRight._y + _bottomLeft._y + _bottomRight._y;
	sum._y /= 4;

	return sum;
}

float Rect2d::getWidth() const {
	float x = _topRight._x - _topLeft._x;
	float y = _topRight._y - _topLeft._y;

	return sqrt(x * x + y * y);
}

float Rect2d::getHeight() const {
	float x = _bottomLeft._x - _topLeft._x;
	float y = _bottomLeft._y - _topLeft._y;

	return sqrt(x * x + y * y);
}

}
