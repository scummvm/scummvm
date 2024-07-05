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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/textconsole.h"
#include "qdengine/qd_precomp.h"
#include "qdengine/system/app_core.h"
#include "qdengine/system/app_error_handler.h"
#include "qdengine/xlibs/xutil/xutil.h"


namespace QDEngine {

std::string appErrorHandler::message_buf_(1024, 0);

const char *appErrorHandler::error_messages_[ERR_MAX_TYPE] = {
	"Файл не найден",
	"Неизвестный формат файла",
	"Несовместимая версия интерфейса миниигры",
	"Ошибка"
};

bool appErrorHandler::is_disabled_;

appErrorHandler app_errH;

appErrorHandler::appErrorHandler() {
}

appErrorHandler::~appErrorHandler() {
}

void appErrorHandler::show_error(const char *subject, error_t err_code) {
	message_box(subject, err_code, ERR_MB_OK);

#ifndef _QUEST_EDITOR
	exit(1);
#endif
}

appErrorHandler::handler_result_t appErrorHandler::message_box(const char *subject, error_t err_code, int format) {
	if (is_disabled_) return ERR_IGNORE;
	xassert(err_code < ERR_MAX_TYPE);
	message_buf_ = error_messages_[err_code];
	if (subject) {
		message_buf_ += "\n";
		message_buf_ += subject;
	}

	int mb_type = MB_ICONERROR;

	if (format & ERR_MB_OK)
		mb_type |= MB_OK;
	if (format & ERR_MB_ABORTRETRYIGNORE)
		mb_type |= MB_ABORTRETRYIGNORE;

	switch (MessageBox(static_cast<HWND>(appGetHandle()), message_buf_.c_str(), "Ошибка", mb_type)) {
	case IDIGNORE:
		return ERR_IGNORE;
	case IDRETRY:
		return ERR_RETRY;
	default:
		return ERR_ABORT;
	}
	return ERR_ABORT;
}

} // namespace QDEngine
