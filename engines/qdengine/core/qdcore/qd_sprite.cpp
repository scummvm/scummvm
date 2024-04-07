/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "gr_dispatcher.h"

#include "qd_sprite.h"
#include "qd_setup.h"

#include "app_error_handler.h"

#include "rle_compress.h"

#include "qd_file_manager.h"

#include "2PassScale.h"
#include "Filters.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

bool operator == (const qdSprite& sp1,const qdSprite& sp2)
{
	if(sp1.size_ == sp2.size_ && sp1.picture_offset_ == sp2.picture_offset_ && sp1.picture_size_ == sp2.picture_size_){
		if(sp1.is_compressed()){
			if(!sp2.is_compressed()) return false;

			if(sp1.rle_data_ && sp2.rle_data_){
				if(*sp1.rle_data_ == *sp2.rle_data_)
					return true;
			}
			else
				return false;
		}
		else {
			if(sp1.data_ && sp2.data_){
				const unsigned char* d1 = sp1.data_;
				const unsigned char* d2 = sp2.data_;
				int sz = sp1.picture_size_.x * sp1.picture_size_.y;

				if(sp1.check_flag(qdSprite::ALPHA_FLAG) && !sp2.check_flag(qdSprite::ALPHA_FLAG)) return false;
				if(sp2.check_flag(qdSprite::ALPHA_FLAG) && !sp1.check_flag(qdSprite::ALPHA_FLAG)) return false;

				switch(sp1.format_){
					case GR_RGB565:
					case GR_ARGB1555:
						sz *= 2;
						if(sp1.check_flag(qdSprite::ALPHA_FLAG)) sz *= 2;
						break;
					case GR_RGB888:
						sz *= 3;
						break;
					case GR_ARGB8888:
						sz *= 4;
						break;
				}

				for(int i = 0; i < sz; i ++)
					if(*d1++ != *d2++) return false;

				return true;
			}
			else
				return false;
		}
	}
	return false;
}

qdSprite::qdSprite() : data_(0),
	rle_data_(0),
	flags_(0)
{
	size_ = picture_size_ = picture_offset_ = Vect2i(0,0);

	format_ = 0;
}

qdSprite::qdSprite(int wid, int hei, int format):
	rle_data_(0),
	flags_(0)
{
	size_ = picture_size_ = Vect2i(wid, hei);
	picture_offset_ = Vect2i(0,0);

	format_ = format;
	
	int bytes_per_pix;
	switch (format)
	{
	case GR_RGB565: 
		bytes_per_pix = 2;
		break;
	case GR_ARGB1555: 
		bytes_per_pix = 2;
		set_flag(ALPHA_FLAG);
		break;
	case GR_RGB888:
		bytes_per_pix = 3;
		break;
	case GR_ARGB8888:
		bytes_per_pix = 4;
		set_flag(ALPHA_FLAG);
		break;
	default:
		bytes_per_pix = 1;
		break;
	};

	data_ = new unsigned char[wid*hei*bytes_per_pix];
}

qdSprite::qdSprite(const qdSprite& spr) : data_(0),
	rle_data_(0),
	flags_(0)
{
	*this = spr;
}

qdSprite::~qdSprite()
{
	free();
}

qdSprite& qdSprite::operator = (const qdSprite& spr)
{
	if(this == &spr) return *this;

	format_ = spr.format_;
	flags_ = spr.flags_;
	size_ = spr.size_;
	picture_size_ = spr.picture_size_;
	picture_offset_ = spr.picture_offset_;

	delete [] data_;
	if(spr.data_){
		int ssx = 2;
		switch(format_){
			case GR_RGB565:
			case GR_ARGB1555:
				if(check_flag(ALPHA_FLAG))
					ssx = 4;
				else
					ssx = 2;
				break;
			case GR_RGB888:
				ssx = 3;
				break;
			case GR_ARGB8888:
				ssx = 4;
				break;
		}
		
		data_ = new unsigned char[picture_size_.x * picture_size_.y * ssx];
		memcpy(data_,spr.data_,picture_size_.x * picture_size_.y * ssx);
	}
	else
		data_ = NULL;

	delete rle_data_;
	if(spr.rle_data_)
		rle_data_ = new rleBuffer(*spr.rle_data_);
	else
		rle_data_ = NULL;

	file_ = spr.file_;

	return *this;
}

void qdSprite::free()
{
	delete [] data_;
	delete rle_data_;

	size_ = picture_size_ = picture_offset_ = Vect2i(0,0);

	format_ = 0;
	data_ = 0;

	rle_data_ = 0;

	drop_flag(ALPHA_FLAG);
}

bool qdSprite::load(const char* fname)
{
	free();

	if(fname)
		set_file(fname);

	int sx,sy,flags,ssx,colors;
	unsigned char header[18];

	XZipStream fh;

	if(!qdFileManager::instance().open_file(fh, file_.c_str()))
		return false;

	int size = fh.size();

	fh.read(header,18);

	if(header[0]){ // Length of Image ID field
		fh.seek(header[0],XS_CUR);
	}

	// ColorMapType. 0 - цветовой таблицы нет. 1 - есть. Остальное не соотв. стандарту.
	// Изображения с цветовой таблицей не обрабатываем.
	if(header[1]){
		app_errH.message_box(file_.c_str(),appErrorHandler::ERR_BAD_FILE_FORMAT);
		return false;
	}

	// ImageType. 2 - truecolor без сжатия, 10 - truecolor со сжатием (RLE).
	if ((header[2] != 2) && (header[2] != 10)) {
		app_errH.message_box(file_.c_str(),appErrorHandler::ERR_BAD_FILE_FORMAT);
		return false;
	}

	sx = picture_size_.x = header[12] + (header[13] << 8); 
	sy = picture_size_.y = header[14] + (header[15] << 8); 

	size_ = picture_size_;

	colors = header[16];
	flags = header[17];

	ssx = sx * colors/8;

	switch(colors/8){
		//! Режим 16 бит не реализован
		//case 2:
		//	format_ = GR_ARGB1555;
		//	break;
		case 3:
			format_ = GR_RGB888;
			break;
		case 4:
			format_ = GR_ARGB8888;
			break;
		// Иначе неверный формат файла
		default:
		{
			app_errH.message_box(file_.c_str(),appErrorHandler::ERR_BAD_FILE_FORMAT);
			return false;
		}
	}

	data_ = new unsigned char[ssx * sy];

	// RLE
	if (10 == header[2]) 
	{
		int cur = 0; // В какую ячейку считываем сейчас
		int i,j;     // Для циклов далее (теор. ускорение)
		unsigned char info, fl, len;
		unsigned char pixel[4];
		unsigned char col_bytes = colors/8;
		while (cur < ssx*sy)
		{
			fh.read(&info, 1);
			fl = (info >> 7) & 0x01;
			len = (info & 0x7F) + 1;
			// Пакет со сжатием
			if (1 == fl) 
			{
				fh.read(&pixel, col_bytes);
				for (i = 0; i < len; i++)
					for (j = 0; j < col_bytes; j++)
					{
						data_[cur] = pixel[j];
						cur++;
					}
			}
			// Пакет без сжатия
			else			
				for (i = 0; i < len; i++)
				{
					fh.read(&pixel, col_bytes);
					for (j = 0; j < col_bytes; j++)
					{
						data_[cur] = pixel[j];
						cur++;
					}
				}
			
		} // while
	}
	// Загрузка изображения без сжатия
	else
		fh.read(data_,ssx * sy);

	// Если 3 и 4 биты ImageDescriptor (fl) нули, то начало изображения - левый нижний угол 
	// экрана и изображение нужно инвертировать. Иначе предполагаем, что изображение корректно. 
	// Xотя не факт, что это так, но иное маловероятно + другие значения не документированы...
	if(!(flags & 0x20)){
		int y;

		unsigned char* str_buf = new unsigned char[ssx];
		unsigned char* str0,*str1;

		str0 = data_;
		str1 = data_ + ssx * (sy - 1);

		for(y = 0; y < sy/2; y ++){
			memcpy(str_buf,str0,ssx);
			memcpy(str0,str1,ssx);
			memcpy(str1,str_buf,ssx);

			str0 += ssx;
			str1 -= ssx;
		}

		delete [] str_buf;
	}

	fh.close();

	if(format_ == GR_ARGB8888){
		set_flag(ALPHA_FLAG);
		for(int i = 0; i < picture_size_.x * picture_size_.y; i ++){
			unsigned short r,g,b,a;
			const unsigned min_color = 8;

			b = data_[i * 4 + 0]; 
			g = data_[i * 4 + 1]; 
			r = data_[i * 4 + 2]; 
			a = data_[i * 4 + 3];

			if(a >= 250 && r < min_color && g < min_color && b < min_color){
				r = g = b = min_color;
			}

			data_[i * 4 + 0] = b * a >> 8; 
			data_[i * 4 + 1] = g * a >> 8; 
			data_[i * 4 + 2] = r * a >> 8;
			data_[i * 4 + 3] = 255 - a;
		}
	}
	else {
		for(int i = 0; i < picture_size_.x * picture_size_.y; i ++){
			const unsigned min_color = 8;					 

			unsigned b = data_[i * 3 + 0];
			unsigned g = data_[i * 3 + 1]; 
			unsigned r = data_[i * 3 + 2]; 

			if((r || g || b) && (r < min_color && g < min_color && b < min_color))
				data_[i * 3 + 0] = data_[i * 3 + 1] = data_[i * 3 + 2] = min_color;
		}
	}

	return true;
}

void qdSprite::save(const char* fname)
{
	if(format_ != GR_RGB888 && format_ != GR_ARGB8888) return;

	const char* out_file = (fname) ? fname : file_.c_str();

	static unsigned char header[18];

	XStream fh(out_file,XS_OUT);

	memset(header,0,18);
	header[2] = 2;

	header[13] = (picture_size_.x >> 8) & 0xFF;
	header[12] = picture_size_.x & 0xFF;

	header[15] = (picture_size_.y >> 8) & 0xFF;
	header[14] = picture_size_.y & 0xFF;

	header[16] = (format_ == GR_ARGB8888) ? 32 : 24;
	header[17] = 0x20;

	fh.write(header,18);

	if(format_ == GR_ARGB8888){
		unsigned char* buf = new unsigned char[picture_size_.x * picture_size_.y * 4];
		unsigned char* p = buf;
		unsigned char* dp = data_;

		for(int i = 0; i < picture_size_.x * picture_size_.y; i ++){
			unsigned short r,g,b,a;
			const unsigned min_color = 8;

			r = dp[0]; 
			g = dp[1]; 
			b = dp[2]; 
			a = 255 - dp[3];

			if(a){
				p[0] = (r << 8) / a; 
				p[1] = (g << 8) / a;
				p[2] = (b << 8) / a;
			}
			else 
				p[0] = p[1] = p[2] = 0;
			
			p[3] = a;

			p += 4;
			dp += 4;
		}
		
		fh.write(buf,picture_size_.x * picture_size_.y * 4);
		delete [] buf;
	}
	else
		fh.write(data_,picture_size_.x * picture_size_.y * 3);

	fh.close();
}

bool qdSprite::compress()
{
	if(is_compressed()) return false;

	switch(format_){
		case GR_RGB565:
		case GR_ARGB1555:
			if(data_){
				rle_data_ = new rleBuffer;

				if(!check_flag(ALPHA_FLAG)){
					unsigned char* p = new unsigned char[picture_size_.x * picture_size_.y * 4];
					unsigned short* dp = reinterpret_cast<unsigned short*>(p);
					unsigned short* sp = reinterpret_cast<unsigned short*>(data_);
					for(int i = 0; i < picture_size_.x * picture_size_.y; i ++){
						*dp++ = *sp++;
						*dp++ = 0;
					}
					rle_data_ -> encode(picture_size_.x,picture_size_.y,p);
					delete [] p;
				}
				else
					rle_data_ -> encode(picture_size_.x,picture_size_.y,data_);

				delete [] data_;
				data_ = 0;

				return true;
			}
			return false;
		case GR_RGB888: 
			if(data_){
				unsigned char* p = new unsigned char[picture_size_.x * picture_size_.y * 4];
				unsigned char* ptr = p;
				unsigned char* data_ptr = data_;
				for(int i = 0; i < picture_size_.x * picture_size_.y; i ++){
					ptr[0] = data_ptr[0];
					ptr[1] = data_ptr[1];
					ptr[2] = data_ptr[2];
					ptr[3] = 0;

					ptr += 4;
					data_ptr += 3;
				}

				rle_data_ = new rleBuffer;
				rle_data_ -> encode(picture_size_.x,picture_size_.y,p);

				delete [] p;
				delete [] data_;
				data_ = 0;

				return true;
			}
			return false;
		case GR_ARGB8888:
			if(data_){
				rle_data_ = new rleBuffer;
				rle_data_ -> encode(picture_size_.x,picture_size_.y,data_);
				set_flag(ALPHA_FLAG);

				delete [] data_;
				data_ = 0;
				return true;
			}
			return false;
	}
	return false;
}

bool qdSprite::uncompress()
{
	if(!is_compressed()) return false;

	switch(format_){
	case GR_RGB565:
	case GR_ARGB1555:
		if(check_flag(ALPHA_FLAG)){
			data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];
			unsigned char* p = data_;
			for(int i = 0; i < picture_size_.y; i ++){
				rle_data_ -> decode_line(i,p);
				p += picture_size_.x * sizeof(unsigned);
			}
		}
		else {
			data_ = new unsigned char[picture_size_.x * picture_size_.y * 2];
			unsigned short* p = reinterpret_cast<unsigned short*>(data_);
			for(int i = 0; i < picture_size_.y; i ++){
				const unsigned short* rle_p = reinterpret_cast<const unsigned short*>(rleBuffer::get_buffer(0));
				rle_data_ -> decode_line(i);

				for(int j = 0; j < picture_size_.x; j ++){
					*p++ = *rle_p++;
					rle_p++;
				}
			}
		}
		break;
	case GR_RGB888: 
		if(!check_flag(ALPHA_FLAG)){
			data_ = new unsigned char[picture_size_.x * picture_size_.y * 3];
			unsigned char* p = data_;
			for(int i = 0; i < picture_size_.y; i ++){
				const unsigned char* rle_p = reinterpret_cast<const unsigned char*>(rleBuffer::get_buffer(0));
				rle_data_ -> decode_line(i);

				for(int j = 0; j < picture_size_.x; j ++){
					p[0] = rle_p[0];
					p[1] = rle_p[1];
					p[2] = rle_p[2];
					p += 3;
					rle_p += 4;
				}
			}
		}
		else {
			data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];
			unsigned char* p = data_;

			for(int i = 0; i < picture_size_.y; i ++){
				rle_data_ -> decode_line(i,p);
				p += picture_size_.x * 4;
			}

			format_ = GR_ARGB8888;
		}
		break;
	case GR_ARGB8888:
		if(!check_flag(ALPHA_FLAG)){
			data_ = new unsigned char[picture_size_.x * picture_size_.y * 3];
			unsigned char* p = data_;
			for(int i = 0; i < picture_size_.y; i ++){
				const unsigned char* rle_p = reinterpret_cast<const unsigned char*>(rleBuffer::get_buffer(0));
				rle_data_ -> decode_line(i);

				for(int j = 0; j < picture_size_.x; j ++){
					p[0] = rle_p[0];
					p[1] = rle_p[1];
					p[2] = rle_p[2];
					p += 3;
					rle_p += 4;
				}
			}
			format_ = GR_RGB888;
		}
		else {
			data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];
			unsigned char* p = data_;

			for(int i = 0; i < picture_size_.y; i ++){
				rle_data_ -> decode_line(i,p);
				p += picture_size_.x * 4;
			}
		}
		break;
	}

	delete rle_data_;
	rle_data_ = 0;

	return true;
}

void qdSprite::redraw(int x,int y,int z,int mode) const
{
	int xx = x - size_x()/2;
	int yy = y - size_y()/2;

	if(mode & GR_FLIP_HORIZONTAL)
		xx += size_.x - picture_offset_.x - picture_size_.x;
	else
		xx += picture_offset_.x;

	if(mode & GR_FLIP_VERTICAL)
		yy += size_.y - picture_offset_.y - picture_size_.y;
	else
		yy += picture_offset_.y;

#ifdef _GR_ENABLE_ZBUFFER
	if(!is_compressed()){
		if(!data_) return;
		if(check_flag(ALPHA_FLAG))
			grDispatcher::instance() -> PutSpr_a_z(xx,yy,z,picture_size_.x,picture_size_.y,data_,mode);
		else 
			grDispatcher::instance() -> PutSpr_z(xx,yy,z,picture_size_.x,picture_size_.y,data_,mode);
	}
	else
		grDispatcher::instance() -> PutSpr_rle_z(xx,yy,z,picture_size_.x,picture_size_.y,rle_data_,mode,check_flag(ALPHA_FLAG));
#else
	if(!is_compressed()){
		if(!data_) return;
		if(check_flag(ALPHA_FLAG))
			grDispatcher::instance() -> PutSpr_a(xx,yy,picture_size_.x,picture_size_.y,data_,mode);
		else 
			grDispatcher::instance() -> PutSpr(xx,yy,picture_size_.x,picture_size_.y,data_,mode);
	}
	else
		grDispatcher::instance() -> PutSpr_rle(xx,yy,picture_size_.x,picture_size_.y,rle_data_,mode,check_flag(ALPHA_FLAG));
#endif
}

void qdSprite::redraw_rot(int x,int y,int z, float angle,int mode) const
{
	int xx = x;
	int yy = y;

	Vect2i delta = picture_offset_ + picture_size_/2 - size_/2;

	if(mode & GR_FLIP_HORIZONTAL)
		delta.x = -delta.x;
	if(mode & GR_FLIP_VERTICAL)
		delta.y = -delta.y;

	if(delta.x || delta.y){
		xx += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
		yy += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
	}

	xx -= picture_size_.x/2;
	yy -= picture_size_.y/2;

	if(!is_compressed()){
		if(!data_) return;
		grDispatcher::instance() -> PutSpr_rot(Vect2i(xx, yy), picture_size_, data_, check_flag(ALPHA_FLAG), mode, angle);
	}
	else
		grDispatcher::instance() -> PutSpr_rle_rot(Vect2i(xx, yy), picture_size_, rle_data_, check_flag(ALPHA_FLAG), mode, angle);
}

void qdSprite::redraw_rot(int x,int y,int z, float angle,const Vect2f& scale,int mode) const
{
	int xx = x;
	int yy = y;

	Vect2i delta = picture_offset_ + picture_size_/2 - size_/2;

	if(mode & GR_FLIP_HORIZONTAL)
		delta.x = -delta.x;
	if(mode & GR_FLIP_VERTICAL)
		delta.y = -delta.y;

	delta.x = round(float(delta.x) * scale.x);
	delta.y = round(float(delta.y) * scale.y);

	if(delta.x || delta.y){
		xx += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
		yy += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
	}

	xx -= round(float(picture_size_.x/2) * scale.x);
	yy -= round(float(picture_size_.y/2) * scale.y);

	if(!is_compressed()){
		if(!data_) return;
		grDispatcher::instance() -> PutSpr_rot(Vect2i(xx, yy), picture_size_, data_, check_flag(ALPHA_FLAG), mode, angle, scale);
	}
	else
		grDispatcher::instance() -> PutSpr_rle_rot(Vect2i(xx, yy), picture_size_, rle_data_, check_flag(ALPHA_FLAG), mode, angle, scale);
}

void qdSprite::redraw(int x,int y,int z,float scale,int mode) const
{
	int xx = x - round(float(size_x()) * scale)/2;
	int yy = y - round(float(size_y()) * scale)/2;

	if(mode & GR_FLIP_HORIZONTAL)
		xx += round(float(size_.x - picture_offset_.x - picture_size_.x) * scale);
	else
		xx += round(float(picture_offset_.x) * scale);

	if(mode & GR_FLIP_VERTICAL)
		yy += round(float(size_.y - picture_offset_.y - picture_size_.y) * scale);
	else
		yy += round(float(picture_offset_.y) * scale);

#ifdef _GR_ENABLE_ZBUFFER
	if(!is_compressed()){
		if(!data_) return;
		if(check_flag(ALPHA_FLAG))
			grDispatcher::instance() -> PutSpr_a_z(xx,yy,z,picture_size_.x,picture_size_.y,data_,mode,scale);
		else 
			grDispatcher::instance() -> PutSpr_z(xx,yy,z,picture_size_.x,picture_size_.y,data_,mode,scale);
	}
	else
		grDispatcher::instance() -> PutSpr_rle_z(xx,yy,z,picture_size_.x,picture_size_.y,rle_data_,mode,scale,check_flag(ALPHA_FLAG));
#else
	if(!is_compressed()){
		if(!data_) return;
		if(check_flag(ALPHA_FLAG))
			grDispatcher::instance() -> PutSpr_a(xx,yy,picture_size_.x,picture_size_.y,data_,mode,scale);
		else 
			grDispatcher::instance() -> PutSpr(xx,yy,picture_size_.x,picture_size_.y,data_,mode,scale);
	}
	else
		grDispatcher::instance() -> PutSpr_rle(xx,yy,picture_size_.x,picture_size_.y,rle_data_,mode,scale,check_flag(ALPHA_FLAG));
#endif
}

void qdSprite::draw_mask(int x,int y,int z,unsigned mask_color,int mask_alpha,int mode) const
{
	int xx = x - size_x()/2;
	int yy = y - size_y()/2;

	if(mode & GR_FLIP_HORIZONTAL)
		xx += size_.x - picture_offset_.x - picture_size_.x;
	else
		xx += picture_offset_.x;

	if(mode & GR_FLIP_VERTICAL)
		yy += size_.y - picture_offset_.y - picture_size_.y;
	else
		yy += picture_offset_.y;

	if(!is_compressed()){
		if(!data_) return;
		if(check_flag(ALPHA_FLAG))
			grDispatcher::instance() -> PutSprMask_a(xx,yy,picture_size_.x,picture_size_.y,data_,mask_color,mask_alpha,mode);
		else 
			grDispatcher::instance() -> PutSprMask(xx,yy,picture_size_.x,picture_size_.y,data_,mask_color,mask_alpha,mode);
	}
	else
		grDispatcher::instance() -> PutSprMask_rle(xx,yy,picture_size_.x,picture_size_.y,rle_data_,mask_color,mask_alpha,mode,check_flag(ALPHA_FLAG));
}

void qdSprite::draw_mask(int x,int y,int z,unsigned mask_color,int mask_alpha,float scale,int mode) const
{
	int xx = x - round(float(size_x()) * scale)/2;
	int yy = y - round(float(size_y()) * scale)/2;

	if(mode & GR_FLIP_HORIZONTAL)
		xx += round(float(size_.x - picture_offset_.x - picture_size_.x) * scale);
	else
		xx += round(float(picture_offset_.x) * scale);

	if(mode & GR_FLIP_VERTICAL)
		yy += round(float(size_.y - picture_offset_.y - picture_size_.y) * scale);
	else
		yy += round(float(picture_offset_.y) * scale);

	if(!is_compressed()){
		if(!data_) return;
		if(check_flag(ALPHA_FLAG))
			grDispatcher::instance() -> PutSprMask_a(xx,yy,picture_size_.x,picture_size_.y,data_,mask_color,mask_alpha,mode,scale);
		else 
			grDispatcher::instance() -> PutSprMask(xx,yy,picture_size_.x,picture_size_.y,data_,mask_color,mask_alpha,mode,scale);
	}
	else
		grDispatcher::instance() -> PutSprMask_rle(xx,yy,picture_size_.x,picture_size_.y,rle_data_,mask_color,mask_alpha,mode,scale,check_flag(ALPHA_FLAG));
}

void qdSprite::draw_mask_rot(int x,int y,int z, float angle,unsigned mask_color,int mask_alpha,int mode) const
{
	int xx = x;
	int yy = y;

	Vect2i delta = picture_offset_ + picture_size_/2 - size_/2;

	if(mode & GR_FLIP_HORIZONTAL)
		delta.x = -delta.x;
	if(mode & GR_FLIP_VERTICAL)
		delta.y = -delta.y;

	if(delta.x || delta.y){
		xx += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
		yy += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
	}

	xx -= picture_size_.x/2;
	yy -= picture_size_.y/2;

	if(!is_compressed()){
		if(!data_) return;
		grDispatcher::instance() -> PutSprMask_rot(Vect2i(xx, yy), picture_size_, data_, check_flag(ALPHA_FLAG), mask_color, mask_alpha, mode, angle);
	}
	else
		grDispatcher::instance() -> PutSprMask_rle_rot(Vect2i(xx, yy), picture_size_, rle_data_, check_flag(ALPHA_FLAG), mask_color, mask_alpha, mode, angle);
}

void qdSprite::draw_mask_rot(int x,int y,int z, float angle,unsigned mask_color,int mask_alpha,const Vect2f& scale,int mode) const
{
	int xx = x;
	int yy = y;

	Vect2i delta = picture_offset_ + picture_size_/2 - size_/2;

	if(mode & GR_FLIP_HORIZONTAL)
		delta.x = -delta.x;
	if(mode & GR_FLIP_VERTICAL)
		delta.y = -delta.y;

	delta.x = round(float(delta.x) * scale.x);
	delta.y = round(float(delta.y) * scale.y);

	if(delta.x || delta.y){
		xx += round(float(delta.x) * cosf(angle) - float(delta.y) * sinf(angle));
		yy += round(float(delta.x) * sinf(angle) + float(delta.y) * cosf(angle));
	}

	xx -= round(float(picture_size_.x/2) * scale.x);
	yy -= round(float(picture_size_.y/2) * scale.y);

	if(!is_compressed()){
		if(!data_) return;
		grDispatcher::instance() -> PutSprMask_rot(Vect2i(xx, yy), picture_size_, data_, check_flag(ALPHA_FLAG), mask_color, mask_alpha, mode, angle, scale);
	}
	else
		grDispatcher::instance() -> PutSprMask_rle_rot(Vect2i(xx, yy), picture_size_, rle_data_, check_flag(ALPHA_FLAG), mask_color, mask_alpha, mode, angle, scale);
}

void qdSprite::draw_contour(int x,int y,unsigned color,int mode) const
{
	int xx = x - size_x()/2;
	int yy = y - size_y()/2;

	if(mode & GR_FLIP_HORIZONTAL)
		xx += size_.x - picture_offset_.x - picture_size_.x;
	else
		xx += picture_offset_.x;

	if(mode & GR_FLIP_VERTICAL)
		yy += size_.y - picture_offset_.y - picture_size_.y;
	else
		yy += picture_offset_.y;

	if(is_compressed()){
		grDispatcher::instance() -> DrawSprContour(xx,yy,picture_size_.x,picture_size_.y,rle_data_,color,mode,check_flag(ALPHA_FLAG));
	}
	else {
		if(!data_) return;
		if(check_flag(ALPHA_FLAG))
			grDispatcher::instance() -> DrawSprContour_a(xx,yy,picture_size_.x,picture_size_.y,data_,color,mode);
		else 
			grDispatcher::instance() -> DrawSprContour(xx,yy,picture_size_.x,picture_size_.y,data_,color,mode);
	}
}

void qdSprite::draw_contour(int x,int y,unsigned color,float scale,int mode) const
{
	int xx = x - round(float(size_x()) * scale)/2;
	int yy = y - round(float(size_y()) * scale)/2;

	if(mode & GR_FLIP_HORIZONTAL)
		xx += round(float(size_.x - picture_offset_.x - picture_size_.x) * scale);
	else
		xx += round(float(picture_offset_.x) * scale);

	if(mode & GR_FLIP_VERTICAL)
		yy += round(float(size_.y - picture_offset_.y - picture_size_.y) * scale);
	else
		yy += round(float(picture_offset_.y) * scale);

	if(!is_compressed()){
		if(check_flag(ALPHA_FLAG))
			grDispatcher::instance() -> DrawSprContour_a(xx,yy,picture_size_.x,picture_size_.y,data_,color,mode,scale);
		else 
			grDispatcher::instance() -> DrawSprContour(xx,yy,picture_size_.x,picture_size_.y,data_,color,mode,scale);
	}
	else
		grDispatcher::instance() -> DrawSprContour(xx,yy,picture_size_.x,picture_size_.y,rle_data_,color,mode,scale,check_flag(ALPHA_FLAG));
}

bool qdSprite::hit(int x,int y) const
{
	x += size_.x/2;
	y += size_.y/2;

	if(x < 0 || y < 0 || x >= size_.x || y >= size_.y) return false;

	if(x >= picture_offset_.x && x < picture_offset_.x + picture_size_.x && y >= picture_offset_.y && y < picture_offset_.y + picture_size_.y){
		x -= picture_offset_.x;
		y -= picture_offset_.y;

		if(!is_compressed()){
			if(!data_) return false;

			switch(format_){
				case GR_RGB565:
				case GR_ARGB1555:
					if(check_flag(ALPHA_FLAG)){
						if(reinterpret_cast<unsigned short*>(data_)[(x + y * picture_size_.x) * 2 + 1] < 240) return true;
					}
					else {
						if(reinterpret_cast<unsigned short*>(data_)[x + y * picture_size_.x]) return true;
					}
					break;
				case GR_RGB888:
					if(data_[(x + y * picture_size_.x) * 3] || data_[(x + y * picture_size_.x) * 3 + 1] || data_[(x + y * picture_size_.x) * 3 + 2]) return true;
					break;
				case GR_ARGB8888:
					if(data_[(x + y * picture_size_.x) * 4 + 3] < 240) return true;
					break;
			}
		}
		else {
			unsigned pixel;
			rle_data_ -> decode_pixel(x,y,pixel);
			if(check_flag(ALPHA_FLAG)){
				switch(format_){
					case GR_RGB565:
					case GR_ARGB1555:
						if(reinterpret_cast<unsigned short*>(&pixel)[1] < 240) return true;
						return false;
					case GR_RGB888:
					case GR_ARGB8888:
						if(reinterpret_cast<unsigned char*>(&pixel)[3] < 240) return true;
						return false;
				}
			}
			else {
				if(pixel)
					return true;
				else
					return false;
			}
		}
	}

	return false;
}

bool qdSprite::hit(int x,int y,float scale) const
{
	x = round(float(x) / scale);
	y = round(float(y) / scale);

	return hit(x,y);
}

bool qdSprite::put_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	x -= picture_offset_.x;
	y -= picture_offset_.y;

	if ((x < 0) || (x >= size_.x) || (y < 0) || (y >= size_.y))
		return false;

	int bytes_per_pix;
	unsigned short word;
	switch (format_)
	{
	case GR_RGB565: 
		bytes_per_pix = 2;
		word = grDispatcher::make_rgb565u(r, g, b);
		data_[bytes_per_pix*(y*size_.x + x) + 0] = unsigned char(word & 0x00FF);
		data_[bytes_per_pix*(y*size_.x + x) + 1] = unsigned char(word >> 8 & 0x00FF);
		break;
	case GR_ARGB1555: 
		bytes_per_pix = 2;
		word = grDispatcher::make_rgb555u(r, g, b);
		data_[bytes_per_pix*(y*size_.x + x) + 0] = unsigned char(word & 0x00FF);
		data_[bytes_per_pix*(y*size_.x + x) + 1] = unsigned char(word >> 8 & 0x00FF);
		break;
	case GR_RGB888:
		bytes_per_pix = 3;
		data_[bytes_per_pix*(y*size_.x + x) + 0] = b;
		data_[bytes_per_pix*(y*size_.x + x) + 1] = g;
		data_[bytes_per_pix*(y*size_.x + x) + 2] = r;
		break;
	case GR_ARGB8888:
		bytes_per_pix = 4;
		data_[bytes_per_pix*(y*size_.x + x) + 0] = b;
		data_[bytes_per_pix*(y*size_.x + x) + 1] = g;
		data_[bytes_per_pix*(y*size_.x + x) + 2] = r;
		break;
	default:
		return false;
		break;
	}

	return true;
}

void qdSprite::qda_load(XStream& fh,int version)
{
	free();

	static char str[MAX_PATH];

	int al_flag,compress_flag,len;
	fh > size_.x > size_.y > picture_size_.x > picture_size_.y > picture_offset_.x > picture_offset_.y > format_ > len;

	str[len] = 0;
	fh.read(str,len);
	set_file(str);

	if(version >= 101){
		fh > flags_ > al_flag > compress_flag;
	}
	else {
		flags_ = 0;
		compress_flag = 0;
		fh > al_flag;
	}

	if(!compress_flag){
		if(version < 102){
			switch(format_){
				case GR_RGB565:
				case GR_ARGB1555:
					if(!al_flag)
						data_ = new unsigned char[picture_size_.x * picture_size_.y * 2];
					else
						data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];

					fh.read(data_,picture_size_.x * picture_size_.y * 2);
					break;
				case GR_RGB888:
					if(!al_flag)
						data_ = new unsigned char[picture_size_.x * picture_size_.y * 3];
					else
						data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];

					fh.read(data_,picture_size_.x * picture_size_.y * 3);
					break;
				case GR_ARGB8888:
					data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];
					fh.read(data_,picture_size_.x * picture_size_.y * 4);
					break;
			}
			if(al_flag){
				unsigned char* alpha_data = new unsigned char[picture_size_.x * picture_size_.y];
				fh.read(alpha_data,picture_size_.x * picture_size_.y);

				switch(format_){
					case GR_RGB565:
					case GR_ARGB1555: {
							unsigned char* dp = data_ + picture_size_.x * picture_size_.y * 4 - 4;
							unsigned char* sp = data_ + picture_size_.x * picture_size_.y * 2 - 2;
							unsigned char* ap = alpha_data + picture_size_.x * picture_size_.y - 1;

							for(int i = 0; i < picture_size_.x * picture_size_.y; i ++){
								dp[0] = sp[0];
								dp[1] = sp[1];
								dp[2] = 0;
								dp[3] = *ap--;

								dp -= 4;
								sp -= 2;
							}
						}
						break;
					case GR_RGB888: {
							unsigned char* dp = data_ + picture_size_.x * picture_size_.y * 4 - 4;
							unsigned char* sp = data_ + picture_size_.x * picture_size_.y * 3 - 3;
							unsigned char* ap = alpha_data + picture_size_.x * picture_size_.y - 1;

							for(int i = 0; i < picture_size_.x * picture_size_.y; i ++){
								dp[0] = sp[0];
								dp[1] = sp[1];
								dp[2] = sp[2];
								dp[3] = *ap--;

								dp -= 4;
								sp -= 3;
							}

							format_ = GR_ARGB8888;
						}
						break;
				}

				set_flag(ALPHA_FLAG);
				delete [] alpha_data;
			}
		}
		else {
			switch(format_){
				case GR_RGB565:
				case GR_ARGB1555:
					if(check_flag(ALPHA_FLAG)){
						data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];
						fh.read(data_,picture_size_.x * picture_size_.y * 4);
					}
					else {
						data_ = new unsigned char[picture_size_.x * picture_size_.y * 2];
						fh.read(data_,picture_size_.x * picture_size_.y * 2);
					}
					break;
				case GR_RGB888:
					data_ = new unsigned char[picture_size_.x * picture_size_.y * 3];
					fh.read(data_,picture_size_.x * picture_size_.y * 3);
					break;
				case GR_ARGB8888:
					data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];
					fh.read(data_,picture_size_.x * picture_size_.y * 4);
					break;
			}
		}
	}
	else {
		rle_data_ = new rleBuffer;
		rle_data_ -> load(fh);
	}
}

void qdSprite::qda_load(XZipStream& fh,int version)
{
	free();

	static char str[MAX_PATH];

	int al_flag,compress_flag,len;
	fh > size_.x > size_.y > picture_size_.x > picture_size_.y > picture_offset_.x > picture_offset_.y > format_ > len;

	str[len] = 0;
	fh.read(str,len);
	set_file(str);

	if(version >= 101){
		fh > flags_ > al_flag > compress_flag;
	}
	else {
		flags_ = 0;
		compress_flag = 0;
		fh > al_flag;
	}

	if(!compress_flag){
		if(version < 102){
			switch(format_){
				case GR_RGB565:
				case GR_ARGB1555:
					if(!al_flag)
						data_ = new unsigned char[picture_size_.x * picture_size_.y * 2];
					else
						data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];

					fh.read(data_,picture_size_.x * picture_size_.y * 2);
					break;
				case GR_RGB888:
					if(!al_flag)
						data_ = new unsigned char[picture_size_.x * picture_size_.y * 3];
					else
						data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];

					fh.read(data_,picture_size_.x * picture_size_.y * 3);
					break;
				case GR_ARGB8888:
					data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];
					fh.read(data_,picture_size_.x * picture_size_.y * 4);
					break;
			}
			if(al_flag){
				unsigned char* alpha_data = new unsigned char[picture_size_.x * picture_size_.y];
				fh.read(alpha_data,picture_size_.x * picture_size_.y);

				switch(format_){
					case GR_RGB565:
					case GR_ARGB1555: {
							unsigned char* dp = data_ + picture_size_.x * picture_size_.y * 4 - 4;
							unsigned char* sp = data_ + picture_size_.x * picture_size_.y * 2 - 2;
							unsigned char* ap = alpha_data + picture_size_.x * picture_size_.y - 1;

							for(int i = 0; i < picture_size_.x * picture_size_.y; i ++){
								dp[0] = sp[0];
								dp[1] = sp[1];
								dp[2] = 0;
								dp[3] = *ap--;

								dp -= 4;
								sp -= 2;
							}
						}
						break;
					case GR_RGB888: {
							unsigned char* dp = data_ + picture_size_.x * picture_size_.y * 4 - 4;
							unsigned char* sp = data_ + picture_size_.x * picture_size_.y * 3 - 3;
							unsigned char* ap = alpha_data + picture_size_.x * picture_size_.y - 1;

							for(int i = 0; i < picture_size_.x * picture_size_.y; i ++){
								dp[0] = sp[0];
								dp[1] = sp[1];
								dp[2] = sp[2];
								dp[3] = *ap--;

								dp -= 4;
								sp -= 3;
							}

							format_ = GR_ARGB8888;
						}
						break;
				}

				set_flag(ALPHA_FLAG);
				delete [] alpha_data;
			}
		}
		else {
			switch(format_){
				case GR_RGB565:
				case GR_ARGB1555:
					if(check_flag(ALPHA_FLAG)){
						data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];
						fh.read(data_,picture_size_.x * picture_size_.y * 4);
					}
					else {
						data_ = new unsigned char[picture_size_.x * picture_size_.y * 2];
						fh.read(data_,picture_size_.x * picture_size_.y * 2);
					}
					break;
				case GR_RGB888:
					data_ = new unsigned char[picture_size_.x * picture_size_.y * 3];
					fh.read(data_,picture_size_.x * picture_size_.y * 3);
					break;
				case GR_ARGB8888:
					data_ = new unsigned char[picture_size_.x * picture_size_.y * 4];
					fh.read(data_,picture_size_.x * picture_size_.y * 4);
					break;
			}
		}
	}
	else {
		rle_data_ = new rleBuffer;
		rle_data_ -> load(fh);
	}
}

void qdSprite::qda_save(XStream& fh)
{
	fh < size_.x < size_.y < picture_size_.x < picture_size_.y < picture_offset_.x < picture_offset_.y < format_ < strlen(file_.c_str());

	fh.write(file_.c_str(),strlen(file_.c_str()));

	fh < flags_;

	if(is_compressed()){
		fh < 0L < 1L;

		rle_data_ -> save(fh);
	}
	else {
		fh < 0L < 0L;

		switch(format_){
			case GR_RGB565:
			case GR_ARGB1555:
				if(check_flag(ALPHA_FLAG))
					fh.write(data_,picture_size_.x * picture_size_.y * 4);
				else
					fh.write(data_,picture_size_.x * picture_size_.y * 2);
				break;
			case GR_RGB888:
				fh.write(data_,picture_size_.x * picture_size_.y * 3);
				break;
			case GR_ARGB8888:
				fh.write(data_,picture_size_.x * picture_size_.y * 4);
				break;
		}
	}
}

bool qdSprite::crop()
{
	int left,top,right,bottom;
	if(!get_edges_width(left,top,right,bottom)) return false;
	return crop(left,top,right,bottom);
}

bool qdSprite::crop(int left,int top,int right,int bottom,bool store_offsets)
{
	int sx = picture_size_.x - left - right;
	int sy = picture_size_.y - top - bottom;

	if(sx == picture_size_.x && sy == picture_size_.y) return true;

	int psz = 1;
	switch(format_){
		case GR_RGB565:
		case GR_ARGB1555:
			psz = (check_flag(ALPHA_FLAG)) ? 4 : 2;
			break;
		case GR_RGB888:
			psz = 3;
			break;
		case GR_ARGB8888:
			psz = 4;
			break;
	}

	int idx1 = 0;
	int idx = left * psz + top * picture_size_.x * psz;

	unsigned char* data_new = new unsigned char[sx * sy * psz];
	for(int y = 0; y < sy; y ++){
		memcpy(data_new + idx1,data_ + idx,sx * psz);
		idx += picture_size_.x * psz;
		idx1 += sx * psz;
	}
	delete [] data_;
	data_ = data_new;

	if(store_offsets){
		picture_offset_.x += left;
		picture_offset_.y += top;
	}
	else {
		size_.x = picture_offset_.x + sx;
		size_.y = picture_offset_.x + sy;
	}

	picture_size_.x = sx;
	picture_size_.y = sy;

	return true;
}

bool qdSprite::undo_crop()
{
	if(!data_) return false;

	if(picture_size_ == size_) return false;

	int psx = 1;
	if(format_ == GR_RGB565 || format_ == GR_ARGB1555)
		psx = (check_flag(ALPHA_FLAG)) ? 4 : 2;
	if(format_ == GR_RGB888){
		psx = 3;
		drop_flag(ALPHA_FLAG);
	}
	if(format_ == GR_ARGB8888)
		psx = 4;

	int i;
	unsigned char* new_data = new unsigned char[size_.x * size_.y * psx];
	memset(new_data,0,size_.x * size_.y * psx);

	if(check_flag(ALPHA_FLAG)){
		unsigned char* p = (format_ == GR_ARGB8888) ? new_data + 3 : new_data + 2;
		for(int i = 0; i < size_.x * size_.y; i ++){
			*p = 255;
			p += 4;
		}
	}

	unsigned char* dp = data_;
	unsigned char* p = new_data + (picture_offset_.x + picture_offset_.y * size_.x) * psx;

	for(i = 0; i < picture_size_.y; i ++){
		memcpy(p,dp,picture_size_.x * psx);

		p += size_.x * psx;
		dp += picture_size_.x * psx;
	}

	delete [] data_;
	data_ = new_data;

	picture_size_ = size_;
	picture_offset_ = Vect2i(0,0);

	return true;
}

bool qdSprite::get_edges_width(int& left,int& top,int& right,int& bottom)
{
	if(!data_) return false;

	left = picture_size_.x - 1;
	top = picture_size_.y - 1;

	right = left;
	bottom = top;

	if(format_ == GR_ARGB1555 || format_ == GR_RGB565){
		if(check_flag(ALPHA_FLAG)){
			int idx = 0;
			unsigned short* data_ptr = reinterpret_cast<unsigned short*>(data_);
			for(int y = 0; y < picture_size_.y; y ++){
				int x = 0;
				while(x < picture_size_.x && data_ptr[(idx + x) * 2 + 1] == 255) x ++;
				if(x < left) left = x;
				idx += picture_size_.x - 1;

				x = 0;
				while(x < picture_size_.x && data_ptr[(idx - x) * 2 + 1] == 255) x ++;
				if(x < right) right = x;
				idx ++;
			}

			idx = 0;
			for(int x = 0; x < picture_size_.x; x ++){
				int y = 0;
				while(y < picture_size_.y && data_ptr[(idx + y * picture_size_.x) * 2 + 1] == 255) y ++;
				if(y < top) top = y;

				y = 0;
				while(y < picture_size_.y && data_ptr[(idx - y * picture_size_.x + (picture_size_.y - 1) * picture_size_.x) * 2 + 1] == 255) y ++;
				if(y < bottom) bottom = y;
				idx ++;
			}
		}
		else {
			int idx = 0;
			unsigned short* data_ptr = reinterpret_cast<unsigned short*>(data_);
			for(int y = 0; y < picture_size_.y; y ++){
				int x = 0;
				while(x < picture_size_.x && !data_ptr[idx + x]) x ++;
				if(x < left) left = x;
				idx += picture_size_.x - 1;

				x = 0;
				while(x < picture_size_.x && !data_ptr[idx - x]) x ++;
				if(x < right) right = x;
				idx ++;
			}

			idx = 0;
			for(int x = 0; x < picture_size_.x; x ++){
				int y = 0;
				while(y < picture_size_.y && !data_ptr[idx + y * picture_size_.x]) y ++;
				if(y < top) top = y;

				y = 0;
				while(y < picture_size_.y && !data_ptr[idx - y * picture_size_.x + (picture_size_.y - 1) * picture_size_.x]) y ++;
				if(y < bottom) bottom = y;
				idx ++;
			}
		}
	}
	if(format_ == GR_RGB888){
		int idx = 0;
		for(int y = 0; y < picture_size_.y; y ++){
			int x = 0;
			while(x < picture_size_.x && !(data_[idx + x * 3 + 0] + data_[idx + x * 3 + 1] + data_[idx + x * 3 + 2])) x ++;
			if(x < left) left = x;
			idx += (picture_size_.x - 1) * 3;

			x = 0;
			while(x < picture_size_.x && !(data_[idx - x * 3 + 0] + data_[idx - x * 3 + 1] + data_[idx - x * 3 + 2])) x ++;
			if(x < right) right = x;
			idx += 3;
		}

		idx = 0;
		for(int x = 0; x < picture_size_.x; x ++){
			int y = 0;
			while(y < picture_size_.y && !(data_[idx + y * picture_size_.x * 3 + 0] + data_[idx + y * picture_size_.x * 3 + 1] + data_[idx + y * picture_size_.x * 3 + 2])) y ++;
			if(y < top) top = y;

			y = 0;
			while(y < picture_size_.y && !(data_[idx - y * picture_size_.x * 3 + (picture_size_.y - 1) * picture_size_.x * 3 + 0] + data_[idx - y * picture_size_.x * 3 + (picture_size_.y - 1) * picture_size_.x * 3 + 1] + data_[idx - y * picture_size_.x * 3 + (picture_size_.y - 1) * picture_size_.x * 3 + 2])) y ++;
			if(y < bottom) bottom = y;
			idx += 3;
		}
	}
	if(format_ == GR_ARGB8888){
		int idx = 0;
		for(int y = 0; y < picture_size_.y; y ++){
			int x = 0;
			while(x < picture_size_.x && data_[idx + x * 4 + 3] == 255) x ++;
			if(x < left) left = x;
			idx += (picture_size_.x - 1) * 4;

			x = 0;
			while(x < picture_size_.x && data_[idx - x * 4 + 3] == 255) x ++;
			if(x < right) right = x;
			idx += 4;
		}

		idx = 0;
		for(int x = 0; x < picture_size_.x; x ++){
			int y = 0;
			while(y < picture_size_.y && data_[idx + y * picture_size_.x * 4 + 3] == 255) y ++;
			if(y < top) top = y;

			y = 0;
			while(y < picture_size_.y && data_[idx - y * picture_size_.x * 4 + (picture_size_.y - 1) * picture_size_.x * 4 + 3] == 255) y ++;
			if(y < bottom) bottom = y;
			idx += 4;
		}
	}

	if(left + right >= size_.x){
		left = 0;
		right = size_.x - 1;
	}
	if(top + bottom >= size_.y){
		top = 0;
		bottom = size_.y - 1;
	}

	return true;
}

unsigned qdSprite::data_size() const
{
	if(!is_compressed()){
		unsigned sz = picture_size_.x * picture_size_.y;

		switch(format_){
			case GR_RGB565:
			case GR_ARGB1555:
				sz *= 2;
				if(check_flag(ALPHA_FLAG)) sz *= 2;
				break;
			case GR_RGB888:
				sz *= 3;
				break;
			case GR_ARGB8888:
				sz *= 4;
				break;
		}
		return sz;
	}
	else
		return rle_data_ -> size();
}

bool qdSprite::scale(float coeff_x,float coeff_y)
{
	static scl::C2PassScale<scl::CBilinearFilter> scale_engine;
	static std::vector<unsigned char> temp_buffer(300 * 400 * 4,0);

	bool compress_flag = false;

	int fmt = format_;

	if(is_compressed()){
		uncompress();
		compress_flag = true;
	}

	undo_crop();

	int sx = round(float(picture_size_.x) * coeff_x);
	int sy = round(float(picture_size_.y) * coeff_y);

	unsigned char* src_data = data_; 

	if(format_ == GR_RGB888){
		if(temp_buffer.size() < picture_size_.x * picture_size_.y * 4)
			temp_buffer.resize(picture_size_.x * picture_size_.y * 4);

		src_data = &*temp_buffer.begin();
		memset(src_data,0,picture_size_.x * picture_size_.y * 4);

		unsigned char* p = src_data;
		unsigned char* dp = data_;

		for(int i = 0; i < picture_size_.x * picture_size_.y; i ++){
			p[0] = dp[0];
			p[1] = dp[1];
			p[2] = dp[2];
			p[3] = 0;

			p += 4;
			dp += 3;
		}
	}

	unsigned char* dest_data = new unsigned char[sx * sy * 4];

	scale_engine.Scale(reinterpret_cast<COLORREF*>(src_data),picture_size_.x,picture_size_.y,reinterpret_cast<COLORREF*>(dest_data),sx,sy);

	delete [] data_;

	if(format_ == GR_RGB888){
		data_ = new unsigned char[sx * sy * 3];

		unsigned char* p = dest_data;
		unsigned char* dp = data_;

		for(int i = 0; i < sx * sy; i ++){
			dp[0] = p[0];
			dp[1] = p[1];
			dp[2] = p[2];

			p += 4;
			dp += 3;
		}

		delete [] dest_data;
	}
	else {
		data_ = dest_data;
	}

	picture_size_.x = sx;
	picture_size_.y = sy;

	size_.x = round(float(size_.x) * coeff_x);
	size_.y = round(float(size_.y) * coeff_y);

	picture_offset_.x = round(float(picture_offset_.x) * coeff_x);
	picture_offset_.y = round(float(picture_offset_.y) * coeff_y);

	crop();

	if(compress_flag)
		if(!compress()) return false;

	return true;
}

Vect2i qdSprite::remove_edges()
{
	int left,top,right,bottom;
	get_edges_width(left,top,right,bottom);
	crop(left,top,right,bottom,false);

	return Vect2i(left,top);
}

grScreenRegion qdSprite::screen_region(int mode,float scale) const
{
	int x,y;

	if(mode & GR_FLIP_HORIZONTAL)
		x = round(float(size_.x/2 - picture_offset_.x - picture_size_.x/2) * scale);
	else
		x = round(float(picture_offset_.x + picture_size_.x/2 - size_.x/2) * scale);

	if(mode & GR_FLIP_VERTICAL)
		y = round(float(size_.y/2 - picture_offset_.y - picture_size_.y/2) * scale);
	else
		y = round(float(picture_offset_.y + picture_size_.y/2 - size_.y/2) * scale);

	int sx = round(float(picture_size_.x) * scale) + 4;
	int sy = round(float(picture_size_.y) * scale) + 4;

	return grScreenRegion(x,y,sx,sy);
}
