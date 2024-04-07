#ifndef __IN_PLACE_ARCHIVE_H__
#define __IN_PLACE_ARCHIVE_H__

/*
	InPlace Archive

	ѕринцип работы:
	«апись начинаетс€ с некоторой стуктуры данных, сразу записываетс€ все ее тело, затем все внутренние 
	элементы (структуры и базовые типы) просто провер€ютс€ на принадлежность по диапазону адресов. —троковые типы,
	vector<> и полиморфные указатели инициируют новую запись.

	ѕри загрузке возвращаетс€ указатель на верхнюю структуру.

	ќпционально делаетс€ fixup указателей виртуальных таблиц дл€ полиморфных классов: отказатьс€, если очень сложно
	регистрировать классы (как в случае StringTable), но при этом уменьшаетс€ "врем€ жизни" файлов данных.

	ћеханизмы отключени€ нежелательного кода в serialize():
	
	if(ar.isInput()) - инициализирующий при загрузке код не имеет смысла вызывать
	
	if(ar.isOutput()) - нельз€ вызывать перерасчет при записи, т.к. это может изменить размещение данных.
	ƒл€ перерасчета можно вывести сначала в другой архив.
	
	if(!ar.inPlace()) - закрывать временные объекты, которые лежат в другой области (текуща€ структура уже записана)
	
	if(ar.inPlace()) - сериализовать non-POD данные, которые не сериализованы обычными архивами

	” вектора или строки нулевого размера start == end == end_of_storage могут указывать в некорректную область.

	
*/

#include <vector>
#include "Handle.h"
#include "Serialization\Serialization.h"

class InPlaceOArchive : public Archive
{
public:
	InPlaceOArchive(const char* fname, bool fixVtable);
	~InPlaceOArchive();

	void open(const char* fname); 
	bool close();  // true if there were changes, so file was updated

	bool isInput() const { return false; }
	bool isOutput() const { return false; }

protected:
	bool processBinary(MemoryBlock& buffer, const char* name, const char* nameAlt);

	bool processEnum(int& value, const EnumDescriptor& descriptor, const char* name, const char* nameAlt) { writeValue(value); return true; }
	bool processBitVector(int& flags, const EnumDescriptor& descriptor, const char* name, const char* nameAlt) { writeValue(flags); return true; }

	bool processValue(char& value, const char* name, const char* nameAlt) { writeValue(value); return true; }
	bool processValue(signed char& value, const char* name, const char* nameAlt) { writeValue(value); return true; }
	bool processValue(signed short& value, const char* name, const char* nameAlt) { writeValue(value); return true; }
	bool processValue(signed int& value, const char* name, const char* nameAlt) { writeValue(value); return true; }
	bool processValue(signed long& value, const char* name, const char* nameAlt) { writeValue(value); return true; }
	bool processValue(unsigned char& value, const char* name, const char* nameAlt) { writeValue(value); return true; }
	bool processValue(unsigned short& value, const char* name, const char* nameAlt) { writeValue(value); return true; }
	bool processValue(unsigned int& value, const char* name, const char* nameAlt) { writeValue(value); return true; }
	bool processValue(unsigned long& value, const char* name, const char* nameAlt) { writeValue(value); return true; }
	bool processValue(float& value, const char* name, const char* nameAlt) { writeValue(value); return true; }
	bool processValue(double& value, const char* name, const char* nameAlt) { writeValue(value); return true; }
	bool processValue(bool& value, const char* name, const char* nameAlt) { writeValue(value); return true; }

	bool processValue(ComboListString& t, const char* name, const char* nameAlt);
	bool processValue(string& str, const char* name, const char* nameAlt);

	bool openStructInternal(void* object, int size, const char* name, const char* nameAlt, const char* typeName, bool polymorphic);
	void closeStruct(const char* name);

	bool openContainer(void* array, int& number, const char* name, const char* nameAlt, const char* typeName, const char* elementTypeName, int elementSize, bool readOnly);
	void closeContainer(const char* name);

	int openPointer(void*& object, const char* name, const char* nameAlt, const char* baseName, const char* typeName, const char* typeNameAlt);
	void closePointer(const char* name, const char* baseName, const char* derivedName);


private:
	class Saver
	{
	public:
		Saver::Saver(int initial_size=124)
		{
			buffer_ = (char*)::malloc(initial_size);
			assert(buffer_);
			position_ = buffer_;
			allocated_size_ = initial_size;
		}

		Saver::~Saver()
		{
			::free(buffer_);
		}

		char* buffer() { return buffer_; };
		int size() { return position_ - buffer_; }

		void set(int offset) { position_ = buffer_ + offset; }

		void write(const char* data, size_t size)
		{
			if(buffer_ + allocated_size_ < position_ + size)
				reallocate(max(allocated_size_ + 1024*(size/1024 + 2), allocated_size_*2));
			memcpy(position_, data, size);
			position_ += size;
		}

		template<class T>
		void write(const T& x){ write((const char*)&x, sizeof(x)); }

		void write(const char* str) { write(str, strlen(str) + 1); }
		void write(const string& str) { write(str.c_str(), (int)str.size() + 1); }

	private:
		void reallocate(size_t new_size)
		{
			size_t offset = position_ - buffer_;
			buffer_ = (char*)realloc(buffer_, new_size);
			assert(buffer_);
			position_ = buffer_ + offset;
			allocated_size_ = new_size;
		}

		char* buffer_;
		char* position_;
		size_t allocated_size_;
	};

	class Node
	{
	public:
		Node(const char* address, int size, int offset)
			: address_(address), size_(size), offset_(offset) {}

		// [address, size] лежит внутри this
		bool in(const char* address, int size) const { return address_ <= address && address_ + size_ >= address + size; }
		// [address, size] лежит полностью вне this
		bool out(const char* address, int size) const { return address_ + size_ <= address || address_ >= address + size; }

		int offset(const char* address) const { 
			xassert(in(address, 1)); 
			return address - address_ + offset_; 
		}

	private:
		const char* address_;
		int size_;
		int offset_;
	};

	typedef vector<Node> Stack;
	Stack stack_;

	bool beginBlock_;
	Saver saver_;
	Saver fixUpSaver_;
	bool fixVTable_;
	int fixVTableSize_;
	Saver fixVTableSaver_;
	string fileName;

	void checkIn(const char* object, int size){ xassert("ѕопытка записать в InPlaceOArchive елемент, не принадлежащий структуре" && stack_.back().in(object, size)); }

	void writeString(const string& str);
	template<class T> void writeValue(const T& value) { checkIn((char*)&value, sizeof(value)); }

	const Node& back() const;
};

class InPlaceIArchive
{
public:
	InPlaceIArchive(const char* name);

	bool open(const char* fname);  // true if file exists
	
	template<class T>
	void construct(T*& ptr) { ptr = (T*)data_; }

	static void destruct(void* ptr) { delete ptr; }

private:
	int version_;
	int size_;
	union{
		char* data_;
		int* dataInt_;
	};
};

#endif //__IN_PLACE_ARCHIVE_H__
