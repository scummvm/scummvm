/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <ctype.h>
#include <stdlib.h>

#include "qdscr_define.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

const char* const def_str =  "#define";
const char* const text_str = "#text";
const char* const end_str =  "#end";

qdscrDefineData::qdscrDefineData(void) : src_data_(0), dest_data_(0),
beg_index_(0), end_index_(0), flags_(0),
src_length_(0), dest_length_(0)
{
}

qdscrDefineData::~qdscrDefineData(void)
{
	delete src_data_;
	delete dest_data_;
}

bool qdscrDefineData::init(char* ptr,int sz)
{
	if(!(flags_ & TEXT_FLAG)){
		int index0 = strlen(def_str);
		while(isspace(ptr[index0])){
			index0 ++;
			if(index0 >= sz) break;
		}
		if(index0 >= sz) return false;

		int index1 = index0;
		while(!isspace(ptr[index1])){
			index1 ++;
			if(index1 >= sz) break;
		}
		if(index1 >= sz) return false;

		src_length_ = index1 - index0;
		src_data_ = new char[src_length_];
		memcpy(src_data_,ptr + index0,src_length_);

		index0 = index1;

		int next_flag = 1;
		while(next_flag){
			next_flag = 0;
			while(ptr[index1] != '\n'){
				index1 ++;
				if(index1 >= sz) break;
			}
			if(index1 >= sz) return false;

			for(int index2 = index1; index2 > index0; index2 --){
				if(!isspace(ptr[index2])){
					if(ptr[index2] == '\\'){
						ptr[index2] = ' ';
						next_flag = 1;
						index1 ++;
					}
					break;
				}
			}
		}

		while(isspace(ptr[index0])){
			index0 ++;
			if(index0 >= sz) break;
		}
		if(index0 >= sz) return false;

		dest_length_ = index1 - index0 + 1;
		dest_data_ = new char[dest_length_];
		memcpy(dest_data_,ptr + index0,dest_length_);
		memset(ptr,' ',index1);
	}
	else {
		int index0 = strlen(text_str);
		while(isspace(ptr[index0])){
			index0 ++;
			if(index0 >= sz) break;
		}
		if(index0 >= sz) return false;

		int index1 = index0;
		while(!isspace(ptr[index1])){
			index1 ++;
			if(index1 >= sz) break;
		}
		if(index1 >= sz) return false;

		src_length_ = index1 - index0;
		src_data_ = new char[src_length_];
		memcpy(src_data_,ptr + index0,src_length_);

		while(ptr[index1] != '\n'){
			index1 ++;
			if(index1 >= sz) break;
		}
		if(++index1 >= sz) return false;

		index0 = index1;

		int next_flag = 1;
		while(next_flag){
			while(ptr[index1] != '\n'){
				if(++index1 >= sz) return false;
			}
			index1 += 1;
			if(index1 >= sz) return false;

			if(ptr[index1] == '#' && !strncmp(ptr + index1,end_str,strlen(end_str))){
				memset(ptr + index1,' ',strlen(end_str));
				next_flag = 0;

				index1 --;

				while(ptr[index1] == '\n' || ptr[index1] == '\r')
					ptr[index1 --] = ' ';

				index1 ++;
			}
		}

		if(ptr[index0] != '\"'){
			dest_length_ = index1 - index0 + 2;
			dest_data_ = new char[dest_length_];
			dest_data_[0] = '\"';
			memcpy(dest_data_ + 1,ptr + index0,dest_length_ - 1);
			dest_data_[dest_length_ - 1] = '\"';
		}
		else {
			dest_length_ = index1 - index0 + 1;
			dest_data_ = new char[dest_length_];
			memcpy(dest_data_,ptr + index0,dest_length_);
		}
		memset(ptr,' ',index1);
	}

	return true;
}
