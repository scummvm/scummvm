
#ifndef __PLAYMPP_API_H__
#define __PLAYMPP_API_H__

//! Проигрыватель mp+.
class MpegSound;

//! Интерфейс к проигрывателю mp+.
class mpegPlayer
{
public:
	//! Состояние.
	enum mpeg_status_t {
		//! музыка выключена
		MPEG_STOPPED,
		//! музыка на паузе
		MPEG_PAUSED,
		//! музыка играет
		MPEG_PLAYING
	};

	~mpegPlayer();

	//! Запускает проигрывание mp+ файла file.
	/**
	Если параметр loop == true, то музыка зациклена.
	*/
	bool play(const char* file,bool loop = false,int vol = 256);
	//! Останавливает проигрывание mp+ файла.
	bool stop();
	//! Приостанавливает проигрывание mp+ файла.
	bool pause();
	//! Продолжает проигрывание mp+ файла.
	bool resume();

	//! Возвращает true, если проигрывание музыки разрешено.
	bool is_enabled() const { return is_enabled_; }
	//! Разрешает проигрывание музыки.
	void enable(bool need_resume = true){ is_enabled_ = true; if(need_resume) resume(); }
	//! Запрещает проигрывание музыки.
	void disable(){ is_enabled_ = false; pause(); }

	//! Возвращает текущее состояние проигрывателя.
	mpeg_status_t status() const;
	//! Возвращает true, если музыка играет.
	bool is_playing() const { return (status() == MPEG_PLAYING); }

	//! Возвращает громкость музыки.
	unsigned int volume() const { return volume_; }
	//! Устанавливает громкость музыки.
	void set_volume(unsigned int vol);

	//! Инициализация mp+ библиотеки.
	static bool init_library(void* dsound_device);
	//! Деинициализация mp+ библиотеки.
	static void deinit_library();

	//! Возвращает проигрыватель, через который идет все управление музыкой.
	static mpegPlayer& instance();

private:

	mpegPlayer();

	bool isOGG_;
	MpegSound* oggPlayer_;

	//! Музыка выключена, если false.
	bool is_enabled_;

	//! Громкость музыки, [0, 255].
	unsigned int volume_;
	unsigned int cur_track_volume_;

	//! Если равен true, значит библиотеку уже инициализаровали.
	static bool is_library_initialized_;
};

#endif /* __PLAYMPP_API_H__ */
