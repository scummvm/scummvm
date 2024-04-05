#ifndef __SCANPOLY_H__
#define __SCANPOLY_H__

template<class LineOp>
void scanPolyByLineOp(const Vect2i points[], int n, LineOp& line_op)
{
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

			line_op(x1,x2,Y);

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

				line_op(x1,x2,Y);
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

				line_op(x1,x2,Y);
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
}

//---------------------------------
// Гуро
template<class LineOp, class ColorType>
void scanPolyByLineOp(const Vect2i points[], const ColorType colors[], int n, LineOp& line_op)
{
	// ColorType должен иметь -, /=(int) и быть сдинутым заранее, если используется фп

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

	ColorType cl = colors[lfv];
	ColorType cr = cl;
	ColorType dcl = colors[ltv] - cl;
	ColorType dcr = colors[rtv] - cr;

	if(bl){
		al = DIV(al, bl);
		dcl /= bl;
	}
	if(br){
		ar = DIV(ar, br);
		dcr /= br;
	}

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
			if(xl <= xr)
				line_op(xl >> 16, xr >> 16, Y, cl, cr);
			else
				line_op(xr >> 16, xl >> 16, Y, cr, cl);

			Y++;
			xl += al;
			xr += ar;
			cl += dcl;
			cr += dcr;
		}

		if(where){
			if(ltv == rtv){
				if(xl <= xr)
					line_op(xl >> 16, xr >> 16, Y, cl, cr);
				else
					line_op(xr >> 16, xl >> 16, Y, cr, cl);
				return;
			}

			lfv = ltv;
			ltv = CCW(ltv);

			br -= bl;
			xl = points[lfv].x;
			al = points[ltv].x - xl;
			bl = points[ltv].y - Y;
			xl = (xl << 16) + (1 << 15);
			cl = colors[lfv];
			dcl = colors[ltv] - cl;

			if(bl){
				al = DIV(al, bl);
				dcl /= bl;
			}
		}else{
			if(rtv == ltv){
				if(xl <= xr)
					line_op(xl >> 16, xr >> 16, Y, cl, cr);
				else
					line_op(xr >> 16, xl >> 16, Y, cr, cl);
				return;
			}
			rfv = rtv;
			rtv = CW(rtv);

			bl -= br;
			xr = points[rfv].x;
			ar = points[rtv].x - xr;
			br = points[rtv].y - Y;
			xr = (xr << 16) + (1 << 15);
			cr = colors[rfv];
			dcr = colors[rtv] - cr;
			if(br){
				ar = DIV(ar, br);
				dcr /= br;
			}
		}
	}

	#undef DIV
	#undef CCW
	#undef CW
}

//---------------------------------
template<class PointOp>
class GenericLineOp
{
	PointOp& point_op_;
public:
	GenericLineOp(PointOp& point_op) : point_op_(point_op) {}
	
	void operator()(int x1,int x2,int y)
	{
		while(x1 <= x2) 
			point_op_(x1++,y);
	}

	template<class ColorType>
	void operator()(int x1, int x2, int y, ColorType c1, ColorType c2)
	{
		int len = x2 - x1;
		if(!len){
			point_op_(x1, y, c1);
			return;
		}
		ColorType dc = c2 - c1;
		dc /= len;
		while(len--){
			point_op_(x1++, y, c1);
			c1 += dc;
		}
	}
};

template<class PointOp>
void scanPolyByPointOp(const Vect2i points[], int n, PointOp& point_op)
{
	scanPolyByLineOp(points, n, GenericLineOp<PointOp>(point_op));
}

template<class PointOp, class ColorType>
void scanPolyByPointOp(const Vect2i points[], const ColorType colors[], int n, PointOp& point_op)
{
	scanPolyByLineOp(points, colors, n, GenericLineOp<PointOp>(point_op));
}

#endif //__SCANPOLY_H__
