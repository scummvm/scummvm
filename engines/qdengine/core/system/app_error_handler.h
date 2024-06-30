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

#ifndef QDENGINE_CORE_SYSTEM_APP_ERROR_HANDLER_H
#define QDENGINE_CORE_SYSTEM_APP_ERROR_HANDLER_H



namespace QDEngine {

//! Обработчик ошибок.
class appErrorHandler {
public:
	appErrorHandler();
	~appErrorHandler();

	//! Коды ошибок
	enum error_t {
		ERR_FILE_NOT_FOUND,
		ERR_BAD_FILE_FORMAT,
		ERR_MINIGAME_VERSION,
		ERR_OTHER,

		ERR_MAX_TYPE
	};

	//! Значения, возвращаемые обработчиком.
	enum handler_result_t {
		ERR_ABORT,
		ERR_RETRY,
		ERR_IGNORE
	};

	//! Формат окна сообщения об ошибке.
	enum {
		ERR_MB_OK           = 1,
		ERR_MB_ABORTRETRYIGNORE     = 2
	};

	handler_result_t message_box(const char *subject = NULL, error_t err_code = ERR_OTHER, int format = ERR_MB_OK);

	void show_error(const char *subject = NULL, error_t err_code = ERR_OTHER);

	static void enable() {
		is_disabled_ = false;
	}
	static void disable() {
		is_disabled_ = true;
	}

private:
	static bool is_disabled_;
	static std::string message_buf_;
	static const char *error_messages_[];
};

extern appErrorHandler app_errH;

} // namespace QDEngine

#endif /* QDENGINE_CORE_SYSTEM_APP_ERROR_HANDLER_H */
