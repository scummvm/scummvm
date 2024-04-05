#ifndef __DS_SND_DISPATCHER_H__
#define __DS_SND_DISPATCHER_H__

#include "snd_dispatcher.h"
#include "ds_sound.h"

//! Диспетчер звуков на DirectSound.
class ds_sndDispatcher : public sndDispatcher
{
public:
	ds_sndDispatcher();
	~ds_sndDispatcher();

	//! Логический квант.
	void quant();
	//! Запускает проигрывание звука.
	bool play_sound(const sndSound* snd,bool loop,float start_position = 0.0f,int vol = 255);
	//! Останавливает проигрывание звука.
	bool stop_sound(const sndSound* snd);
	//! Останавливает проигрывание звука.
	bool stop_sound(const sndHandle* handle);
	//! Возвращает состояние звука (играется/остановлен и т.д.).
	sndSound::status_t sound_status(const sndHandle* handle) const;
	//! Возвращает состояние звука (играется/остановлен и т.д.).
	sndSound::status_t sound_status(const sndSound* snd) const;
	//! Возвращает текущую позицию звука, от 0.0 (начало) до 1.0 (конец).
	float sound_position(const sndHandle* snd) const;
	//! Изменение частоты звука.
	bool set_sound_frequency(const sndHandle* snd,float coeff);

	//! Останавливает все звуки.
	void stop_sounds();
	//! Ставит все играющие в данный момент звуки на паузу.
	void pause_sounds();
	//! Возобновляет проигрывание всех звуков, которые были поставлены на паузу.
	void resume_sounds();

protected:

	//! Обновление установки громкости.
	bool update_volume();

	bool update_frequency();

private:

	typedef std::list<dsSound> sound_list_t;
	//! Список активных звуков.
	sound_list_t sounds_;

	//! Указатель на DirectSound интерфейс.
	LPDIRECTSOUND sound_device_;
};

#endif /* __DS_SND_DISPATCHER_H__ */
