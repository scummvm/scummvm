/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "comline_parser.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

comlineParser::comlineParser()
{
}

comlineParser::~comlineParser()
{
}

bool comlineParser::register_option(const char* name,int id)
{
	options_container_t::const_iterator it = std::find(options_.begin(),options_.end(),name);
	if(it != options_.end()) return false;

	options_.push_back(comlineOption(name,id));
	return true;
}

void comlineParser::parse_comline(int argc,char** argv)
{
	arguments_.clear();
	arguments_.reserve(argc);

	for(int i = 1; i < argc; i++){
		if(is_option(argv[i])){
			options_container_t::const_iterator it = std::find(options_.begin(),options_.end(),argv[i] + 1);
			int id = (it == options_.end()) ? -1 : it -> ID_;

			if(i < argc - 1 && !is_option(argv[i + 1])){
				arguments_.push_back(comlineArgument(argv[i + 1],id));
				i++;
			}
			else
				arguments_.push_back(comlineArgument(NULL,id));
		}
		else
			arguments_.push_back(comlineArgument(argv[i],-1));
	}
}

bool comlineParser::has_argument(int id) const
{
	arguments_container_t::const_iterator it = std::find(arguments_.begin(),arguments_.end(),id);
	return (it != arguments_.end());
}

const char* comlineParser::argument_string(int id) const
{
	arguments_container_t::const_iterator it = std::find(arguments_.begin(),arguments_.end(),id);
	if(it != arguments_.end())
		return it -> data_;

	return NULL;
}
