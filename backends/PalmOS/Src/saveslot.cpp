//#include "extras.h"
#include <string.h>
#include "scummsys.h"
#include "saveload.h"

#define	MAX_BLOCK 64000

enum SaveLoadState {
	STATE_LOAD,
	STATE_SAVE,
	STATE_NONE
};

class SaveLoadData {
	public:
		SaveLoadData();

		SaveLoadState _currentState;
		byte * _readWriteData;
		uint32 _readWritePos;
		bool _needDump;
};

SaveLoadData current;

SaveLoadData::SaveLoadData()
{
	_currentState = STATE_NONE;
	_readWritePos = 0;
	_needDump = false;
}

bool SerializerStream::fopen(const char *filename, const char *mode)
{
	if (current._currentState != STATE_NONE)
		fclose();
		
	context = ::fopen(filename, mode);
	if (context != NULL) {
		current._currentState = ((strcmp(mode,"rb")==0) ? STATE_LOAD : STATE_SAVE);
		return true;
	}

	return false;
}

int SerializerStream::fread(void *buf, int size, int cnt)
{
	return ::fread(buf, size, cnt, (FILE *)context);
}

int SerializerStream::fwrite(void *buf, int size, int cnt) {

	int fullsize = size*cnt;

	if (current._currentState == STATE_SAVE && fullsize<=MAX_BLOCK)
	{
		if (!current._readWriteData)
			current._readWriteData = (byte *)malloc(MAX_BLOCK);

		if ((current._readWritePos+fullsize)>MAX_BLOCK) {
			::fwrite(current._readWriteData, current._readWritePos, 1, (FILE *)context);
			current._readWritePos = 0;
			current._needDump = false;
		}
			
		memcpy(current._readWriteData + current._readWritePos, buf, fullsize);
		current._readWritePos += fullsize;
		current._needDump = true;

		return cnt;
	}

	return ::fwrite(buf, size, cnt, (FILE *)context);
}


void SerializerStream::fclose()
{
	if (current._needDump && current._readWriteData != NULL) {
		if (current._currentState == STATE_SAVE)
			::fwrite(current._readWriteData, current._readWritePos, 1, (FILE *)context);

		free(current._readWriteData);
		current._readWriteData = NULL;
	}

	current._readWritePos = 0;
	current._needDump = false;
	current._currentState = STATE_NONE;
	::fclose((FILE *)context);
}