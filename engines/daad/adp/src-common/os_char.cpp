#include <os_char.h>

uint8_t DDB_Char2ISO[128] = 
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,								// Shades
	
	170,	// ª
	161,	// ¡
	191,	// ¿
	171,	// «
	187,	// »
	225,	// á
	233,	// é
	237,	// í
	243,	// ó
	250,	// ú
	241,	// ñ
	209,	// Ñ
	231,	// ç
	199,	// Ç
	252,	// ü
	220,	// Ü
	
	 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,		//  !"#$%&'()*+,-./
	 48,  49,  50,  51,  52,  53,  54,  55,	 56,  57,  58,  59,  60,  61,  62,  63,		// 0123456789:;<=>?
	 64,  65,  66,  67,  68,  69,  70,  71,	 72,  73,  74,  75,  76,  77,  78,  79,		// @ABCDEFGHIJKLMNO
	 80,  81,  82,  83,  84,  85,  86,  87,	 88,  89,  90,  91,  92,  93,  94,  95,		// PQRSTUVWXYZ[\]^_
	 96,  97,  98,  99, 100, 101, 102, 103,	104, 105, 106, 107, 108, 109, 110, 111,		// `abcdefghijklmno
	112, 113, 114, 115, 116, 117, 118, 119,	120, 121, 122, 123, 124, 125, 126, 127		// pqrstuvwxyz{|}~
};

uint8_t DDB_ISO2Char[256] =
{
   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  17,   0,   0,   0,   0,   0,   0,   0,   0,  16,  19,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  20,   0,   0,   0,  18,
   0,   0,   0,   0,   0,   0,   0,  29,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  27,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  31,   0,   0,   0,
   0,  21,   0,   0,   0,   0,   0,  28,   0,  22,   0,   0,   0,  23,   0,   0,
   0,  26,   0,  24,   0,   0,   0,   0,   0,   0,  25,   0,  30,   0,   0,   0,
};

static bool DDB_EncodeUTF8(uint32_t codepoint, char* buffer, size_t size)
{
	if (buffer == 0 || size == 0)
		return false;

	if (codepoint <= 0x7F)
	{
		if (size < 2) return false;
		buffer[0] = (char)codepoint;
		buffer[1] = 0;
		return true;
	}
	if (codepoint <= 0x7FF)
	{
		if (size < 3) return false;
		buffer[0] = (char)(0xC0 | ((codepoint >> 6) & 0x1F));
		buffer[1] = (char)(0x80 | (codepoint & 0x3F));
		buffer[2] = 0;
		return true;
	}
	if (codepoint <= 0xFFFF)
	{
		if (size < 4) return false;
		buffer[0] = (char)(0xE0 | ((codepoint >> 12) & 0x0F));
		buffer[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
		buffer[2] = (char)(0x80 | (codepoint & 0x3F));
		buffer[3] = 0;
		return true;
	}
	return false;
}

const char* DDB_CharToUTF8(uint8_t ch)
{
	static char buffers[8][5];
	static int nextBuffer = 0;

	uint32_t codepoint = 0;
	bool known = true;

	if (ch < 128)
	{
		codepoint = DDB_Char2ISO[ch];
	}
	else
	{
		switch (ch)
		{
			case 127: codepoint = 0x00DF; break;
			case 144: codepoint = 0x00E0; break;
			case 145: codepoint = 0x00E3; break;
			case 146: codepoint = 0x00E4; break;
			case 147: codepoint = 0x00E2; break;
			case 148: codepoint = 0x00E8; break;
			case 149: codepoint = 0x00EB; break;
			case 150: codepoint = 0x00EA; break;
			case 151: codepoint = 0x00EC; break;
			case 152: codepoint = 0x00EF; break;
			case 154: codepoint = 0x00EE; break;
			case 155: codepoint = 0x00F2; break;
			case 156: codepoint = 0x00F5; break;
			case 157: codepoint = 0x00F4; break;
			case 158: codepoint = 0x00F9; break;
			case 159: codepoint = 0x00FB; break;
			case 160: codepoint = 0x00C0; break;
			case 161: codepoint = 0x00C3; break;
			case 162: codepoint = 0x00C4; break;
			case 163: codepoint = 0x00C2; break;
			case 164: codepoint = 0x00C8; break;
			case 165: codepoint = 0x00CB; break;
			case 166: codepoint = 0x00CA; break;
			case 167: codepoint = 0x00CC; break;
			case 168: codepoint = 0x00CF; break;
			case 169: codepoint = 0x00CE; break;
			case 170: codepoint = 0x00D2; break;
			case 171: codepoint = 0x00D5; break;
			case 172: codepoint = 0x00D6; break;
			case 173: codepoint = 0x00D4; break;
			case 174: codepoint = 0x00D9; break;
			case 175: codepoint = 0x00DB; break;
			case 186: codepoint = 0x00FD; break;
			case 187: codepoint = 0x00DD; break;
			case 188: codepoint = 0x00FE; break;
			case 189: codepoint = 0x00DE; break;
			case 190: codepoint = 0x00E5; break;
			case 191: codepoint = 0x00C5; break;
			case 221: codepoint = 0x00F0; break;
			case 222: codepoint = 0x00D0; break;
			case 223: codepoint = 0x00F8; break;
			case 224: codepoint = 0x00D8; break;
			case 226: codepoint = 0x20AC; break;
			case 251: codepoint = 0x00C1; break;
			case 252: codepoint = 0x00C9; break;
			case 253: codepoint = 0x00CD; break;
			case 254: codepoint = 0x00D3; break;
			case 255: codepoint = 0x00DA; break;
			default: known = false; break;
		}
	}

	if (!known)
		return 0;

	char* out = buffers[nextBuffer];
	nextBuffer = (nextBuffer + 1) & 7;
	if (!DDB_EncodeUTF8(codepoint, out, sizeof(buffers[0])))
		return 0;
	return out;
}
