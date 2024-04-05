#include "StdAfx.h"
#include "BinaryArchive.h"
#include "XMath\xmath.h"
#include "Dictionary.h"
#include "crc.h"
#include "EnumDescriptor.h"

const short int CONTAINER_ID = 8383; // подстраховка при слитии блоков
const char* fileHeader = "KDV";

///////////////////////////////////////////////////////

BinaryOArchive::BinaryOArchive(const char* fname)
{
	open(fname);
}

BinaryOArchive::~BinaryOArchive()
{
	close();
}

void BinaryOArchive::open(const char* fname)
{
	if(fname){
		fileName = fname;
		int version = 0;
		serialize(version, fileHeader, 0);
	}
}

bool BinaryOArchive::close()
{
	if(fileName.empty())
		return false;

	return saveFileSmart(fileName.c_str(), saver_.buffer(), saver_.size());
}

bool BinaryOArchive::processBinary(MemoryBlock& buffer, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(buffer.size());
	saver_.write(buffer.buffer(), buffer.size());
	closeNode();
	return true;
}

bool BinaryOArchive::processEnum(int& value, const EnumDescriptor& descriptor, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(descriptor.name(value));
	closeNode();
	return true;
}

bool BinaryOArchive::processBitVector(int& flags, const EnumDescriptor& descriptor, const char* name, const char* nameAlt) 
{
	openNode(name);
	EnumDescriptor::NameCombination::const_iterator it;
	EnumDescriptor::NameCombination strings;
	descriptor.nameCombinationStrings(flags, strings);
	saver_.write((unsigned char)strings.size());
	FOR_EACH(strings, it)
		saver_.write(*it);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(char& value, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(value);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(signed char& value, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(value);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(signed short& value, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(value);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(signed int& value, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(value);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(signed long& value, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(value);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(unsigned char& value, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(value);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(unsigned short& value, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(value);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(unsigned int& value, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(value);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(unsigned long& value, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(value);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(float& value, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(value);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(double& value, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(value);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(ComboListString& t, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(t.value());
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(wstring& str, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(str);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(string& str, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(str);
	closeNode();
	return true;
}

bool BinaryOArchive::processValue(bool& value, const char* name, const char* nameAlt) 
{
	openNode(name);
	saver_.write(value);
	closeNode();
	return true;
}

bool BinaryOArchive::openStructInternal(void* object, int size, const char* name, const char* nameAlt, const char* typeName, bool polymorphic) 
{
	openNode(name);
	return true;
}

void BinaryOArchive::closeStruct(const char* name) 
{
	closeNode();
}

bool BinaryOArchive::openContainer(void* array, int& number, const char* name, const char* nameAlt, const char* typeName, const char* elementTypeName, int elementSize, bool readOnly) 
{
	openNode(name);
	saver_.write(CONTAINER_ID);
	saver_.write((unsigned short int)number);
	return true;
}

void BinaryOArchive::closeContainer(const char* name) 
{
	closeNode();
}

int BinaryOArchive::openPointer(void*& object, const char* name, const char* nameAlt, const char* baseName, const char* typeName, const char* typeNameAlt) 
{
	openNode(name);
	string typeNameStr = typeName ? typeName : "";
	processValue(typeNameStr, "typeName", 0);
	return NULL_POINTER;
}

void BinaryOArchive::closePointer(const char* name, const char* baseName, const char* derivedName) 
{
	closeNode();
}

bool BinaryOArchive::operator==(const BinaryOArchive& rhs) const
{
	if(saver_.size() != rhs.saver_.size())
		return false;
	
	if(saver_.size() > 0 &&
		memcmp(saver_.buffer(), rhs.saver_.buffer(), saver_.size()) != 0)
		return false;

	return true;
}

///////////////////////////////////////////////////////

BinaryIArchive::BinaryIArchive(const char* fname)
: ignoreUnregisteredClasses_(false)
{
	data_ = 0;
	version_ = 0;
	open(fname);
}

BinaryIArchive::BinaryIArchive(char* data, size_t size)
: ignoreUnregisteredClasses_(false)
{
	size_ = size;
	data_ = new char[size_];
	memcpy(data_, data, size);

	blocks_.push_back(Block(data_, size_));
}

BinaryIArchive::BinaryIArchive(const BinaryOArchive& oa)
: ignoreUnregisteredClasses_(false)
{
	size_ = oa.saver_.size();
	data_ = new char[size_];
	memcpy(data_, oa.saver_.buffer(), size_);

	blocks_.push_back(Block(data_, size_));
}

BinaryIArchive::~BinaryIArchive()
{
	close();
}

bool BinaryIArchive::open(const char* fname)
{
	if(data_){
		delete data_;
		data_ = 0;
	}

	XStream ff(0);
	if(!ff.open(fname, XS_IN))
		return false;
	size_ = (int)ff.size();
	data_ = new char[size_];
	ff.read(data_, size_);

	if(strcmp(data_, fileHeader)){
		delete data_;
		data_ = 0;
		return false;
	}
	
	blocks_.push_back(Block(data_, size_));

	serialize(version_, fileHeader, 0);

	return true;
}

bool BinaryIArchive::openBlockInternal(const char* name)
{
	if(mergeBlocks_)
		return true;

	Block block(0, 0);
	if(currentBlock().get(name, block)){
		blocks_.push_back(block);
		return true;
	}
	return false;
}

void BinaryIArchive::closeBlockInternal() 
{
	if(!mergeBlocks_){
		xassert(currentBlock().validToClose());
		blocks_.pop_back();
	}
}

bool BinaryIArchive::close()
{
	return true;
}

unsigned int BinaryIArchive::crc() 
{
	return crc32((unsigned char*)data_, size_, startCRC32);
}

bool BinaryIArchive::processEnum(int& value, const EnumDescriptor& descriptor, const char* name, const char* nameAlt) {
	if(!openNode(name))
		return false;
	string valueName;
	currentBlock().read(valueName);
	value = descriptor.keyByName(valueName.c_str());
	closeNode();
	return true;
}

bool BinaryIArchive::processBitVector(int& flags, const EnumDescriptor& descriptor, const char* name, const char* nameAlt ) 
{
	if(!openNode(name))
		return false;
	flags = 0;
	unsigned char size;
	currentBlock().read(size);
	while(size--){
		string valueName;
		currentBlock().read(valueName);
		flags |= descriptor.keyByName(valueName.c_str());
	}
	closeNode();
	return true;
}


bool BinaryIArchive::processBinary(MemoryBlock& buffer, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	int size;
	currentBlock().read(size);
	buffer.alloc(size);
	currentBlock().read(buffer.buffer(), size);
	closeNode();
	return true;
}

int BinaryIArchive::openPointer(void*& object, const char* name, const char* nameAlt, const char* baseName, const char* typeName, const char* typeNameAlt) 
{
	if(openBlockInternal(name) || openBlockInternal(name)){
		string type_name;
		processValue(type_name, "typeName", 0);
		if(type_name.empty()){
			closeBlockInternal();
			return NULL_POINTER;
		}
		int result = indexInComboListString(typeName, type_name.c_str());
		if(result == NULL_POINTER) {
			if(!ignoreUnregisteredClasses_){
				XBuffer msg;
				msg < "ERROR! no such class registered: ";
				msg < type_name.c_str();
				xassertStr(0, static_cast<const char*>(msg));
			}
			closeBlockInternal();
			return NULL_POINTER;
		}
		return result;
	}
	return NULL_POINTER;
}

bool BinaryIArchive::processValue(char& value, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(value);
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(signed char& value, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(value);
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(signed short& value, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(value);
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(signed int& value, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(value);
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(signed long& value, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(value);
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(unsigned char& value, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(value);
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(unsigned short& value, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(value);
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(unsigned int& value, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(value);
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(unsigned long& value, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(value);
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(float& value, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(value);
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(double& value, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(value);
	closeNode();
	return true;
}


bool BinaryIArchive::processValue(ComboListString& t, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	string value;
	currentBlock().read(value);
	t = value;
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(string& str, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(str);
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(wstring& str, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(str);
	closeNode();
	return true;
}

bool BinaryIArchive::processValue(bool& value, const char* name, const char* nameAlt) 
{
	if(!openNode(name))
		return false;
	currentBlock().read(value);
	closeNode();
	return true;
}

bool BinaryIArchive::openStructInternal(void* object, int size, const char* name, const char* nameAlt, const char* typeName, bool polymorphic) 
{
	return openBlockInternal(name) || openBlockInternal(name);
}

void BinaryIArchive::closeStruct(const char* name) 
{
	closeBlockInternal();
}

bool BinaryIArchive::openContainer(void* array, int& number, const char* name, const char* nameAlt, const char* typeName, const char* elementTypeName, int elementSize, bool readOnly) 
{
	if(openBlockInternal(name) || openBlockInternal(name)){
		unsigned short int sizeShort, id;
		currentBlock().read(id);
		currentBlock().read(sizeShort);
		xassert(id == CONTAINER_ID && "Неправильно прочитан размер массива");
		number = sizeShort;
		return true;
	}
	else
		return false;
}

void BinaryIArchive::closeContainer(const char* name) 
{
	closeBlockInternal();
}

void BinaryIArchive::closePointer(const char* name, const char* baseName, const char* derivedName) 
{
	closeBlockInternal();
}

bool BinaryIArchive::Block::get(const char* name, Block& block) 
{
	complex_ = true;
	for(;;){
		if(curr_ >= end_){
			curr_ = begin_;
			return false;
		}

		for(const char* p = name; ;++p, ++curr_){
			if(!*curr_){
				++curr_;
				int size;
				read(size);
				if(!*p){
					block = Block(curr_, size);
					curr_ += size;
					return true;
				}
				else{
					curr_ += size;
					break;
				}
			}
			else if(*p != *curr_){
				while(*curr_++);
				int size;
				read(size);
				curr_ += size;
				break;
			}
		}
	}
}


////////////////////////////////////////////////////////////
MemoryBlock::MemoryBlock(int size) 
: makeFree_(false)
{ 
	alloc(size); 
}

MemoryBlock::MemoryBlock(const MemoryBlock& block)
: makeFree_(false)
{
	*this = block;
}

MemoryBlock& MemoryBlock::operator=(const MemoryBlock& block)
{ 
	alloc(block.size()); 
	memcpy(buffer_, block.buffer(), size());
	return *this;
}

void MemoryBlock::alloc(int size)
{
	free();
	size_ = size;
	if(size){
		makeFree_ = true;
		buffer_ = new char[size];
	}
	else{
		makeFree_ = false;
		buffer_ = 0;
	}
}

void MemoryBlock::free()
{
	if(makeFree_){
		delete buffer_;
		makeFree_ = false;
		buffer_ = 0;
		size_ = 0;
	}
}

