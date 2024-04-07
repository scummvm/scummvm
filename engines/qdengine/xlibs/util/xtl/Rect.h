#ifndef __PERIMETER_RECT_H_INCLUDED__
#define __PERIMETER_RECT_H_INCLUDED__

#include <vector>
#include "XMath\xmath.h"
#include "Serialization\Range.h"

/*
 * FIXME: Подразумевается, что left < right и top < bottom, добавить
 * стратегию для кустомизации этого понятия?
 */

/// Абстрактый прямоугольник.
/**
 *  @param ScalarType - скалярный тип
 *  @param VectType - векторный тип
 */
template<typename scalar_type, class vect_type>
struct Rect
{
	typedef typename vect_type VectType;
	typedef typename scalar_type ScalarType;
	typedef Rect<ScalarType, VectType> RectType;
	typedef Rangef RangeType;

	// конструкторы
	Rect () :
		left_ (ScalarType (0)),
		top_ (ScalarType (0)),
		width_ (ScalarType (0)),
		height_ (ScalarType (0)) {}

	/// Создаёт Rect размера \a _size, левый-верхний угол остаётся в точке (0, 0).
	Rect (const VectType& _size) :
		top_ (ScalarType (0)),
		left_ (ScalarType (0)),
		width_ (_size.x),
		height_ (_size.y) {}

	Rect (ScalarType _left, ScalarType _top, ScalarType _width, ScalarType _height) :
		left_ (_left),
		top_ (_top),
		width_ (_width),
		height_ (_height) {}

	Rect (const VectType& _top_left, const VectType& _size) :
		left_ (_top_left.x),
		top_ (_top_left.y),
		width_ (_size.x),
		height_ (_size.y) {}

	void set(const VectType& _leftTop, const VectType& _size){
		left_ = _leftTop.x;
		top_ = _leftTop.y;
		width_ = _size.x;
		height_ = _size.y;
	}

	void set(ScalarType _left, ScalarType _top, ScalarType _width, ScalarType _height){
		left_ = _left;
		top_ = _top;
		width_ = _width;
		height_ = _height;
	}

	ScalarType left () const { return left_; }
	ScalarType top () const { return top_; }
	ScalarType width () const { return width_; }
	ScalarType height () const { return height_; }

	VectType left_top() const { return VectType(left_, top_); }
	VectType right_top() const { return VectType(left_ + width_, top_); }
	VectType left_bottom() const { return VectType(left_, top_ + height_); }
	VectType right_bottom() const { return VectType(left_ + width_, top_ + height_); }

	// аксессоры (вычисляющие):
	ScalarType right () const { return left_ + width_; }
	ScalarType bottom () const { return top_ + height_; }

	/*
	* FIXME: для float и double деление на 2 лучше заменить на умножение на 0.5,
	* для целых типов лучше исползовать сдвиг.
	*/

	/// Возвращает координаты цетра прямоугольника.
	VectType   center () const { return VectType (left_ + width_ / ScalarType(2),
														top_ + height_ / ScalarType(2)); }
	/// Возвращает размер прямоугольника.
	VectType   size () const { return VectType (width_, height_); }

	// сеттеры:
	void       left (ScalarType _left) { left_ = _left; }
	void       top (ScalarType _top) { top_ = _top; }
	void       width (ScalarType _width) { width_ = _width; }
	void       height (ScalarType _height) { height_ = _height; }

	// сеттеры (вычисляющие):
	void       right (ScalarType _right) { left_ = _right - width_; }
	void       bottom (ScalarType _bottom) { top_ = _bottom - height_; }

	/// Переносит центр прямоугольника в точку \a _center не изменяя его размер.
	void       center (const VectType& _center)
	{
		left_ = _center.x - width_ / ScalarType (2);
		top_ = _center.y - height_ / ScalarType (2);
	}
	/*
	* FIXME: размер должен менятся относительно левого-верхнего угла (как у
	* сеттеров width и height) или относительно центра? Добавить
	* класс-стратегию для этих целей? Фунцию с другим именем (напр
	* scale (), которая принимает центр, относительно которого происходит
	* скэлинг)?
	*/
	/// Устанавливает новые размеры, сохраняя левый-верхний угол в преждней точке.
	void       size (const VectType& _size) { width_ = _size.x; height_ = _size.y; }

	// утилиты:

	/// Проверяет не находится ли точка \a _point внутри прямоугольника
	bool       point_inside (const VectType& _point) const
	{
		if (_point.x >= left () && _point.y >= top () &&
			_point.x <= right () && _point.y <= bottom ())
			return true;
		else
			return false;
	}
	/// Проверяет не находится ли прямоугольник \a _rect внутри прямоугольника
	bool       rect_inside (const RectType& _rect) const
	{
		if (_rect.left () >= left () && _rect.top () >= top () &&
			_rect.bottom () <= bottom () && _rect.right () <= right ())
			return true;
		else
			return false;
	}
	bool       rect_overlap (const RectType& _rect) const
	{
		if(left() > _rect.right() || right() < _rect.left()
			|| top() > _rect.bottom() || bottom() < _rect.top())
			return false;
		
		return true;
	}

	/// Производит скэлинг.
	/**
	*  Возвращает копию прямоугольника, над которой произведён скэлинг 
	*  относительно точки \a _origin.
	*/
	RectType   scaled (const VectType& _scale, const VectType& _origin) const
	{
	    
		/*
		if (_scale.x < ScalarType (0) || _scale.y < ScalarType (0))
		{
			RectType result ((*this - _origin) * _scale + _origin);
			result.validate ();
			return result;
		}
		else
		*/
			return (*this - _origin) * _scale + _origin;
	}

	/// Исправляет отрицательную ширину/высоту
	void validate ()
	{
		if (width () < ScalarType (0))
		{
			left (left () + width ());
			width (-width ());
		}
		if (height () < ScalarType (0))
		{
			top (top () + height ());
			height (-height ());
		}
	}

	RectType   intersection (const RectType& _rect) const
	{
		RangeType xRange = RangeType(left(), right()).intersection(RangeType(_rect.left(), _rect.right()));
		RangeType yRange = RangeType(top(), bottom()).intersection(RangeType(_rect.top(), _rect.bottom()));
		return RectType(xRange.minimum(), yRange.minimum(), xRange.length(), yRange.length());
	}

	RectType   merge (const RectType& _rect) const
	{
		RangeType xRange = RangeType(left(), right()).merge(RangeType(_rect.left(), _rect.right()));
		RangeType yRange = RangeType(top(), bottom()).merge(RangeType(_rect.top(), _rect.bottom()));
		return RectType(xRange.minimum(), yRange.minimum(), xRange.length(), yRange.length());
	}

	void addBound(const VectType& point)
	{
		left_ = min(left_, point.x);
		width_ = max(width_, point.x - left_);
		top_ = min(top_, point.y);
		height_ = max(height_, point.y - top_);
	}

	void addBound(const RectType& rect)
	{
		left_ = min(left_, rect.left());
		width_ = max(width_, rect.right() - left_);
		top_ = min(top_, rect.top());
		height_ = max(height_, rect.bottom() - top_);
	}

	/*
	* FIXME: каков должен быть порядок точек? По часовой стрелке?
	* сейчас порядок точек таков, что их можно сразу передавать в cQuadBuffer
	* для рисования Quad-ов (2,3,1,0)
	*/

	/// Конвертирует в массив (std::vector) точек
	std::vector<VectType> to_polygon () const
	{
		std::vector<VectType> points;
		points.push_back (VectType (left (), top ()));
		points.push_back (VectType (right (), top ()));
		points.push_back (VectType (left (), bottom ()));
		points.push_back (VectType (right (), bottom ()));
		return points;
	}

	// Операторы
	RectType operator+(const VectType& _point) const
	{
		return RectType (left () + _point.x, top () + _point.y,
						width (), height ());
	}
	RectType operator-(const VectType& _point) const
	{
		return RectType (left () - _point.x, top () - _point.y,
						width (), height ());
	}
	RectType operator*(const VectType& point) const
	{
		return RectType (left () * point.x, top () * point.y,
						width () * point.x, height () * point.y);
	}

	RectType operator*(const RectType& rhs) const
	{
		VectType leftTop(left() + width() * rhs.left(), top() + height() * rhs.top());
		VectType size(this->size() * rhs.size());
		return RectType(leftTop, size);
	}

	RectType operator/(const RectType& rhs) const
	{
		VectType leftTop((left() - rhs.left()) / rhs.width(), (top() - rhs.top()) / rhs.height());
		VectType size(width() / rhs.width(), height() / rhs.height());
		return RectType(leftTop, size);
	}

	RectType operator/ (const VectType& _point) const
	{
		return RectType (left () / _point.x, top () / _point.y,
						width () / _point.x, height () / _point.y);
	}

	bool operator==(const RectType& rect) const 
	{
		return (left_ == rect.left_ && top_ == rect.top_ &&
			width_ == rect.width_ && height_ == rect.height_);
	}

	bool eq(const RectType& rect, ScalarType eps = FLT_COMPARE_TOLERANCE) const 
	{
		return (abs(left_ - rect.left_) < eps && abs(top_ - rect.top_) < eps &&
			abs(width_ - rect.width_) < eps && abs(height_ - rect.height_) < eps);
	}

	bool operator!=(const RectType& rect) const 
	{
		return(left_ != rect.left_ || top_ != rect.top_ ||
			width_ != rect.width_ || height_ != rect.height_);
	}

	void serialize(Archive& ar){
		ar.serialize (left_,   "left",   "&Слева");
		ar.serialize (top_,    "top",    "&Сверху");
		ar.serialize (width_,  "width",  "&Ширина");
		ar.serialize(height_, "height", "&Высота");
	}

protected:
	ScalarType left_;
	ScalarType top_;
	ScalarType width_;
	ScalarType height_;

public:
	// SideKick на этом обламывается:
	template<class ST, class VT>
	operator ::Rect<ST, VT>() const {
		return ::Rect<ST, VT>(static_cast<ST>(left()),
							static_cast<ST>(top()),
							static_cast<ST>(width()),
							static_cast<ST>(height()));
	}


	bool clipLine(VectType& pos0, VectType& pos1) const;
};

typedef Rect<float, Vect2f> Rectf;
typedef Rect<int, Vect2i>   Recti;
//typedef Rect<double, Vect2d> Rectd;

template<typename ScalarType, class VectType>
bool Rect<ScalarType, VectType>::clipLine(VectType& pos0, VectType& pos1) const
{
	VectType p0(pos0), p1(pos1);

	bool b0 = point_inside(p0);
	bool b1 = point_inside(p1);

	if(b0 && b1) // вся линия внутри clip
		return true;
	else {
		float tc;
		float t[4] = {-1.0f, -1.0f, -1.0f, -1.0f};
		int find = 0;
		ScalarType dx = p1.x - p0.x;
		ScalarType dy = p1.y - p0.y;

		ScalarType crd;

		if(abs(dy) > 0){
			tc = (float)(top() - p0.y) / dy;
			if(tc >= 0.0f && tc <= 1.0f){
				crd = p0.x + tc * dx;
				if(crd >= left() && crd <= right())
					t[find++] = tc;
			}

			tc = (float)(bottom() - p0.y) / dy;
			if(tc >= 0.0f && tc <= 1.0f){
				crd = p0.x + tc * dx;
				if(crd >= left() && crd <= right())
					t[find++] = tc;
			}
		}

		if(abs(dx) > 0){
			tc = (float)(left() - p0.x) / dx;
			if(tc >= 0.0f && tc <= 1.0f){
				crd = p0.y + tc * dy;
				if(crd >= top() && crd <= bottom())
					t[find++] = tc;
			}

			tc = (float)(right() - p0.x) / dx;
			if(tc >= 0.0f && tc <= 1.0f){
				crd = p0.y + tc * dy;
				if(crd >= top() && crd <= bottom())
					t[find++] = tc;
			}
		}

		if(b0){ //внутри только точка p0
			pos1.set(p0.x + t[0]*dx, p0.y + t[0]*dy);
			pos0.set(p0.x, p0.y);
		}
		else if(b1){//внутри только точка p1
			pos0.set(p0.x + t[0]*dx, p0.y + t[0]*dy);
			pos1.set(p1.x, p1.y);
		}
		else if(find){ //обе точки снаружи, но часть отрезка внутри
			if(t[0] < t[1]) {
				pos0.set(p0.x + t[0]*dx, p0.y + t[0]*dy);
				pos1.set(p0.x + t[1]*dx, p0.y + t[1]*dy);
			}
			else {
				pos1.set(p0.x + t[0]*dx, p0.y + t[0]*dy);
				pos0.set(p0.x + t[1]*dx, p0.y + t[1]*dy);
			}
		}
		else
			return false;
	}
	return true;
}

#endif
