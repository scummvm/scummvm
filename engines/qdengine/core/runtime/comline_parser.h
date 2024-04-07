#ifndef __COMLINE_PARSER_H__
#define __COMLINE_PARSER_H__

class comlineParser
{
public:
	comlineParser();
	~comlineParser();

	bool register_option(const char* name,int id);

	void parse_comline(int argc,char** argv);

	static bool is_option(const char* arg_str){ if(arg_str[0] == '-' || arg_str[0] == '/') return true; return false; }

	bool has_argument(int id) const;
	const char* argument_string(int id) const;

private:

	struct comlineArgument 
	{
		const char* data_;
		int optionID_;

		comlineArgument(const char* data,int id) : data_(data), optionID_(id) { }
		comlineArgument() : data_(NULL), optionID_(-1) { }

		bool operator == (int opt_id) const { return (optionID_ == opt_id); }
	};

	struct comlineOption
	{
		std::string name_;
		int ID_;

		comlineOption(const char* name,int id) : name_(name), ID_(id) { }

		bool operator == (const char* str) const { if(!str) return false; return !stricmp(name_.c_str(),str); }
	};

	typedef std::vector<comlineArgument> arguments_container_t;
	arguments_container_t arguments_;

	typedef std::list<comlineOption> options_container_t;
	options_container_t options_;
};

#endif /* __COMLINE_PARSER_H__ */

