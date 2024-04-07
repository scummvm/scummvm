/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <memory.h>

#include "rle_compress.h"
#include "gr_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

std::vector<unsigned char> rleBuffer::buffer0_(4096);
std::vector<unsigned char> rleBuffer::buffer1_(4096);

bool operator == (const rleBuffer& buf1,const rleBuffer& buf2)
{
	if(!(buf1.header_offset_ == buf2.header_offset_)) return false;
	if(!(buf1.data_offset_ == buf2.data_offset_)) return false;
	if(!(buf1.header_ == buf2.header_)) return false;
	if(!(buf1.data_ == buf2.data_)) return false;

	return true;
}

rleBuffer::rleBuffer() : bits_per_pixel_(32)
{
}

rleBuffer::rleBuffer(const rleBuffer& buf) : header_offset_(buf.header_offset_),
	data_offset_(buf.data_offset_),
	header_(buf.header_),
	data_(buf.data_),
	bits_per_pixel_(buf.bits_per_pixel_)
{
}

rleBuffer::~rleBuffer()
{
	header_offset_.clear();
	data_offset_.clear();
	header_.clear();

	data_.clear();
}

rleBuffer& rleBuffer::operator = (const rleBuffer& buf)
{
	if(this == &buf) return *this;

	header_offset_ = buf.header_offset_;
	data_offset_ = buf.data_offset_;

	header_ = buf.header_;
	data_ = buf.data_;

	bits_per_pixel_ = buf.bits_per_pixel_;

	return *this;
}

bool rleBuffer::encode(int sx,int sy,const unsigned char* buf)
{
	header_offset_.resize(sy);
	data_offset_.resize(sy);

	header_.clear();
	data_.clear();

	data_.reserve(sx * sy);

	const unsigned* buffer = reinterpret_cast<const unsigned*>(buf);

	for(int y = 0; y < sy; y ++){
		int count = 0;

		header_offset_[y] = header_.size();
		data_offset_[y] = data_.size();

		while(count < sx){
			int index = count;
			unsigned pixel = buffer[index ++];

			while(index < sx && index - count < 127 && buffer[index] == pixel)
				index ++;

			if(index - count == 1){
				while(index < sx && index - count < 127 && (buffer[index] != buffer[index - 1] || index > 1 && buffer[index] != buffer[index - 2]))
					index ++;

				while(index < sx && buffer[index] == buffer[index - 1])
					index --;

				header_.push_back(static_cast<char>(count - index));
				for(int i = count; i < index; i ++)
					data_.push_back(buffer[i]);
			} 
			else {
				header_.push_back(static_cast<char>(index - count));
				data_.push_back(pixel);
			}

			count = index;
		}
		buffer += sx;
	}
	std::vector<unsigned>(data_).swap(data_);

	resize_buffers();

	return true;
}

bool rleBuffer::decode_line(int y,unsigned char* out_buf) const
{
	const char* header_ptr = &*(header_.begin() + header_offset_[y]);
	const unsigned* data_ptr = &*(data_.begin() + data_offset_[y]);

	unsigned* out_ptr = reinterpret_cast<unsigned*>(out_buf);

	int size = line_header_length(y);

	int line_len = 0;

	for(int i = 0; i < size; i ++){
		char count = *header_ptr++;
		if(count > 0){
			for(int j = 0; j < count; j ++)
				*out_ptr++ = *data_ptr;
			data_ptr ++;
		}
		else {
			count = -count;
			memcpy(out_ptr,data_ptr,count * sizeof(unsigned));

			out_ptr += count;
			data_ptr += count;
		}

		line_len += abs(count);
	}

	return true;
}

bool rleBuffer::decode_pixel(int x,int y,unsigned& pixel)
{
	const char* header_ptr = &*(header_.begin() + header_offset_[y]);
	const unsigned* data_ptr = &*(data_.begin() + data_offset_[y]);

	int xx = 0;
	char count = *header_ptr++;

	while(xx + abs(count) < x){
		if(count > 0){
			data_ptr ++;
		}
		else {
			count = -count;
			data_ptr += count;
		}
		xx += count;
		count = *header_ptr++;
	}

	if(count > 0){
		pixel = *data_ptr;
	}
	else {
		data_ptr += x - xx;
		pixel = *data_ptr;
	}

	return true;
}

unsigned rleBuffer::size()
{
	return data_.size() * sizeof(unsigned) + data_offset_.size() + header_offset_.size() * sizeof(unsigned) + header_.size();
}

bool rleBuffer::convert_data(int bits_per_pixel)
{
	if(bits_per_pixel_ == bits_per_pixel)
		return true;

	int sz = data_.size();

	switch(bits_per_pixel_){
	case 15:
	case 16:
		if(bits_per_pixel == 24 || bits_per_pixel == 32){
			unsigned short* short_ptr = reinterpret_cast<unsigned short*>(&*data_.begin());

			for(int i = 0; i < sz; i ++){
				short_ptr++;
				*short_ptr++ <<= 8;
			}

			short_ptr = reinterpret_cast<unsigned short*>(&*data_.begin());
			unsigned char* char_ptr = reinterpret_cast<unsigned char*>(&*data_.begin());

			for(int i = 0; i < sz; i ++){
				unsigned r,g,b;

				if(bits_per_pixel_ == 15)
					grDispatcher::split_rgb555u(*short_ptr++,r,g,b);
				else
					grDispatcher::split_rgb565u(*short_ptr++,r,g,b);

				short_ptr++;
				
				char_ptr[0] = b;
				char_ptr[1] = g;
				char_ptr[2] = r;

				char_ptr += 4;
			}
		}
		else {
			unsigned short* short_ptr = reinterpret_cast<unsigned short*>(&*data_.begin());

			for(int i = 0; i < sz; i ++){
				unsigned r,g,b;

				if(bits_per_pixel_ == 15){
					grDispatcher::split_rgb555u(*short_ptr,r,g,b);
					*short_ptr++ = grDispatcher::make_rgb565u(r,g,b);
				}
				else {
					grDispatcher::split_rgb565u(*short_ptr,r,g,b);
					*short_ptr++ = grDispatcher::make_rgb555u(r,g,b);
				}

				short_ptr++;
			}
		}
		break;
	case 24:
	case 32:
		if(bits_per_pixel == 15 || bits_per_pixel == 16){
			unsigned char* src_ptr = reinterpret_cast<unsigned char*>(&*data_.begin());
			unsigned short* dest_ptr = reinterpret_cast<unsigned short*>(&*data_.begin());

			for(int i = 0; i < sz; i ++){
				*dest_ptr++ = (bits_per_pixel == 15) ? grDispatcher::make_rgb555u(src_ptr[2],src_ptr[1],src_ptr[0]) : grDispatcher::make_rgb565u(src_ptr[2],src_ptr[1],src_ptr[0]);
				*dest_ptr++ >>= 8;
				src_ptr += 4;
			}
		}
		break;
	}

	bits_per_pixel_ = bits_per_pixel;

	return true;
}

void rleBuffer::resize_buffers()
{
	unsigned len = line_length() * sizeof(unsigned);

	if(buffer0_.size() < len)
		buffer0_.resize(len);
	if(buffer1_.size() < len)
		buffer1_.resize(len);
}

int rleBuffer::line_length()
{
	if(header_offset_.empty()) return 0;

	int sz = (header_offset_.size() > 1) ? header_offset_[1] : header_.size();

	int len = 0;
	for(int i = 0; i < sz; i ++){
		len += abs(header_[i]);
	}

	return len;
}

int rleBuffer::line_header_length(int line_num) const
{
	if(line_num < header_offset_.size() - 1)
		return header_offset_[line_num + 1] - header_offset_[line_num];
	else
		return header_.size() - header_offset_[line_num];
}

bool rleBuffer::save(class XStream& fh)
{
	fh < header_offset_.size() < data_offset_.size() < header_.size() < data_.size();

	fh.write(&*header_offset_.begin(),header_offset_.size() * sizeof(unsigned));
	fh.write(&*data_offset_.begin(),data_offset_.size() * sizeof(unsigned));
	fh.write(&*header_.begin(),header_.size());
	fh.write(&*data_.begin(),data_.size() * sizeof(unsigned));

	return true;
}

bool rleBuffer::load(XStream& fh)
{
	int sz;
	fh > sz;
	header_offset_.resize(sz);

	fh > sz;
	data_offset_.resize(sz);

	fh > sz;
	header_.resize(sz + 1);
	header_[sz] = 0;

	fh > sz;
	data_.resize(sz);

	fh.read(&*header_offset_.begin(),header_offset_.size() * sizeof(unsigned));
	fh.read(&*data_offset_.begin(),data_offset_.size() * sizeof(unsigned));
	fh.read(&*header_.begin(),header_.size() - 1);
	fh.read(&*data_.begin(),data_.size() * sizeof(unsigned));

	resize_buffers();

	return true;
}

bool rleBuffer::load(XZipStream& fh)
{
	int sz;
	fh > sz;
	header_offset_.resize(sz);

	fh > sz;
	data_offset_.resize(sz);

	fh > sz;
	header_.resize(sz + 1);
	header_[sz] = 0;

	fh > sz;
	data_.resize(sz);

	fh.read(&*header_offset_.begin(),header_offset_.size() * sizeof(unsigned));
	fh.read(&*data_offset_.begin(),data_offset_.size() * sizeof(unsigned));
	fh.read(&*header_.begin(),header_.size() - 1);
	fh.read(&*data_.begin(),data_.size() * sizeof(unsigned));

	resize_buffers();

	return true;
}
