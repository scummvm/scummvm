
#ifndef __QDSCR_PARSER_H__
#define __QDSCR_PARSER_H__

#include "qdscr_keywords.h"

// qdscrDataBlock flags...
#define QDSCR_ALLOC_DATA		0x01

struct qdscrDataBlock;
typedef std::list<qdscrDataBlock*> qdscrDataBlockList;

struct qdscrDataBlock
{
	int ID;
	int flags;
	int dataSize;
	int dataType;

	int dataSize0;
	char* name;

	union {
		int* i_dataPtr;
		double* d_dataPtr;
		char* c_dataPtr;
	};

	qdscrDataBlock* owner;
	qdscrDataBlockList nextLevel;

	void saveInfo(XStream& fh);
	void saveData(XStream& fh);
	void loadInfo(XStream& fh);
	void loadData(XStream& fh);

	void loadInfo(XBuffer& fh);
	void loadData(XBuffer& fh);

	void initName(char* p);

	void alloc(int tp,int sz);
	void allocData(void);
	void freeData(void);

	void add(qdscrDataBlock* p);
	void dump(XStream& fh,int idx,int mode = 0);

	qdscrDataBlock* find(int id);

	qdscrDataBlock(int tp);
	qdscrDataBlock(void);
	~qdscrDataBlock(void);
};

extern "C" {
__declspec(dllexport) qdscrDataBlock* __cdecl qdscrParseScript(const char* fname);
__declspec(dllexport) void __cdecl qdscrFreeDataBlock(qdscrDataBlock* p);
};

#endif /* __QDSCR_PARSER_H__ */
