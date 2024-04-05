/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

//#include <fstream>

#ifndef _XML_ONLY_BINARY_SCRIPT_
#include <expat.h>
#endif

#include "xml_parser.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

namespace xml {

#ifndef _XML_ONLY_BINARY_SCRIPT_
static void start_element_handler(void* userData,const XML_Char* name,const XML_Char** atts);
static void end_element_handler(void* userData,const XML_Char* name);
static void character_data_handler(void* userData,const XML_Char* s,int len);
static int unknown_encoding_handler(void* encodingHandlerData,const XML_Char *name,XML_Encoding* info);

static const char* UTF8_convert(const char* input_string,int input_string_length = -1);
#endif

static bool write_tag(XStream& ff,const tag& tg,int depth = 0);
	
}; /* namespace xml */

/* --------------------------- DEFINITION SECTION --------------------------- */

namespace xml {

#ifndef _XML_ONLY_BINARY_SCRIPT_
static const char* UTF8_convert(const char* input_string,int input_string_length)
{
	static std::wstring wstr(1024,0);
	static std::string str(1024,0);

	unsigned int length = MultiByteToWideChar(CP_UTF8,0,input_string,input_string_length,NULL,0);
	if(wstr.length() < length)
		wstr.resize(length,0);

	MultiByteToWideChar(CP_UTF8,0,input_string,input_string_length,&*wstr.begin(),length);

	if(str.length() < length + 1)
		str.resize(length + 1,0);
	str[length] = 0;

	WideCharToMultiByte(CP_ACP,0,wstr.c_str(),length,&*str.begin(),length,NULL,NULL);

	return str.c_str();
}

static int unknown_encoding_handler(void* encodingHandlerData,const XML_Char* name,XML_Encoding* info)
{
	if(!strcmp(name,"WINDOWS-1251")){
		info -> data = NULL;
		info -> convert = NULL;
		info -> release = NULL;

		for(int i = 0; i < 256; i ++){
			char c = i;
			unsigned short cc;

			MultiByteToWideChar(1251,0,&c,1,&cc,1);

			info -> map[i] = cc;
		}

		return 1;
	}

	return 0;
}

static void start_element_handler(void* userData,const XML_Char* name,const XML_Char** atts)
{
	parser* p = static_cast<parser*>(userData);
	p -> start_element_handler(name,atts);
}

static void end_element_handler(void* userData,const XML_Char* name)
{
	parser* p = static_cast<parser*>(userData);
	p -> end_element_handler(name);
}

static void character_data_handler(void* userData,const XML_Char* s,int len)
{
	parser* p = static_cast<parser*>(userData);
	p -> character_data_handler(s,len);
}
#endif

parser::parser() : data_pool_position_(0), data_buffer_(1024,0), cur_level_(0), skip_mode_(false), binary_script_(false)
{
	root_tag_.set_data(&data_pool_);
}

parser::~parser()
{
}

void parser::clear()
{
	root_tag_.clear();
	while(!tag_stack_.empty()) tag_stack_.pop();

	cur_level_ = 0;
	skip_mode_ = false;

	data_pool_.clear();
	std::vector<char>(data_pool_).swap(data_pool_);
}

#ifndef _XML_ONLY_BINARY_SCRIPT_
void parser::start_element_handler(const char* tag_name,const char** tag_attributes)
{
	if(!skip_mode_){
		const tag* fmt = get_tag_format(tag_name);
		if(fmt){
			tag tg(*fmt);
			tg.set_data(&data_pool_);

			int sz = 0;
			while(tag_attributes[sz]) sz++;

			if(sz >= 2){
				for(int i = 0; i < sz; i += 2){
					const tag* afmt = get_tag_format(tag_attributes[i]);
					if(afmt){
						tag att(*afmt);
						att.set_data(&data_pool_);

						read_tag_data(att,tag_attributes[i + 1],strlen(tag_attributes[i + 1]));
						tg.add_subtag(att);
					}
				}
			}

			if(!tag_stack_.empty())
				tag_stack_.push(&tag_stack_.top() -> add_subtag(tg));
		}
		else {
			skip_mode_ = true;
			cur_level_ = 0;
		}

		data_buffer_.clear();
	}
	else
		cur_level_ ++;
}

void parser::end_element_handler(const char* tag_name)
{
	if(!skip_mode_){
		if(!tag_stack_.empty())
			read_tag_data(*tag_stack_.top(),data_buffer_.c_str(),strlen(data_buffer_.c_str()));

		tag_stack_.pop();
	}
	else {
		if(!cur_level_--) skip_mode_ = false;
	}
}

void parser::character_data_handler(const char* data,int data_length)
{
	data_buffer_.append(data,data_length);
}
#endif

bool parser::parse_file(const char* fname)
{
	if(is_script_binary(fname))
		return read_binary_script(fname);

#ifndef _XML_ONLY_BINARY_SCRIPT_
	binary_script_ = false;

	XML_Parser p = XML_ParserCreate(NULL);

	XML_SetUserData(p,this);
	XML_SetElementHandler(p,xml::start_element_handler,xml::end_element_handler);
	XML_SetCharacterDataHandler(p,xml::character_data_handler);

	XML_SetUnknownEncodingHandler(p,unknown_encoding_handler,NULL);

	if(!p) return false;

	XStream ff(fname,XS_IN);

	unsigned int fsize = ff.size();

	void* buf = XML_GetBuffer(p,fsize);
	if(!buf) return false;

	ff.read(static_cast<char*>(buf),fsize);
	ff.close();

	if(data_pool_.size() < fsize/2)
		data_pool_.resize(fsize/2);

	tag_stack_.push(&root_tag_);

	if(XML_ParseBuffer(p,fsize,1) == XML_STATUS_OK){
		XML_ParserFree(p);
		return true;
	}

	XML_Error err_code = XML_GetErrorCode(p);

	XBuffer err_buf;
	err_buf < XML_ErrorString(err_code) < "\nLine: " <= XML_GetCurrentLineNumber(p);

	MessageBox(NULL,err_buf.c_str(),"XML Parser error",MB_OK);
	XML_ParserFree(p);
#endif
	return false;
}

#ifndef _XML_ONLY_BINARY_SCRIPT_
bool parser::read_tag_data(tag& tg,const char* data_ptr,int data_length)
{
	if(tg.data_size() && tg.data_format() != tag::TAG_DATA_VOID){
		if(tg.data_format() == tag::TAG_DATA_STRING){
			const char* str = UTF8_convert(data_ptr,data_length);
			tg.set_data_size(strlen(str) + 1);

			tg.set_data_offset(data_pool_position_);
			unsigned int sz = tg.data_size() * tg.data_element_size();
			if(data_pool_.size() < data_pool_position_ + sz)
				data_pool_.resize(data_pool_position_ + sz);

			char* p = &*(data_pool_.begin() + data_pool_position_);
			strcpy(p,str);

			data_pool_position_ += sz;

			return true;
		}

		tag_buffer buf(data_ptr,data_length);
		if(tg.data_size() == -1){
			int sz;
			buf >= sz;
			tg.set_data_size(sz);
		}

		tg.set_data_offset(data_pool_position_);

		unsigned int sz = tg.data_size() * tg.data_element_size();
		if(data_pool_.size() < data_pool_position_ + sz)
			data_pool_.resize(data_pool_position_ + sz);

		switch(tg.data_format()){
		case tag::TAG_DATA_SHORT: {
			short* p = reinterpret_cast<short*>(&*(data_pool_.begin() + data_pool_position_));
			for(int j = 0; j < tg.data_size(); j ++) buf >= p[j];
			}
			break;
		case tag::TAG_DATA_UNSIGNED_SHORT: {
			unsigned short* p = reinterpret_cast<unsigned short*>(&*(data_pool_.begin() + data_pool_position_));
			for(int j = 0; j < tg.data_size(); j ++) buf >= p[j];
			}
			break;
		case tag::TAG_DATA_INT: {
			int* p = reinterpret_cast<int*>(&*(data_pool_.begin() + data_pool_position_));
			for(int j = 0; j < tg.data_size(); j ++) buf >= p[j];
			}
			break;
		case tag::TAG_DATA_UNSIGNED_INT: {
			unsigned int* p = reinterpret_cast<unsigned int*>(&*(data_pool_.begin() + data_pool_position_));
			for(int j = 0; j < tg.data_size(); j ++) buf >= p[j];
			}
			break;
		case tag::TAG_DATA_FLOAT: {
			float* p = reinterpret_cast<float*>(&*(data_pool_.begin() + data_pool_position_));
			for(int j = 0; j < tg.data_size(); j ++) buf >= p[j];
			}
			break;
		}

		data_pool_position_ += sz;
	}
	return true;
}
#endif

XStream& operator < (XStream& ff,const tag& tg)
{
	int id = tg.ID();
	ff.write(reinterpret_cast<const char*>(&id),sizeof(int));

	int data_format = tg.data_format();
	ff.write(reinterpret_cast<const char*>(&data_format),sizeof(int));

	int data_size = tg.data_size();
	ff.write(reinterpret_cast<const char*>(&data_size),sizeof(int));

	int data_offset = tg.data_offset();
	ff.write(reinterpret_cast<const char*>(&data_offset),sizeof(int));

	int num_subtags = tg.num_subtags();
	ff.write(reinterpret_cast<const char*>(&num_subtags),sizeof(int));

	for(tag::subtag_iterator it = tg.subtags_begin(); it != tg.subtags_end(); ++it)
		ff < *it;

	return ff;
}

XStream& operator > (XStream& ff,tag& tg)
{
	int id = 0;
	ff.read(reinterpret_cast<char*>(&id),sizeof(int));

	int data_format = 0;
	ff.read(reinterpret_cast<char*>(&data_format),sizeof(int));

	int data_size = 0;
	ff.read(reinterpret_cast<char*>(&data_size),sizeof(int));

	int data_offset = 0;
	ff.read(reinterpret_cast<char*>(&data_offset),sizeof(int));

	tg = tag(tag(id,tag::tag_data_format(data_format),data_size,data_offset));

	int num_subtags = 0;
	ff.read(reinterpret_cast<char*>(&num_subtags),sizeof(int));

	for(int i = 0; i < num_subtags; i++){
		tag stg;
		ff > stg;

		tg.add_subtag(stg);
	}

	return ff;
}

bool parser::read_binary_script(const char* fname)
{
	XStream ff(fname, XS_IN);

	binary_script_ = true;

	int v = 0;
	ff.read(reinterpret_cast<char*>(&v),sizeof(int));

	int size = 0;
	ff.read(reinterpret_cast<char*>(&size),sizeof(int));

	if(data_pool_.size() < size)
		data_pool_.resize(size);

	ff.read(&*data_pool_.begin(),size);

	root_tag_.clear();
	ff > root_tag_;

	root_tag_.set_data(&data_pool_);

	ff.close();

	return true;
}

bool parser::write_binary_script(const char* fname) const
{
	XStream ff(fname, XS_OUT);

	int v = 8383;
	ff.write(reinterpret_cast<const char*>(&v),sizeof(int));

	ff.write(reinterpret_cast<const char*>(&data_pool_position_),sizeof(int));
	ff.write(&*data_pool_.begin(),data_pool_position_);

	ff < root_tag_;

	ff.close();

	return true;
}

bool parser::is_script_binary(const char* fname) const
{
	XStream ff(fname, XS_IN);

	int v = 0;
	ff.read(reinterpret_cast<char*>(&v),sizeof(int));

	ff.close();

	if(v == 8383) return true;

	return false;
}

}; /* namespace xml */
