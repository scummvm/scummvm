/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef QDENGINE_QDCORE_QD_INTERFACE_ELEMENT_STATE_MODE_H
#define QDENGINE_QDCORE_QD_INTERFACE_ELEMENT_STATE_MODE_H

#include "common/path.h"
#include "qdengine/qdcore/qd_contour.h"

namespace QDEngine {

//! Событие, привязанное к состоянию интерфейсного элемента.
/**
Включает анимацию и звук при выполнении некоторых условий (например при наведении
мыши на элемент интерфейса.
*/
class qdInterfaceElementStateMode {
public:
	qdInterfaceElementStateMode();
	qdInterfaceElementStateMode(const qdInterfaceElementStateMode &ev);
	~qdInterfaceElementStateMode();

	qdInterfaceElementStateMode &operator = (const qdInterfaceElementStateMode &ev);

	//! Устанавливает имя файла звука.
	/**
	Если надо убрать звук - передать NULL в качестве имени файла.
	*/
	void set_sound_file(const Common::Path name);
	//! Возвращает имя файла звука.
	const Common::Path sound_file() const {
		return _sound_file;
	}
	//! Устанавливает звук события.
	void set_sound(const qdSound *p) {
		_sound = p;
	}
	//! Возвращает звук события.
	const qdSound *sound() const {
		return _sound;
	}
	//! Возвращает true, если к событию привязан звук.
	bool has_sound() const {
		return !_sound_file.empty();
	}

	//! Устанавливает имя файла для анимации.
	/**
	Если надо убрать анимацию - передать NULL в качестве имени файла.
	*/
	void set_animation_file(const Common::Path name);
	//! Возвращает имя файла для анимации.
	const Common::Path animation_file() const {
		return _animation_file;
	}
	//! Возвращает флаги анимации.
	int animation_flags() const {
		return _animation_flags;
	}

	//! Устанавливает флаг анимации.
	void set_animation_flag(int fl) {
		_animation_flags |= fl;
	}
	//! Скидывает флаг анимации.
	void drop_animation_flag(int fl) {
		_animation_flags &= ~fl;
	}
	//! Возвращает true, если для анимации установлен флаг fl.
	bool check_animation_flag(int fl) const {
		if (_animation_flags & fl) return true;
		return false;
	}

	//! Устанавливает указатель на анимацию события.
	void set_animation(const qdAnimation *p) {
		_animation = p;
	}
	//! Возвращает указатель на анимацию события.
	const qdAnimation *animation() const {
		return _animation;
	}
	//! Возвращает true, если к состоянию привязана анимация.
	bool has_animation() const {
		return !_animation_file.empty();
	}
	//! Возвращает true, если у состояния задан контур.
	bool has_contour() const {
		return !_contour.is_contour_empty();
	}
	//! Возвращает true, если точка с экранными координатами (x,у) попадает внутрь контура.
	bool hit_test(int x, int y) const {
		return _contour.is_inside(Vect2s(x, y));
	}

	//! Записывает контур в cnt.
	bool get_contour(qdContour &cnt) const {
		cnt = _contour;
		return true;
	}
	//! Устанавливает контур.
	bool set_contour(const qdContour &cnt) {
		_contour = cnt;
		return true;
	}

	//! Запись данных в скрипт.
	/**
	type_id - тип события
	*/
	bool save_script(Common::WriteStream &fh, int type_id = 0, int indent = 0) const;
	//! Загрузка данных из скрипта.
	bool load_script(const xml::tag *p);

private:

	//! Имя файла звука.
	Common::Path _sound_file;

	//! Звук, включаемый событием.
	const qdSound *_sound;

	//! Имя файла для анимации
	Common::Path _animation_file;
	//! Флаги для анимации.
	int _animation_flags;

	//! Анимация, включаемая событием.
	const qdAnimation *_animation;

	//! Контур.
	qdContour _contour;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_INTERFACE_ELEMENT_STATE_MODE_H
