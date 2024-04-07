#include "qd_precomp.h"
#include "qd_font_info.h"
#include "qd_file_manager.h"

qdFontInfo::qdFontInfo(const qdFontInfo& fi) : qdNamedObject(fi),
	type_ (fi.type()), font_file_name_ (fi.font_file_name())
{		
	if (NULL != fi.font())
	{
		font_ = new grFont;
		*font_ = *fi.font();
	}
	else
		font_ = NULL;
}


qdFontInfo::~qdFontInfo()
{
	delete font_;
}

qdFontInfo& qdFontInfo::operator = (const qdFontInfo& fi)
{
	if (this == &fi) return *this;

	*static_cast<qdNamedObject*>(this) = fi;

	type_ = fi.type();
	font_file_name_ = fi.font_file_name();

	delete font_;
	
	if (NULL != fi.font())
	{
		font_ = new grFont;
		*font_ = *fi.font();
	}
	else
		font_ = NULL;

	return *this;
}


bool qdFontInfo::load_script(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_TYPE:
			set_type(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_FILE:
			set_font_file_name(it -> data());
			break;
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		}
	}

	return true;
}

bool qdFontInfo::save_script(class XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<font_info type=\"" <= type_ < "\"";
		
	if(!font_file_name_.empty())
		fh < " file=\"" < qdscr_XML_string(font_file_name_.c_str()) < "\"";

	if(name())
		fh < " name=\"" < qdscr_XML_string(name()) < "\"";

	fh < "/>\r\n";

	return true;
}

bool qdFontInfo::load_font()
{
	grFont* buf_font = new grFont;

	bool load_fl = true; // По умолчанию загрузка прошла успешно
	XZipStream fh;
	if(qdFileManager::instance().open_file(fh,font_file_name(),false)){
		// Грузим альфу шрифта из .tga
		if(buf_font -> load_alpha(fh)){
			// Меняем расширение с .tga на .idx
			char drive[_MAX_DRIVE];
   			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];

			_splitpath(font_file_name(), drive, dir, fname, NULL);
			std::string idx_fname = "";
			idx_fname += drive;
			idx_fname += dir;
			idx_fname += fname;
			idx_fname = idx_fname + ".idx";

			// Открываем .idx и грузим индекс
			XZipStream fh;
			if(qdFileManager::instance().open_file(fh,idx_fname.c_str(),false)){
				if(!buf_font -> load_index(fh)) 
					load_fl = false;
			}
			else load_fl = false;
		}
		else load_fl = false;
	}
	else load_fl = false;

	if (!load_fl)
	{
		delete buf_font;
		return false;
	}
	font_ = buf_font;
	return true;
}
