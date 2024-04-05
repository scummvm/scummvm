#ifndef __QDSCR_CONVERT_H__
#define __QDSCR_CONVERT_H__

//! Преобразование старого скрипта в новый XML формат.
bool qdscr_convert_to_XML(const char* file_name,const char* new_file_name = NULL);
//! Возвращает true, если скрипт в новом формате (проверяет только соответствие расширения имени файла). 
bool qdscr_is_XML(const char* file_name);
//! Меняет расширение имени файла (на .qds).
const char* qdscr_get_XML_file_name(const char* file_name);

#endif /* __QDSCR_CONVERT_H__ */
