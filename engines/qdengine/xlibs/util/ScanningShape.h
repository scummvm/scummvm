#pragma once

#include "Terra\terra.h"

class ScanningShape
{
	struct Interval {
		short xl, xr;
		Interval(short _xl, short _xr) : xl(_xl), xr(_xr) {}
	};
	

public:
	typedef vector<Vect2i> Vect2iVect;
	void setCircle(int radius); // Симметрично относительно начала координат
	void setRectangle(int xSize, int ySize); // Симметрично или +1,+1
	void setPolygon(const Vect2i* points, int n);

	typedef vector<Interval> Intervals;
	typedef Intervals::const_iterator const_iterator;
	const Intervals& intervals() const { return intervals_; }

	const sRect& rect() const { return rect_; } // Описанный прямоугольник
	int area() const { return area_; } 

private:
	Intervals intervals_;
	sRect rect_;
	int area_;
};
