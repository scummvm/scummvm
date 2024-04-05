#include "xglobal.h"
#include "WBuffer.h"
#include <string>

#define SET_LO(rf, data) rf = (data & 0x3F) | 0x80; data >>= 6;
#define SET_HI(rf, data) rf <<= 6; rf |= (data & 0x3F);

void ucs2_to_utf8(const wchar_t* ucs2, int size16, char* utf8, int size8)
{
	int cur;
	while(size16-- && (cur = *ucs2++)){
		if(cur <= 0x007F){
			if(size8 > 0){
				*utf8++ = (char)cur;
				--size8;
			}
			else
				break;
		}
		else if(cur <= 0x07FF){
			if(size8 > 1){
				SET_LO(utf8[1], cur);
				utf8[0] = cur | 0xC0;
				utf8 += 2;
				size8 -= 2;
			}
			else
				break;
		}
		else if(cur < 0xD800 || cur > 0xDBFF || *ucs2 < 0xDC00 || *ucs2 > 0xDFFF || size16 == 0){
			if(size8 > 2){
				SET_LO(utf8[2], cur);
				SET_LO(utf8[1], cur);
				utf8[0] = cur | 0xE0;
				utf8 += 3;
				size8 -= 3;
			}
			else
				break;
		}
		else {
			if(size8 > 3){
				unsigned int seq = (cur - 0xD800) << 10;
				seq |= (*ucs2 - 0xDC00) & 0x3FF;
				seq += 0x10000;
				SET_LO(utf8[3], seq);
				SET_LO(utf8[2], seq);
				SET_LO(utf8[1], seq);
				utf8[0] = seq | 0xF0;
				utf8 += 4;
				size8 -= 4;
			}
			else
				break;
			++ucs2;
			--size16;
		}
	}
	
	if(size8 > 0)
		*utf8 = 0;
}

void utf16_to_utf8(const wchar_t* utf16, int size16, char* utf8, int size8)
{
	int cur;
	while(size16-- && (cur = *utf16++)){
		if(cur <= 0x007F){
			if(size8 > 0){
				*utf8++ = (char)cur;
				--size8;
			}
			else
				break;
		}
		else if(cur <= 0x07FF){
			if(size8 > 1){
				SET_LO(utf8[1], cur);
				utf8[0] = cur | 0xC0;
				utf8 += 2;
				size8 -= 2;
			}
			else
				break;
		}
		else if(cur < 0xFFFE /*&& (cur < 0xFDD0 || cur > 0xFDEF)*/){
			if(cur < 0xD800 || cur > 0xDFFF){
				if(size8 > 2){
					SET_LO(utf8[2], cur);
					SET_LO(utf8[1], cur);
					utf8[0] = cur | 0xE0;
					utf8 += 3;
					size8 -= 3;
				}
				else
					break;
			}
			else if(cur >= 0xD800 && cur <= 0xDBFF && *utf16 >= 0xDC00 && *utf16 <= 0xDFFF && size16 > 0){
				//HI = (UTF32 Ц 0x10000) / 0x400 + 0xD800;
				//LO = (UTF32 Ц 0x10000) % 0x400 + 0xDC00;
				unsigned int seq = (cur - 0xD800) << 10;
				seq |= (*utf16 - 0xDC00) & ((1 << 10) - 1);
				seq += 0x10000;
				if(size8 > 3){
					SET_LO(utf8[3], seq);
					SET_LO(utf8[2], seq);
					SET_LO(utf8[1], seq);
					utf8[0] = seq | 0xF0;
					utf8 += 4;
					size8 -= 4;
				}
				else
					break;
				++utf16;
				--size16;
			}
			else {
				// ѕо стандарту тут об€зательно надо генерировать ошибку.
			}
		}
	}

	if(size8 > 0)
		*utf8 = 0;
}

void utf32_to_utf8(const unsigned int* utf32, int size32, char* utf8, int size8)
{
	unsigned int cur;
	while(size32-- && (cur = *utf32++)){
		if(cur <= 0x007F){
			if(size8 > 0){
				*utf8++ = (char)cur;
				--size8;
			}
			else
				break;
		}
		else if(cur <= 0x07FF){
			if(size8 > 1){
				SET_LO(utf8[1], cur);
				utf8[0] = cur | 0xC0;
				utf8 += 2;
				size8 -= 2;
			}
			else
				break;
		}
		else if(cur <= 0xFFFF){
			if(size8 > 2 && (cur > 0xDFFF || cur < 0xD800)){
				SET_LO(utf8[2], cur);
				SET_LO(utf8[1], cur);
				utf8[0] = cur | 0xE0;
				utf8 += 3;
				size8 -= 3;
			}
			else
				break;
		}
		else if(cur <= 0x0010FFFF){
			if(size8 > 3){
				SET_LO(utf8[3], cur);
				SET_LO(utf8[2], cur);
				SET_LO(utf8[1], cur);
				utf8[0] = cur | 0xF0;
				utf8 += 4;
				size8 -= 4;
			}
			else
				break;
		}
	}

	if(size8 > 0)
		*utf8 = 0;
}


void utf8_to_utf16(const char* utf8, int size8, wchar_t* utf16, int size16)
{
	unsigned char cur;
	while(size16 > 0 && size8 > 0 && (cur = *utf8)){
		if((cur & 0xC0) == 0x80){
			++utf8;
			--size8;
			continue;
		}
		else if(cur <= 0x007F){
			*utf16 = (wchar_t)cur;
			++utf8;
			--size8;
		}
		else if(cur <= 0xDF){
			if(size8 > 1){
				*utf16 = (cur & 0x1F);
				SET_HI(*utf16, utf8[1]);
				utf8 += 2;
				size8 -= 2;
			}
			else
				break;
		}
		else if(cur <= 0xEF){
			if(size8 > 2){
				*utf16 = (cur & 0x0F);
				SET_HI(*utf16, utf8[1]);
				SET_HI(*utf16, utf8[2]);
				utf8 += 3;
				size8 -= 3;
			}
			else
				break;
		}
		else if(cur <= 0xF7){
			unsigned int sc;
			if(size8 > 3){
				sc = (cur & 0x07);
				SET_HI(sc, utf8[1]);
				SET_HI(sc, utf8[2]);
				SET_HI(sc, utf8[3]);
				utf8 += 4;
				size8 -= 4;
			}
			else
				break;
			if(sc >= 0x10000){  // суррогатна€ пара
				if(size16 > 1){
					utf16[0] = ((sc - 0x10000) >> 10) + 0xD800;
					utf16[1] = ((sc - 0x10000) & ((1 << 10) - 1)) + 0xDC00;
					utf16 += 2;
					size16 -= 2;
					continue;
				}
				else
					break;
			}
			else {
				// по стандарту 3.1 такое можно игнорировать
			}
		}
		else {
			++utf8;
			--size8;
			continue;		
		}
		if(*utf16 < 0xFFFE){
			--size16;
			++utf16;
		}
	}

	if(size16 > 0)
		*utf16 = 0;
}

void utf8_to_utf32(const char* utf8, int size8, unsigned int* utf32, int size16)
{
	unsigned char cur;
	while(size16 > 0 && size8 > 0 && (cur = *utf8)){
		if((cur & 0xC0) == 0x80){
			++utf8;
			--size8;
			continue;
		}
		else if(cur <= 0x007F){
			*utf32 = cur;
			++utf8;
			--size8;
		}
		else if(cur <= 0xDF){
			if(size8 > 1){
				*utf32 = (cur & 0x1F);
				SET_HI(*utf32, utf8[1]);
				utf8 += 2;
				size8 -= 2;
			}
			else
				break;
		}
		else if(cur <= 0xEF){
			if(size8 > 2){
				*utf32 = (cur & 0x0F);
				SET_HI(*utf32, utf8[1]);
				SET_HI(*utf32, utf8[2]);
				utf8 += 3;
				size8 -= 3;
			}
			else
				break;
		}
		else if(cur <= 0xF7){
			if(size8 > 3){
				*utf32 = (cur & 0x07);
				SET_HI(*utf32, utf8[1]);
				SET_HI(*utf32, utf8[2]);
				SET_HI(*utf32, utf8[3]);
				utf8 += 4;
				size8 -= 4;
			}
			else
				break;
		}
		else {
			++utf8;
			--size8;
			continue;		
		}
		if(*utf32 <= 0x0010FFFF && (*utf32 > 0xDFFF || *utf32 < 0xD800)){
			--size16;
			++utf32;
		}
	}

	if(size16 > 0)
		*utf32 = 0;
}


size_t getUTF8size(const wchar_t* ucs2)
{
	size_t size = 1;
	for(; *ucs2; ++ucs2)
		if(*ucs2 <= 0x007F)
			++size;
		else if(*ucs2 <= 0x07FF)
			size += 2;
		else
			size += 4;

	return size + size % 4;
}

const char* toUTF8(XBuffer& buf, const std::wstring& ucs2)
{
	if(ucs2.empty()){
		buf.init();
		return buf.c_str();
	}

	size_t size = getUTF8size(ucs2.c_str());

	if(buf.size() < size)
		buf.alloc(size);

	ucs2_to_utf8(ucs2.c_str(), ucs2.size(), buf.buffer(), size - 1);
	buf.buffer()[size - 1] = 0;

	return buf.c_str();
}

const char* toUTF8(XBuffer& buf, const wchar_t* ucs2)
{
	if(!ucs2 || !*ucs2){
		buf.init();
		return buf.c_str();
	}

	size_t size = getUTF8size(ucs2);

	if(buf.size() < size)
		buf.alloc(size);

	ucs2_to_utf8(ucs2, wcslen(ucs2), buf.buffer(), size - 1);
	buf.buffer()[size - 1] = 0;

	return buf.c_str();
}

size_t getUTF16size(const char* utf8)
{
	size_t size = 1;
	unsigned char cur;
	for(;cur = *utf8; ++utf8){
		if((cur & 0xC0) == 0x80)
			continue;
		else if(cur <= 0x00EF)
			++size;
		else if(cur <= 0xF7){
			++size;
			unsigned int sc = (cur & 0x07);
			SET_HI(sc, utf8[1]);
			SET_HI(sc, utf8[2]);
			SET_HI(sc, utf8[3]);
			if(sc >= 0x10000) // суррогатна€ пара
				++size;
		}
	}
	
	return size + size % 2;
}

const wchar_t* fromUTF8(WBuffer& buf, const std::string& utf8)
{
	if(utf8.empty()){
		buf.init();
		return buf.c_str();
	}

	size_t size = getUTF16size(utf8.c_str());

	if(buf.size() < size * sizeof(wchar_t))
		buf.alloc(size * sizeof(wchar_t));

	wchar_t* ptrbuf = reinterpret_cast<wchar_t*>(buf.buffer());

	utf8_to_utf16(utf8.c_str(), utf8.size(), ptrbuf, size - 1);
	ptrbuf[size - 1] = L'\0';

	return buf.c_str();
}

const wchar_t* fromUTF8(WBuffer& buf, const char* utf8)
{
	if(!utf8 || !*utf8){
		buf.init();
		return buf.c_str();
	}

	size_t size = getUTF16size(utf8);

	if(buf.size() < size * sizeof(wchar_t))
		buf.alloc(size * sizeof(wchar_t));

	wchar_t* ptrbuf = reinterpret_cast<wchar_t*>(buf.buffer());

	utf8_to_utf16(utf8, strlen(utf8), ptrbuf, size - 1);
	ptrbuf[size - 1] = L'\0';

	return buf.c_str();
}