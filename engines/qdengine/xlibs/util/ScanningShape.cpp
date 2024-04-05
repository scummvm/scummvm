#include "StdAfx.h"
#include "ScanningShape.h"

void ScanningShape::setCircle(int radius)
{
	intervals_.clear();
	intervals_.reserve(radius << 1 + 1);
	area_ = 0;
	for(int y = -radius; y <= radius; y++){
		int x = round(sqrtf(sqr(radius) - sqr(y)));
		intervals_.push_back(Interval(-x, x));
		area_ += x << 1 + 1;
	}
	rect_ = sRect(-radius, -radius, radius << 1, radius << 1);
}

void ScanningShape::setRectangle(int xSize, int ySize)
{
	int y0 = -(ySize >> 1);
	int x0 = -(xSize >> 1);
	int x1 = x0 + xSize;
	int y1 = y0 + ySize;
	intervals_.clear();
	intervals_.reserve(ySize + 1);
	area_ = 0;
	for(int y = y0; y <= y1; y++){
		intervals_.push_back(Interval(x0, x1));
		area_ += xSize + 1;
	}
	rect_ = sRect(x0, y0, xSize, ySize);
}

void ScanningShape::setPolygon(const Vect2i* points, int n)
{
	intervals_.clear();
	area_ = 0;
	int xlo = INT_INF, ylo = INT_INF; 
	int xhi = -INT_INF, yhi = -INT_INF;

#define DIV(a,b)	(((a) << 16)/(b))
#define CCW(i)		(i == 0 ? n - 1 : i - 1)
#define CW(i)		(i == n - 1 ? 0 : i + 1)

	int vals_up = 0;
	for(int i = 1; i < n; i++)
		if(points[vals_up].y > points[i].y)
			vals_up = i;

	int lfv = vals_up;
	int rfv = vals_up;
	int ltv = CCW(lfv);
	int rtv = CW(rfv);

	int Y = points[lfv].y; 
	int xl = points[lfv].x;
	int al = points[ltv].x - xl; 
	int bl = points[ltv].y - Y;
	int ar = points[rtv].x - xl; 
	int br = points[rtv].y - Y;
	int xr = xl = (xl << 16) + (1 << 15);

	if(bl)
		al = DIV(al, bl);
	if(br)
		ar = DIV(ar, br);

	int d, where;
	while(1){
		if(bl > br){
			d = br;
			where = 0;
		}else{
			d = bl;
			where = 1;
		}

		while(d-- > 0){
			int x1 = xl >> 16;
			int x2 = xr >> 16;

			if(x1 > x2)
				swap(x1, x2);

			if(xlo > x1) 
				xlo = x1;
			if(xhi < x2) 
				xhi = x2;
			if(ylo > Y) 
				ylo = Y;
			if(yhi < Y) 
				yhi = Y;
			intervals_.push_back(Interval(x1, x2));
			area_ += x2 - x1 + 1;

			Y++;
			xl += al;
			xr += ar;
		}

		if(where){
			if(ltv == rtv){
				int x1 = xl >> 16;
				int x2 = xr >> 16;

				if(x1 > x2)
					swap(x1, x2);

				if(xlo > x1) 
					xlo = x1;
				if(xhi < x2) 
					xhi = x2;
				if(ylo > Y) 
					ylo = Y;
				if(yhi < Y) 
					yhi = Y;
				intervals_.push_back(Interval(x1, x2));
				area_ += x2 - x1 + 1;
				rect_ = sRect(xlo, ylo, xhi - xlo + 1, yhi - ylo + 1);
				return;
			}

			lfv = ltv;
			ltv = CCW(ltv);

			br -= bl;
			xl = points[lfv].x;
			al = points[ltv].x - xl;
			bl = points[ltv].y - Y;
			xl = (xl << 16) + (1 << 15);
			if(bl)
				al = DIV(al, bl);
		}else{
			if(rtv == ltv){
				int x1 = xl >> 16;
				int x2 = xr >> 16;

				if(x1 > x2)
					swap(x1, x2);

				if(xlo > x1) 
					xlo = x1;
				if(xhi < x2) 
					xhi = x2;
				if(ylo > Y) 
					ylo = Y;
				if(yhi < Y) 
					yhi = Y;
				intervals_.push_back(Interval(x1, x2));
				area_ += x2 - x1 + 1;
				rect_ = sRect(xlo, ylo, xhi - xlo + 1, yhi - ylo + 1);
				return;
			}
			rfv = rtv;
			rtv = CW(rtv);

			bl -= br;
			xr = points[rfv].x;
			ar = points[rtv].x - xr;
			br = points[rtv].y - Y;
			xr = (xr << 16) + (1 << 15);
			if(br)
				ar = DIV(ar, br);
		}
	}

#undef DIV
#undef CCW
#undef CW

rect_ = sRect(xlo, ylo, xhi - xlo + 1, yhi - ylo + 1);

}