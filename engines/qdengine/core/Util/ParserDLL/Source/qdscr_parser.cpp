
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <stdlib.h>
#include <string.h>

#include "qdscr_parser.h"
#include "qdscr_mathexp.h"

#include "qdscr_error.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

qdscrDataBlock* qdscrLoadScript(char* fname);
qdscrDataBlock* qdscrLoadScript(XStream& fh);

int qdscrPreprocessScript(char* fname,int& outsz,char** outbuf);
void qdscrFinitScript(void);

void qdscrPrepareBuffer(char* ptr,int sz);

void qdscrInitStringData(char* ptr,int sz,int mode = 1);
int qdscrGetStringNumber(int index);
void qdscrFreeStringData(void);

int qdscrParseBuffer(char* fname,int sz = 0,int file_flag = 1);
qdscrKeyword* qdscrGetKeyword(char* ptr);

/* --------------------------- DEFINITION SECTION --------------------------- */

int** qdscrStringData = NULL;

char* qdscrOutputDIR = "";
char* scrOutputFILE = NULL;
char* qdscrInputDIR = "";

qdscrDataBlock* qdscrRoot;
int qdscrVerboseFlag = 1;
int qdscrSavePacked = 1;

int qdscrVersion = 1;

void qdscrFreeDataBlock(qdscrDataBlock* p)
{
	delete p;
}

qdscrDataBlock* qdscrParseScript(const char* fname)
{
	int sz;
	char* buf;

	XBuffer name_str;

	name_str.init();
	name_str < qdscrInputDIR < fname;

	if(!qdscrPreprocessScript((char*)name_str,sz,&buf)) return NULL;
	if(!qdscrParseBuffer(buf,sz,0)) return NULL;
	qdscrFinitScript();

	return qdscrRoot;
}

qdscrDataBlock::qdscrDataBlock(void)
{
	ID = 0;
	dataSize = 0;
	dataSize0 = -1;
	dataType = 0;
	flags = 0;

	name = NULL;
	owner = NULL;
}

qdscrDataBlock::qdscrDataBlock(int tp)
{
	ID = 0;
	dataSize = 1;
	dataType = tp;
	flags = 0;

	name = NULL;
	owner = NULL;
}

qdscrDataBlock::~qdscrDataBlock(void)
{
	freeData();

	qdscrDataBlockList::iterator it = nextLevel.begin();
	FOR_EACH(nextLevel,it){
		delete *it;
	}
	nextLevel.clear();

	if(name)
		delete name;
}

void qdscrDataBlock::add(qdscrDataBlock* p)
{
	nextLevel.push_back(p);
	p -> owner = this;
}

void qdscrDataBlock::alloc(int tp,int sz)
{
	dataType = tp;
	dataSize = sz;

	allocData();
}

qdscrDataBlock* qdscrDataBlock::find(int id)
{ 
	qdscrDataBlockList::iterator it = nextLevel.begin();
	FOR_EACH(nextLevel,it){
		if((*it) -> ID == id)
			return *it;
	}

	return NULL; 
}

void qdscrDataBlock::allocData(void)
{
	freeData();
	switch(dataType){
		case QDSCR_DATA_INT:
			i_dataPtr = new int[dataSize];
			break;
		case QDSCR_DATA_DOUBLE:
			d_dataPtr = new double[dataSize];
			break;
		case QDSCR_DATA_CHAR:
			c_dataPtr = new char[dataSize];
			break;
	}
	flags |= QDSCR_ALLOC_DATA;
}

void qdscrDataBlock::freeData(void)
{
	if(flags & QDSCR_ALLOC_DATA){
		switch(dataType){
			case QDSCR_DATA_INT:
				delete i_dataPtr;
				break;
			case QDSCR_DATA_DOUBLE:
				delete d_dataPtr;
				break;
			case QDSCR_DATA_CHAR:
				delete c_dataPtr;
				break;
		}
		flags ^= QDSCR_ALLOC_DATA;
	}
}

void qdscrDataBlock::initName(char* p)
{
	int sz = strlen(p) + 1;
	name = new char[sz];
	strcpy(name,p);
}

void qdscrFinitScript(void)
{
}

#define NEXT_WORD(ptr,i)	while(ptr[i]) if(++i >= sz) break;	\
				if(i >= sz) break;			\
				while(!ptr[i]) if(++i >= sz) break;	\
				if(i >= sz) break;

#define SKIP_SPACE(ptr,i)	while(!ptr[i]) if(++i >= sz) break;	\
				if(i >= sz) break;

int qdscrParseBuffer(char* fname,int sz,int file_flag)
{
	int i,idx,dsz;
	qdscrDataBlock* p,*cp;
	qdscrKeyword* wrd;
	char* ptr;

	if(file_flag){
		XStream fh(fname,XS_IN);

		sz = fh.size();
		ptr = new char[sz];
		fh.read(ptr,sz);
		fh.close();
	}
	else {
		ptr = fname;
	}

	qdscrInitStringData(ptr,sz,0);
	qdscrPrepareBuffer(ptr,sz);

	qdscrRoot = new qdscrDataBlock(QDSCR_DATA_VOID);

	cp = qdscrRoot;

	i = 0;

	while(!ptr[i]) if(++i >= sz) return 1;
	if(i >= sz) return 1;

	while(i < sz){
		wrd = qdscrGetKeyword(ptr + i);
		if(!wrd){
			qdscr_Error("Error: Undefined keyword\nString: %d\n %s",qdscrGetStringNumber(i),ptr + i);
			return 0;
		}

		switch(wrd -> ID){
			case QDSCR_BLOCK_BEG:
				cp = p;
				break;
			case QDSCR_BLOCK_END:
				cp = cp -> owner;
				break;
			default:
				p = new qdscrDataBlock;
				p -> dataType = wrd -> dataType;

				dsz = wrd -> dataSize;
				p -> dataSize0 = wrd -> dataSize;
				if(wrd -> dataType == QDSCR_DATA_INT){
					if(dsz == -1){
						NEXT_WORD(ptr,i);
						i_parseMathExpr(ptr + i,dsz);
					}
					p -> dataSize = dsz;
					p -> allocData();
					for(idx = 0; idx < dsz; idx ++){
						NEXT_WORD(ptr,i);
						i_parseMathExpr(ptr + i,p -> i_dataPtr[idx]);
					}
				}
				if(wrd -> dataType == QDSCR_DATA_DOUBLE){
					if(dsz == -1){
						NEXT_WORD(ptr,i);
						i_parseMathExpr(ptr + i,dsz);
					}
					p -> dataSize = dsz;
					p -> allocData();
					for(idx = 0; idx < dsz; idx ++){
						NEXT_WORD(ptr,i);
						d_parseMathExpr(ptr + i,p -> d_dataPtr[idx]);
					}
				}
				if(wrd -> dataType == QDSCR_DATA_CHAR){
					if(dsz == -1){
						NEXT_WORD(ptr,i);
						if(ptr[i] == '\"'){
							p -> dataSize = strlen(ptr + i);
							p -> allocData();
							strcpy(p -> c_dataPtr,ptr + i + 1);
							p -> c_dataPtr[p -> dataSize - 2] = 0;
						}
						else {
							p -> dataSize = strlen(ptr + i) + 1;
							p -> allocData();
							strcpy(p -> c_dataPtr,ptr + i);
						}
					}
					else {
						p -> dataSize = dsz;
						p -> allocData();
						for(idx = 0; idx < dsz; idx ++){
							NEXT_WORD(ptr,i);
							p -> c_dataPtr[idx] = ptr[i];
						}
					}
				}
				p -> initName(wrd -> name);
				p -> ID = wrd -> ID;
				cp -> add(p);
				break;
		}
		NEXT_WORD(ptr,i);
	}

	qdscrFreeStringData();
	delete ptr;

	return 1;
}

void qdscrDataBlock::dump(XStream& fh,int idx,int mode)
{
	int i;

	if(!mode){
		for(i = 0; i < idx; i ++){
			fh < "\t";
		}
		if(name) fh < name;
		else fh <= ID;
	}

	if(dataType != QDSCR_DATA_CHAR && dataSize0 == -1) fh < " " <= dataSize;

	switch(dataType){
		case QDSCR_DATA_INT:
			for(i = 0; i < dataSize; i ++){
				fh < " " <= i_dataPtr[i];
			}
			break;

		case QDSCR_DATA_DOUBLE:
			for(i = 0; i < dataSize; i ++){
				fh < " " <= d_dataPtr[i];
			}
			break;
		case QDSCR_DATA_CHAR:
			fh < " \"" < c_dataPtr < "\"";
			break;
	}

	if(!nextLevel.empty()){
		if(!mode) fh < " {\r\n";

		qdscrDataBlockList::iterator it = nextLevel.begin();
		FOR_EACH(nextLevel,it){
			(*it) -> dump(fh,idx + 1);
		}

		for(i = 0; i < idx; i ++) fh < "\t";
		if(!mode) fh < "}\r\n";
	}

	fh < "\r\n";
}

void dumpScript(char* fname,qdscrDataBlock* p)
{
	XStream fh(fname,XS_OUT);
	p -> dump(fh,-1,1);
	fh.close();
}

void qdscrDataBlock::saveData(XStream& fh)
{
	if(dataSize){
		switch(dataType){
			case QDSCR_DATA_INT:
				fh.write((char*)i_dataPtr,dataSize * sizeof(int));
				break;
			case QDSCR_DATA_DOUBLE:
				fh.write((char*)d_dataPtr,dataSize * sizeof(double));
				break;
			case QDSCR_DATA_CHAR:
				fh.write(c_dataPtr,dataSize);
				break;
		}
	}
	if(!nextLevel.empty()){
		qdscrDataBlockList::iterator it = nextLevel.begin();
		FOR_EACH(nextLevel,it){
			(*it) -> saveData(fh);
		}
	}
}

void qdscrDataBlock::loadData(XStream& fh)
{
	if(dataSize){
		switch(dataType){
			case QDSCR_DATA_INT:
				fh.read((char*)i_dataPtr,dataSize * sizeof(int));
				break;
			case QDSCR_DATA_DOUBLE:
				fh.read((char*)d_dataPtr,dataSize * sizeof(double));
				break;
			case QDSCR_DATA_CHAR:
				fh.read(c_dataPtr,dataSize);
				break;
		}
	}
	if(!nextLevel.empty()){
		qdscrDataBlockList::iterator it = nextLevel.begin();
		FOR_EACH(nextLevel,it){
			(*it) -> loadData(fh);
		}
	}
}

void qdscrDataBlock::loadData(XBuffer& xb)
{
	if(dataSize){
		switch(dataType){
			case QDSCR_DATA_INT:
				xb.read((char*)i_dataPtr,dataSize * sizeof(int));
				break;
			case QDSCR_DATA_DOUBLE:
				xb.read((char*)d_dataPtr,dataSize * sizeof(double));
				break;
			case QDSCR_DATA_CHAR:
				xb.read(c_dataPtr,dataSize);
				break;
		}
	}
	if(!nextLevel.empty()){
		qdscrDataBlockList::iterator it = nextLevel.begin();
		FOR_EACH(nextLevel,it){
			(*it) -> loadData(xb);
		}
	}
}

void qdscrDataBlock::saveInfo(XStream& fh)
{
	int sz = 0;

	fh < ID < dataSize < dataType < dataSize0;
	if(name) sz = strlen(name);

	fh < sz;
	if(name) fh < name;

	if(!nextLevel.empty()){
		fh < (int)1;

		qdscrDataBlockList::iterator it = nextLevel.begin();
		FOR_EACH(nextLevel,it){
			(*it) -> saveInfo(fh);
		}
	}
	else {
		fh < (int)0;
	}
}

void qdscrDataBlock::loadInfo(XStream& fh)
{
	int i,sz = 0;
	qdscrDataBlock* p;

	fh > ID > dataSize > dataType > sz;

	if(qdscrVersion >= 2)
		fh > dataSize0;

	if(sz){
		name = new char[sz + 1];
		fh.read(name,sz);
		name[sz] = 0;
	}
	fh > sz;

	allocData();

	if(sz){
		for(i = 0; i < sz; i ++){
			p = new qdscrDataBlock;
			p -> loadInfo(fh);
			add(p);
		}
	}
}

void qdscrDataBlock::loadInfo(XBuffer& xb)
{
	int i,sz = 0;
	qdscrDataBlock* p;

	xb > ID > dataSize > dataType;

	if(qdscrVersion >= 2)
		xb > dataSize0;

	xb > sz;

	if(sz){
		name = new char[sz + 1];
		xb.read(name,sz);
		name[sz] = 0;
	}
	xb > sz;

	allocData();

	if(sz){
		for(i = 0; i < sz; i ++){
			p = new qdscrDataBlock;
			p -> loadInfo(xb);
			add(p);
		}
	}
}

void saveScript(char* fname,qdscrDataBlock* p)
{
	int sz0,sz1,type = 2;
	char* p0,*p1;

	XStream fh(fname,XS_OUT);
	p -> saveInfo(fh);
	p -> saveData(fh);
	fh.close();

	if(qdscrSavePacked){
		fh.open(fname,XS_IN);
		sz0 = fh.size();
		p0 = new char[sz0];
		fh.read(p0,sz0);
		fh.close();

		p1 = new char[sz0];
		sz1 = ZIP_compress(p1,sz0,p0,sz0);
		fh.open(fname,XS_OUT);
		fh < type;
		fh.write(p1,sz1);
		fh.close();

		delete p0;
		delete p1;
	}
}

qdscrDataBlock* qdscrLoadScript(char* fname)
{
	XStream fh(fname,XS_IN);
	return qdscrLoadScript(fh);
}

qdscrDataBlock* qdscrLoadScript(XStream& fh)
{
	int sz0,sz1,type;
	char* p0,*p1;

	qdscrDataBlock* p = new qdscrDataBlock;

	fh > type;

	qdscrVersion = type;

	if(!type){
		fh.seek(0,XS_BEG);
		sz1 = fh.size();
		p1 = new char[sz1];
		fh.read(p1,sz1);
	}
	else {
		sz0 = fh.size() - sizeof(int);
		p0 = new char[sz0];
		fh.read(p0,sz0);

		sz1 = ZIP_GetExpandedSize(p0);
		p1 = new char[sz1];
		ZIP_expand(p1,sz1,p0,sz0);

		delete p0;
	}

	XBuffer buf(p1,sz1);

	p -> loadInfo(buf);
	p -> loadData(buf);

	fh.close();
	delete p1;

	return p;
}

void qdscrSetVerbose(int v)
{
	qdscrVerboseFlag = v;
}

void qdscrSetInputDir(char* p)
{
	qdscrInputDIR = strdup(p);
}

void qdscrSetOutputDir(char* p)
{
	qdscrOutputDIR = strdup(p);
}

void qdscrSetOutputFile(char* p)
{
	scrOutputFILE = strdup(p);
}

void qdscrWriteDefine(XStream& fh,char* name,int val,int indent)
{
	int i;
	fh < "#define " < name;
	for(i = 0; i < indent; i ++) fh < "\t";
	fh <= val < "\r\n";
}