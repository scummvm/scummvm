
#ifndef GRAPHICS_RECT2D_H
#define GRAPHICS_RECT2D_H

namespace Graphics {

class Vector2d {
public:
	Vector2d();
	Vector2d(float x, float y);
	Vector2d(const Vector2d &vec);

	Vector2d &operator=(const Vector2d &vec);
	Vector2d &operator-(const Vector2d &vec);
	Vector2d &operator+(const Vector2d &vec);

	void rotateAround(const Vector2d &point, float angle);
	float getAngle() const;

// private:
	float _x;
	float _y;
};

inline Vector2d operator-(const Vector2d& v1, const Vector2d& v2) {
	Vector2d result(v1._x - v2._x, v1._y - v2._y);;
	return result;
}

class Rect2d {
public:
	Rect2d();
	Rect2d(const Vector2d &topLeft, const Vector2d &topRight,
		   const Vector2d &bottomLeft, const Vector2d &bottomRight);

	void rotateAroundCenter(float angle);
	bool intersectsRect(const Rect2d &rect) const;
	bool intersectsCircle(const Vector2d &center, float radius) const;

	Vector2d getCenter() const;
	float getWidth() const;
	float getHeight() const;

// private:
	Vector2d _topLeft;
	Vector2d _topRight;
	Vector2d _bottomLeft;
	Vector2d _bottomRight;
};

}

#endif
