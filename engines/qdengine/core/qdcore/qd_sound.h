#ifndef __QD_SOUND_H__
#define __QD_SOUND_H__

#include "xml_fwd.h"

#include "qd_named_object.h"
#include "qd_resource.h"
#include "qd_sound_handle.h"

#ifndef __QD_SYSLIB__
#include "wav_sound.h"
#endif

//! Звук.
class qdSound : public qdNamedObject, public qdResource
{
public:
	qdSound();
	~qdSound();

	int named_object_type() const { return QD_NAMED_OBJECT_SOUND; }

	//! Загрузка данных из скрипта.
	void load_script(const xml::tag* p);
	//! Запись данных в скрипт.
	bool save_script(XStream& fh,int indent = 0) const;

	bool load_resource();
	bool free_resource();
	//! Устанавливает имя файла, в котором хранятся данные ресурса.
	void set_resource_file(const char* file_name){ set_file_name(file_name); }
	//! Возвращает имя файла, в котором хранится анимация.
	const char* resource_file() const { return file_name(); }
#ifdef __QD_DEBUG_ENABLE__
	unsigned resource_data_size() const { return sound_.data_length(); }
#endif

	//! Возвращает имя файла, в котором хранится звук.
	const char* file_name() const { if(!file_name_.empty()) return file_name_.c_str(); return 0; }
	//! Устанавливает имя файла, в котором хранится звук.
	void set_file_name(const char* fname){ file_name_ = fname; }

	//! Запускает проигрывание звука.
	/**
	handle нужен для дальнейшего управления звуком и отслеживания его состояния,
	start_position - начальная позиция при запуске звука, значения - [0.0, 1.0]
	*/
	bool play(const qdSoundHandle* handle = NULL,bool loop = false,float start_position = 0.0f) const;
	//! Останавливает проигрывание звука.
	/**
	Если handle нулевой - то выключится первый попавшийся активный звук с данными
	из sound_, если ненулевой - первый соответствующий этому хэндлу.
	*/
	bool stop(const qdSoundHandle* handle = NULL) const;

	//! Возвращает текущую позицию звука, диапазон возвращаемых значений - [0.0, 1.0].
	/**
	Если звук не играется, текущая позиция считается нулевой. 
	*/
	float position(const qdSoundHandle* handle = NULL) const;

	//! Изменение частоты звука в frequency_coeff число раз.
	bool set_frequency(const qdSoundHandle* handle,float frequency_coeff) const;

	//! Возвращает длительность звука в секундах.
	float length() const { 
#ifndef __QD_SYSLIB__
		return sound_.length(); 
#endif
	}

	//! Возвращает true, если звук не проигрывается.
	bool is_stopped(const qdSoundHandle* handle = NULL) const;

	int volume() const { return volume_; }
	void set_volume(int vol){ volume_ = vol; }

private:

	//! Имя файла.
	std::string file_name_;
	//! Громкость звука, [0, 255].
	int volume_;
	//! Данные.
#ifndef __QD_SYSLIB__
	wavSound sound_;
#endif
};

typedef std::list<qdSound*> qdSoundList;

#endif /* __QD_SOUND_H__ */

