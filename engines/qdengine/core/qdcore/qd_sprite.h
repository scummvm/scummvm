#ifndef __QD_SPRITE_H__
#define __QD_SPRITE_H__

#include "gr_screen_region.h"
#include "qd_resource.h"

class XZipStream;

//! Спрайт.
class qdSprite : public qdResource
{
public:
	enum { // flags
		ALPHA_FLAG		= 0x01
	};

	qdSprite();
	qdSprite(int wid, int hei, int format);
	qdSprite(const qdSprite& spr);

	~qdSprite();

	qdSprite& operator = (const qdSprite& spr);

	const Vect2i& size() const { return size_; }
	void set_size(const Vect2i& size){ size_ = size; }

	int size_x() const { return size_.x; }
	int size_y() const { return size_.y; }

	int picture_x() const { return picture_offset_.x; }
	int picture_y() const { return picture_offset_.y; }
	void set_picture_offset(const Vect2i& offs){ picture_offset_ = offs; }

	int picture_size_x() const { return picture_size_.x; }
	int picture_size_y() const { return picture_size_.y; }
	void set_picture_size(const Vect2i& size){ picture_size_ = size; }

	int format() const { return format_; }
	const unsigned char* data() const { return data_; }
	unsigned data_size() const;

	void set_file(const char* fname){ if(fname) file_ = fname; else file_.clear(); }
	const char* file() const { return file_.c_str(); }
	bool has_file() const { return !file_.empty(); }

	bool load(const char* fname = 0);
	void save(const char* fname = 0);
	void free();

	virtual void qda_load(XStream& fh,int version = 100);
	virtual void qda_load(XZipStream& fh,int version = 100);
	virtual void qda_save(XStream& fh);

	void redraw(int x,int y,int z,int mode = 0) const;
	void redraw_rot(int x,int y,int z, float angle,int mode = 0) const;
	void redraw_rot(int x,int y,int z, float angle, const Vect2f& scale,int mode = 0) const;

	void redraw(int x,int y,int z,float scale,int mode = 0) const;
	void draw_mask(int x,int y,int z,unsigned mask_color,int mask_alpha,int mode = 0) const;
	void draw_mask(int x,int y,int z,unsigned mask_color,int mask_alpha,float scale,int mode = 0) const;

	void draw_mask_rot(int x,int y,int z, float angle,unsigned mask_color,int mask_alpha,int mode = 0) const;
	void draw_mask_rot(int x,int y,int z, float angle,unsigned mask_color,int mask_alpha,const Vect2f& scale,int mode = 0) const;

	void draw_contour(int x,int y,unsigned color,int mode = 0) const;
	void draw_contour(int x,int y,unsigned color,float scale,int mode = 0) const;

	bool hit(int x,int y) const;
	bool hit(int x,int y,float scale) const;

	bool put_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);

	bool crop();
	bool crop(int left,int top,int right,int bottom,bool store_offsets = true);
	bool undo_crop();

	bool get_edges_width(int& left,int& top,int& right,int& bottom);
	Vect2i remove_edges();

	bool compress();
	bool uncompress();
	bool is_compressed() const { if(rle_data_) return true; else return false; }

	bool scale(float coeff_x,float coeff_y);

	void set_flag(int fl){ flags_ |= fl; }
	void drop_flag(int fl){ flags_ &= ~fl; }
	bool check_flag(int fl) const { if(flags_ & fl){ return true; } return false; }
	void clear_flags(){ flags_ = 0; }
	int flags() const { return flags_; }

	//! Загружает в память данные ресурса.
	bool load_resource(){ return load(); }
	//! Выгружает из памяти данные ресурса.
	bool free_resource(){ free(); return true; }

	//! Устанавливает имя файла, в котором хранятся данные ресурса.
	void set_resource_file(const char* file_name){ set_file(file_name); }
	//! Возвращает имя файла, в котором хранятся данные ресурса.
	/**
	Если оно не задано, должна возвращаеть NULL.
	*/
	const char* resource_file() const { if(has_file()) return file(); return NULL; }
#ifdef __QD_DEBUG_ENABLE__
	unsigned resource_data_size() const { return data_size(); }
#endif

	//! Возвращает область экрана, занимаемую спрайтом.
	/**
	Координаты области - смещение от центра спрайта.
	В mode задаются повороты спрайта по горизонтали и вертикали
	(GR_FLIP_HORIZONTAL, GR_FLIP_VERTICAL)
	*/
	grScreenRegion screen_region(int mode = 0,float scale = 1.0f) const;

private:
	int format_;
	int flags_;

	Vect2i size_;

	Vect2i picture_size_;
	Vect2i picture_offset_;

	unsigned char* data_;
	class rleBuffer* rle_data_;

	std::string file_;

	friend bool operator == (const qdSprite& sp1,const qdSprite& sp2);
};

#endif /* __QD_SPRITE_H__ */
