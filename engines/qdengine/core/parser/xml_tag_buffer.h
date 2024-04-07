#ifndef __XML_TAG_BUFFER_H__
#define __XML_TAG_BUFFER_H__

#include "xml_tag.h"

#ifdef _DEBUG
#define XML_ASSERT(a) assert(a)
#else
#define XML_ASSERT(a)
#endif

namespace xml {

class tag_buffer 
{
public:
	tag_buffer(const tag& tg);
	tag_buffer(const char* dp,int len);
	tag_buffer(const tag_buffer& tb);
	~tag_buffer();

	tag_buffer& operator = (const tag_buffer& tb);

	tag_buffer& operator >= (short& var);
	tag_buffer& operator >= (unsigned short& var);
	tag_buffer& operator >= (int& var);
	tag_buffer& operator >= (unsigned int& var);
	tag_buffer& operator >= (float& var);

	tag_buffer& operator > (short& var){
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_SHORT);
		var = *reinterpret_cast<const short*>(data_ + data_offset_);
		data_offset_ += sizeof(short);

		return *this;
	}
	tag_buffer& operator > (unsigned short& var){
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_UNSIGNED_SHORT);
		var = *reinterpret_cast<const unsigned short*>(data_ + data_offset_);
		data_offset_ += sizeof(unsigned short);

		return *this;
	}
	tag_buffer& operator > (int& var){
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_INT);
		var = *reinterpret_cast<const int*>(data_ + data_offset_);
		data_offset_ += sizeof(int);

		return *this;
	}
	tag_buffer& operator > (unsigned int& var){
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_UNSIGNED_INT);
		var = *reinterpret_cast<const unsigned int*>(data_ + data_offset_);
		data_offset_ += sizeof(unsigned int);

		return *this;
	}
	tag_buffer& operator > (float& var){
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_FLOAT);
		var = *reinterpret_cast<const float*>(data_ + data_offset_);
		data_offset_ += sizeof(float);

		return *this;
	}

	short get_short(){
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_SHORT);
		short v;
		*this > v;
		return v;
	}
	unsigned short get_ushort(){
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_UNSIGNED_SHORT);
		unsigned short v;
		*this > v;
		return v;
	}
	int get_int(){
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_INT);
		int v;
		*this > v;
		return v;
	}
	unsigned int get_uint(){
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_UNSIGNED_INT);
		unsigned int v;
		*this > v;
		return v;
	}
	float get_float(){
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_FLOAT);
		float v;
		*this > v;
		return v;
	}

	bool end_of_storage() const { return data_size_ > data_offset_; }
	void reset(){ data_offset_ = 0; }

private:
	int data_size_;
	int data_offset_;

#ifdef _DEBUG
	tag::tag_data_format data_format_;
#endif

	const char* data_;
};

}; /* namespace xml */

#endif /* __XML_TAG_BUFFER_H__ */
