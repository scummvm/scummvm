#ifndef SEARCH_H
#define SEARCH_H

#include "util.h"

#include <list>
#include <map>
#include <vector>
#include <cstring>

class Hasher {
public:
	struct Hash {
		uint8 digest[16];

		bool operator==(const Hash &r) const {
			return !std::memcmp(digest, r.digest, sizeof(digest));
		}

		bool operator<(const Hash &r) const {
			return std::memcmp(digest, r.digest, sizeof(digest)) < 0;
		}
	};

	static Hash createHash(const byte *data, uint32 size);
};

struct SearchData {
	uint32 size;

	uint32 byteSum;
	Hasher::Hash hash;

	bool operator==(const SearchData &r) const {
		return (size == r.size && byteSum == r.byteSum && hash == r.hash);
	}

	bool operator<(const SearchData &r) const {
		return (size < r.size && byteSum < r.byteSum && hash < r.hash);
	}
};

class SearchCreator {
public:
	static SearchData create(const char *filename);
	static SearchData create(const byte *buffer, uint32 size);
};

class SumCreator {
public:
	typedef std::list<uint32> InputList;
	typedef std::vector<byte> DataInput;

	SumCreator(InputList list, const DataInput &input);

	bool nextByte();

	uint32 getOffset() const { return _curOffset; }

	bool hasSum(uint32 size) const;
	uint32 getSum(uint32 size) const;
private:
	uint32 _curOffset;
	DataInput _input;

	typedef std::map<uint32, uint32> SumMap;
	SumMap _sums;
	typedef std::map<uint32, uint> SumCount;
	SumCount _sumCount;
};

class Search {
public:
	Search(const char *filename);
	Search(const byte *data, uint32 size);

	void addData(SearchData data);

	struct ResultData {
		SearchData data;
		uint32 offset;
		
		ResultData() : data(), offset() {}
		ResultData(SearchData d, uint32 o) : data(d), offset(o) {}
	};

	typedef std::list<ResultData> ResultList;
	bool search(ResultList &res);
private:
	SumCreator::DataInput _data;

	typedef std::list<SearchData> SearchList;
	SearchList _search;
};

#endif

