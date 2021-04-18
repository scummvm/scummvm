#include <fstream>
#include <sstream>
#include "decompiler.h"

namespace Private {

Decompiler::~Decompiler() {
}

Decompiler::Decompiler(const std::string &fileName, bool mac) {
	std::ifstream infile(fileName);
	if (!infile.good())
		throw std::invalid_argument("File does not exist");
	std::ifstream input(fileName, std::ios::binary);
	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
	decompile(buffer, mac);
}
Decompiler::Decompiler(std::vector<unsigned char> &buffer, bool mac) {
	decompile(buffer, mac);
}

void Decompiler::decompile(std::vector<unsigned char> &buffer, bool mac) {
	std::vector<unsigned char>::iterator it = buffer.begin();

	std::string firstBytes(it, it + kHeader.length());
	if (firstBytes != kHeader) {
		throw std::invalid_argument("Not a precompiled game matrix");
	}

	std::stringstream ss;
	bool inDefineRects = false;
	for (it += kHeader.length() ; it != buffer.end() ; ) {
		unsigned char byte = *it++;
		if (byte == kCodeString) {
			unsigned char len = *it++;
			std::string s(it,it+len);
			it += len;
			ss << "\"" << s << "\"";
		} else if (byte == kCodeShortLiteral || byte == kCodeShortId) {
			unsigned char b1 = *it++;
			unsigned char b2 = *it++;
			unsigned int number = mac ? b2 + (b1 << 8) : b1 + (b2 << 8);
			if (byte == kCodeShortId) ss << "k";
			ss << number;
		} else if (byte == kCodeRect && inDefineRects) {
			ss << "RECT"; // override CRect
		} else if (byte <= kCodeShortId && kCodeTable[byte].length() > 0) {
			ss << kCodeTable[byte];
		} else {
			throw std::invalid_argument("Unknown byte code");
		}

		if (byte == kCodeRects) {
			inDefineRects = true;
		} else if (byte == kCodeBraceClose && inDefineRects) {
			inDefineRects = false;
		}
	}
	_result = ss.str();
}

void Decompiler::getResult(std::string &result) const {
	result = _result;
}

}
