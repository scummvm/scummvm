#ifndef __APP_LOG_FILE_H__
#define __APP_LOG_FILE_H__

#ifndef _QUEST_EDITOR
#define __APP_LOG_ENABLE__
#endif

class XStream;

//! Отладочный лог-файл.
/**
	Включается только в игре (не в редакторе) ключем /log в командной строке.
	По умолчанию пишет в файл qd_engine.log, при перезапуске игры лог обнуляется.
*/
class appLogFile : public appLog
{
public:
	explicit appLogFile(const char* fname = NULL);
	~appLogFile();

	//! Открытвает лог-файл с именем fname.
	bool open(const char* fname = NULL);
	//! Возвращает true, если лог-файл открыт.
	bool is_opened() const { if(file_) return true; else return false; }
	//! Закрывает лог-файл.
	bool close();

	//! Пишет в лог-файл строку текста.
	appLog& operator << (const char* str);
	//! Пишет в лог-файл целое число.
	appLog& operator << (int data);
	//! Пишет в лог-файл положительное целое число.
	appLog& operator << (unsigned int data);
	//! Пишет в лог-файл число с плавающей точкой.
	appLog& operator << (float data);

private:
	//! Поток для записи в лог-файл.
	XStream* file_;
	//! Имя лог-файла.
	std::string file_name_;

	XStream* file(){ if(!is_opened()) open(); return file_; }
};

#endif /* __APP_LOG_FILE_H__ */
