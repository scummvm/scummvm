#ifndef __XPRM_ARCHIVE_H__
#define __XPRM_ARCHIVE_H__

#include <vector>
#include "Handle.h"
#include "Serialization\Serialization.h"

class MultiIArchive;
class MultiOArchive;

class XPrmOArchive : public Archive
{
friend MultiOArchive;
public:
	XPrmOArchive(const char* fname, bool disableInvalidChars = true);
	~XPrmOArchive();

	void open(const char* fname); 
	bool close();  // true if there were changes, so file was updated

	bool isText() const{ return true; }
	bool isOutput() const { return true; }
	bool isInput() const { return false; }

	// To simulate sub-blocks
	bool openBlock(const char* name, const char* nameAlt) { return true; }
	void closeBlock() {}

	bool openStructInternal(void* object, int size, const char* name, const char* nameAlt, const char* typeName, bool polymorphic);
    void closeStruct(const char* name);

protected:
    bool processBinary(MemoryBlock& buffer, const char* name, const char* nameAlt);
	bool processEnum(int& value, const EnumDescriptor& descriptor, const char* name, const char* nameAlt);
    bool processBitVector(int& flags, const EnumDescriptor& descriptor, const char* name, const char* nameAlt);

    bool processValue(char& value, const char* name, const char* nameAlt);
    bool processValue(signed char& value, const char* name, const char* nameAlt);
    bool processValue(signed short& value, const char* name, const char* nameAlt);
    bool processValue(signed int& value, const char* name, const char* nameAlt);
    bool processValue(signed long& value, const char* name, const char* nameAlt);
    bool processValue(unsigned char& value, const char* name, const char* nameAlt);
    bool processValue(unsigned short& value, const char* name, const char* nameAlt);
    bool processValue(unsigned int& value, const char* name, const char* nameAlt);
    bool processValue(unsigned long& value, const char* name, const char* nameAlt);
    bool processValue(float& value, const char* name, const char* nameAlt);
    bool processValue(double& value, const char* name, const char* nameAlt);
	bool processValue(std::wstring& t, const char* name, const char* nameAlt);
    bool processValue(ComboListString& t, const char* name, const char* nameAlt);
    bool processValue(string& str, const char* name, const char* nameAlt);
    bool processValue(bool& value, const char* name, const char* nameAlt);

protected:
	bool openNode(const char* name);
    void closeNode(const char* name);

    bool openContainer(void* array, int& number, const char* name, const char* nameAlt, const char* typeName, const char* elementTypeName, int elementSize, bool readOnly);
    void closeContainer(const char* name);

    int openPointer(void*& object, const char* name, const char* nameAlt, const char* baseName, const char* typeName, const char* typeNameAlt);
    void closePointer(const char* name, const char* baseName, const char* derivedName);

private:
	XBuffer buffer_;
	string offset_;
	string fileName_;
	char disabledChars_[256];

	XStream binaryStream_;
    
    vector<bool> isContainer_;
    vector<int> containerSize_;

	///////////////////////////////////

	bool inContainer() {
		if(isContainer_.empty())
			return false;
		else
			return isContainer_.back();
	}
	void saveString(const char* value) {
		buffer_ < value;
	}
	void saveStringEnclosed(const char* value);

	void openBracket();
	void closeBracket();
};


class XPrmIArchive : public Archive
{
friend MultiIArchive;
public:
	XPrmIArchive(const char* fname = 0);
	~XPrmIArchive();

	bool isOutput() const { return false; }
	bool isInput() const { return true; }

	bool open(const char* fname, int blockSize = 0);  // true if file exists
	bool close();
	bool findSection(const char* sectionName);

	void setVersion(int version) { version_ = version; } // Для сложной конверсии: вручную записывать, выставлять и кастить архив к XPrmIArchive
	int version() const { return version_; }

	unsigned int crc();

	// To simulate sub-blocks
	bool openBlock(const char* name, const char* nameAlt) { return true; }
	void closeBlock() {}

	bool isText() const { return true; }

    bool openStructInternal(void* object, int size, const char* name, const char* nameAlt, const char* typeName, bool polymorphic);
    void closeStruct(const char* name);

protected:
    bool processBinary(MemoryBlock&, const char* name, const char* nameAlt);

	bool processEnum(int& value, const EnumDescriptor& descriptor, const char* name, const char* nameAlt);
	bool processBitVector(int& flags, const EnumDescriptor& descriptor, const char* name, const char* nameAlt);

    bool openContainer(void* array, int& number, const char* name, const char* nameAlt, const char* typeName, const char* elementTypeName, int elementSize, bool readOnly);
    void closeContainer(const char* name);

	int openPointer(void*& object, const char* name, const char* nameAlt, const char* baseName, const char* typeName, const char* typeNameAlt);
    void closePointer(const char* name, const char* typeName, const char* derivedName);

    bool openNode(const char* name);
    void closeNode(const char* name);

    bool processValue(char& value, const char* name, const char* nameAlt); 
    bool processValue(signed char& value, const char* name, const char* nameAlt); 
    bool processValue(signed short& value, const char* name, const char* nameAlt); 
    bool processValue(signed int& value, const char* name, const char* nameAlt); 
    bool processValue(signed long& value, const char* name, const char* nameAlt); 

    bool processValue(unsigned char& value, const char* name, const char* nameAlt); 
    bool processValue(unsigned short& value, const char* name, const char* nameAlt); 
    bool processValue(unsigned int& value, const char* name, const char* nameAlt); 
    bool processValue(unsigned long& value, const char* name, const char* nameAlt); 

    bool processValue(float& value, const char* name, const char* nameAlt);
    bool processValue(double& value, const char* name, const char* nameAlt);
    bool processValue(ComboListString& value, const char* name, const char* nameAlt);
    bool processValue(std::string& value, const char* name, const char* nameAlt);
    bool processValue(std::wstring& value, const char* name, const char* nameAlt);
    bool processValue(bool& value, const char* name, const char* nameAlt);

private:
	string fileName_;
	XBuffer buffer_;
	char replaced_symbol;
	int putTokenOffset_;
	int version_;
	XStream binaryStream_;

    struct ParserState {
        bool isContainer;
        int  blockStart;
		ParserState(bool _isContainer, int _blockStart) : isContainer(_isContainer), blockStart(_blockStart) {}
    };

    vector<ParserState> parserStack_;

	/////////////////////////////////////
	const char* getToken();
	void releaseToken();
	void putToken();
	void skipValue(int openCounter = 0);

	void passString(const char* value);
	bool loadString(string& value); // false if zero string should be loaded
	int line() const;

	void openBracket(bool isContainer);
	void closeBracket();
	bool isContainer() { return parserStack_.empty() ? false : parserStack_.back().isContainer; }
};

#endif //__XPRM_ARCHIVE_H__
