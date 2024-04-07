#ifndef __QD_ANIMATION_H__
#define __QD_ANIMATION_H__

#include "xml_fwd.h"

#include "gr_screen_region.h"
#include "gr_tile_animation.h"

#include "qd_resource.h"
#include "qd_animation_frame.h"
#include "qd_named_object.h"

class qdAnimationInfo;

const int QD_ANIMATION_FLAG_REFERENCE		= 0x01;
const int QD_ANIMATION_FLAG_LOOP			= 0x04;
const int QD_ANIMATION_FLAG_FLIP_HORIZONTAL	= 0x08;
const int QD_ANIMATION_FLAG_FLIP_VERTICAL	= 0x10;
const int QD_ANIMATION_FLAG_BLACK_FON		= 0x20;
const int QD_ANIMATION_FLAG_SUPPRESS_ALPHA	= 0x40;
const int QD_ANIMATION_FLAG_CROP			= 0x80;
const int QD_ANIMATION_FLAG_COMPRESS		= 0x100;
const int QD_ANIMATION_FLAG_TILE_COMPRESS	= 0x200;

enum qdAnimationStatus
{
	QD_ANIMATION_STOPPED = 0,
	QD_ANIMATION_PLAYING,
	QD_ANIMATION_PAUSED,
	QD_ANIMATION_END_PLAYING
};

//! Анимация.
class qdAnimation : public qdNamedObject, public qdResource
{
public:
	qdAnimation();
	qdAnimation(const qdAnimation& anm);
	~qdAnimation();

	qdAnimation& operator = (const qdAnimation& anm);

	int named_object_type() const { return QD_NAMED_OBJECT_ANIMATION; }

	const qdAnimationFrame* get_cur_frame() const;
	const qdAnimationFrame* get_cur_frame(float& scale) const;
	qdAnimationFrame* get_cur_frame();

	void set_cur_frame(int number);
	int get_cur_frame_number() const;

	qdAnimationFrame* get_frame(int number);
	const qdAnimationFrame* get_scaled_frame(int number,int scale_index) const;

	int num_frames() const { return num_frames_; }

	float length() const { return length_; }
	float cur_time() const { return cur_time_; }

	void set_time(float tm){ cur_time_ = tm; }

	float cur_time_rel() const { 
		if(length_ > 0.01f) 
			return cur_time_/length_; 
		return 0.0f; 
	}
	void set_time_rel(float tm){ if(tm < 0.0f) tm = 0.0f; if(tm > 0.99f) tm = 0.99f; cur_time_ = length_ * tm; }
	void advance_time(float tm);

	void init_size();
	int size_x() const { return sx_; }
	int size_y() const { return sy_; }

	int picture_size_x() const;
	int picture_size_y() const;

	bool is_playing() const { 
		return (status_ == QD_ANIMATION_PLAYING || 
			status_ == QD_ANIMATION_END_PLAYING); 
	}

	int status() const { return status_; }
	bool is_finished() const { return is_finished_; }
	bool need_stop() const { return status_ == QD_ANIMATION_END_PLAYING; }

	void start(){ 
		status_ = QD_ANIMATION_PLAYING; 
		is_finished_ = false;
		cur_time_ = 0.0f; 
	}
	void stop(){ status_ = QD_ANIMATION_STOPPED; is_finished_ = true; }
	void pause(){ status_ = QD_ANIMATION_PAUSED; }
	void resume(){ status_ = QD_ANIMATION_PLAYING; }

	void quant(float dt);

	void redraw(int x,int y,int z,int mode = 0) const;
	void redraw(int x,int y,int z,float scale,int mode = 0) const;

	void redraw_rot(int x,int y,int z,float angle,int mode = 0) const;
	void redraw_rot(int x,int y,int z,float angle, const Vect2f& scale,int mode = 0) const;

	void draw_mask(int x,int y,int z,unsigned mask_color,int mask_alpha,int mode = 0) const;
	void draw_mask(int x,int y,int z,unsigned mask_color,int mask_alpha,float scale,int mode = 0) const;

	void draw_mask_rot(int x,int y,int z,float angle,unsigned mask_color,int mask_alpha,int mode = 0) const;
	void draw_mask_rot(int x,int y,int z,float angle,unsigned mask_color,int mask_alpha, const Vect2f& scale,int mode = 0) const;

	void draw_contour(int x,int y,unsigned color) const;
	void draw_contour(int x,int y,unsigned color,float scale) const;

	bool hit(int x,int y) const;
	bool hit(int x,int y,float scale) const;

	bool add_frame(qdAnimationFrame* p,qdAnimationFrame* insert_pos = 0,bool insert_after = true);
	bool remove_frame(int number);
	bool remove_frame_range(int number0,int number1);
	bool reverse_frame_range(int number0,int number1);

	void load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	const char* qda_file() const { if(!qda_file_.empty()) return qda_file_.c_str(); return 0; }
	void qda_set_file(const char* fname);

	void qda_save(const char* fname);
	bool qda_load(const char* fname);
	bool qda_load_header(const char* fname);

	bool load_resources();
	void free_resources();

	bool scale(float coeff_x,float coeff_y);

	bool crop();
	bool undo_crop();

	Vect2i remove_edges();

	bool compress();
	bool uncompress();
	bool tileCompress(grTileCompressionMethod method = TILE_UNCOMPRESSED,int tolerance = 0);

	qdAnimationFrameList& frames_list() { return frames; };

	void create_reference(qdAnimation* p,const qdAnimationInfo* inf = NULL) const;
	bool is_reference(const qdAnimation* p) const { 
		if(p -> check_flag(QD_ANIMATION_FLAG_REFERENCE) && p -> parent_ == this) return true;
		return false;
	}

	void clear(){ 
		stop(); 
		frames_ptr = &frames;
		parent_ = NULL;
	}

	bool is_empty() const { return (frames_ptr -> empty()); }

	//! Возвращает область экрана, занимаемую анимацией.
	/**
	Координаты области - смещение от центра анимации.
	В mode задаются повороты анимации по горизонтали и вертикали
	(QD_ANIMATION_FLAG_FLIP_HORIZONTAL, QD_ANIMATION_FLAG_FLIP_VERTICAL)
	*/
	grScreenRegion screen_region(int mode = 0,float scale = 1.0f) const;

	const qdAnimation* parent() const { return parent_; }

	// qdResource
	bool load_resource();
	bool free_resource();
	//! Устанавливает имя файла, в котором хранятся данные ресурса.
	void set_resource_file(const char* file_name){ qda_set_file(file_name);	}
	//! Возвращает имя файла, в котором хранится анимация.
	const char* resource_file() const { 
		if(!qda_file()){
			if(!check_flag(QD_ANIMATION_FLAG_REFERENCE) && !frames.empty()){
				if(frames.front() -> has_file())
					return frames.front() -> file();
				else
					return NULL;
			}
			else
				return NULL;
		}
		else
			return qda_file(); 
	}
#ifdef __QD_DEBUG_ENABLE__
	unsigned resource_data_size() const;
#endif

	bool load_resource_header(const char* fname = NULL);

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	bool add_scale(float value);
	bool create_scaled_frames();

	const std::vector<float>& scales() const { if(check_flag(QD_ANIMATION_FLAG_REFERENCE) && parent_) return parent_ -> scales_; else return scales_; }
	void clear_scales(){ scales_.clear(); }

private:
	int sx_;
	int sy_;

	enum {
		qda_version = 104
	};

	float length_;
	float cur_time_;

	float playback_speed_;

	int num_frames_;

	const qdAnimationFrameList* frames_ptr;
	qdAnimationFrameList frames;

	const qdAnimationFrameList* scaled_frames_ptr_;
	qdAnimationFrameList scaled_frames_;
	std::vector<float> scales_;

	grTileAnimation* tileAnimation_;

	int status_;
	bool is_finished_;

	std::string qda_file_;

	const qdAnimation* parent_;

	int get_scale_index(float& scale_value) const;

	bool copy_frames(const qdAnimation& anm);
	void clear_frames();

	const grTileAnimation* tileAnimation() const
	{
		if(check_flag(QD_ANIMATION_FLAG_REFERENCE) && parent_)
			return parent_->tileAnimation_;
		else
			return tileAnimation_;
	}
};

#endif /* __QD_ANIMATION_H__ */

