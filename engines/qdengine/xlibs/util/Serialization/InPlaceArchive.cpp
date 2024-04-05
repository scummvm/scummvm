#include "stdafx.h"
#include "InPlaceArchive.h"
#include "Serialization\SerializationFactory.h"

InPlaceOArchive::InPlaceOArchive(const char* fname, bool fixVtable)
{
	beginBlock_ = true;
	inPlace_ = true;
	fixVTableSize_ = 0;
	fixVTable_ = fixVtable;
	open(fname);
}

InPlaceOArchive::~InPlaceOArchive()
{
	close();
}

void InPlaceOArchive::open(const char* fname)
{
	if(fname)
		fileName = fname;
}

bool InPlaceOArchive::close()
{
	if(fileName.empty())
		return false;

	Saver buffer;
	buffer.write(version());
	buffer.write(saver_.size());
	buffer.write(saver_.buffer(), saver_.size());
	buffer.write(fixUpSaver_.size()/sizeof(int));
	buffer.write(fixUpSaver_.buffer(), fixUpSaver_.size());
	buffer.write(fixVTableSize_);
	buffer.write(fixVTableSaver_.buffer(), fixVTableSaver_.size());

	return saveFileSmart(fileName.c_str(), buffer.buffer(), buffer.size());
}

bool InPlaceOArchive::processBinary(MemoryBlock& buffer, const char* name, const char* nameAlt) 
{
	checkIn((char*)&buffer, sizeof(buffer));

	xassert(!stack_.empty());
	int offset = stack_.back().offset((const char*)&buffer);
	
	fixUpSaver_.write(offset);

	int dataOffset = saver_.size();
	saver_.write(buffer.buffer(), buffer.size());
	int dataEnd = saver_.size();
	saver_.set(offset);
	saver_.write(dataOffset);
	saver_.set(dataEnd);

	return true;
}

bool InPlaceOArchive::processValue(ComboListString& t, const char* name, const char* nameAlt) 
{
	writeString(t.value());
	return true;
}

bool InPlaceOArchive::processValue(string& str, const char* name, const char* nameAlt) 
{
	writeString(str);
	return true;
}

bool InPlaceOArchive::openStructInternal(void* objectVoid, int size, const char* name, const char* nameAlt, const char* typeName, bool polymorphic) 
{ 
	char* object = (char*)objectVoid;
	if(beginBlock_){
		beginBlock_ = false;
		stack_.push_back(Node(object, size, saver_.size()));
		saver_.write(object, size);
	}
	else{
		checkIn(object, size);
		stack_.push_back(Node(object, size, stack_.back().offset(object)));
	}

	if(fixVTable_ && polymorphic){
		fixVTableSize_++;
		fixVTableSaver_.write(stack_.back().offset(object));
		fixVTableSaver_.write(typeName);
		
		int offset = saver_.size();
		saver_.set(stack_.back().offset(object));
		saver_.write(int(0));
		saver_.set(offset);
	}
	return true; 
}

void InPlaceOArchive::closeStruct(const char* name) 
{ 
	xassert(!stack_.empty());
	stack_.pop_back();
}

void InPlaceOArchive::writeString(const string& str) 
{
	// [begin][end][end_of_allocated]

	checkIn((char*)&str, sizeof(str));

	xassert(!stack_.empty());
	int offset = stack_.back().offset((const char*)&str);
	
	fixUpSaver_.write(offset);
	fixUpSaver_.write(offset + 4);
	fixUpSaver_.write(offset + 8);

	int dataOffset = saver_.size();
	saver_.write(str.c_str());
	int dataEnd = saver_.size();
	saver_.set(offset);
	saver_.write(dataOffset);
	saver_.write(dataEnd - 1); // 0 не считается
	saver_.write(dataEnd); 
	saver_.set(dataEnd);
}

bool InPlaceOArchive::openContainer(void* array, int& number, const char* name, const char* nameAlt, const char* typeName, const char* elementTypeName, int elementSize, bool readOnly) 
{
	if(!readOnly){ // object = &vector<>
		int offset = stack_.back().offset((char*)array);
		if(number){
			int size = number*elementSize;
			fixUpSaver_.write(offset);
			fixUpSaver_.write(offset + 4);
			fixUpSaver_.write(offset + 8);

			char* data = *(char**)array;
			int dataOffset = saver_.size();
			saver_.write((char*)data, size);
			int dataEnd = saver_.size();

			saver_.set(offset);
			saver_.write(dataOffset);
			saver_.write(dataEnd);
			saver_.write(dataEnd);
			saver_.set(dataEnd);

			stack_.push_back(Node(data, size, dataOffset));
		}
		else{
			int dataEnd = saver_.size();

			saver_.set(offset);
			saver_.write(0);
			saver_.write(0);
			saver_.write(0);
			saver_.set(dataEnd);

			stack_.push_back(stack_.back());
		}
	}
	else
		stack_.push_back(stack_.back());

	return true;
}

void InPlaceOArchive::closeContainer(const char* name)
{
	xassert(!stack_.empty());
	stack_.pop_back();
}

int InPlaceOArchive::openPointer(void*& object, const char* name, const char* nameAlt, const char* baseName, const char* typeName, const char* typeNameAlt) 
{
	if(object){
		int offset = stack_.back().offset((char*)&object);
		fixUpSaver_.write(offset);

		int dataOffset = saver_.size();
		saver_.set(offset);
		saver_.write(dataOffset);
		saver_.set(dataOffset);

		beginBlock_ = true;
	}

	return NULL_POINTER;
}

void InPlaceOArchive::closePointer(const char* name, const char* baseName, const char* derivedName) 
{
}

const InPlaceOArchive::Node& InPlaceOArchive::back() const 
{ 
	return stack_.back(); 
}


//////////////////////////////////////////////////

InPlaceIArchive::InPlaceIArchive(const char* fname)
{
	data_ = 0;
	version_ = 0;
	open(fname);
}

bool InPlaceIArchive::open(const char* fname)
{
	XStream ff(0);
	if(!ff.open(fname, XS_IN))
		return false;
	ff.read(version_);
	ff.read(size_);
	data_ = new char[size_];
	ff.read(data_, size_);

	int auxSize = ff.size() - ff.tell();
	char* auxData = new char[auxSize];
	ff.read(auxData, auxSize);
	int* fixUp = (int*)auxData;
	int fixUpSize = *fixUp++;
	while(fixUpSize--)
		*(int*)(data_ + *fixUp++) += (int)data_;

	int fixVTableSize = *fixUp++;
	while(fixVTableSize--){
		int offset = *fixUp++;
		char* typeName = (char*)fixUp;
		*(int*)(data_ + offset) = (int)VTableFactory::getVTable(typeName);
		typeName += strlen(typeName) + 1;
		fixUp = (int*)typeName;
	}

	delete auxData;

	return true;
}


