#ifndef __QD_VIDEO_H__
#define __QD_VIDEO_H__

#include "xml_fwd.h"

#include "qd_sprite.h"
#include "qd_conditional_object.h"

#include "qd_file_owner.h"

//! Видеовставка.
class qdVideo : public qdConditionalObject, public qdFileOwner
{
public:
	enum { // flags
		//! расположить в центре экрана
		VID_CENTER_FLAG			= 0x01,
		//! растягивать на весь экран
		VID_FULLSCREEN_FLAG		= 0x02,
		//! нельзя прерывать ролик нажатием мыши/кнопки
		VID_DISABLE_INTERRUPT_FLAG	= 0x04,
		//! показывать ролик при запуске игры до главного меню
		VID_INTRO_MOVIE			= 0x08,
		//! не прерывать музыку при показе видео
		VID_ENABLE_MUSIC		= 0x10
	};

	qdVideo();
	qdVideo(const qdVideo& v);
	~qdVideo();

	qdVideo& operator = (const qdVideo& v);
	int named_object_type() const { return QD_NAMED_OBJECT_VIDEO; }

	const char* file_name() const { return file_name_.c_str(); }
	void set_file_name(const char* fname){ file_name_ = fname; }

	// Фон, на котором будет проигрываться видео
	void set_background_file_name(const char* fname){ background_.set_file(fname); }
	const char* background_file_name() const { return background_.file(); }
	bool draw_background();

	// Экранные координаты верхнего левого угла
	const Vect2s& position() const { return position_; }
	void set_position(const Vect2s pos){ position_ = pos; }

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	qdConditionalObject::trigger_start_mode trigger_start();

	bool adjust_files_paths(const char* copy_dir, const char* pack_dir, bool can_overwrite = false);
	bool get_files_list(qdFileNameList& files_to_copy,qdFileNameList& files_to_pack) const;

	bool is_intro_movie() const { return check_flag(VID_INTRO_MOVIE); }

private:

	Vect2s position_;
	std::string file_name_;

	qdSprite background_;
};

#endif /* __QD_VIDEO_H__ */

