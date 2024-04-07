#ifndef __BINARY_ARCHIVE_H__
#define __BINARY_ARCHIVE_H__

#include <vector>
#include "Handle.h"
#include "Serialization\Serialization.h"


class BinaryOArchive : public Archive
{
public:
	BinaryOArchive(const char* fname = 0);
	~BinaryOArchive();

	void open(const char* fname); 
	bool close();  // true if there were changes, so file was updated

	bool isOutput() const { return true; }
	bool isInput() const { return false; }

	char* data() const{ return saver_.buffer(); }
	size_t size() const{ return size_t(saver_.size()); }

	bool operator==(const BinaryOArchive& rhs) const;
	bool operator!=(const BinaryOArchive& rhs) const{ return !operator==(rhs); }
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
	bool processValue(ComboListString& t, const char* name, const char* nameAlt);
	bool processValue(string& str, const char* name, const char* nameAlt);
	bool processValue(wstring& str, const char* name, const char* nameAlt);
	bool processValue(bool& value, const char* name, const char* nameAlt);

	bool openNode(const char* name) 
	{
		if(!mergeBlocks_)
			saver_.push(name);
		return true;
	}
	void closeNode() 
	{
		if(!mergeBlocks_)
			saver_.pop();
	}
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

		char* buffer() const { return buffer_; };
		int size() const { return position_ - buffer_; }

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
			void write(const wstring& str) { write((const char*)str.c_str(), (int)(str.size() + 1) * sizeof(wchar_t)); }

			void push(const char* id)
			{
				write(id);
				int blockSize = 0;
				write(blockSize);
				size_t offset = position_ - buffer_;
				stack_.push_back(offset);
			}

			void pop()
			{
				size_t old_offset = position_ - buffer_;
				size_t n = stack_.size() - 1;
				size_t min = stack_.back();
				position_ = buffer_ + (min - 4);
				size_t size = old_offset - min;
				write(size);
				stack_.pop_back();
				position_ = buffer_ + old_offset;
			}

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
		/// вектор смещений, вместо вектора указателей
		vector<size_t> stack_;
	};

	Saver saver_;
	string fileName;

	friend class BinaryIArchive;
};

class BinaryIArchive : public Archive
{
public:
	BinaryIArchive(const char* fname = 0);
	BinaryIArchive(const BinaryOArchive& oa);
	BinaryIArchive(char* data, size_t size);
	~BinaryIArchive();

	bool isOutput() const { return false; }
	bool isInput() const { return true; }

	bool open(const char* fname);  // true if file exists
	bool close();

	void setIgnoreUnregisteredClasses(bool ignore){ ignoreUnregisteredClasses_ = ignore; }
	void setVersion(int version) { version_ = version; } // ƒл€ сложной конверсии: вручную записывать, выставл€ть и кастить архив к XPrmIArchive
	int version() const { return version_; }

	unsigned int crc();

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
	bool processValue(ComboListString& t, const char* name, const char* nameAlt);
	bool processValue(string& str, const char* name, const char* nameAlt);
	bool processValue(wstring& t, const char* name, const char* nameAlt);
	bool processValue(bool& value, const char* name, const char* nameAlt);

protected:
	bool openNode(const char* name)
	{
		return openBlockInternal(name) || openBlockInternal(name);
	}
	void closeNode()
	{
		closeBlockInternal();
	}

	bool openStructInternal(void* object, int size, const char* name, const char* nameAlt, const char* typeName, bool polymorphic);
	void closeStruct(const char* name);

	bool openContainer(void* array, int& number, const char* name, const char* nameAlt, const char* typeName, const char* elementTypeName, int elementSize, bool readOnly);
	void closeContainer(const char* name);

	int openPointer(void*& object, const char* name, const char* nameAlt, const char* baseName, const char* typeName, const char* typeNameAlt);
	void closePointer(const char* name, const char* baseName, const char* derivedName);

private:
	int version_;
	int size_;
	char* data_;
	bool ignoreUnregisteredClasses_;

	class Block
	{
	public:
		Block(char* data, int size) : 
		  begin_(data), curr_(data), end_(data + size), complex_(false) {}

		bool get(const char* name, Block& block);

		void read(void *data, int size)
		{
			xassert(curr_ + size <= end_);
			memcpy(data, curr_, size);
			curr_ += size;	
		}

		template<class T>
		void read(T& x){ read(&x, sizeof(x)); }

		void read(string& s)
		{
			xassert(curr_ + strlen(curr_) < end_);
			s = curr_;
			curr_ += strlen(curr_) + 1;
		}
		void read(wstring& s)
		{
			xassert(curr_ + sizeof(wchar_t) * wcslen((wchar_t*)curr_) < end_);
			s = (wchar_t*)curr_;
			curr_ += (wcslen((wchar_t*)curr_) + 1) * sizeof(wchar_t);
		}

		bool validToClose() const { return complex_ || curr_ == end_; } // ѕростые блоки должны быть вычитаны точно

	private:
		char* begin_;
		char* end_;
		char* curr_;
		bool complex_;

		void read(char* s);
	};

	typedef vector<Block> Blocks;
	Blocks blocks_;

	bool openBlockInternal(const char* name);
	void closeBlockInternal();
	Block& currentBlock() { return blocks_.back(); }
};

template<class T>
void cloneByBinaryArchive(T& clone, const T& original)
{
	BinaryOArchive oa;
	oa.serialize(original, "clone", 0);
	BinaryIArchive ia(oa);
	ia.serialize(clone, "clone", 0);
}

#endif //__BINARY_ARCHIVE_H__
