#include "StdAfx.h"

#include "XMath\xmath.h"
#include "XMath\Mat4f.h"
#include "Serialization\XPrmArchive.h"
#include "Serialization\EnumDescriptor.h"
#include "Dictionary.h"
#include "crc.h"
#include "FileUtils\FileUtils.h"
#include "UnicodeConverter.h"

///////////////////////////////////////////////////////////////////////////////////////
//			String Util
///////////////////////////////////////////////////////////////////////////////////////
inline string& expand_spec_chars(string& s)
{
	replaceSubString(s, "\\", "\\\\");
	replaceSubString(s, "\n", "\\n");
	replaceSubString(s, "\r", "\\r");
	replaceSubString(s, "\a", "\\a");
	replaceSubString(s, "\t", "\\t");
	replaceSubString(s, "\v", "\\v");
	replaceSubString(s, "\"", "\\\"");
	return s;
}

inline string& collapse_spec_chars(string& s)
{
	int pos = 0;
	while(1){
		pos = s.find("\\", pos);
		if(pos >= s.size() - 1)
			break;
		char* dest;
		switch(s[pos + 1]){
			case '\\':
				dest = "\\";
				break;
			case 'n':
				dest = "\n";
				break;
			case 'r':
				dest = "\r";
				break;
			case 'a':
				dest = "\a";
				break;
			case 't':
				dest = "\t";
				break;
			case 'v':
				dest = "\v";
				break;
			case '"':
				dest = "\"";
				break;

			default:
				xassert(0);
				ErrH.Abort("Unknown special character");
			}

		s.replace(pos, 2, dest);
		pos += strlen(dest);
		}

	return s;
}

const char* refineNodeName(const char* name)
{
	if(name[0] != '|')
		return name;
	static string buffer;
	buffer = name + 1;
	int pos = buffer.find("|");
	if(pos != string::npos)
		buffer.erase(pos);
	return buffer.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////
//			ScriptParser
///////////////////////////////////////////////////////////////////////////////////////
XPrmOArchive::XPrmOArchive(const char* fname, bool disableInvalidChars) :
buffer_(10, 1), 
binaryStream_(0)
{
	memset(disabledChars_, 0, sizeof(disabledChars_));
	if(disableInvalidChars){
		disabledChars_[' '] = 1;
		disabledChars_['.'] = 1;
		disabledChars_[','] = 1;
		disabledChars_['!'] = 1;
		disabledChars_['#'] = 1;
		disabledChars_['$'] = 1;
		disabledChars_['%'] = 1;
		disabledChars_['^'] = 1;
		disabledChars_['&'] = 1;
		disabledChars_['*'] = 1;
		disabledChars_['('] = 1;
		disabledChars_[')'] = 1;
		disabledChars_['/'] = 1;
		disabledChars_['\\'] = 1;
		disabledChars_['<'] = 1;
		disabledChars_['>'] = 1;
		for(int i = 128; i < 256; i++)
			disabledChars_[i] = 1;
	}

	open(fname);
}

XPrmOArchive::~XPrmOArchive() 
{
	close();
}

void XPrmOArchive::open(const char* fname)
{
	if(fname)
		fileName_ = fname;
	buffer_.alloc(10000);
	buffer_.SetDigits(6);
}

bool XPrmOArchive::close()
{
	xassert(offset_.empty() && "Block isnt closed");
	if(binaryStream_.isOpen()){
		if(binaryStream_.ioError())
			return false;
		binaryStream_.close();
	}
	return saveFileSmart(fileName_.c_str(), buffer_, buffer_.tell());
}

struct BinaryLink{
	long offset;
	long length;
	unsigned long crc;

	BinaryLink()
	: offset(0)
	, length(0)
	, crc(0)
	{
	}
    
	void serialize(Archive& ar){
		ar.serialize(offset, "offset", 0);
		ar.serialize(length, "length", 0);
		ar.serialize(crc, "crc", 0);
	}
};

bool XPrmOArchive::processBinary(MemoryBlock& buffer, const char* name, const char* nameAlt)
{
	if(!binaryStream_.isOpen()){
		XBuffer file_name;
		file_name < fileName_.c_str() < ".bin";
		binaryStream_.open(file_name, XS_OUT);
	}
	xassert(binaryStream_.isOpen());
	if(!binaryStream_.isOpen())
		return false;

	BinaryLink link;
	link.offset = binaryStream_.tell();
    link.length = buffer.size();
	link.crc = crc32((unsigned char*)(buffer.buffer()), buffer.size(), 0);
	
	binaryStream_.write(buffer.buffer(), buffer.size());

	serialize(link, name, nameAlt);
    return true;
}


//////////////////////////////////////////////////////
void XPrmOArchive::saveStringEnclosed(const char* prmString)
{
	if(prmString){
		const char* begin = prmString;
		const char* end = prmString + strlen(prmString);
		while(begin < end && *begin == ' ')
			++begin;
		while(end > begin && *(end - 1) == ' ')
			--end;
		string s1(begin, end);
		expand_spec_chars(s1);
		buffer_ < "\"" < s1.c_str() < "\"";
	}
	else
		buffer_ < "0";
}

bool XPrmOArchive::openStructInternal(void* object, int size, const char* name, const char* nameAlt, const char* typeName, bool polymorphic) 
{
	openNode(name);
	openBracket();

	isContainer_.push_back(false);
	return true;
}

void XPrmOArchive::closeStruct( const char* name ) 
{
	isContainer_.pop_back();

	closeBracket();
	if(inContainer())
		saveString(",\r\n");
	else
		saveString(";\r\n");
}

bool XPrmOArchive::openContainer(void* array, int& number, const char* name, const char* nameAlt, const char* typeName, const char* elementTypeName, int elementSize, bool readOnly) 
{
	containerSize_.push_back(number);

	openNode(name);
	openBracket();
	buffer_ < offset_.c_str() <= number < ";\r\n";

	isContainer_.push_back(true);
	return true;
}

void XPrmOArchive::closeContainer( const char* name ) 
{
	xassert(!isContainer_.empty());
	isContainer_.pop_back();

	if(containerSize_.back() != 0){
		buffer_ -= 3;
		buffer_ < "\r\n";
	}
	closeBracket();

	if(inContainer())
		saveString(",\r\n");
	else
		saveString(";\r\n");
	xassert(!containerSize_.empty());
	containerSize_.pop_back();
}

int XPrmOArchive::openPointer(void*& object, const char* name, const char* nameAlt, const char* baseName, const char* typeName, const char* typeNameAlt ) 
{
	openNode(name);

	if(typeName) {
		if(inContainer()) {
			//saveString(offset_.c_str());
		}
		else{
			//				saveString(name);
			//				saveString(" = ");
		}
		saveStringEnclosed(typeName);
		saveString(" ");
		openBracket();
	}
	else{
		//saveString(offset_.c_str());
		saveString("0");
	}
	isContainer_.push_back(false);
	return NULL_POINTER;
}

void XPrmOArchive::closePointer( const char* name, const char* baseName, const char* derivedName ) 
{
	isContainer_.pop_back();
	if(derivedName) {
		closeBracket();
	}
	if(inContainer())
		saveString(",\r\n");
	else
		saveString(";\r\n");
	//closeItem(name);
}

void XPrmOArchive::openBracket() 
{
	buffer_ < "{\r\n";
	offset_ += "\t";
}

void XPrmOArchive::closeBracket() 
{
	xassert(! offset_.empty() && "Trying to pop element from empty stack!");
	offset_.erase(offset_.end() - 1);
	buffer_ < offset_.c_str() < "}";
}

bool XPrmOArchive::openNode(const char* name) 
{
	if(name && strlen(name) && !inContainer()){
#ifndef _FINAL_VERSION_
		const char* p = name;
		while(*p){
			if(disabledChars_[*(p++)]){
				XBuffer buf;
				buf < "Недопустимый символ в имени скриптов: " < name;
				ErrH.Abort(buf);
			}
		}
#endif // _FINAL_VERSION_
		buffer_ < offset_.c_str() < refineNodeName(name) < " = ";
	}
	else
		saveString(offset_.c_str());

	return true;
}

void XPrmOArchive::closeNode(const char* name) 
{
	if(inContainer()) {
		saveString(",\r\n");
	}
	else
		if(name)
			buffer_ < ";\r\n";
}

bool XPrmOArchive::processValue(std::wstring& value, const char* name, const char* nameAlt)
{
	if(value.empty()){
		openNode(name);
		saveString("\"\"");
		closeNode(name);
	}
	else{
		XBuffer utfBuffer;
		openNode(name);
		saveStringEnclosed(toUTF8(utfBuffer, value));
		closeNode(name);
	}
	return true;
}

bool XPrmOArchive::processValue(char& value, const char* name, const char* nameAlt)
{
	openNode(name);
	buffer_ <= value;
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(signed char& value, const char* name, const char* nameAlt)
{
	openNode(name);
	buffer_ <= value;
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(signed short& value, const char* name, const char* nameAlt)
{
	openNode(name);
	buffer_ <= value;
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(signed int& value, const char* name, const char* nameAlt)
{
	openNode(name);
	buffer_ <= value;
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(signed long& value, const char* name, const char* nameAlt)
{
	openNode(name);
	buffer_ <= value;
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(unsigned char& value, const char* name, const char* nameAlt)
{
	openNode(name);
	buffer_ <= value;
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(unsigned short& value, const char* name, const char* nameAlt)
{
	openNode(name);
	buffer_ <= value;
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(unsigned int& value, const char* name, const char* nameAlt)
{
	openNode(name);
	buffer_ <= value;
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(unsigned long& value, const char* name, const char* nameAlt)
{
	openNode(name);
	buffer_ <= value;
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(float& value, const char* name, const char* nameAlt)
{
	openNode(name);
	buffer_ <= value;
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(double& value, const char* name, const char* nameAlt)
{
	openNode(name);
	buffer_ <= value;
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(ComboListString& t, const char* name, const char* nameAlt)
{
	openNode(name);
	saveStringEnclosed(t);
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(string& str, const char* name, const char* nameAlt)
{
	openNode(name);
	saveStringEnclosed(str.c_str()); 
	closeNode(name);
	return true;
}

bool XPrmOArchive::processValue(bool& value, const char* name, const char* nameAlt)
{
	openNode(name);
	buffer_ < (value ? "true" : "false");
	closeNode(name);
	return true;
}

bool XPrmOArchive::processEnum(int& value, const EnumDescriptor& descriptor, const char* name, const char* nameAlt)
{
	openNode(name);
	saveString(descriptor.name(value));
	closeNode(name);
	return true;
}

bool XPrmOArchive::processBitVector(int& flags, const EnumDescriptor& descriptor, const char* name, const char* nameAlt)
{
	openNode(name);
	saveString(descriptor.nameCombination(flags).c_str());
	closeNode(name);
	return true;
}



//////////////////////////////////////////////
namespace {
	static bool isalpha_table[256];
	static bool isdigit_table[256];
	static bool iscsym_table[256];
	static bool isspace_table[256];
	static bool is_table_initialized = false;
};

XPrmIArchive::XPrmIArchive(const char* fname) :
buffer_(10, 1),
binaryStream_(0)
{
	version_ = 0;

	if(!is_table_initialized) {
		for(int c = 0; c < 128; ++c) {
			isalpha_table[c] = isalpha(c) != 0;
			isdigit_table[c] = isdigit(c) != 0;
			isspace_table[c] = isspace(c) != 0;
			iscsym_table[c] = __iscsym(c) != 0;
		}
		for(int c = 128; c <= 255; ++c) {
			isalpha_table[c] = false;
			isdigit_table[c] = false;
			isspace_table[c] = false;
			iscsym_table[c] = false;
		}
		is_table_initialized = true;
	}
	
	if(fname && !open(fname))
		ErrH.Abort("File not found: ", XERR_USER, 0, fname);
}

XPrmIArchive::~XPrmIArchive() 
{
	close();
}

bool XPrmIArchive::processBitVector(int& flags, const EnumDescriptor& descriptor, const char* name, const char* nameAlt) 
{
    if(openNode(name)){
		flags = 0;
        for(;;){
            string name;
            loadString(name);
            if(name == ";"){
                putToken();
                break;
            }
            else if(name == "|") {
                continue;
            }
			flags |= descriptor.keyByName(name.c_str());
        }
        closeNode(name);
        return true;
    }
    else
        return false;
}

bool XPrmIArchive::open(const char* fname, int blockSize)
{
	fileName_ = fname;
	XStream ff(0);
	if(!ff.open(fname, XS_IN))
		return false;
	if(!blockSize)
		blockSize = ff.size();
	else
		blockSize = min(blockSize, ff.size());
	buffer_.alloc(blockSize + 1);
	ff.read(buffer_.buffer(), blockSize);
	buffer_[blockSize] = 0;
	replaced_symbol = 0;
	putTokenOffset_ = 0;
	return true;
}

bool XPrmIArchive::close()
{
	buffer_.alloc(10);
	return true;
}

bool XPrmIArchive::processValue(char& value, const char* name, const char* nameAlt) 
{
    if(openNode(name)){
        signed short val;
        buffer_ >= val;
        value = char(val);
		closeNode(name);
		return true;
	}
	else
		return false;
} 

bool XPrmIArchive::processValue(signed char& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        signed short val;
        buffer_ >= val;
        value = signed char(val);
		closeNode(name);
		return true;
	}
	else
		return false;
} 

bool XPrmIArchive::processValue(signed short& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        buffer_ >= value;
		closeNode(name);
		return true;
	}
	else 
		return false;
} 

bool XPrmIArchive::processValue(signed int& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        buffer_ >= value;
		closeNode(name);
		return true;
	}
	else
		return false;
} 

bool XPrmIArchive::processValue(signed long& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        buffer_ >= value;
		closeNode(name);
		return true;
	}
	else 
		return false;
} 

bool XPrmIArchive::processValue(unsigned char& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        buffer_ >= value;
		closeNode(name);
		return true;
	}
	else
		return false;
} 
bool XPrmIArchive::processValue(unsigned short& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        buffer_ >= value;
		closeNode(name);
		return true;
	}
	else
		return false;
} 

bool XPrmIArchive::processValue(unsigned int& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        buffer_ >= value;
		closeNode(name);
		return true;
	}
	else
		return false;
} 

bool XPrmIArchive::processValue(unsigned long& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        buffer_ >= value;
		closeNode(name);
		return true;
	} 
	else 
		return false;
} 

bool XPrmIArchive::processValue(float& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        buffer_ >= value;
		closeNode(name);
		return true;
	}
	else
		return false;
}

bool XPrmIArchive::processValue(double& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        buffer_ >= value;
		closeNode(name);
		return true;
	}
	else
		return false;
}

bool XPrmIArchive::processValue(std::wstring& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
		bool result = false;
        string str;
        if(loadString(str)){
			std::vector<wchar_t> buffer;
			if(str.empty()){
				value = L"";
				result = true;
			}
			else{
				int length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), strlen(str.c_str()), 0, 0);
				if(length > 0){
					buffer.resize(length + 1);
					if(MultiByteToWideChar(CP_UTF8, 0, str.c_str(), strlen(str.c_str()), &buffer[0], length) == length){
						buffer[length] = '\0';
						value = &buffer[0];
						result = true;
					}
				}
				else{
					//kdWarning("&Serialization", "XPrmIArchive: unable to translate unicode string");
				}
			}
		}
		closeNode(name);
		return result;
	}
	else
		return false;
}

bool XPrmIArchive::processValue(ComboListString& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        loadString(value.value());
		closeNode(name);
		return true;
	}
	else
		return false;
}

bool XPrmIArchive::processValue(string& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        loadString(value);
		closeNode(name);
		return true;
	}
	else
		return false;
}


bool XPrmIArchive::processValue(bool& value, const char* name, const char* nameAlt) 
{
	if(openNode(name)){
        string str;
        loadString(str);
        if(str == "true")
            value = true;
        else if(str == "false")
            value = false;
        else
            value = atoi(str.c_str()) != 0;
		closeNode(name);
		return true;
	}
	else
		return false;
}

bool __forceinline isalphaX(unsigned char c) 
{
	return isalpha_table[c];
}
bool __forceinline isdigitX(unsigned char c) 
{
	return isdigit_table[c];
}
bool __forceinline iscsymX(unsigned char c) 
{
	return iscsym_table[c];
}
bool __forceinline isspaceX(unsigned char c) {
	return isspace_table[c];
}

const char* XPrmIArchive::getToken()	
{
	xassert(!replaced_symbol && "Unreleased token");
	
	putTokenOffset_ = buffer_.tell();

	// Search begin of token
	const char* i = &buffer_();
	for(;;){
		if(!*i)
			return 0; // eof

		if(*i == '/'){	     
			if(*(i + 1) == '/'){ // //-comment
				i += 2;
				if((i = strstr(i, "\n")) == 0) 
					return 0;
				i++;
				continue;
				}
			if(*(i + 1) == '*'){ // /* */-comment
				i += 2;
				while(!(*i == '*' && *(i + 1) == '/')){
					if(!*i)
						return 0; // error
					i++;
					}
				i += 2;
				continue;
				}
			}
		if(isspaceX(*i))
			i++;
		else
			break;
		}

	// Search end of token
	const char* marker = i;
	if(isalphaX(*i) || *i == '_'){ // Name
		i++;
		while(iscsymX(*i))
			i++;
		}
	else
		if(isdigitX(*i) || (*i == '.' && (isdigitX(*(i + 1)) || *(i + 1) == 'f'))){ // Numerical Literal
			i++;
			while(iscsymX(*i) || *i == '.' || (*i == '+' || *i == '-') && (*(i - 1) == 'E' || *(i - 1) == 'e'))
				i++;
		}
		else
			if(*i == '"'){ // Character Literal 
				i++;
				// пытаемся найти '"', если перед '"' стоит '\' и после '"' не следует ';',
				// то это не завершающая '"', все остальные '"' считаются завершающими 
				while((i = strstr(i, "\"")) != 0){
					if(*(i - 1) == '\\' && *(i + 1) != ';')
						i++;
					else {
						i++;
						break;
					}
				}
				if(i == 0)
					return 0; // error
			}
			else
				if(*i == '-' && *(i + 1) == '>'){ // ->
					i += 2;
					if(*i == '*')			// ->*
						i++;
					}
				else
					if(*i == '<' && *(i + 1) == '<'){ // <<
						i += 2;
						if(*i == '=')			// <<=
							i++;
						}
					else
						if(*i == '>' && *(i + 1) == '>'){ // >>
							i += 2;
							if(*i == '=')			// >>=
								i++;
							}
						else
							if(*i == '.' && *(i + 1) == '.' && *(i + 2) == '.') // ...
								i += 3;
							else
								if(*i == '#' && *(i + 1) == '#' || // ##
									*i == ':' && *(i + 1) == ':' || // ::
									*i == '&' && *(i + 1) == '&' || // &&
									*i == '|' && *(i + 1) == '|' || // ||
									*i == '.' && *(i + 1) == '*' || // .*
									*i == '+' && *(i + 1) == '+' || // ++
									*i == '-' && *(i + 1) == '-' || // --
									(*i == '+' || *i == '-' || *i == '*' || *i == '/' || *i == '%' || 
									*i == '^' || *i == '|' || *i == '&' || 
									*i == '!' || *i == '<' || *i == '>' || *i == '=') && *(i + 1) == '=') // x=
										i += 2;
								else
									i++;

	buffer_.set(i - buffer_.buffer());
	replaced_symbol = buffer_();
	buffer_() = 0;
	return marker;
}

void XPrmIArchive::releaseToken()
{
	//	xassert(replaced_symbol);
	buffer_() = replaced_symbol;
	replaced_symbol = 0;
}

void XPrmIArchive::putToken() 
{
	buffer_.set(putTokenOffset_);
}


void XPrmIArchive::passString(const char* token)
{
	const char* s = getToken();
	xassert(s);
	if(strcmp(s, token) != 0){
		XBuffer msg;
		msg  < "Expected Token: \"" < token
			< "\", Received Token: \"" < s < "\", file: \"" < fileName_.c_str() < "\", line: " <= line();
		xassertStr(0 && "Expected another token", msg);
		releaseToken();
		ErrH.Abort(msg);
	} 
	else 
		releaseToken();
}

bool XPrmIArchive::loadString(string& str)
{
	const char* s = getToken();
	xassert(s);
	str = s;
	releaseToken();
	if(str[0] == '"'){
		if(str[str.size() - 1] != '"'){
			XBuffer err;
			err < "Quotes aren't closed: " < s < "\n";
			err < "Line: " <= line();
			ErrH.Abort(err);
			} 
		str.erase(0, 1);
		xassert(!str.empty());
		str.pop_back();
		collapse_spec_chars(str);
	}
	else if(str == "0")
		return false;
	return true;
}

void XPrmIArchive::skipValue(int open_counter)
{
	if(open_counter)
		parserStack_.pop_back();

	for(;;){
		const char* str = getToken();
		xassert(str);
		if(str[0] == '{' && str[1] == '\0')
			++open_counter;
		else if(str[0] == '}' && str[1] == '\0'
				&& open_counter)
			--open_counter;
		else if(open_counter == 0) {
			if((str[0] == '}' || str[0] == ';' || str[0] == ',') && str[1] == '\0') {
				releaseToken();
				putToken();
				break;
			}
		}
		releaseToken();
	}
}

bool XPrmIArchive::findSection(const char* sectionName)
{
	for(int i = 0; i < 2; i++){
		string name;
		loadString(name);
		if(name == sectionName){
			putToken();
			return true;
		}
		else{
			passString("=");
			skipValue();
			passString(";");
			bool endOfFile = !getToken();
			releaseToken();
			if(!endOfFile)
				putToken();
			else{
				buffer_.set(0);
				replaced_symbol = 0;
				putTokenOffset_ = 0;
			}
		}
	}
	return false;
}

int XPrmIArchive::line() const 
{
	return 1 + count((const char*)buffer_, (const char*)buffer_ + buffer_.tell(), '\n');
}

bool XPrmIArchive::openNode(const char* name)
{
    if(isContainer())
        return true;

	if(name[0] == '|'){
		for(;;){
			const char* nameRefined = refineNodeName(name);
			if(openNode(nameRefined))
				return true;
			name += strlen(nameRefined) + 1;
			if(!strlen(name))
				return false;
		}
	}

	bool firstConversion = true;
	if(name){
		int pass = 0;
		for(;;){
			const char* str = getToken();
			const char* token = str ? str : "}"; // to simulate end of block when end of file
			if(!str)
				token = "}";
			if(strcmp(token, name) == 0){
				releaseToken();
				break;
			}
			if(token[0] == '}' && token[1] == '\0'){
				releaseToken();
				if(isContainer() || pass++ == 2){
					putToken();
					return false;
				}
				else
					buffer_.set(!parserStack_.empty() ? parserStack_.back().blockStart : 0);
			}
			else{
				releaseToken();
				// if(firstConversion){
				//     XBuffer buf;
				//     buf < "Происходит конверсия/пропуск элемента \"" < name < "\" при чтении: "
				//       < fileName_.c_str() < ", line " <= line() < "\nПопробуйте перезаписать данные.";
				//     kdWarning("&Serialization", buf);
				//     firstConversion = false;
				// }
				passString("=");
				skipValue();
				passString(";");
			}
		}

		passString("=");
	}

	return true;
}

void XPrmIArchive::closeNode(const char* name) 
{
	if(!isContainer()){
		if(name)
			passString(";");
	}
    else{
        string tok = getToken();
        releaseToken();
        if(tok != ",")
            putToken();
    }
}

void XPrmIArchive::openBracket(bool isContainer) 
{
	passString("{");
	parserStack_.push_back(ParserState(isContainer, buffer_.tell()));
}

void XPrmIArchive::closeBracket() 
{
	for(;;){
		string token;
		loadString(token);
		if(token == "}"){
			break;
		}
		else{
			passString("=");
			skipValue();
			passString(";");
		}
	}

	parserStack_.pop_back();
}

bool XPrmIArchive::openContainer(void* array, int& number, const char* name, const char* nameAlt, const char* typeName, const char* elementTypeName, int elementSize, bool readOnly)
{
	if(!openNode(name))
		return false;

	openBracket(true);

	string str_size;
    loadString(str_size);
    passString(";");
    number = atoi(str_size.c_str());
	return true;
}

void XPrmIArchive::closeContainer(const char* name) 
{
	if(name){
		string tok = getToken();
		releaseToken();
	    
		if(tok == ","){
			tok = getToken();
			releaseToken();
		}
		else
			putToken();

		while(tok != "}"){
			putToken();

			skipValue();

			tok = getToken();
			releaseToken();

			if(tok == ","){
				tok = getToken();
				releaseToken();
			}
		}
		putToken();
	}

	closeBracket();
	closeNode(name);
}

bool XPrmIArchive::openStructInternal(void* object, int size, const char* name, const char* nameAlt, const char* typeName, bool polymorphic) 
{
	if(!openNode(name))
		return false;

	openBracket(false);

	return true;
}

void XPrmIArchive::closeStruct(const char* name) 
{
	closeBracket();
	closeNode(name);
}

int XPrmIArchive::openPointer(void*& object, const char* name, const char* nameAlt, const char* baseName, const char* typeName, const char* typeNameAlt)
{
    if(!openNode(name))
		return -1;

	string str;
	loadString(str);
	if(strcmp(str.c_str(), "0") == 0) {
		closeNode(name);
		return -1;
	}
	else{
		openBracket(false);
		int result = indexInComboListString(typeName, str.c_str());
		if(result == -1){
			XBuffer msg(256, 1);
			msg < "ERROR! no such class registered: ";
			msg < str.c_str();
			xassertStr(0, static_cast<const char*>(msg));
			skipValue(1);
			closeNode(name);
			return -1;
		}
		return result;
	}
}

void XPrmIArchive::closePointer(const char* name, const char* typeName, const char* derivedName)
{
    closeBracket();
	closeNode(name);
}

bool XPrmIArchive::processEnum(int& value, const EnumDescriptor& descriptor, const char* name, const char* nameAlt)
{
	if(openNode(name)){
		string str;
		loadString(str);
	    closeNode(name);
		value = descriptor.keyByName(str.c_str());
		if(!descriptor.nameExists(str.c_str())){
			XBuffer msg;
			msg < str.c_str();
			if(strcmp(str.c_str(), str.c_str()) != 0)
				msg < "(\"" < str.c_str() < "\")";
			msg < "\nfile: \"" < fileName_.c_str() < "\", line: " <= line();
			xassertStr(0 && "Unregistered Enum value:", msg);
			return false;
		}
		return true;
	}
	else
		return false;
}


bool XPrmIArchive::processBinary(MemoryBlock& buffer, const char* name, const char* nameAlt)
{
	BinaryLink link;
	if(serialize(link, name, nameAlt)){
		bool result = true;
		if(!binaryStream_.isOpen()){
			XBuffer file_name;
			file_name < fileName_.c_str() < ".bin";
			binaryStream_.open(file_name, XS_IN);
		}
        if(binaryStream_.isOpen()){
            binaryStream_.seek(link.offset, XS_BEG);

			if(buffer.size()){
				if(buffer.size() != link.length){
					xassert(0 && "Static MemoryBlock has changed it's size");
					return false;
				}
			}
			else
				buffer.alloc(link.length);

            long readLength = binaryStream_.read(buffer.buffer(), link.length);
			if(readLength == link.length){
				if(link.crc){
					unsigned long calculatedCRC = crc32((unsigned char*)(buffer.buffer()), link.length, 0);
					if(calculatedCRC != link.crc){
						xassert(0 && "Binary block's CRC doesn't match!");
						return false;
					}
				}
				return true;
			}
			else{
				xassert(0 && "Unable to read entire block");
				return false;
			}
        }
		else{
			//xassert(0 && "Unable to open .bin file");
			return false;
		}
	}
	else
		return false;
}

unsigned int XPrmIArchive::crc() 
{
	return crc32((unsigned char*)buffer_.buffer(), buffer_.size(), startCRC32);
}

bool saveFileSmart(const char* fname, const char* buffer, int size)
{
	XStream testf(0);
	if(testf.open(fname, XS_IN)){
		if(testf.size() == size){
			PtrHandle<char> buf = new char[size];
			testf.read(buf, size);
			if(!memcmp(buffer, buf, size))
				return true;
		}
	}
	testf.close();
	
	XStream ff(0);
	if(ff.open(fname, XS_OUT)) {
		ff.write(buffer, size);
	} 
#ifndef _FINAL_VERSION_
	else{
		XBuffer buf;
		buf < "Unable to write file: \n" < fname;
		xxassert(0, buf);
	}
#endif

	return !ff.ioError();
}

string transliterate(const char* name)
{
	// Транслитерация неточная с точки зрения чтения, но удовлетворяющая требованиям
	// имен переменных - не должно быть цифр (Ч - 4) и знаков (Ъ - ')
	static const char* table[256] = {
		"\x0", "\x1", "\x2", "\x3", "\x4", "\x5", "\x6", "\x7", "\x8", "\x9", "\xa", "\xb", 
		"\xc", "\xd", "\xe", "\xf", "\x10", "\x11", "\x12", "\x13", "\x14", "\x15", "\x16", 
		"\x17", "\x18", "\x19", "\x1a", "\x1b", "\x1c", "\x1d", "\x1e", "\x1f", "_", 
		"_", "\x22", "\x23", "\x24", "\x25", "\x26", "_", "_", "_", "_", 
		"_", "_", "_", "_", "_", 
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?", "@", 
		"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", 
		"R", "S", "T", "U", "V", "W", "X", "Y", "Z", "_", "_", "_", "_", "_", "_", "a", "b", 
		"c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", 
		"t", "u", "v", "w", "x", "y", "z", "_", "_", "_", "_", "", 
		
/*		DOS
		//"А", "Б", "В", "Г", "Д", "Е", "Ж", "З", "И", "Й", "К", "Л", "М", "Н", "О", "П", "Р", 
		"A", "B", "V", "G", "D", "E", "J", "Z", "I", "J", "K", "L", "M", "N", "O", "P", "R", 
		//"С", "Т", "У", "Ф", "Х", "Ц", "Ч", "Ш", "Щ", "Ъ", "Ы", "Ь", "Э", "Ю", "Я", 
		"S", "T", "U", "F", "X", "C", "Ch", "Sh", "Sh", "h", "I", "h", "E", "U", "Ja", 
		//"а", "б", "в", "г", "д", "е", "ж", "з", "и", "й", "к", "л", "м", "н", "о", "п", 
		"a", "b", "v", "g", "d", "e", "j", "z", "i", "j", "k", "l", "m", "n", "o", "p", 
		"-", "-", "-", "¦", "+", "¦", "¦", "¬", "¬", "¦", "¦", "¬", "-", "-", "-", "¬", 
		"L", "+", "T", "+", "-", "+", "¦", "¦", "L", "г", "¦", "T", "¦", "=", "+", "¦", 
		"¦", "T", "T", "L", "L", "-", "г", "+", "+", "-", "-", "-", "-", "¦", "¦", "-", 
		//"р", "с", "т", "у", "ф", "х", "ц", "ч", "ш", "щ", "ъ", "ы", "ь", "э", "ю", "я", 
		"r", "s", "t", "u", "f", "x", "c", "ch", "sh", "sh", "h", "i", "h", "e", "u", "ja", 
		//"Ё", "ё", "Є", "є", "Ї", "ї", "Ў", "ў", "°", "•", "·", "v", "№", "¤", "¦", " " 
		"E", "e", "Є", "є", "Ї", "ї", "Ў", "ў", "°", "•", "·", "v", "№", "¤", "¦", " " 
*/
		"_", "_", "'", "_", "\"", ":", "+", "+", "_", "%", "_", "<", "_", "_", "_", "_", "_", 
		"'", "'", "\"", "\"", "", "-", "-", "_", "T", "_", ">", "_", "_", "_", "_", " ", "Ў", 
		"ў", "_", "¤", "_", "¦", "", "E", "c", "Є", "<", "¬", "-", "R", "Ї", "°", "+", "_", 
		"_", "_", "ч", "", "·", "e", "№", "є", ">", "_", "_", "_", "ї", 
		//"А", "Б", "В", "Г", "Д", "Е", "Ж", "З", "И", "Й", "К", "Л", "М", "Н", "О", "П", "Р", 
		"A", "B", "V", "G", "D", "E", "J", "Z", "I", "J", "K", "L", "M", "N", "O", "P", "R", 
		//"С", "Т", "У", "Ф", "Х", "Ц", "Ч", "Ш", "Щ", "Ъ", "Ы", "Ь", "Э", "Ю", "Я", 
		"S", "T", "U", "F", "X", "C", "Ch", "Sh", "Sh", "b", "I", "b", "E", "U", "Ja", 
		//"а", "б", "в", "г", "д", "е", "ж", "з", "и", "й", "к", "л", "м", "н", "о", "п", "р", 
		"a", "b", "v", "g", "d", "e", "j", "z", "i", "j", "k", "l", "m", "n", "o", "p", 
		//"с", "т", "у", "ф", "х", "ц", "ч", "ш", "щ", "ъ", "ы", "ь", "э", "ю", "я"
		"r", "s", "t", "u", "f", "x", "c", "ch", "sh", "sh", "b", "i", "b", "e", "u", "ja"
	};
	
	string result;
	while(*name){
		int c = unsigned char(*name++);
		result += table[c];
	}
	return result; 
}

