#ifndef __RLE_COMPRESS_H__
#define __RLE_COMPRESS_H__

//#include <vector>

//! Массив, сжатый методом RLE.
class rleBuffer
{
public:
	rleBuffer();
	rleBuffer(const rleBuffer& buf);
	~rleBuffer();

	rleBuffer& operator = (const rleBuffer& buf);

	bool encode(int sx,int sy,const unsigned char* buf);

	bool decode_line(int y,unsigned char* out_buf) const;

	inline bool decode_line(int y,int buffer_id = 0) const {
		if(buffer_id)
			return decode_line(y,&*buffer1_.begin());
		else
			return decode_line(y,&*buffer0_.begin());
	}

	bool decode_pixel(int x,int y,unsigned& pixel);

	static inline const unsigned char* get_buffer(int buffer_id){
		if(buffer_id) return &*buffer1_.begin();
		else return &*buffer0_.begin();
	}

	void resize_buffers();

	unsigned size();
	int line_length();
	int line_header_length(int line_num) const;

	unsigned header_size() const { return header_.size(); }
	unsigned data_size() const { return data_.size(); }

	const char* header_ptr(int y = 0) const { return &*(header_.begin() + header_offset_[y]); }
	const unsigned* data_ptr(int y = 0) const { return &*(data_.begin() + data_offset_[y]); }

	bool save(class XStream& fh);
	bool load(class XStream& fh);
	bool load(class XZipStream& fh);

	bool convert_data(int bits_per_pixel = 16);

private:
	std::vector<unsigned> header_offset_;
	std::vector<unsigned> data_offset_;

	std::vector<char> header_;
	std::vector<unsigned> data_;

	int bits_per_pixel_;

	static std::vector<unsigned char> buffer0_;
	static std::vector<unsigned char> buffer1_;

	friend bool operator == (const rleBuffer& buf1,const rleBuffer& buf2);
};

#endif /* __RLE_COMPRESS_H__ */

