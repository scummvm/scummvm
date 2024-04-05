#ifndef __QD_CONTOUR_H__
#define __QD_CONTOUR_H__

#include <vector>

#include "xml_fwd.h"

class XStream;

//! Контур.
/**
Используется для задания масок на статических объектах,
зон на сетке и формы элементов GUI.
*/
class qdContour
{
public:
	enum qdContourType {
		//! Прямоугольник
		CONTOUR_RECTANGLE,
		//! Окружность
		CONTOUR_CIRCLE,
		//! Произвольный многоугольник
		CONTOUR_POLYGON
	};
		
	qdContour(qdContourType tp = CONTOUR_POLYGON);
	qdContour(const qdContour& ct);
	virtual ~qdContour();

	qdContour& operator = (const qdContour& ct);

	//! Возвращает тип контура.
	qdContourType contour_type() const { return contour_type_; }
	//! Устанавливает тип контура.
	void set_contour_type(qdContourType tp){ contour_type_ = tp; }

	//! Возвращает горизонтальный размер контура.
	int size_x() const { return size_.x; }
	//! Возвращает вертикальный размер контура.
	int size_y() const { return size_.y; }

	//! Возвращает true, если точка с координатами pos лежит внутри контура.
	bool is_inside(const Vect2s& pos) const;

	//! Запись данных в скрипт.
	virtual bool save_script(XStream& fh,int indent = 0) const;
	//! Чтение данных из скрипта.
	virtual bool load_script(const xml::tag* p);

	//! Добавляет точку в контур.
	/**
	pt - координаты точки
	*/
	void add_contour_point(const Vect2s& pt);
	//! Вставляет точку в контур.
	/**
	pt - координаты точки, insert_pos - номер точки, перед которой добавится новая.
	*/
	void insert_contour_point(const Vect2s& pt,int insert_pos = 0);
	//! Удаляет точку номер pos из контура.
	bool remove_contour_point(int pos);
	//! Присваивает точке номер pos контура координаты pt.
	bool update_contour_point(const Vect2s& pt,int pos);

	//! Удаляет все точки из контура.
	void clear_contour(){ 
		contour_.clear();
#ifdef _QUEST_EDITOR
		contour_updated_.clear();
#endif
	}

	//! Делит координаты контура на 2 в степени shift.
	void divide_contour(int shift = 1);
	//! Умножает координаты контура на 2 в степени shift.
	void mult_contour(int shift = 1);
	//! Добавляет к соответствующим координатам dx, dy.
	void shift_contour(int dx, int dy);

	//! Возвращает координаты точки контура номер pos.
	const Vect2s& get_point(int pos) const { return contour_[pos]; }

	//! Возвращает размеры маски.
	const Vect2s& mask_size() const { return size_; }

	//! Возвращает координаты центра маски.
	const Vect2s& mask_pos() const { return mask_pos_; }

	//! Возвращает количество точек в контуре.
	int contour_size() const { return contour_.size(); }
	//! Возвращает массив точек контура.
	const std::vector<Vect2s>& get_contour() const { return contour_; }
#ifdef _QUEST_EDITOR
	void set_contour(std::vector<Vect2s> const& contour);
#endif // _QUEST_EDITOR
	bool is_mask_empty() const { return contour_.empty(); }

	bool update_contour();

	// можно ли замкнуть текущий контур.
	// для типов контура CONTOUR_CIRCLE и CONTOUR_RECTANGLE
	// всегда возвращается false
	bool can_be_closed() const;

	//! Возвращает true, если контур пустой.
	bool is_contour_empty() const;

private:
	//! Тип контура.
	qdContourType contour_type_;

	//! Размеры контура.
	Vect2s size_;

	Vect2s mask_pos_;

	//! Контур.
	/**
	Произвольный замкнутый контур. Задается для CONTOUR_POLYGON.
	*/
	std::vector<Vect2s> contour_;
#ifdef _QUEST_EDITOR
	std::vector<Vect2s> contour_updated_;
#endif
};

#endif /* __QD_CONTOUR_H__ */

