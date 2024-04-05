#ifndef __QD_CONDITION_DATA_H__
#define __QD_CONDITION_DATA_H__

#include <vector>

#include "xml_fwd.h"

class XStream;

class qdConditionData
{
public:
	enum data_t {
		DATA_INT,
		DATA_FLOAT,
		DATA_STRING
	};

	qdConditionData();
	qdConditionData(data_t data_type,int data_size = 0);
	qdConditionData(const qdConditionData& data);
	~qdConditionData();

	qdConditionData& operator = (const qdConditionData& data);

	data_t type() const { return type_; }
	void set_type(data_t tp){ type_ = tp; }

	int get_int(int index = 0) const { return reinterpret_cast<const int*>(&*data_.begin())[index]; }
	bool put_int(int value,int index = 0){
		if(static_cast<int>(data_.size()) >= static_cast<int>((index - 1) * sizeof(int))){
			reinterpret_cast<int*>(&*data_.begin())[index] = value;
			return true;
		}

		return false;
	}

	float get_float(int index = 0) const { return reinterpret_cast<const float*>(&*data_.begin())[index]; }
	bool put_float(float value,int index = 0){
		if(static_cast<int>(data_.size()) >= 
			static_cast<int>((index - 1) * sizeof(float))){
			reinterpret_cast<float*>(&*data_.begin())[index] = value;
			return true;
		}

		return false;
	}

	const char* get_string() const { 
		if(!data_.empty()) 
			return &*data_.begin(); 
		else
			return NULL;
	}

	bool put_string(const char* str){
		if(alloc_data(strlen(str) + 1)){
			strcpy(&*data_.begin(),str);
			return true;
		}

		return false;
	}

	bool load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	bool alloc_data(int size);

private:

	data_t type_;
	std::vector<char> data_;
};

#endif /* __QD_CONDITION_DATA_H__ */

