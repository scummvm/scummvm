/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef _DCPACKAGE_H_
#define _DCPACKAGE_H_


#define PACKAGE_MAGIC_1   0xDEC0ADDE
#define PACKAGE_MAGIC_2   0x4B4E554A    // "JUNK"
#define PACKAGE_VERSION   0x00000200
#define PACKAGE_EXTENSION "dcp"

#include <time.h>

namespace WinterMute {

typedef struct {
	uint32 Magic1;
	uint32 Magic2;
	uint32 PackageVersion;
	uint32 GameVersion;
	byte Priority;
	byte CD;
	bool MasterIndex;
#ifdef __WIN32__
	__time32_t CreationTime;
#else
	time_t CreationTime;
#endif
	char Desc[100];
	uint32 NumDirs;
} TPackageHeader;

/*
v2:  uint32 DirOffset


Dir: byte NameLength
     char Name [NameLength]
     byte CD;
     uint32 NumEntries


Entry: byte NameLength
       char Name [NameLength]
       uint32 Offset
       uint32 Length
       uint32 CompLength
       uint32 Flags
v2:    uint32 TimeDate1
       uint32 TimeDate2  // not used

*/

} // end of namespace WinterMute

#endif // _DCPACKAGE_H_
