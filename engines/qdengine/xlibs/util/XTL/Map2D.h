#ifndef __MAP2D_H__
#define __MAP2D_H__

#include "XMath\xmath.h"

template <class T, int tileSizeShl_>
class Map2D
{
public:
	enum
	{
		tileSizeShl = tileSizeShl_,
		tileSize = 1 << tileSizeShl
	};

	Map2D(int sizeX, int sizeY)
	: map_(0)
	{
		resize(sizeX, sizeY);
	}

	Map2D(int sizeX, int sizeY, const T& value)
	: map_(0)
	{
		resize(sizeX, sizeY);
		set(value);
	}

	~Map2D() { delete[] map_; }

	void resize(int sizeX, int sizeY)
	{
		if(map_)
			delete[] map_;
		sizeX_ = sizeX/tileSize;
		sizeY_ = sizeY/tileSize;
		map_ = new T[sizeX_*sizeY_];
	}

	void set(const T& value)
	{
		for(int i = 0; i < sizeX_*sizeY_; i++)
			map_[i] = value;
	}

	int sizeX() const {return sizeX_;};
	int sizeY() const {return sizeY_;};
	const T* map() const { return map_; }
	T* map() { return map_; }

	// Map coords
	bool inside(const Vect2i& v) const { return v.x >= 0 && v.x < sizeX() && v.y >= 0 && v.y < sizeY(); }
	T& operator()(int x, int y) { xassert(x >= 0 && x < sizeX() && y >= 0 && y < sizeY()); return map_[y*sizeX_+x]; }
	const T& operator()(int x, int y) const { xassert(x >= 0 && x < sizeX() && y >= 0 && y < sizeY()); return map_[y*sizeX_+x]; }
	T& operator()(const Vect2i& v) { return (*this)(v.x, v.y); }
	const T& operator()(const Vect2i& v) const { return (*this)(v.x, v.y); }

	// World coords
	bool lookable(int x, int y) const { return x >= 0 && y >= 0 && w2m(x) < sizeX() && w2m(y) < sizeY(); }
	T& look(int x, int y) { return (*this)(w2m(x), w2m(y)); }
	const T& look(int x, int y) const { return (*this)(w2m(x), w2m(y)); }
	T& look(const Vect2i& v) { return look(v.x, v.y); }
	const T& look(const Vect2i& v) const { return look(v.x, v.y); }

	// Map coords
	template <class Op>
	void scanMap(const Vect2i& leftTop, const Vect2i& size, Op& op)
	{
		int x2 = leftTop.x + size.x;
		int y2 = leftTop.y + size.y;
		
		for(int y = leftTop.y; y < y2; y++)
			for(int x = leftTop.x; x < x2; x++)
				op((*this)(x,y));
	}

	// World coords
	template <class Op>
	void scanWorld(const Vect2i& leftTop, const Vect2i& size, Op& op)
	{
		int x1 = w2mFloor(leftTop.x);
		int y1 = w2mFloor(leftTop.y);

		int x2 = w2mFloor(leftTop.x + size.x);
		int y2 = w2mFloor(leftTop.y + size.y);

		for(int y = y1; y <= y2; y++)
			for(int x = x1; x <= x2; x++)
				op((*this)(x,y));
	}

	template <class Stream>
	void save(Stream& out) const 
	{
		out < sizeX_ < sizeY_;
		out.write(map_, sizeX_*sizeY_*sizeof(T));
	}

	template <class Stream>
	void load(Stream& in)
	{
		int sizeX, sizeY;
		in > sizeX > sizeY;
		xassert(sizeX == sizeX_ && sizeY == sizeY_);
		sizeX_ = sizeX; sizeY_ = sizeY;
		in.read(map_, sizeX_*sizeY_*sizeof(T));
	}

	// World to map conversion
	static int w2m(int x) { return (x + tileSize/2) >> tileSizeShl; }
	static float w2m(float x) { return x*(1.f/float(tileSize)); }
	static int w2mFloor(int x) { return x >> tileSizeShl; }
	static int w2mCeil(int x) { return (x + tileSize - 1) >> tileSizeShl; }

	static Vect2i w2m(const Vect2i& v) { return Vect2i(w2m(v.x),w2m(v.y)); }
	static Vect2f w2m(const Vect2f& v) { return Vect2i(w2m(v.x),w2m(v.y)); }
	static Vect2i w2mFloor(const Vect2i& v) { return Vect2i(w2mFloor(v.x),w2mFloor(v.y)); }
	static Vect2i w2mCeil(const Vect2i& v) { return Vect2i(w2mCeil(v.x),w2mCeil(v.y)); }

	// Map to world conversion
	static int m2w(int x) { return x << tileSizeShl; }
	static float m2w(float x) { return x*float(tileSize); }
	static Vect2i m2w(const Vect2i& v) { return Vect2i(m2w(v.x),m2w(v.y)); }
	static Vect2f m2w(const Vect2f& v) { return Vect2i(m2w(v.x),m2w(v.y)); }

private:
	int sizeX_, sizeY_;
	T* map_;
};



#endif //__MAP2D_H__
