#ifndef __QD_GRID_ZONE_H__
#define __QD_GRID_ZONE_H__

#include "xml_fwd.h"

#include "qd_contour.h"
#include "qd_grid_zone_state.h"

class qdCamera;

const int QD_NO_SHADOW_ALPHA		= -1;
const int QD_SHADOW_ALPHA_MIN		= 0;
const int QD_SHADOW_ALPHA_MAX		= 255;

//! Зона на сетке сцены.
class qdGridZone : public qdNamedObject, public qdContour
{
public:
	qdGridZone();
	qdGridZone(const qdGridZone& gz);

	qdGridZone& operator = (const qdGridZone& gz);

	~qdGridZone();

	//! Команды для изменения состояния зоны в триггерах.
	enum status_change {
		//! выключить зону
		ZONE_OFF = 0,
		//! включить зону
		ZONE_ON,
		//! переключить зону
		ZONE_TOGGLE
	};

	//! Флаги
	enum {
		//! т.н. "зона выхода" - при наведении на нее мышью курсор меняется
		ZONE_EXIT_FLAG = 0x01
	};

	int named_object_type() const { return QD_NAMED_OBJECT_GRID_ZONE; }

	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag* p);
	//! Запись данных в скрипт.
	bool save_script(XStream& fh,int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	bool state() const { return state_; }
	bool set_state(bool st);
	bool toggle_state(){ return set_state(!state_); }

	unsigned height() const { return height_; }
	bool set_height(int _h);

	bool select(qdCamera* camera, bool bSelect) const;
	bool select(bool bSelect) const;

	bool is_object_in_zone(const qdGameObject* obj) const;
	bool is_point_in_zone(const Vect2f& vPoint) const;

	unsigned int update_timer() const { return update_timer_; }

	qdGridZoneState* get_state(const char* state_name);

	//! Инициализация данных, вызывается при старте и перезапуске игры.
	bool init();

	bool is_any_personage_in_zone() const;

	int shadow_alpha() const { return shadow_alpha_; }
	void set_shadow_alpha(int alpha){ shadow_alpha_ = alpha; }

	unsigned shadow_color() const { return shadow_color_; }
	void set_shadow_color(unsigned color){ shadow_color_ = color; }
	bool has_shadow() const { return (shadow_alpha_ != QD_NO_SHADOW_ALPHA); }

private:

	//! Текущее состояние зоны (вкл/выкл).
	bool state_;
	//! Начальное состояние зоны (вкл/выкл).
	bool initial_state_;

	//! Цвет затенения персонажа, когда он находится в зоне, RGB().
	unsigned shadow_color_;
	//! Прозрачность затенения персонажа, когда он находится в зоне.
	/**
	Значения - [0, 255].
	Допускается также QD_NO_SHADOW_ALPHA, затенения при этом отключается.
	*/
	int shadow_alpha_;

	//! Высота зоны.
	unsigned int height_;

	//! Время изменения состояния.
	unsigned int update_timer_;

	//! Состояние включающее зону.
	qdGridZoneState state_on_;
	//! Состояние выключающее зону.
	qdGridZoneState state_off_;

	bool apply_zone() const;
};

#endif /* __QD_GRID_ZONE_H__ */

