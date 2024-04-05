
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "qdscr_parser.h"
#include "qdscr_define.h"

#include "qdscr_error.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */

struct qdscrString
{
	int startIndex;
	int endIndex;
	int strLen;

	char* data;

	qdscrString(void);
	~qdscrString(void);
};

/* ----------------------------- EXTERN SECTION ----------------------------- */

extern int qdscrVerboseFlag;

/* --------------------------- PROTOTYPE SECTION ---------------------------- */

int qdscrPreprocessScript(char* fname,int& outsz,char** outbuf);
void qdscrPrepareBuffer(char* p,int sz);

void qdscrStripComments(char* ptr,int sz);
void qdscrProcessIncludes(char** ptr,int& sz);

int qdscrProcessDefines(char** ptr,int& sz);
void qdscrPrepareDefines(char* ptr,int sz);
int qdscrReplaceDefines(void);

void qdscrInitStringData(char* ptr,int sz,int mode = 1);
void qdscrFreeStringData(void);

int qdscrGetStringNumber(int index);

int mth_isMathOp(int v);

/* --------------------------- DEFINITION SECTION --------------------------- */

const char* const inc_str =  "#include";
const char* const def_str =  "#define";
const char* const text_str = "#text";
const char* const end_str =  "#end";

int qdscrNumStrings;
qdscrString** qdscrStrList;
qdscrDefineDataList qdscrDefineList;

int bufferSize = 0;

int qdscrPreprocessScript(char* fname,int& outsz,char** outbuf)
{
	int idx,idx1;
	char* buf,*buf1;

	XStream fh(fname,XS_IN);
	int sz = fh.size();
	buf = new char[sz + 2];
	fh.read(buf,sz);
	fh.close();
	buf[sz] = '\r';
	buf[sz + 1] = '\n';
	sz += 2;

	qdscrStripComments(buf,sz);
	qdscrProcessIncludes(&buf,sz);
	if(!qdscrProcessDefines(&buf,sz)) return 0;

	int sz1 = 0;
	int q_flag = 0;
	for(int i = 0; i < qdscrNumStrings; i ++){
		int flag = 0;
		for(idx = 0; idx < qdscrStrList[i] -> strLen; idx ++){
			if(q_flag)
				flag = 1;

			if(qdscrStrList[i] -> data[idx] == '\"')
				q_flag ^= 1;

			if(!flag && qdscrStrList[i] -> data[idx] && !isspace(qdscrStrList[i] -> data[idx]))
				flag = 1;
		}
		if(flag){
			sz1 += qdscrStrList[i] -> strLen;
		}
	}
	buf1 = new char[sz1 + 2];
	idx1 = 0;
	q_flag = 0;
	for(i = 0; i < qdscrNumStrings; i ++){
		int flag = 0;
		for(idx = 0; idx < qdscrStrList[i] -> strLen; idx ++){
			if(q_flag)
				flag = 1;

			if(qdscrStrList[i] -> data[idx] == '\"')
				q_flag ^= 1;

			if(!flag && qdscrStrList[i] -> data[idx] && !isspace(qdscrStrList[i] -> data[idx]))
				flag = 1;
		}
		if(flag){
			memcpy(buf1 + idx1,qdscrStrList[i] -> data,qdscrStrList[i] -> strLen);
			idx1 += qdscrStrList[i] -> strLen;
		}
	}
	qdscrFreeStringData();
	delete buf;

	sz = sz1 + 2;
	buf = buf1;
	buf[sz1] = '\r';
	buf[sz1 + 1] = '\n';

	qdscrInitStringData(buf,sz);

	sz1 = 0;
	q_flag = 0;
	for(i = 0; i < qdscrNumStrings; i ++){
		int flag = 0;
		for(idx = 0; idx < qdscrStrList[i] -> strLen; idx ++){
			if(!flag && q_flag){
				sz1 += qdscrStrList[i] -> strLen;
				flag = 1;
			}

			if(qdscrStrList[i] -> data[idx] == '\"')
				q_flag ^= 1;

			if(!flag && qdscrStrList[i] -> data[idx] && !isspace(qdscrStrList[i] -> data[idx])){
				sz1 += qdscrStrList[i] -> strLen - idx;
				flag = 1;
			}
		}
	}
	idx1 = 0;
	buf1 = new char[sz1];
	q_flag = 0;
	for(i = 0; i < qdscrNumStrings; i ++){
		int flag = 0;
		for(idx = 0; idx < qdscrStrList[i] -> strLen; idx ++){
			if(!flag && q_flag){
				memcpy(buf1 + idx1,qdscrStrList[i] -> data + idx,qdscrStrList[i] -> strLen - idx);
				idx1 += qdscrStrList[i] -> strLen - idx;
				flag = 1;
			}

			if(qdscrStrList[i] -> data[idx] == '\"')
				q_flag ^= 1;

			if(!flag && qdscrStrList[i] -> data[idx] && !isspace(qdscrStrList[i] -> data[idx])){
				memcpy(buf1 + idx1,qdscrStrList[i] -> data + idx,qdscrStrList[i] -> strLen - idx);
				idx1 += qdscrStrList[i] -> strLen - idx;
				flag = 1;
			}
		}
	}
	outbuf[0] = buf1;
	outsz = sz1;

	bufferSize = sz1;

	qdscrFreeStringData();
	delete buf;

	return 1;
}

void qdscrStripComments(char* ptr,int sz)
{
	int index = 0;
	while(index < sz){
		if(ptr[index] == '\"'){
			index ++;
			if(index < sz - 1){
				while(ptr[index] != '\"'){
					index ++;
					if(index >= sz) return;
				}
				index ++;
			}
		}
		else {
			if(ptr[index] == '/'){
				if(index < sz - 1){
					if(ptr[index + 1] == '/'){
						while(ptr[index] != '\n'){
							ptr[index] = ' ';
							index ++;
							if(index >= sz) return;
						}
					}
					else {
						if(ptr[index + 1] == '*'){
							ptr[index + 1] = ' ';
							int flag = 1;
							while(flag){
								if(ptr[index] == '*' && ptr[index + 1] == '/'){
									flag = 0;
								}
								else {
									ptr[index] = ' ';
									index ++;
									if(index >= sz - 1) return;
								}
							}
							ptr[index] = ptr[index + 1] = ' ';
						}
						else
							index ++;
					}
				}
			}
			else
				index ++;
		}
	}
}

void qdscrProcessIncludes(char** buf,int& sz)
{
	char* p,*p1,*tmp,*ptr;
	XStream fh;

	int flag = 1,count = 0,index,index1,inc_len,sz1;

	inc_len = strlen(inc_str);

	ptr = *buf;

	while(flag){
		flag = 0;
		index = 0;
		while(index < sz - inc_len){
			if(ptr[index] == '\"'){
				index ++;
				if(index < sz - 1){
					while(ptr[index] != '\"'){
						index ++;
						if(index >= sz - inc_len) return;
					}
					index ++;
				}
			}
			else {
				if(ptr[index] == '#'){
					if(!strncmp(ptr + index,inc_str,inc_len)){
						flag = 1;
						index1 = index;
						index += inc_len;
						while(ptr[index] != '\"'){
							index ++;
							if(index >= sz) return;
						}
						index ++;
						p = ptr + index;
						while(ptr[index] != '\"'){
							index ++;
							if(index >= sz) return;
						}
						ptr[index] = 0;
						fh.open(p,XS_IN);
						sz1 = fh.size();
						p1 = new char[sz1];
						fh.read(p1,sz1);
						qdscrStripComments(p1,sz1);
						fh.close();

						tmp = new char[sz + sz1 - (index - index1)];
						memcpy(tmp,ptr,index1);
						memcpy(tmp + index1,p1,sz1);
						memcpy(tmp + index1 + sz1,ptr + index,sz - index);

						delete p1;
						delete ptr;

						*buf = tmp;
						ptr = tmp;

						sz = sz + sz1 - (index - index1);
						break;
					}
					else
						index ++;
				}
				else
					index ++;
			}
		}
	}
}

int qdscrProcessDefines(char** ptr,int& sz)
{
	qdscrDefineData* p,*p1;
	qdscrPrepareDefines(*ptr,sz);

	qdscrDefineDataList::iterator it = qdscrDefineList.begin(),it_n;
	FOR_EACH(qdscrDefineList,it){
		p = *it;

		it_n = it;
		it_n ++;

		while(it_n != qdscrDefineList.end()){
			p1 = *it_n;

			if(p1 -> src_length() == p -> src_length() && !strncmp(p -> src_data(),p1 -> src_data(),p -> src_length())){
				p -> set_end_index(p1 -> beg_index());
				break;
			}

			it_n ++;
		}
	}

	qdscrInitStringData(*ptr,sz);
	if(!qdscrReplaceDefines()) return 0;

	it = qdscrDefineList.begin();
	FOR_EACH(qdscrDefineList,it){
		delete *it;
	}
	qdscrDefineList.clear();

	return 1;
}

void qdscrPrepareDefines(char* ptr,int sz)
{
	qdscrDefineData* p;
	int index,def_len,text_len;

	def_len = strlen(def_str);
	text_len = strlen(text_str);

	index = 0;
	while(index < sz - def_len){
		if(ptr[index] == '\"'){
			index ++;
			if(index < sz - 1){
				while(ptr[index] != '\"'){
					index ++;
					if(index >= sz - def_len) return;
				}
				index ++;
			}
		}
		else {
			if(ptr[index] == '#'){
				if(!strncmp(ptr + index,def_str,def_len)){
					p = new qdscrDefineData;
					p -> init(ptr + index,sz - index);
					p -> set_beg_index(index);
					p -> set_end_index(sz);

					if(p -> src_length() && p -> dest_length())
						qdscrDefineList.push_back(p);
				}
				if(!strncmp(ptr + index,text_str,text_len)){
					p = new qdscrDefineData;
					p -> set_flag(qdscrDefineData::TEXT_FLAG);
					p -> init(ptr + index,sz - index);
					p -> set_beg_index(index);
					p -> set_end_index(sz);

					if(p -> src_length() && p -> dest_length())
						qdscrDefineList.push_back(p);
				}
				index ++;
			}
			else
				index ++;
		}
	}
}

int qdscrReplaceDefines(void)
{
	int i,idx,idx1,start_flag,end_flag,define_flag,sz;

	char* ptr,*tmp;

	qdscrDefineData* p;

	idx = 0;
	sz = qdscrDefineList.size();

	qdscrDefineDataList::iterator it = qdscrDefineList.begin();
	FOR_EACH(qdscrDefineList,it){
		p = *it;

		idx ++;

		start_flag = end_flag = 0;
		for(i = 0; i < qdscrNumStrings; i ++){
			if(!start_flag){
				if(p -> beg_index() >= qdscrStrList[i] -> startIndex && p -> beg_index() < qdscrStrList[i] -> endIndex){
					p -> set_beg_index(i);
					start_flag = 1;
				}
			}
			if(!end_flag){
				if(p -> end_index() >= qdscrStrList[i] -> startIndex && p -> end_index() < qdscrStrList[i] -> endIndex){
					p -> set_end_index(i);
					end_flag = 1;
				}
			}
			if(start_flag && end_flag) break;
		}
	}

	for(i = 0; i < qdscrNumStrings; i ++){
		for(idx = 0; idx < qdscrStrList[i] -> strLen; idx ++){
			if(qdscrStrList[i] -> data[idx] == '$'){
				idx1 = idx;
				define_flag = 0;

				while(!isspace(qdscrStrList[i] -> data[idx1]) && !mth_isMathOp(qdscrStrList[i] -> data[idx1]) && qdscrStrList[i] -> data[idx1] != '\"'){
					idx1 ++;
					if(idx1 >= qdscrStrList[i] -> strLen - 1) break;
				}
				idx1 -= idx + 1;
				it = qdscrDefineList.begin();
				FOR_EACH(qdscrDefineList,it){
					p = *it;
					if(p -> beg_index() <= i && p -> end_index() > i && p -> src_length() == idx1){
						if(!strncmp(p -> src_data(),qdscrStrList[i] -> data + idx + 1,p -> src_length())){
							define_flag = 1;
							ptr = new char[qdscrStrList[i] -> strLen + p -> dest_length() - p -> src_length() - 1];
							if(idx)
								memcpy(ptr,qdscrStrList[i] -> data,idx);

							memcpy(ptr + idx,p -> dest_data(),p -> dest_length());
							memcpy(ptr + idx + p -> dest_length(),qdscrStrList[i] -> data + idx + p -> src_length() + 1,qdscrStrList[i] -> strLen - (idx + p -> src_length() + 1));

							delete qdscrStrList[i] -> data;
							qdscrStrList[i] -> data = ptr;
							qdscrStrList[i] -> strLen = qdscrStrList[i] -> strLen + p -> dest_length() - p -> src_length() - 1;
							idx = -1;
							break;
						}
					}
				}
				if(!define_flag){
					tmp = new char[256];
					memset(tmp,0,256);
					if(strlen(qdscrStrList[i] -> data + idx) >= 256)
						memcpy(tmp,qdscrStrList[i] -> data + idx,255);
					else
						memcpy(tmp,qdscrStrList[i] -> data + idx,strlen(qdscrStrList[i] -> data + idx));

					qdscr_Error("Error: Unknown #define: %d\n %s",i,tmp);
					delete tmp;
					return 0;
				}
			}
		}
	}

	return 1;
}


void qdscrInitStringData(char* ptr,int sz,int mode)
{
	int i,num,index0;
	qdscrNumStrings = 1;

	for(i = 0; i < sz; i ++){
		if(ptr[i] == '\n') qdscrNumStrings ++;
	}

	num = 0;
	index0 = 0;
	qdscrStrList = new qdscrString*[qdscrNumStrings];
	for(i = 0; i < qdscrNumStrings; i ++)
		qdscrStrList[i] = new qdscrString;

	for(i = 0; i < sz; i ++){
		if(ptr[i] == '\n'){
			qdscrStrList[num] -> startIndex = index0;
			qdscrStrList[num] -> endIndex = i;
			qdscrStrList[num] -> strLen = i - index0 + 1;

			if(mode){
				qdscrStrList[num] -> data = new char[qdscrStrList[num] -> strLen];
				memcpy(qdscrStrList[num] -> data,ptr + index0,qdscrStrList[num] -> strLen);
			}

			index0 = i + 1;
			num ++;
		}
	}
	if(index0 < sz){
		qdscrStrList[num] -> startIndex = index0;
		qdscrStrList[num] -> endIndex = sz - 1;
		qdscrStrList[num] -> strLen = sz - index0;
		if(mode){
			qdscrStrList[num] -> data = new char[qdscrStrList[num] -> strLen];
			memcpy(qdscrStrList[num] -> data,ptr + index0,qdscrStrList[num] -> strLen);
		}
	}
}

void qdscrFreeStringData(void)
{
	int i;
	for(i = 0; i < qdscrNumStrings; i ++)
		delete qdscrStrList[i];
	delete qdscrStrList;
}

qdscrString::qdscrString(void)
{
	startIndex = endIndex = strLen = 0;
	data = NULL;
}

qdscrString::~qdscrString(void)
{
	if(data)
		delete data;
}

void qdscrPrepareBuffer(char* ptr,int sz)
{
	int index;

	index = 0;
	while(index < sz){
		if(ptr[index] == '\"'){
			int idx0 = index;
			index ++;
			if(index < sz - 1){
				while(ptr[index] != '\"'){
					index ++;
					if(index >= sz) break;
				}
				if(index >= sz) break;
				if(idx0 != index - 1)
					ptr[index] = ptr[idx0] = 0;
				index ++;
			}
		}
		else {
			if(isspace(ptr[index])) ptr[index] = 0;
			index ++;
		}
	}
/*
	for(index = 0; index < sz; index ++){
		if(ptr[index] == '\"') ptr[index] = 0;
	}
*/
}

int qdscrGetStringNumber(int index)
{
	int i;
	for(i = 0; i < qdscrNumStrings; i ++){
		if(index >= qdscrStrList[i] -> startIndex && index < qdscrStrList[i] -> endIndex){
			return i + 1;
		}
	}
	return 0;
}

