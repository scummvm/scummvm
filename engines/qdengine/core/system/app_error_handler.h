#ifndef __APP_ERROR_HANDLER_H__
#define __APP_ERROR_HANDLER_H__

//! Обработчик ошибок.
class appErrorHandler
{
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
		ERR_MB_OK			= 1,
		ERR_MB_ABORTRETRYIGNORE		= 2
	};

	handler_result_t message_box(const char* subject = NULL,error_t err_code = ERR_OTHER,int format = ERR_MB_OK);

	void show_error(const char* subject = NULL,error_t err_code = ERR_OTHER);

	static void enable(){ is_disabled_ = false; }
	static void disable(){ is_disabled_ = true; }

private:
	static bool is_disabled_;
	static std::string message_buf_;
	static const char* error_messages_[];
};

extern appErrorHandler app_errH;

#endif /* __APP_ERROR_HANDLER_H__ */

