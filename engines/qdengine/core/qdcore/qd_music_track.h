#ifndef __QD_MUSIC_TRACK_H__
#define __QD_MUSIC_TRACK_H__

#include "xml_fwd.h"

#include "qd_named_object.h"
#include "qd_conditional_object.h"

//! Музыкальная композиция.
/**
Формат - mp+.
*/

enum qdMusicTrackFlags
{
	QD_MUSIC_TRACK_CYCLED				= 1,
	QD_MUSIC_TRACK_DISABLE_RESTART		= 2,
	QD_MUSIC_TRACK_DISABLE_SWITCH_OFF	= 4
};

class qdMusicTrack : public qdConditionalObject
{
public:
	qdMusicTrack();
	qdMusicTrack(const qdMusicTrack& trk);
	~qdMusicTrack();

	qdMusicTrack& operator = (const qdMusicTrack& trk);

	int named_object_type() const { return QD_NAMED_OBJECT_MUSIC_TRACK; }

	void set_file_name(const char* fname){ file_name_ = fname; }
	const char* file_name() const { return file_name_.c_str(); }
	bool has_file_name() const { return !file_name_.empty(); }

	void toggle_cycle(bool v){ if(v) set_flag(QD_MUSIC_TRACK_CYCLED); else drop_flag(QD_MUSIC_TRACK_CYCLED); }
	bool is_cycled() const { return check_flag(QD_MUSIC_TRACK_CYCLED); }

	int volume() const { return volume_; }
	void set_volume(int vol){ if(vol < 0) vol = 0; else if(vol > 256) vol = 256; volume_ = vol; }

	//! Чтение данных из скрипта.
	bool load_script(const xml::tag* p);
	//! Запись данных в скрипт.
	bool save_script(class XStream& fh,int indent = 0) const;

	qdConditionalObject::trigger_start_mode trigger_start();

private:

	//! Имя mp+ файла.
	std::string file_name_;

	//! Громкость, [0, 256].
	int volume_;
};

#endif /* __QD_MUSIC_H__ */
