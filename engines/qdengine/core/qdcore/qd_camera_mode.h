
#ifndef __QD_CAMERA_MODE_H__
#define __QD_CAMERA_MODE_H__

#include "xml_fwd.h"

class qdGameObjectState;
class qdGameObjectAnimated;

class XStream;

//! Режим работы камеры.
class qdCameraMode
{
public:
	qdCameraMode();
	~qdCameraMode(){ }

	//! идентификаторы режимов камеры
	enum camera_mode_t {
		//! камера не выпускает объект с экрана
		MODE_OBJECT_ON_SCREEN,
		//! камера жестко привязана к объекту
		MODE_CENTER_OBJECT,
		//! камера все время центруется на объекте
		MODE_FOLLOW_OBJECT,
		//! когда объект подходит к краю экрана камера плавно на нем центруется
		MODE_CENTER_OBJECT_WHEN_LEAVING,
		//! режим не установлен
		MODE_UNASSIGNED
	};

	void set_camera_mode(camera_mode_t mode){ camera_mode_ = mode; }
	camera_mode_t camera_mode() const { return camera_mode_; }

	void set_work_time(float tm){ work_time_ = tm; }
	float work_time() const { return work_time_; }
	bool has_work_time() const { return work_time_ > 0.001f; }
	
	void set_scrolling_speed(float v){ scrolling_speed_ = v; }
	float scrolling_speed() const { return scrolling_speed_; }

	void set_scrolling_distance(int dist){ scrolling_distance_ = dist; }
	int scrolling_distance() const { return scrolling_distance_; }

	bool smooth_switch() const { return smooth_switch_; }
	void set_smooth_switch(bool v){ smooth_switch_ = v; }

	const Vect2i& center_offset() const { return center_offset_; }
	void set_center_offset(const Vect2i& offs){
		center_offset_ = offs;
	}

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

private:

	//! Идентификатор режима.
	camera_mode_t camera_mode_;

	//! Время работы режима (в секундах).
	/**
	По истечении этого времени камера переключается 
	на активного персонажа (если он есть).

	Если это время нулевое, то сменить режим можно будет
	только в принудительном порядке из триггера.
	*/
	float work_time_;
	
	//! Скорость, с которой камера скроллируется (в пикселах в секунду).
	float scrolling_speed_;

	//! Расстояние от центра объекта до края экрана, при котором включается скроллинг.
	/**
	В режимах, когда камера не дает объекту выйти за пределы экрана или
	центруется на объекте когда он близко к краю экрана.

	Задается в пикселах.
	*/
	int scrolling_distance_;

	//! Смещение центра экрана в пикселах.
	/**
	Позволяет задавать режим, когда камера, центруясь на объекте,
	держит его в определенной точке экрана.
	*/
	Vect2i center_offset_;

	//! Плавное переключение в режим, если true.
	bool smooth_switch_;
};

#endif /* __QD_CAMERA_MODE_H__ */
