//////////////////////////////////////////////////////////////////////////////
// 2D сетка для оптимизации поиска объектов.
// Доступ 
//////////////////////////////////////////////////////////////////////////////
#ifndef __GRID_2D__
#define __GRID_2D__

#include "Render\inc\RenderMT.h"
#include "DebugUtil.h"

// Прямоугольная область для сканироавния 
// и 2D баундов
struct GridRectangle 
{
	int x0, y0, x1, y1;
	
	GridRectangle(){}
	GridRectangle(int x0_, int y0_, int x1_, int y1_) : x0(x0_), y0(y0_), x1(x1_), y1(y1_) {}
	int operator == (const GridRectangle& r) const { return x0 == r.x0 && y0 == r.y0 && x1 == r.x1 && y1 == r.y1; }
	friend XStream& operator<= (XStream& s,const GridRectangle& r){ s <= r.x0 < " " <= r.y0 < " " <= r.x1 < " " <= r.y1 < " ";  return s; }
	GridRectangle operator + (const GridRectangle& r) const { return GridRectangle(min(x0,r.x0), min(y0,r.y0), max(x1,r.x1), max(y1,r.y1)); }
	bool pointIn(int x, int y) const { return x >= x0 && x <= x1 && y >= y0 && y <= y1; }
};

// Базовый класс для объектов, помещаемых в сетку
class GridElementType
{
	mutable int passCounterLogic_;
	mutable int passCounterGraphics_;
	mutable int insert_counter;
	mutable GridRectangle rectangle;
	
	int& passCounter() const { return MT_IS_LOGIC() ? passCounterLogic_ : passCounterGraphics_; }

	friend class GridPassDispatcher;

public:
	GridElementType() : passCounterLogic_(0), passCounterGraphics_(0), insert_counter(0) {}
	int belongSquare(int x0, int y0, int sx, int sy) const { return 1; } // pseudo-virtual due to template using
	const GridRectangle& getRectangle() const { return rectangle; }
	int inserted() const { return insert_counter; }
	void incrInsertion() const { ++insert_counter; }
	void decrInsertion() const { --insert_counter; }
};

// Вспомогательный класс для учета 
// вызовов при сканировании
class GridPassDispatcher
{
public:
	GridPassDispatcher(){ passCounterLogic_ = passCounterGraphics_ = 0; scanned_ = false; }
	void scanNext() const { passCounter()++; } // Сбрасывает scanAgain
	void scanAgain() const { passCounter()--; } // Делает следующий скан атоммарным с предыдущим, т.е не будут вызываться уже вызванные объекты
	void setAsPassed(const GridElementType& el) const // Отмечает элемент как уже обработанный
	{
		el.passCounter() = passCounter() + 1; 
	}
	bool doPass(const GridElementType& el) const 
	{ 
		if(el.passCounter() != passCounter()){
			el.passCounter() = passCounter(); 
			return true;
		}
		else
			return false; 
	}
	
	bool scanned() const { return scanned_; }
	void setScanned(bool scanned) const { if(MT_IS_LOGIC()) scanned_ = scanned; }

	static void setRectangle(const GridElementType& el, const GridRectangle& rect) { el.rectangle = rect; }

protected:
	class ScanLock {
	public:
		ScanLock(const GridPassDispatcher& grid) : grid_(grid) { grid_.setScanned(true); }
		~ScanLock() { grid_.setScanned(false); }
	private:
		const GridPassDispatcher& grid_;
	};

private:
	mutable int passCounterLogic_;
	mutable int passCounterGraphics_;
	mutable bool scanned_;

	int& passCounter() const { return MT_IS_LOGIC() ? passCounterLogic_ : passCounterGraphics_; }
};

// Шаблон для создания сетки из  векторов
// Быстрее работает, но занимает больше памяти
template<class T, int reserve_size = 0>
class GridVector : public vector<T*>
{
public:
	GridVector() { if(reserve_size) reserve(reserve_size); }
	void insert(T* obj) { push_back(obj); obj->incrInsertion(); }
	void remove(T* obj) 
	{  // Ищем для удаления в обратную сторону,
		// т.к. более подвижные объекты лежат в конце.
		xassert(!empty()); // всегда не пустой
		for(iterator i = end() - 1; i >= begin(); --i)
			if(*i == obj)
			{ 
				erase(i); 
				obj->decrInsertion();
				return;
			} 
	}
};

//	Сетка
template <class T, int cell_size_len, class CellList >	
class Grid2D : public GridPassDispatcher
{
public:	
	Grid2D(int map_sx,int map_sy)
	: cell_table(0)
	{
		Set(map_sx,map_sy);
	}

	~Grid2D()
	{
		reset();
	}

	void Set(int map_sx, int map_sy)
	{
		reset();

		size_x = map_sx / cell_size;
		size_y = map_sy / cell_size;
		//m_mask_x = size_x - 1;
		//m_mask_y = size_y - 1;
	
		cell_table = new CellList*[size_y];
		for(int i = 0;i < size_y;i++)
			cell_table[i] = new CellList[size_x];
	}
	
	void Insert(T& obj, int xc, int yc, int side)
	{
		xassert(!obj.inserted() && "Grid: multiple Insert");
		xassert(!scanned());

		GridRectangle rect(xc - side, yc - side, xc + side, yc + side);
		prepRectangle(rect);
		setRectangle(obj, rect);
		for(int y = rect.y0;y <= rect.y1;y++)
			for(int x = rect.x0;x <= rect.x1;x++)
				if(obj.belongSquare(x*cell_size, y*cell_size, cell_size, cell_size))
					table(x, y).insert(&obj);
	}

	void Move(T& obj, int xc, int yc, int side)
	{
		xassert(obj.inserted() && "Grid: Move without Insert");
		xassert(!scanned());
		
		GridRectangle rect(xc - side, yc - side, xc + side, yc + side);
		prepRectangle(rect);
		const GridRectangle& prev_rect = obj.getRectangle();

		if(prev_rect == rect)
			return;

		GridRectangle rectSum = rect + prev_rect;

		for(int y = rectSum.y0;y <= rectSum.y1;y++)
			for(int x = rectSum.x0;x <= rectSum.x1;x++) {
				bool pointInRect = rect.pointIn(x,y);
				bool pointInPrevRect = prev_rect.pointIn(x,y);
				if(pointInRect && !pointInPrevRect)
					table(x, y).insert(&obj);
				else if(!pointInRect && pointInPrevRect)
					table(x, y).remove(&obj);
			}

		setRectangle(obj, rect);
/*
		for(int y = prev_rect.y0;y <= prev_rect.y1;y++)
			for(int x = prev_rect.x0;x <= prev_rect.x1;x++)
				table(x, y).remove(&obj);

		xassert(!obj.inserted() && "Grid: incomplete remove in Move");

		setRectangle(obj, rect);
		for(y = rect.y0;y <= rect.y1;y++)
			for(int x = rect.x0;x <= rect.x1;x++)
				if(obj.belongSquare(x*cell_size, y*cell_size, cell_size, cell_size))
					table(x, y).insert(&obj);
*/
	}

	void Remove(T& obj)
	{
		xassert(obj.inserted() && "Grid: Remove without Insert");
		xassert(!scanned());
		
		const GridRectangle& rect = obj.getRectangle();
		for(int y = rect.y0;y <= rect.y1;y++)
			for(int x = rect.x0;x <= rect.x1;x++)
				if(obj.belongSquare(x * cell_size, y * cell_size, cell_size, cell_size))
					table(x, y).remove(&obj);

		xassert(!obj.inserted() && "Grid: incomplete remove in Remove");
	}

	void Clear()
	{
		for(int y = 0;y < size_y;y++)
			for(int x = 0;x < size_x;x++){
				cell_table[y][x].clear();
			}
	}

	int size() const // for Debug purpose mostly
	{
		int sz = 0;
		for(int y = 0;y < sy;y++)
			for(int x = 0;x < sx;x++)
				sz += cell_table[y][x].size();
		return sz;
	}

	////////////////////////////////////////////////////////////////////////////////////
	//   Операции доступа и сканирования
	////////////////////////////////////////////////////////////////////////////////////
	template <class Op>
	void Scan(const Vect2i& position, int side, Op& op) const { Scan(position.x - side, position.y - side, position.x + side, position.y + side, op); }

	template <class Op>
	void Scan(int xc, int yc, int side, Op& op) const { Scan(xc - side, yc - side, xc + side, yc + side, op); }

	template <class Op>
	void Scan(int x0, int y0, int x1, int y1, Op& op) const
	{
		scanNext();
		ScanLock scanLock(*this);

		GridRectangle rect(x0, y0, x1, y1);
		prepRectangle(rect);
		for(int y = rect.y0;y <= rect.y1;y++)
			for(int x = rect.x0;x <= rect.x1;x++){
				CellList& root = table(x, y);
				CellList::iterator i;
				FOR_EACH(root, i)
					if(doPass(**i))
						op(*i);
			}
	}

	template <class Op>
	bool ConditionScan(int xc, int yc, int side, Op& op) const { return ConditionScan(xc - side, yc - side, xc + side, yc + side, op); }

	template <class Op>
	bool ConditionScan(int x0, int y0, int x1, int y1, Op& op) const
	{
		scanNext();
		ScanLock scanLock(*this);
		
		GridRectangle rect(x0, y0, x1, y1);
		prepRectangle(rect);
		for(int y = rect.y0;y <= rect.y1;y++)
			for(int x = rect.x0;x <= rect.x1;x++){
				CellList& root = table(x, y);
				CellList::iterator i;
				FOR_EACH(root, i)
					if(doPass(**i))
						if(!(op(*i)))
							return false;
				}
		return true;
	}

	template <class Op>
	void scanPoint(int x0, int y0, Op& op) const
	{
		start_timer_auto();
		scanNext();
		ScanLock scanLock(*this);
		
		GridRectangle rect(x0, y0, x0, y0);
		prepRectangle(rect);
		CellList& root = table(rect.x0, rect.y0);
		CellList::iterator i;
		FOR_EACH(root, i)
			if(doPass(**i))
				op(*i);
	}

	template <class Op>
	bool scanPointConditional(int x0, int y0, Op& op) const
	{
		start_timer_auto();
		scanNext();
		ScanLock scanLock(*this);
		
		GridRectangle rect(x0, y0, x0, y0);
		prepRectangle(rect);
		CellList& root = table(rect.x0, rect.y0);
		CellList::iterator i;
		FOR_EACH(root, i)
			if(doPass(**i))
				if(!(op(*i)))
					return false;
		return true;
	}

	template <class Op>
	void Cell(int x, int y, Op& op) const
	{
		if(insideMap(x, y))
		{
			CellList& root = table(x, y);
			CellList::iterator i;
			FOR_EACH(root, i)
				if(doPass(**i))
					op(*i);
		}
	}

	template <class Op>
	int ConditionCell(int x, int y, Op& op) const
	{
		if(insideMap(x, y))
		{
			CellList& root = table(x, y);
			CellList::iterator i;
			FOR_EACH(root, i)
				if(doPass(**i))
					if(!(op(*i)))
						return 0;
		}
		return 1;
	}

	template <class Op>
	void Line(int x0, int y0, int x1, int y1, Op& op,int width = 1) const
	{
		int i;
		int x,y;
		int dx,dy;
		int tx,ty;
		int step,max_step;
		const int PRECISION = 16;

		scanNext();
		ScanLock scanLock(*this);
		
		GridRectangle rect(x0, y0, x1, y1);
		prepRectangle(rect);
		dx = rect.x1 - rect.x0;
		dy = rect.y1 - rect.y0;

		if(!dx && !dy)
			Cell(rect.x0,rect.y0,op);
		else{
			if(abs(dx) > abs(dy)){
				if(dx > 0){
					max_step = dx;
					dy = (dy << PRECISION) / max_step;
					dx = 1 << PRECISION;
				}else{
					max_step = -dx;
					dy = (dy << PRECISION) / max_step;
					dx = -(1 << PRECISION);
				}

				step = max_step;
				x = rect.x0 << PRECISION;
				y = rect.y0 << PRECISION;

				while(step >= 0){
					tx = x >> PRECISION;
					ty = y >> PRECISION;
					Cell(tx,ty,op);
					for(i = 1;i <= width;i++){
						Cell(tx,ty + i,op);
						Cell(tx,ty - i,op);
					}

					x += dx;
					y += dy;
					step--;
				}
			}else{
				if(dy > 0){
					max_step = dy;
					dx = (dx << PRECISION) / max_step;
					dy = 1 << PRECISION;
				}else{
					max_step = -dy;
					dx = (dx << PRECISION) / max_step;
					dy = -(1 << PRECISION);
				}

				step = max_step;
				x = rect.x0 << PRECISION;
				y = rect.y0 << PRECISION;

				while(step >= 0){
					tx = x >> PRECISION;
					ty = y >> PRECISION;

					Cell(tx,ty,op);
					for(i = 1;i <= width;i++){
						Cell(tx + i,ty,op);
						Cell(tx - i,ty,op);
					}

					x += dx;
					y += dy;
					step--;
				}
			}
		}
	}

	template <class Op>
	int ConditionLine(int x0, int y0, int x1, int y1, Op& op,int width = 1) const
	{
		int i;
		int x,y;
		int dx,dy;
		int tx,ty;
		int step,max_step;
		const int PRECISION = 16;

		scanNext();
		ScanLock scanLock(*this);
		
		GridRectangle rect(x0, y0, x1, y1);
		prepRectangle(rect);
		dx = rect.x1 - rect.x0;
		dy = rect.y1 - rect.y0;

		if(!dx && !dy)
			return ConditionCell(rect.x0,rect.y0,op);
		else{
			if(abs(dx) > abs(dy)){
				if(dx > 0){
					max_step = dx;
					dy = (dy << PRECISION) / max_step;
					dx = 1 << PRECISION;
				}else{
					max_step = -dx;
					dy = (dy << PRECISION) / max_step;
					dx = -(1 << PRECISION);
				}

				step = max_step;
				x = rect.x0 << PRECISION;
				y = rect.y0 << PRECISION;

				while(step > 0){
					tx = x >> PRECISION;
					ty = y >> PRECISION;
					if(!ConditionCell(tx,ty,op))
						return 0;
					for(i = 1;i <= width;i++){
						if(!ConditionCell(tx,ty + i,op))
							return 0;
						if(!ConditionCell(tx,ty - i,op))
							return 0;
					}

					x += dx;
					y += dy;
					step--;
				}
			}else{
				if(dy > 0){
					max_step = dy;
					dx = (dx << PRECISION) / max_step;
					dy = 1 << PRECISION;
				}else{
					max_step = -dy;
					dx = (dx << PRECISION) / max_step;
					dy = -(1 << PRECISION);
				}

				step = max_step;
				x = rect.x0 << PRECISION;
				y = rect.y0 << PRECISION;

				while(step > 0){
					tx = x >> PRECISION;
					ty = y >> PRECISION;

					if(!ConditionCell(tx,ty,op))
						return 0;
					for(i = 1;i <= width;i++){
						if(!ConditionCell(tx + i,ty,op))
							return 0;
						if(!ConditionCell(tx - i,ty,op))
							return 0;
					}

					x += dx;
					y += dy;
					step--;
				}
			}
		}
		return 1;
	}

	template <class Op>
	void ScanPoly(int pX[], int pY[], int n, Op& op)
	{
		#define DIV(a, b)	(((a) << 16)/(b))
		#define CCW(i)	(i == 0 ? n - 1 : i - 1)
		#define CW(i)	(i == n - 1 ? 0 : i + 1)

		scanNext();
		ScanLock scanLock(*this);
		        
		for(int i = 0; i < n; i++){
			pX[i] >>= cell_size_len;
			pY[i] >>= cell_size_len;
		}		

		int vals_up = 0;
		for(int i = 1; i < n; i++)
			if(pY[vals_up] > pY[i])
				vals_up = i;

		int lfv = vals_up;
		int rfv = vals_up;
		int ltv = CCW(lfv);
		int rtv = CW(rfv);

		int Y = pY[lfv]; 
		int xl = pX[lfv];
		int al = pX[ltv] - xl; 
		int bl = pY[ltv] - Y;
		int ar = pX[rtv] - xl; 
		int br = pY[rtv] - Y;
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
				}
			else{
				d = bl;
				where = 1;
				}
			while(d-- > 0){
				int x1 = xl >> 16;
				int x2 = xr >> 16;

				if(x1 > x2)
					swap(x1, x2);

				while(x1 <= x2)
					Cell(x1++, Y, op);

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

					while(x1 <= x2)
						Cell(x1++, Y, op);
					return;
				}
				lfv = ltv;
				ltv = CCW(ltv);

				br -= bl;
				xl = pX[lfv];
				al = pX[ltv] - xl;
				bl = pY[ltv] - Y;
				xl = (xl << 16) + (1 << 15);
				if(bl)
					al = DIV(al, bl);
				}
			else{
				if(rtv == ltv){
					int x1 = xl >> 16;
					int x2 = xr >> 16;

					if(x1 > x2)
						swap(x1, x2);

					while(x1 <= x2)
						Cell(x1++, Y, op);
					return;
				}
				rfv = rtv;
				rtv = CW(rtv);

				bl -= br;
				xr = pX[rfv];
				ar = pX[rtv] - xr;
				br = pY[rtv] - Y;
				xr = (xr << 16) + (1 << 15);
				if(br)
					ar = DIV(ar, br);
				}
			}

		#undef DIV
		#undef CCW
		#undef CW
	}

	template <class Op>
	int ConditionScanPoly(int pX[], int pY[], int n, Op& op)
	{
		#define DIV(a, b)	(((a) << 16)/(b))
		#define CCW(i)	(i == 0 ? n - 1 : i - 1)
		#define CW(i)	(i == n - 1 ? 0 : i + 1)

		scanNext();
		ScanLock scanLock(*this);
		        
		for(int i = 0; i < n; i++){
			pX[i] >>= cell_size_len;
			pY[i] >>= cell_size_len;
		}

		int vals_up = 0;
		for(i = 1; i < n; i++)
			if(pY[vals_up] > pY[i])
				vals_up = i;

		int lfv = vals_up;
		int rfv = vals_up;
		int ltv = CCW(lfv);
		int rtv = CW(rfv);

		int Y = pY[lfv]; 
		int xl = pX[lfv];
		int al = pX[ltv] - xl; 
		int bl = pY[ltv] - Y;
		int ar = pX[rtv] - xl; 
		int br = pY[rtv] - Y;
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
				}
			else{
				d = bl;
				where = 1;
				}
			while(d-- > 0){
				int x1 = xl >> 16;
				int x2 = xr >> 16;

				if(x1 > x2)
					swap(x1, x2);

				while(x1 <= x2)
					if(!ConditionCell(x1++, Y, op))
						return 0;

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

					while(x1 <= x2)
						if(!ConditionCell(x1++, Y, op))
							return 0;
					return 1;
				}
				lfv = ltv;
				ltv = CCW(ltv);

				br -= bl;
				xl = pX[lfv];
				al = pX[ltv] - xl;
				bl = pY[ltv] - Y;
				xl = (xl << 16) + (1 << 15);
				if(bl)
					al = DIV(al, bl);
				}
			else {
				if(rtv == ltv) {
					int x1 = xl >> 16;
					int x2 = xr >> 16;

					if(x1 > x2)
						swap(x1, x2);

					while(x1 <= x2)
						if(!ConditionCell(x1++, Y, op))
							return 0;
					return 1;
				}
				rfv = rtv;
				rtv = CW(rtv);

				bl -= br;
				xr = pX[rfv];
				ar = pX[rtv] - xr;
				br = pY[rtv] - Y;
				xr = (xr << 16) + (1 << 15);
				if(br)
					ar = DIV(ar, br);
				}
			}

		#undef DIV
		#undef CCW
		#undef CW
		return 1;
	}

	template <class Op>
	void ScanHelix(int x,int y,int r, Op& op)
	{
		const int Dx[4] = { 0,1, 0,-1};
		const int Dy[4] = {-1,0, 1, 0};
		int i,n,d,tx,ty;
		
		x >>= cell_size_len;
		y >>= cell_size_len;
		r >>= cell_size_len;

		n = 0;
		d = 1;
		tx = x;
		ty = y;
		while(d <= r){
			for(i = 0;i < d;i++){
				Cell(tx,ty,op);
				tx += Dx[n];
				ty += Dy[n];
			}
			n++;
			n &= 3;

			for(i = 0;i < d;i++){
				Cell(tx,ty,op);
				tx += Dx[n];
				ty += Dy[n];
			}
			n++;

			n &= 3;
			d++;
		}
	}

	template <class Op>
	int ScanConditionHelix(int x,int y,int r, Op& op)
	{
		const int Dx[4] = { 0,1, 0,-1};
		const int Dy[4] = {-1,0, 1, 0};
		int i,n,d,tx,ty;
		
		x >>= cell_size_len;
		y >>= cell_size_len;
		r >>= cell_size_len;

		n = 0;
		d = 1;
		tx = x;
		ty = y;
		while(d <= r){
			for(i = 0;i < d;i++){
				if(!ConditionCell(tx,ty,op))
					return 0;
				tx += Dx[n];
				ty += Dy[n];
			}
			n++;
			n &= 3;

			for(i = 0;i < d;i++){
				if(!ConditionCell(tx,ty,op))
					return 0;
				tx += Dx[n];
				ty += Dy[n];
			}
			n++;
			n &= 3;
			d++;
		}
		return 1;
	}

private:

	enum 
	{
		cell_size = 1 << cell_size_len,
	};

 	CellList** cell_table;

	int size_x, size_y;
//	int m_mask_x, m_mask_y;

	// Реализован Clamped-режим:
	// объекты за пределами границ карты не рассматриваются,
	// сканироание происходит строго в пределах карты.
	int mask_x(int x) const { return x; }
	int mask_y(int y) const { return y; }
	int clamp_x(int x) const { return x > 0 ? (x < size_x ? x : size_x - 1) : 0; }
	int clamp_y(int y) const { return y > 0 ? (y < size_y ? y : size_y - 1) : 0; }
	int insideMap(int x, int y) const {	return x >= 0 && x < size_x && y >= 0 && y < size_y; }

	// Masked-mode
//	int mask_x(int x) const { return x & m_mask_x; }
//	int mask_y(int y) const { return y & m_mask_y; }
//	int clamp_x(int x) const { return x; }
//	int clamp_y(int y) const { return y; }
//	int insideMap(int x, int y) const {	return 1; }

	CellList& table(int x, int y) const { xassert(x >= 0 && x < size_x && y >= 0 && y < size_y); return cell_table[mask_y(y)][mask_x(x)]; }

	// Подготовка области для сканирования
	void prepRectangle(GridRectangle& rectangle)  const
	{
		rectangle.x0 = clamp_x(rectangle.x0 >> cell_size_len);
		rectangle.y0 = clamp_y(rectangle.y0 >> cell_size_len);
		rectangle.x1 = clamp_x(rectangle.x1 >> cell_size_len);
		rectangle.y1 = clamp_y(rectangle.y1 >> cell_size_len);
	}

	void reset()
	{
		if(!cell_table)
			return;
		for(int i = 0;i < size_y;i++)
			delete[] cell_table[i];
		delete[] cell_table;
	}
};

#endif  // __GRID_2D__
