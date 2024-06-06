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

#ifndef __APP_LOG_H__
#define __APP_LOG_H__


//! Отладочный лог - базовый класс.
/**
Включается только в игре (не в редакторе) ключем /log в командной строке.
По умолчанию пишет в файл qd_engine.log, при перезапуске игры лог обнуляется.
*/
namespace QDEngine {

class appLog {
public:
	appLog();
	virtual ~appLog() = 0;

	//! Возвращает ссылку на лог по умолчанию (qd_engine.log).
	static appLog &default_log();

	//! Возвращает true, если запись в лог разрешена.
	bool is_enabled() const {
		return is_enabled_;
	}
	//! Разрешает запись в лог.
	void enable() {
		is_enabled_ = true;
	}

	//! Пишет в лог строку текста.
	virtual appLog &operator << (const char *str) {
		return *this;
	}
	//! Пишет в лог целое число.
	virtual appLog &operator << (int data) {
		return *this;
	}
	//! Пишет в лог положительное целое число.
	virtual appLog &operator << (unsigned int data) {
		return *this;
	}
	//! Пишет в лог число с плавающей точкой.
	virtual appLog &operator << (float data) {
		return *this;
	}

	void set_time(unsigned tm) {
		time_ = tm;
	}
	const char *time_string() const;

private:
	//! Равно true, если запись в лог-файл разрешена.
	bool is_enabled_;

	//! Текущее время.
	unsigned time_;
};

} // namespace QDEngine

#endif /* __APP_LOG_H__ */
