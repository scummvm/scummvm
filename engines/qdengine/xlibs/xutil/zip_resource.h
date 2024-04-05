
#ifndef __ZIP_RESOURCE_H__
#define __ZIP_RESOURCE_H__

class XZIP_FileHeader 
{
	char* fileName;
	unsigned dataOffset;
	unsigned dataSize;

	int extDataSize;
	char* extData;

public:
	void* list;
	XZIP_FileHeader* prev;
	XZIP_FileHeader* next;

	void SetName(char* p);

	unsigned size(void) const { return dataSize; }
	unsigned offset(void) const { return dataOffset; }
	char* data(void) const { return extData; }
	char* name(void) const { return fileName; }

	void save(XStream& fh);

	XZIP_FileHeader(void);
	XZIP_FileHeader(char* fname,unsigned offs,unsigned size,void* ext_ptr,int ext_sz);
	XZIP_FileHeader(XStream& fh);
	~XZIP_FileHeader(void);
};

// XZIP_Resource flags
const XZIP_ENABLE_EXTERNAL_FILES	= 0x01;
const XZIP_ENABLE_ZIP_HEADERS		= 0x02;

class XZIP_Resource 
{
	int flags;

	char* fileName;
	char* idxName;

	xtList<XZIP_FileHeader> fileList;
	XStream file;

	XZIP_FileHeader* find(char* fname);
public:
	int open(char* fname,XStream& fh,int mode = 0);

	void LoadHeaders(void);
	void LoadIndex(void);
	void SaveIndex(void);

	void dump(char* fname);

	XZIP_Resource(char* fname,int fl);
	~XZIP_Resource(void);
};

#endif 