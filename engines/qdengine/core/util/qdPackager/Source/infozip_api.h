#ifndef __INFOZIP_API_H__
#define __INFOZIP_API_H__

//! Интерфейс к Info-ZIP для создания контейнеров ресурсов.
/**
Опробован на Info-ZIP v2.3.
*/
namespace infozip_api
{
	//! Инициализация Info-ZIP. По умолчанию паковка файлов отключена.
	bool init();
	//! Установка опций.
	//! Компрессия - от 0 (мин.) до 9 (макс.).
	bool set_options(int compression = 0);
	//! Добавление файла с именем file_name в архив с именем zip_file_name.
	bool add_to_zip(const char* zip_file_name, const char* file_name, int compression = 0);
};

#endif // __INFOZIP_API_H__

