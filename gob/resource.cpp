/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#include "gob/gob.h"
#include "gob/global.h"
#include "gob/video.h"
#include "gob/resource.h"
#include "gob/debug.h"

namespace Gob {

char *resourceBuf = 0;

static char *resourcePtr;

void res_Free(void) {
	if (resourceBuf != 0) {
		free(resourceBuf);
		resourceBuf = 0;
	}
}

void res_Init(void) {
	int16 handle;
	int16 fileSize;
	struct stat statBuf;
	handle = open("ALL.ASK", O_RDONLY);
	if (handle < 0) {
		error("ALL.ASK is missing.");
	}
	if (stat("ALL.ASK", &statBuf) == -1)
		error("res_Init: Error with stat()\n");
	fileSize = statBuf.st_size;

	resourceBuf = (char *)malloc(fileSize * 4);
	read(handle, resourceBuf, fileSize);
	close(handle);
}

void res_Search(char resid) {
	int16 lang;
	if (resourceBuf == 0)
		res_Init();

	lang = (language == 5) ? 2 : language;

	resourcePtr = resourceBuf;
	while (*resourcePtr != '#') {
		if (resourcePtr[0] == '@' && resourcePtr[1] == resid &&
		    ((int16)lang + '0') == (int16)resourcePtr[2]) {
			resourcePtr += 5;
			return;

		}
		resourcePtr++;
	}
	resourcePtr = resourceBuf;
	while (resourcePtr[0] != '#') {
		if (resourcePtr[0] == '@' && resourcePtr[1] == resid) {
			resourcePtr += 5;
			return;
		}
		resourcePtr++;
	}
	return;
}

void res_Get(char *buf) {
	int16 i = 0;
	while (1) {
		if (*resourcePtr == '\r')
			resourcePtr++;

		if (*resourcePtr == '\n') {
			resourcePtr++;
			break;
		}

		buf[i] = *resourcePtr;
		i++;
		resourcePtr++;
	}
	buf[i] = 0;
}

} // End of namespace Gob
