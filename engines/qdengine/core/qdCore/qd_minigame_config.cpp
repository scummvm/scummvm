/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qdscr_parser.h"
#include "xml_tag_buffer.h"

#include "qd_setup.h"
#include "qd_minigame_config.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdMinigameConfigParameter::qdMinigameConfigParameter() : data_type_(PRM_DATA_STRING)
{
	data_count_ = 1;
	is_data_valid_ = true;
}

qdMinigameConfigParameter::~qdMinigameConfigParameter()
{
}

bool qdMinigameConfigParameter::load_script(const xml::tag* p)
{
	set_data_string(p -> data());

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_NAME:
				set_name(it -> data());
				break;
//			case QDSCR_COMMENT:
//				set_comment(it -> data());
//				break;
//			case QDSCR_TYPE:
//				set_data_type(data_type_t(xml::tag_buffer(*it).get_int()));
//				break;
			case QDSCR_SIZE:
				set_data_count(xml::tag_buffer(*it).get_int());
				break;
		}
	}

	return true;
}

bool qdMinigameConfigParameter::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<minigame_config_prm";

	fh < " name=\"" < qdscr_XML_string(name()) < "\"";
//	fh < " type=\"" <= int(data_type_) < "\"";

//	if(!comment_.empty())
//		fh < " comment=\"" < qdscr_XML_string(comment()) < "\"";

	if(data_count_ > 1)
		fh < " size=\"" <= data_count_ < "\"";

	fh < ">";

	fh < qdscr_XML_string(data_string());

	fh < "</minigame_config_prm>\r\n";
	
	return true;
}

bool qdMinigameConfigParameter::validate_data()
{
	is_data_valid_ = true;
	return true;
}

bool qdMinigameConfigParameter::load_ini(const char* ini_file,const char* ini_section)
{
	set_name(ini_section);
	char* str = getIniKey(ini_file,ini_section,"type");
	if(strlen(str)){
		if(!stricmp(str,"string"))
			set_data_type(PRM_DATA_STRING);
		else if(!stricmp(str,"file"))
			set_data_type(PRM_DATA_FILE);
		else if(!stricmp(str,"object"))
			set_data_type(PRM_DATA_OBJECT);
	}

	str = getIniKey(ini_file,ini_section,"count");
	if(strlen(str))
		set_data_count(atoi(str));

	str = getIniKey(ini_file,ini_section,"value");
	if(strlen(str)){
		set_data_string(str);
		int cnt = 0;

	}

	str = getIniKey(ini_file,ini_section,"comment");
	if(strlen(str))
		set_comment(str);

	return true;
}
