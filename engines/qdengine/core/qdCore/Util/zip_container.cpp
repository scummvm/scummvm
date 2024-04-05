
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "zip_container.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

zipContainer::zipContainer() : stream_(0)
{
	index_offset_ = index_size_ = 0;
}

zipContainer::~zipContainer()
{
	close_container();
}

bool zipContainer::open_container(const char* file_name,bool load_idx)
{
	close_container();

	if(!stream_.open(file_name,XS_IN))
		return false;

	stream_file_name_ = file_name;

	if(load_idx){
		long idx_offset = index_offset_;
		long idx_size = index_size_;

		if(find_index()){
			if(!file_map_.empty() && idx_offset == index_offset_ && idx_size == index_size_){
				stream_.seek(0,XS_BEG);
				return true;
			}

			int tm = xclock();

			file_map_.clear();

			char* buf = new char[index_size_];
			XBuffer idx_buf(buf, index_size_);

			stream_.seek(index_offset_,XS_BEG);
			stream_.read(buf,index_size_);

			char fname[MAX_PATH];

			while(!idx_buf.end()){
				const long header_signature = 0x02014b50L;

				// File header:
				// header signature			4 bytes
				// version made by			2 bytes 
				// version needed to extract		2 bytes 
				// general purpose bit flag		2 bytes 
				// compression method			2 bytes 
				// last mod file time			2 bytes 
				// last mod file date			2 bytes 
				// crc-32				4 bytes 
				// compressed size			4 bytes 
				// uncompressed size			4 bytes 
				// file name length			2 bytes 
				// extra field length			2 bytes 
				// file comment length			2 bytes 
				// disk number start			2 bytes 
				// internal file attributes		2 bytes 
				// external file attributes		4 bytes 
				// relative offset of local header	4 bytes 
				// file name				(variable size) 
				// extra field				(variable size) 
				// file comment				(variable size) 

				long sig;
				idx_buf > sig;
				if(sig == header_signature){
					idx_buf.set(16,XB_CUR);
					long compressed_size,uncompressed_size,relative_offset_local_header; 
					unsigned short filename_length,extra_field_length,file_comment_length; 

					idx_buf > compressed_size > uncompressed_size; 
					idx_buf > filename_length > extra_field_length > file_comment_length;

					idx_buf.set(8,XB_CUR);

					idx_buf > relative_offset_local_header;

					idx_buf.read(fname,filename_length);
					fname[filename_length] = 0;

					for(int i = 0; i < filename_length; i++){
						if(fname[i] == '/') fname[i] = '\\';
					}

					idx_buf.set(extra_field_length,XB_CUR);
					idx_buf.set(file_comment_length,XB_CUR);

					stream_.seek(relative_offset_local_header + 28,XS_BEG);
					stream_ > extra_field_length;

					file_map_.insert(file_map_t::value_type(convert_file_name(fname),file_info(relative_offset_local_header + 30 + filename_length + extra_field_length,uncompressed_size)));
				}
			}

			appLog::default_log() << "zipContainer::open_container() time " << xclock() - tm << "ms\r\n";
			stream_.seek(0,XS_BEG);
		}
		else
			return false;

#ifdef _ZIP_CONTAINER_DEBUG_
		dump_info();
#endif		
	}

	return true;
}

bool zipContainer::close_container()
{
	stream_.close();
	return true;
}

bool zipContainer::open_file(const char* file_name,XStream& fh)
{
	if(stream_.isOpen()){
		if(const file_info* fp = find_file(convert_file_name(file_name))){
//			if(app_io::is_file_exist(stream_file_name_.c_str())){
			if(app_io::is_file_exist(stream_.GetFileName())){
//			if(true){
				fh.open(&stream_,fp -> offset,fp -> size);
				return true;
			}
		}
	}

	return false;
}

const zipContainer::file_info* zipContainer::find_file(const char* file_name) const
{
	file_map_t::const_iterator it = file_map_.find(file_name);
	if(it != file_map_.end())
		return &it -> second;
	
	return NULL;
}

const char* zipContainer::convert_file_name(const char* file_name)
{
	static XBuffer conv_buf(1024);

	conv_buf.init();

	int sz = strlen(file_name);
	for(int i = 0; i < sz; i ++){
		unsigned char chr = file_name[i];
		if(chr >= 128){
			if(chr >= 192 && chr <= 223)
				chr += 32;

			if(chr == 168)
				chr = 184;
		}
		else {
			if(chr >= 65 && chr <= 90)
				chr += 32;
		}

		conv_buf < chr;
	}

	conv_buf < '\0';

	return conv_buf.c_str();
}

bool zipContainer::save_index(XStream& fh) const
{
	int sz = file_map_.size();
	fh < sz < index_offset_ < index_size_;

	for(file_map_t::const_iterator it = file_map_.begin(); it != file_map_.end(); ++it){
		int sz = it -> first.size();
		fh < sz < it -> first.c_str() < '\0' < it -> second.offset < it -> second.size;
	}

	return true;
}

bool zipContainer::load_index(XStream& fh)
{
	int sz;
	fh > sz > index_offset_ > index_size_;

	file_map_.clear();

	char file_name[MAX_PATH];
	file_info inf(0,0);

	for(int i = 0; i < sz; i++){
		int str_sz;
		fh > str_sz;

		assert(str_sz < MAX_PATH);

		fh.read(file_name,str_sz + 1);

		fh > inf.offset > inf.size;

		file_map_.insert(file_map_t::value_type(file_name,inf));
	}
	
	return true;
}

bool zipContainer::find_index()
{
	const int buf_sz = 1024;
	char buf[buf_sz];

	stream_.seek(0,XS_END);

	while(stream_.tell() >= buf_sz){
		stream_.seek(-buf_sz,XS_CUR);
		stream_.read(buf,buf_sz);

		const unsigned long idx_signature = 0x06054b50L;

		for(int i = 0; i < buf_sz - sizeof(long) * 3; i++){
			if(*((unsigned long*)(buf + i)) == idx_signature){
				XBuffer xbuf(buf + i + sizeof(long) + sizeof(unsigned short) * 4,sizeof(long) * 2);
				xbuf > index_size_ > index_offset_;
				return true;
			}
		}
	}

	return false;
}

#ifdef _ZIP_CONTAINER_DEBUG_
void zipContainer::dump_info()
{
	static int count = 0;

	XBuffer fname;
	fname < "container" <= count++ < ".lst";

	XStream fh(fname.c_str(),XS_OUT);

	for(file_map_t::const_iterator it = file_map_.begin(); it != file_map_.end(); ++it){
		fh < it -> first.c_str() < " " <= it -> second.offset < " " <= it -> second.size < "\r\n";
	}

	fh.close();
}
#endif	
