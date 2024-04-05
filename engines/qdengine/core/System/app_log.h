#ifndef __APP_LOG_H__
#define __APP_LOG_H__

//! Отладочный лог - базовый класс.
/**
Включается только в игре (не в редакторе) ключем /log в командной строке.
По умолчанию пишет в файл qd_engine.log, при перезапуске игры лог обнуляется.
*/
class appLog
{
public:
	appLog();
	virtual ~appLog() = 0;

	//! Возвращает ссылку на лог по умолчанию (qd_engine.log).
	static appLog& default_log();

	//! Возвращает true, если запись в лог разрешена.
	bool is_enabled() const { return is_enabled_; }
	//! Разрешает запись в лог.
	void enable(){ is_enabled_ = true; }

	//! Пишет в лог строку текста.
	virtual appLog& operator << (const char* str){ return *this; }
	//! Пишет в лог целое число.
	virtual appLog& operator << (int data){ return *this; }
	//! Пишет в лог положительное целое число.
	virtual appLog& operator << (unsigned int data){ return *this; }
	//! Пишет в лог число с плавающей точкой.
	virtual appLog& operator << (float data){ return *this; }

	void set_time(unsigned tm){ time_ = tm; }
	const char* time_string() const;

private:
	//! Равно true, если запись в лог-файл разрешена.
	bool is_enabled_;

	//! Текущее время.
	unsigned time_;
};

#endif /* __APP_LOG_H__ */
