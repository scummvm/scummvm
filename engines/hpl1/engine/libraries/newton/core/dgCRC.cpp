/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "dgStdafx.h"
#include "dgCRC.h"
#include "dgList.h"
#include "dgTree.h"
#include "dgDebug.h"
#include "dgRandom.h"

namespace InternalCRC {
const dgInt32 STRING_POOL_SIZE = 1024 * 8 - 256;

static dgUnsigned32 randBits[] = {
	0x00000001, 0x2C11F801, 0xDFD8F60E, 0x6C8FA2B7, 0xB573754C, 0x1522DCDD,
	0x21615D3A, 0xE1B307F3, 0x12AFA158, 0x53D18179, 0x70950126, 0x941702EF,
	0x756FE824, 0x694801D5, 0x36DF4DD2, 0x63D80FAB, 0xB8AE95B0, 0x902439F1,
	0x090C6F3E, 0x2B7C6A27, 0x8344B5FC, 0x67D3C5CD, 0x22F5516A, 0x2FB00E63,
	0xFC761508, 0x87B00169, 0x27EBA056, 0x8CC0B85F, 0xE33D3ED4, 0x95DA08C5,
	0x13E5C802, 0x9DD9E41B, 0xD4577F60, 0x3DD6B7E1, 0x096AF46E, 0x1A00CD97,
	0x4B10E2AC, 0x22EAAABD, 0x683F119A, 0x62D070D3, 0xA8D034B8, 0xAA363D59,
	0x58CECB86, 0x40F589CF, 0x4F630184, 0x38918BB5, 0xB85B8E32, 0x0A6A948B,
	0x9A099510, 0x402871D1, 0x11E7859E, 0xEE73CD07, 0x4142FB5C, 0x39D68BAD,
	0x0FE19DCA, 0xE35B2F43, 0x75590068, 0x66433549, 0x929182B6, 0x71EC773F,
	0xBBAC3034, 0xF2BD8AA5, 0x5743A062, 0x5AB120FB, 0x5ABFD6C0, 0xDDD867C1,
	0xDC3522CE, 0xD0EC6877, 0xE106000C, 0xB7C6689D, 0xED3FF5FA, 0xC75749B3,
	0x126B7818, 0x1A75E939, 0x0546C5E6, 0x8A9C80AF, 0x48A3CAE4, 0x756D0595,
	0x7060FE92, 0xA594896B, 0x12354470, 0x896599B1, 0xDAC6CBFE, 0xCB419FE7,
	0x9C44F0BC, 0xAFA9418D, 0xB87D1A2A, 0x428BC023, 0x33229BC8, 0xC92D5929,
	0xB1C19516, 0x0FBCA61F, 0xE594D194, 0x716EFC85, 0x0036A8C2, 0xD7BBCDDB,
	0x16E4DE20, 0xD10F07A1, 0x68CF812E, 0x390A7357, 0x8BAACD6C, 0x2C2E167D,
	0x3E7C0A5A, 0x167F9293, 0x3D596B78, 0x08888519, 0x9994F046, 0x0FC3E78F,
	0x008A4444, 0x87526F75, 0xB0079EF2, 0x238DEE4B, 0xCA09A3D0, 0x4ED3B191,
	0xFA42425E, 0x379DE2C7, 0x1EA2961C, 0x1FC3E76D, 0x90DFC68A, 0x0279C103,
	0xF9AAE728, 0xF2666D09, 0xEF13D776, 0x92E944FF, 0x364F22F4, 0x37665E65,
	0x05D6E122, 0x7131EABB, 0x479E9580, 0x98729781, 0x4BD20F8E, 0x1612EE37,
	0xCB574ACC, 0x5499B45D, 0x360B4EBA, 0x33814B73, 0x43720ED8, 0x146610F9,
	0x45514AA6, 0x0B23BE6F, 0x026E6DA4, 0xD1B9C955, 0x94676F52, 0xCE8EC32B,
	0x165EB330, 0x2F6AB971, 0x92F1E8BE, 0xC54095A7, 0xBEB3EB7C, 0x5C9E7D4D,
	0x5921A2EA, 0xB45D31E3, 0xB8C9E288, 0x5FE670E9, 0xC02049D6, 0xC42A53DF,
	0x6F332454, 0x661BB045, 0x2B3C4982, 0xDF4B779B, 0xD7C4FCE0, 0x70FB1761,
	0xADD4CDEE, 0x47BDD917, 0x8C63782C, 0x8181423D, 0xFA05C31A, 0xDD947453,
	0x6A8D6238, 0x1A068CD9, 0x4413D506, 0x5374054F, 0xC5A84704, 0xB41B1335,
	0x06986FB2, 0x4CCF080B, 0xF80C7290, 0x8622B151, 0x536DBF1E, 0x21E1B887,
	0xDED0F0DC, 0xB4B1032D, 0x1D5AAF4A, 0xC56E12C3, 0x8C578DE8, 0xCBA564C9,
	0xA67EEC36, 0x0837D2BF, 0x3D98D5B4, 0x1B06F225, 0xFF7EE1E2, 0x3640747B,
	0x5E301440, 0x53A08741, 0x436FBC4E, 0xC9C333F7, 0x2727558C, 0x7F5CC01D,
	0xFC83677A, 0xAFF10D33, 0x24836598, 0x3161F8B9, 0xDD748F66, 0x5B6CBC2F,
	0xAD8FD064, 0x89EE4D15, 0xBBB2A012, 0xA086BCEB, 0x1BEAE1F0, 0x69F39931,
	0x764DC57E, 0x17394B67, 0x4D51A63C, 0xF273790D, 0x35A2EBAA, 0x7EE463A3,
	0xBC2BE948, 0x2B9B48A9, 0x2FC7BE96, 0x5FC9C19F, 0x3AD83714, 0x6FA02405,
	0xDDB6AA42, 0xE648E15B, 0x1DB7DBA0, 0xF55AE721, 0x4D3ADAAE, 0xB3DAFED7,
	0x5FFAE2EC, 0x96A42DFD, 0xFB9C3BDA, 0x21CF1613, 0x0F2C18F8, 0xAE705499,
	0x650B79C6, 0x31C5E30F, 0x097D09C4, 0xAAAB76F5, 0x34CE0072, 0x27EDE1CB,
	0xDAD20150, 0xADD57111, 0xC229FBDE, 0x8AFF4E47, 0x448E0B9C, 0x5C5DDEED,
	0x4612580A, 0x05F82483, 0xBC1EF4A8, 0xB1C01C89, 0xF592C0F6, 0x6798207F,
	0xEC494874, 0x795F45E5, 0xECFBA2A2, 0xBB9CBE3B, 0xF567104f, 0x47289407,
	0x25683fa6, 0x2fde5836,
};

struct StringPool {
	char buff[STRING_POOL_SIZE];

	StringPool() {
	}

	StringPool(const StringPool &arg) {
	}
};

/*
 class CRCStringLookup: public dgTree<char *, dgUnsigned32>, public dgList<StringPool>
 {

 dgInt32 size;
 char *ptr;

 void AddContainer ()
 {
 StringPool tmp;
 dgListNode *node;

 //         Addtop (tmp);
 //         node = GetFirst();
 node = Addtop (tmp);

 ptr = node->GetInfo().buff;
 size = STRING_POOL_SIZE;
 }

 public:

 CRCStringLookup ()
 :dgTree<char *, dgUnsigned32>(NULL), dgList<StringPool>()
 {
 AddContainer();
 }

 ~CRCStringLookup ()
 {
 }


 void AddString (dgUnsigned32 crc, const char *string)
 {
 dgInt32 length;

 if (dgTree<char *, dgUnsigned32>::Find(crc)) {
 return;
 }

 length = dgInt32 (strlen (string)) + 1;
 if (size < length) {
 AddContainer();
 }

 strcpy (ptr, string);
 Insert (ptr, crc);

 ptr += length;
 size -= length;
 }

 const char* FindString (dgUnsigned32 crc) const
 {
 dgTreeNode *node;

 node = dgTree<char *, dgUnsigned32>::Find(crc);
 //         return node ? node->GetInfo() : NULL;
 return node ? node->GetInfo() : NULL;
 }
 };

 static CRCStringLookup& GetDatabase ()
 {
 static CRCStringLookup database;
 return database;
 }
 */

/*
 dgUnsigned32 RSHash(char* str, dgUnsigned32 len)
 {
 dgUnsigned32 b    = 378551;
 dgUnsigned32 a    = 63689;
 dgUnsigned32 hash = 0;
 dgUnsigned32 i    = 0;

 for(i = 0; i < len; str++, i++)    {
 hash = hash * a + (*str);
 a    = a * b;
 }

 return (hash & 0x7FFFFFFF);
 }
 // End Of RS Hash Function


 dgUnsigned32 JSHash(char* str, dgUnsigned32 len)
 {
 dgUnsigned32 hash = 1315423911;
 dgUnsigned32 i    = 0;

 for(i = 0; i < len; str++, i++)
 {
 hash ^= ((hash << 5) + (*str) + (hash >> 2));
 }

 return (hash & 0x7FFFFFFF);
 }
 // End Of JS Hash Function


 dgUnsigned32 PJWHash(char* str, dgUnsigned32 len)
 {
 dgUnsigned32 BitsInUnsignedInt = (dgUnsigned32)(sizeof(dgUnsigned32) * 8);
 dgUnsigned32 ThreeQuarters     = (dgUnsigned32)((BitsInUnsignedInt  * 3) / 4);
 dgUnsigned32 OneEighth         = (dgUnsigned32)(BitsInUnsignedInt / 8);
 dgUnsigned32 HighBits          = (dgUnsigned32)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
 dgUnsigned32 hash              = 0;
 dgUnsigned32 test              = 0;
 dgUnsigned32 i                 = 0;

 for(i = 0; i < len; str++, i++)
 {
 hash = (hash << OneEighth) + (*str);

 if((test = hash & HighBits)  != 0)
 {
 hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
 }
 }

 return (hash & 0x7FFFFFFF);
 }
 // End Of  P. J. Weinberger Hash Function


 dgUnsigned32 ELFHash(char* str, dgUnsigned32 len)
 {
 dgUnsigned32 hash = 0;
 dgUnsigned32 x    = 0;
 dgUnsigned32 i    = 0;

 for(i = 0; i < len; str++, i++)
 {
 hash = (hash << 4) + (*str);
 if((x = hash & 0xF0000000L) != 0)
 {
 hash ^= (x >> 24);
 hash &= ~x;
 }
 }

 return (hash & 0x7FFFFFFF);
 }
 // End Of ELF Hash Function


 dgUnsigned32 BKDRHash(char* str, dgUnsigned32 len)
 {
 dgUnsigned32 seed = 131; // 31 131 1313 13131 131313 etc..
 dgUnsigned32 hash = 0;
 dgUnsigned32 i    = 0;

 for(i = 0; i < len; str++, i++)
 {
 hash = (hash * seed) + (*str);
 }

 return (hash & 0x7FFFFFFF);
 }
 // End Of BKDR Hash Function


 dgUnsigned32 SDBMHash(char* str, dgUnsigned32 len)
 {
 dgUnsigned32 hash = 0;
 dgUnsigned32 i    = 0;

 for(i = 0; i < len; str++, i++)
 {
 hash = (*str) + (hash << 6) + (hash << 16) - hash;
 }

 return (hash & 0x7FFFFFFF);
 }
 // End Of SDBM Hash Function


 dgUnsigned32 DEKHash(char* str, dgUnsigned32 len)
 {
 dgUnsigned32 hash = len;
 dgUnsigned32 i    = 0;

 for(i = 0; i < len; str++, i++)
 {
 hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
 }
 return (hash & 0x7FFFFFFF);
 }
 // End Of DEK Hash Function


 dgUnsigned32 APHash(char* str, dgUnsigned32 len)
 {
 dgUnsigned32 hash = 0;
 dgUnsigned32 i    = 0;

 for(i = 0; i < len; str++, i++)
 {
 hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ (*str) ^ (hash >> 3)) :
 (~((hash << 11) ^ (*str) ^ (hash >> 5)));
 }

 return (hash & 0x7FFFFFFF);
 }
 */

// End Of DJB Hash Function
dgUnsigned32 DJBHash(const char *const str, dgInt32 len) {
	dgUnsigned32 hash = 5381;
	for (dgInt32 i = 0; i < len; i++) {
		//hash = ((hash << 5) + hash) + (*str);
		hash = ((hash << 5) + hash) + str[i];
	}

	return (hash & 0x7FFFFFFF);
}
}

dgUnsigned32 dgApi dgCRC(const char *const name) {
//	dgUnsigned32 c;
//	dgUnsigned32 crc;
//	unsigned char *ptr;
//	dgUnsigned32 val;
	if (!name) {
		return 0;
	}

	dgUnsigned32 crc = 0;
	//for (ptr = (unsigned char*)name; *ptr; ptr ++) {
	for (dgInt32 i = 0; name[i]; i++) {
		char c = name[i];
		dgUnsigned32 val = InternalCRC::randBits[((crc >> 24) ^ c) & 0xff];
		crc = (crc << 8) ^ val;
	}

	NEWTON_ASSERT(0);
//	InternalCRC::GetDatabase().AddString (crc, name);
	return crc;
}

dgUnsigned32 dgApi dgCRC(const void *const buffer, dgInt32 size,
                         dgUnsigned32 crcAcc) {
	NEWTON_ASSERT(buffer);
	const unsigned char *const ptr = (const unsigned char *) buffer;
	for (dgInt32 i = 0; i < size; i++) {
		dgUnsigned32 c = ptr[i];
		dgUnsigned32 val = InternalCRC::randBits[((crcAcc >> 24) ^ c) & 0xff];
		crcAcc = (crcAcc << 8) ^ val;
	}
	return crcAcc;
}

const char *dgApi dgInverseCRC(dgUnsigned32 crc) {
	NEWTON_ASSERT(0);
	return NULL;
//	return InternalCRC::GetDatabase().FindString (crc);
}

dgUnsigned32 dgApi dgHash(const void *const string, int size) {
	return InternalCRC::DJBHash((const char *)string, size);
}
