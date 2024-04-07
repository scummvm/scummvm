/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag_buffer.h"
#include "qdscr_parser.h"

#include "qd_named_object_reference.h"
#include "qd_game_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

int qdNamedObjectReference::objects_counter_ = 0;

qdNamedObjectReference::qdNamedObjectReference()
{
	objects_counter_++;
}

qdNamedObjectReference::qdNamedObjectReference(int levels,const int* types,const char* const* names)
{
	object_types_.reserve(levels);
	object_names_.reserve(levels);

	for(int i = 0; i < num_levels(); i ++){
		object_names_.push_back(names[i]);
		object_types_.push_back(types[i]);
	}

	objects_counter_++;
}

qdNamedObjectReference::qdNamedObjectReference(const qdNamedObjectReference& ref) : object_types_(ref.object_types_),
	object_names_(ref.object_names_)
{
	objects_counter_++;
}

qdNamedObjectReference::qdNamedObjectReference(const qdNamedObject* p)
{
	init(p);

	objects_counter_++;
}

qdNamedObjectReference::~qdNamedObjectReference()
{
}

qdNamedObjectReference& qdNamedObjectReference::operator = (const qdNamedObjectReference& ref)
{
	if(this == &ref) return *this;

	object_types_ = ref.object_types_;
	object_names_ = ref.object_names_;

	return *this;
}

bool qdNamedObjectReference::init(const qdNamedObject* p)
{
	clear();

	int num_levels = 0;

	const qdNamedObject* obj = p;
	while(obj && obj -> named_object_type() != QD_NAMED_OBJECT_DISPATCHER){
#ifdef _QUEST_EDITOR
		obj = obj -> ref_owner();
#else
		obj = obj -> owner();
#endif
		num_levels ++;
	}

	object_types_.reserve(num_levels);
	object_names_.reserve(num_levels);

	for(int i = 0; i < num_levels; i ++){
		obj = p;
		for(int j = 0; j < num_levels - i - 1; j ++){
#ifdef _QUEST_EDITOR
			obj = obj -> ref_owner();
#else
			obj = obj -> owner();
#endif
		}
		if(obj->name()){
			object_names_.push_back(obj -> name());
			object_types_.push_back(obj -> named_object_type());
		}
	}

	return true;
}

void qdNamedObjectReference::load_script(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		xml::tag_buffer buf(*it);
		switch(it -> ID()){
		case QDSCR_SIZE:
			object_types_.reserve(xml::tag_buffer(*it).get_int());
			object_names_.reserve(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_NAME:
			object_names_.push_back(it -> data());
			break;
		case QDSCR_TYPE:
			object_types_.push_back(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_NAMED_OBJECT_TYPES:
			object_types_.resize(it -> data_size());
			object_names_.reserve(it -> data_size());
			for(int i = 0; i < it -> data_size(); i++)
				object_types_[i] = buf.get_int();
			break;
		}
	}
}

bool qdNamedObjectReference::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";

	fh < "<named_object";

	fh < " types=\"" <= num_levels();
	for(int i = 0; i < num_levels(); i ++)
		fh < " " <= object_types_[i];
	fh < "\"";

	fh < ">\r\n";

	for(int j = 0; j < num_levels(); j ++){
		for(int i = 0; i <= indent; i ++) fh < "\t";
		fh < "<name>" < qdscr_XML_string(object_names_[j].c_str()) < "</name>\r\n";
	}

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</named_object>\r\n";

	return true;
}

bool qdNamedObjectReference::load_data(qdSaveStream& fh,int version)
{
	int num_levels = 0;

	fh > num_levels;

	object_types_.resize(num_levels);
	object_names_.resize(num_levels);

	std::string str(0,256);

	for(int i = 0; i < num_levels; i ++){
		int type,name_len;
		fh > type > name_len;
		if(str.size() < name_len + 1) str.resize(name_len + 1);
		fh.read(&*str.begin(),name_len);

		object_types_[i] = type;
		object_names_[i] = str.c_str();
	}

	return true;
}

bool qdNamedObjectReference::save_data(qdSaveStream& fh) const
{
	fh < num_levels();

	for(int i = 0; i < num_levels(); i ++){
		fh < object_types_[i] < strlen(object_names_[i].c_str()) + 1;
		fh.write(object_names_[i].c_str(),strlen(object_names_[i].c_str()) + 1);
//		fh < '\0';
	}

	return true;
}

qdNamedObject* qdNamedObjectReference::object() const
{
	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
		return dp -> get_named_object(this);

	return NULL;
}
