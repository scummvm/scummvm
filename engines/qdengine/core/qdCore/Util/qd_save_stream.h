#ifndef __QD_SAVE_STREAM_H__
#define __QD_SAVE_STREAM_H__

//#define __QD_SAVE_DEBUG__

#ifdef __QD_SAVE_DEBUG__

#include "app_log_file.h"

class qdSaveStream
{
public:
	qdSaveStream(int err = 1);
	qdSaveStream(const char* name, unsigned flags,int err = 1);
	~qdSaveStream();

	int open(const char* name, unsigned f = XS_IN);
	void close();

	unsigned long read(void* buf, unsigned long len);
	unsigned long write(const void* buf, unsigned long len);

	template<class T> qdSaveStream& operator < (T value){
		stream_ < value;
		log_stream_ << log_record_index_++ << "\t\t" << value << "\r\n";
		return *this;
	}

	template<class T> qdSaveStream& operator > (T& value){
		stream_ > value;
		log_stream_ << log_record_index_++ << "\t\t" << value << "\r\n";
		return *this;
	}

private:

	XStream stream_;
	appLogFile log_stream_;

	int log_record_index_;

	void open_log(const char* fname,int flags);
};

#else
	class XStream;
	typedef XStream qdSaveStream;
#endif // __QD_SAVE_DEBUG__

#endif // __QD_SAVE_STREAM_H__
