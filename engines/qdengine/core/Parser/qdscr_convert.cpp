/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "app_error_handler.h"
#include "qdscr_parser.h"
#include "qdscr_convert.h"
#include "xml_parser.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */

// Data types...
enum qdscrDataType
{
	QDSCR_DATA_VOID = 0,
	QDSCR_DATA_INT,
	QDSCR_DATA_DOUBLE,
	QDSCR_DATA_CHAR
};

// qdscrDataBlock flags...
#define QDSCR_ALLOC_DATA		0x01

struct qdscrDataBlock;
typedef std::list<qdscrDataBlock*> qdscrDataBlockList;

class XStream;
class XBuffer;

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
	void allocData();
	void freeData();

	void add(qdscrDataBlock* p);
	void dump(XStream& fh,int idx,int mode = 0);

	qdscrDataBlock* find(int id);

	qdscrDataBlock(int tp);
	qdscrDataBlock();
	~qdscrDataBlock();
};

/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

const char* qdscr_XML_string(const char* p);

/* --------------------------- DEFINITION SECTION --------------------------- */

static void qdscr_write_tag(qdscrDataBlock* p,XStream& ff,int depth)
{
	for(int i = 0; i < depth; i ++) ff < "\t";

	if(p -> name)
		ff < "<" < p -> name;

	if(!p -> nextLevel.empty()){
		if(p -> name){
			switch(p -> dataType){
				case QDSCR_DATA_INT:
					ff < " type=\"" <= *p -> i_dataPtr < "\"";
					break;
				case QDSCR_DATA_CHAR:
					if(p -> dataSize0 == -1)
						ff < " name=\"" < qdscr_XML_string(p -> c_dataPtr) < "\"";
					break;
			}
			ff < ">\r\n";
		}

		qdscrDataBlockList::iterator it = p -> nextLevel.begin();
		FOR_EACH(p -> nextLevel,it)
			qdscr_write_tag(*it,ff,depth + 1);

		if(p -> name){
			for(int i = 0; i < depth; i ++) ff < "\t";
			ff < "</" < p -> name < ">\r\n";
		}
	}
	else {
		if(p -> name){
			ff < ">";
			int j;
			switch(p -> dataType){
				case QDSCR_DATA_INT:
					if(p -> dataSize0 == -1)
						ff <= p -> dataSize < " ";
					for(j = 0; j < p -> dataSize; j ++){
						ff <= p -> i_dataPtr[j];
						if(j < p -> dataSize - 1) ff < " ";
					}
					break;
				case QDSCR_DATA_DOUBLE:
					if(p -> dataSize0 == -1)
						ff <= p -> dataSize < " ";
					for(j = 0; j < p -> dataSize; j ++){
						ff <= p -> d_dataPtr[j];
						if(j < p -> dataSize - 1) ff < " ";
					}
					break;
				case QDSCR_DATA_CHAR:
					if(p -> dataSize0 == -1)
						ff < qdscr_XML_string(p -> c_dataPtr);
					break;
			}
			ff < "</" < p -> name < ">\r\n";
		}
	}
}

bool qdscr_convert_to_XML(const char* file_name,const char* new_file_name)
{
	typedef qdscrDataBlock* (*parse_proc)(const char* fname);
	typedef void (*free_block_proc)(qdscrDataBlock* p);

#ifdef _DEBUG
	const char* dll_name = "parser_dbg.dll";
#else
	const char* dll_name = "parser.dll";
#endif

	HMODULE hlib = LoadLibrary(dll_name);
	if(!hlib){ 
		app_errH.show_error(dll_name,appErrorHandler::ERR_FILE_NOT_FOUND);
		return false;
	}

	parse_proc parse_p = (parse_proc)GetProcAddress(hlib,"qdscrParseScript");
	free_block_proc free_p = (free_block_proc)GetProcAddress(hlib,"qdscrFreeDataBlock");

	if(!parse_p || !free_p){
		app_errH.show_error(dll_name,appErrorHandler::ERR_BAD_FILE_FORMAT);
		FreeLibrary(hlib);
		return false;
	}

	qdscrDataBlock* p = (*parse_p)(file_name);
	if(!p){ 
		FreeLibrary(hlib);
		return false;
	}

	XStream fh;

	if(!new_file_name)
		fh.open(qdscr_get_XML_file_name(file_name),XS_OUT);
	else
		fh.open(new_file_name,XS_OUT);

	fh < "<?xml version=\"1.0\" encoding=\"WINDOWS-1251\"?>\r\n<qd_script>\r\n";
	qdscr_write_tag(p,fh,0);
	fh < "</qd_script>\r\n";

	fh.close();

	(*free_p)(p);

	FreeLibrary(hlib);

	return true;
}

bool qdscr_is_XML(const char* file_name)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
   	char ext[_MAX_EXT];

	_splitpath(file_name,drive,dir,fname,ext);

	if(!stricmp(ext,".qml")) return true;

	return false;
}

const char* qdscr_get_XML_file_name(const char* file_name)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
   	char ext[_MAX_EXT];

	static std::string xml_file_name;

	_splitpath(file_name,drive,dir,fname,ext);

	xml_file_name = drive;
	xml_file_name += dir;
	xml_file_name += fname;
	xml_file_name += ".qml";

	return xml_file_name.c_str();
}
