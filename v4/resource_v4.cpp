#include "stdafx.h"
#include "scumm.h"

void Scumm_v4::loadCharset(int no) {
	uint32 size;

	checkRange(4 ,0 ,no , "Loading illegal charset %d");
	openRoom(-1);

	openRoom(900+no);

	size = fileReadDwordLE();

	fileRead(_fileHandle, createResource(6, no, size), size);
	openRoom(-1);
}
