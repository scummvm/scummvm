/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "app_core.h"
#include "app_error_handler.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

std::string appErrorHandler::message_buf_(1024,0);

const char* appErrorHandler::error_messages_[ERR_MAX_TYPE] = 
{
	"Файл не найден",
	"Неизвестный формат файла",
	"Несовместимая версия интерфейса миниигры",
	"Ошибка"
};

bool appErrorHandler::is_disabled_;

appErrorHandler app_errH;

appErrorHandler::appErrorHandler()
{
}

appErrorHandler::~appErrorHandler()
{
}

void appErrorHandler::show_error(const char* subject,error_t err_code)
{
	message_box(subject,err_code,ERR_MB_OK);

#ifndef _QUEST_EDITOR
	exit(1);
#endif
}

appErrorHandler::handler_result_t appErrorHandler::message_box(const char* subject,error_t err_code,int format)
{
	if(is_disabled_) return ERR_IGNORE;

	xassert(err_code < ERR_MAX_TYPE);

	message_buf_ = error_messages_[err_code];
	if(subject){
		message_buf_ += "\n";
		message_buf_ += subject;
	}

	int mb_type = MB_ICONERROR;

	if(format & ERR_MB_OK)
		mb_type |= MB_OK;
	if(format & ERR_MB_ABORTRETRYIGNORE)
		mb_type |= MB_ABORTRETRYIGNORE;

	switch(MessageBox(static_cast<HWND>(appGetHandle()),message_buf_.c_str(),"Ошибка",mb_type)){
		case IDIGNORE:
			return ERR_IGNORE;
		case IDRETRY:
			return ERR_RETRY;
		default:
			return ERR_ABORT;
	}
}
