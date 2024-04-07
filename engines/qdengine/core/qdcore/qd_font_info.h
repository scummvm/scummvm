#ifndef __QD_FONT_INFO_H__
#define __QD_FONT_INFO_H__

#include "xml_fwd.h"
#include "xml_tag.h"
#include "xml_tag_buffer.h"
#include "qdscr_parser.h"

#include "gr_font.h"
#include "qd_named_object.h"

#include "qd_object_list_container.h"


//! Шрифт
class qdFontInfo : public qdNamedObject
{
public:
	qdFontInfo() : type_(0), font_(NULL) {}
	explicit qdFontInfo(int tp) : type_(tp), font_(NULL) {}
	// Копирующий конструктор
	qdFontInfo(const qdFontInfo& fi);
	~qdFontInfo();

	int named_object_type() const { return QD_NAMED_OBJECT_FONT_INFO; }

	bool operator == (const qdFontInfo& fi) const
	{
		return (fi.type() == type_) && (fi.font_file_name_ == font_file_name_);
	}
	bool operator == (int t) const
	{
		return (t == type_);
	}
	qdFontInfo& operator = (const qdFontInfo& fi);

	int type() const { return type_; }
	void set_type(int tp){ type_ = tp; }

	void set_font_file_name(const char* fname){ font_file_name_ = fname; }
	const char* font_file_name() const { return font_file_name_.c_str(); }

	const grFont* font() const { return font_; }
		
	bool load_script(const xml::tag* p);
	bool save_script(class XStream& fh,int indent = 0) const;

	//! Грузит шрифт из файлов: font_file_name_ (хранит путь к *.tga части)
	//! и файла аналогичного font_file_name_, но с расш-ем. *.idx (индексная часть)
	bool load_font();

private:

	//! Тип шрифта.
	int type_;	
	//! Файл шрифта (*.tga).
	std::string font_file_name_;
	//! Сам шрифт
	grFont* font_;
};

typedef std::list<qdFontInfo*> qdFontInfoList;


#endif // __QD_FONT_INFO_H__