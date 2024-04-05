#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__

#include "xml_tag_buffer.h"

namespace xml {

#ifdef _FINAL_VERSION_
#define _XML_ONLY_BINARY_SCRIPT_
#endif

class parser
{
public:
	typedef std::hash_map<std::string,tag> tag_format_t;
	typedef std::stack<tag*> tag_stack_t;

	parser();
	virtual ~parser();

	bool parse_file(const char* fname);

	bool read_binary_script(const char* fname);
	bool write_binary_script(const char* fname) const;
	bool is_script_binary(const char* fname) const;
	bool is_script_binary() const { return binary_script_; }

	const tag& root_tag() const { return root_tag_; }

	void clear();

#ifndef _XML_ONLY_BINARY_SCRIPT_
	virtual void start_element_handler(const char* tag_name,const char** tag_attributes);
	virtual void end_element_handler(const char* tag_name);
	virtual void character_data_handler(const char* data,int data_length);
#endif

	void resize_data_pool(unsigned int pool_sz){ data_pool_.resize(pool_sz); }

	bool register_tag_format(const char* tag_name,const tag& tg){
		tag_format_t::iterator it = tag_format_.find(tag_name);
		if(it != tag_format_.end())
			return false;

		tag_format_.insert(tag_format_t::value_type(tag_name,tg));
		return true;
	}
	const tag* get_tag_format(const char* tag_name) const {
		tag_format_t::const_iterator it = tag_format_.find(tag_name);
		if(it != tag_format_.end())
			return &it -> second;
	
		return NULL;
	}

	int num_tag_formats() const { return tag_format_.size(); }

private:

	tag root_tag_;

	int data_pool_position_;
	std::vector<char> data_pool_;
	std::string data_buffer_;

	bool binary_script_;

	tag_stack_t tag_stack_;
	tag_format_t tag_format_;
	int cur_level_;
	bool skip_mode_;

#ifndef _XML_ONLY_BINARY_SCRIPT_
	bool read_tag_data(tag& tg,const char* data_ptr,int data_length);
#endif
};

}; /* namespace xml */

#endif /* __XML_PARSER_H__ */

