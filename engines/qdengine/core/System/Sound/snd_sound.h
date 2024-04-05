
#ifndef __SND_SOUND_H__
#define __SND_SOUND_H__

class wavSound;
class qdNamedObject;

//! Класс для управления звуками.
class sndHandle
{
public:
	sndHandle(){ };
	virtual ~sndHandle(){ };
};

//! Базовый класс для звуков.
class sndSound
{
public:
	explicit sndSound(const wavSound* snd,const sndHandle* h = NULL) : sound_(snd), handle_(h) { }
	sndSound(const sndSound& snd) : sound_(snd.sound_), handle_(snd.handle_) { }
	virtual ~sndSound(){ };

	sndSound& operator = (const sndSound& s){
		if(this == &s) return *this;

		sound_ = s.sound_;
		handle_ = s.handle_;

		return *this;
	}

	//! Состояние звука.
	enum status_t {
		//! звук не проигрывается
		SOUND_STOPPED,
		//! звук приостановлен
		SOUND_PAUSED,
		//! звук пригрывается
		SOUND_PLAYING
	};

	//! Возвращает состояние звука.
	virtual status_t status() const { return SOUND_STOPPED; }

	//! Возвращает указатель на данные звука.
	const wavSound* sound() const { return sound_; }
	//! Возвращает указатель на хэндл звука.
	const sndHandle* handle() const { return handle_; }

private:
	//! Указатель на данные.
	const wavSound* sound_;
	//! Указатель на хэндл звука.
	const sndHandle* handle_;
};

#endif /* __SND_SOUND_H__ */