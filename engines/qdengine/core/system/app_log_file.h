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

#ifndef __APP_LOG_FILE_H__
#define __APP_LOG_FILE_H__

#ifndef _QUEST_EDITOR
#define __APP_LOG_ENABLE__
#endif



namespace QDEngine {

class XStream;

//! Отладочный лог-файл.
/**
    Включается только в игре (не в редакторе) ключем /log в командной строке.
    По умолчанию пишет в файл qd_engine.log, при перезапуске игры лог обнуляется.
*/
class appLogFile : public appLog {
public:
	explicit appLogFile(const char *fname = NULL);
	~appLogFile();

	//! Открытвает лог-файл с именем fname.
	bool open(const char *fname = NULL);
	//! Возвращает true, если лог-файл открыт.
	bool is_opened() const {
		if (file_) return true;
		else return false;
	}
	//! Закрывает лог-файл.
	bool close();

	//! Пишет в лог-файл строку текста.
	appLog &operator << (const char *str);
	//! Пишет в лог-файл целое число.
	appLog &operator << (int data);
	//! Пишет в лог-файл положительное целое число.
	appLog &operator << (unsigned int data);
	//! Пишет в лог-файл число с плавающей точкой.
	appLog &operator << (float data);

private:
	//! Поток для записи в лог-файл.
	XStream *file_;
	//! Имя лог-файла.
	std::string file_name_;

	XStream *file() {
		if (!is_opened()) open();
		return file_;
	}
};

} // namespace QDEngine

#endif /* __APP_LOG_FILE_H__ */
