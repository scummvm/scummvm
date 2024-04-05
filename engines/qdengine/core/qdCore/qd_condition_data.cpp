/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <stdlib.h>

#include "xml_tag_buffer.h"
#include "qdscr_parser.h"

#include "qd_condition_data.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdConditionData::qdConditionData() : type_(DATA_STRING)
{
}

qdConditionData::qdConditionData(data_t data_type,int data_size) : type_(data_type)
{
	if(data_size)
		alloc_data(data_size);
}

qdConditionData::qdConditionData(const qdConditionData& data) :	type_(data.type_),
	data_(data.data_)
{
}

qdConditionData::~qdConditionData()
{
}

qdConditionData& qdConditionData::operator = (const qdConditionData& data)
{
	if(this == &data) return *this;

	type_ = data.type_;
	data_ = data.data_;

	return *this;
}

bool qdConditionData::alloc_data(int size)
{
	switch(type_){
	case DATA_INT:
		size *= sizeof(int);
		break;
	case DATA_FLOAT:
		size *= sizeof(float);
		break;
	case DATA_STRING:
		size++;
		break;
	}

	if(data_.size() < size)
		data_.resize(size);

	return true;
}

bool qdConditionData::load_script(const xml::tag* p)
{
	switch(type_){
	case DATA_INT: {
			xml::tag_buffer buf(*p);
			for(int i = 0; i < p -> data_size(); i ++)
				put_int(buf.get_int(),i);
		}
		break;
	case DATA_FLOAT: {
			xml::tag_buffer buf(*p);
			for(int i = 0; i < p -> data_size(); i ++)
				put_float(buf.get_float(),i);
		}
		break;
	case DATA_STRING:
		put_string(p -> data());
		break;
	}

	return true;
}

bool qdConditionData::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";

	switch(type_){
	case DATA_INT:
		fh < "<condition_data_int>" <= data_.size()/sizeof(int);
		for(int i = 0; i < data_.size()/sizeof(int); i ++) fh < " " <= get_int(i);
		fh < "</condition_data_int>\r\n";
		break;
	case DATA_FLOAT:
		fh < "<condition_data_float>" <= data_.size()/sizeof(float);
		for(int i = 0; i < data_.size()/sizeof(float); i ++) fh < " " <= get_float(i);
		fh < "</condition_data_float>\r\n";
		break;
	case DATA_STRING:
		fh < "<condition_data_string>";
		if(!data_.empty())
			fh < qdscr_XML_string(&*data_.begin());
		fh < "</condition_data_string>\r\n";
		break;
	}

	return true;
}

