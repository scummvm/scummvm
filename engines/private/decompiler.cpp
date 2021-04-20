#include "private/decompiler.h"

namespace Private {

Decompiler::Decompiler(char *buf, uint32 fileSize, bool mac) {

    Common::Array<unsigned char> array;
	uint32 i = 0;
	while (i < fileSize) {
		array.push_back(buf[i]);
		i++;
	}
	decompile(array, mac);
}

void Decompiler::decompile(Common::Array<unsigned char> &buffer, bool mac) {
	Common::Array<unsigned char>::iterator it = buffer.begin();

	Common::String firstBytes((const char *) it, (const char *) it + kHeader.size());
    //debug("first bytes \"%s\"", firstBytes.c_str());

	if (firstBytes != kHeader) {
		error("Not a precompiled game matrix");
	}
    
	Common::String ss;
	bool inDefineRects = false;
	for (it += kHeader.size() ; it != buffer.end() ; ) {
		unsigned char byte = *it++;
		if (byte == kCodeString) {
			unsigned char len = *it++;
			Common::String s((const char *)it,(const char *)it+len);
			it += len;
			ss += Common::String::format("\"%s\"",  s.c_str());
		} else if (byte == kCodeShortLiteral || byte == kCodeShortId) {
			unsigned char b1 = *it++;
			unsigned char b2 = *it++;
			unsigned int number = mac ? b2 + (b1 << 8) : b1 + (b2 << 8);
			if (byte == kCodeShortId) ss += "k";
			ss += Common::String::format("%d",  number);
		} else if (byte == kCodeRect && inDefineRects) {
			ss += "RECT"; // override CRect
		} else if (byte <= kCodeShortId && strlen(kCodeTable[byte]) > 0) {
			ss += kCodeTable[byte];
		} else {
			error("Unknown byte code");
		}

		if (byte == kCodeRects) {
			inDefineRects = true;
		} else if (byte == kCodeBraceClose && inDefineRects) {
			inDefineRects = false;
		}
	}
	_result = ss;
}

Common::String Decompiler::getResult() const {
	return _result;
}

}