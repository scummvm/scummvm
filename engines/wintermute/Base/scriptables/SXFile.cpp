/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
#define FORBIDDEN_SYMBOL_EXCEPTION_fopen
#define FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#define FORBIDDEN_SYMBOL_EXCEPTION_ftell
#define FORBIDDEN_SYMBOL_EXCEPTION_fseek
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#include "engines/wintermute/Sys/SysClassRegistry.h"
#include "engines/wintermute/Sys/SysClass.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/file/BFile.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/scriptables/SXFile.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CSXFile, false)

CBScriptable *makeSXFile(CBGame *inGame, CScStack *stack) {
	return new CSXFile(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
CSXFile::CSXFile(CBGame *inGame, CScStack *stack): CBScriptable(inGame) {
	stack->correctParams(1);
	CScValue *Val = stack->pop();

	_filename = NULL;
	if (!Val->isNULL()) CBUtils::setString(&_filename, Val->getString());

	_readFile = NULL;
	_writeFile = NULL;

	_mode = 0;
	_textMode = false;
}


//////////////////////////////////////////////////////////////////////////
CSXFile::~CSXFile() {
	cleanup();
}

//////////////////////////////////////////////////////////////////////////
void CSXFile::cleanup() {
	delete[] _filename;
	_filename = NULL;
	close();
}


//////////////////////////////////////////////////////////////////////////
void CSXFile::close() {
	if (_readFile) {
		Game->_fileManager->closeFile(_readFile);
		_readFile = NULL;
	}
	if ((FILE *)_writeFile) {
		fclose((FILE *)_writeFile);
		_writeFile = NULL;
	}
	_mode = 0;
	_textMode = false;
}

//////////////////////////////////////////////////////////////////////////
const char *CSXFile::scToString() {
	if (_filename) return _filename;
	else return "[file object]";
}

#define FILE_BUFFER_SIZE 32768
//////////////////////////////////////////////////////////////////////////
HRESULT CSXFile::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetFilename
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetFilename") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();
		cleanup();
		CBUtils::setString(&_filename, filename);
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OpenAsText / OpenAsBinary
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OpenAsText") == 0 || strcmp(name, "OpenAsBinary") == 0) {
		stack->correctParams(1);
		close();
		_mode = stack->pop()->getInt(1);
		if (_mode < 1 || _mode > 3) {
			script->runtimeError("File.%s: invalid access mode. Setting read mode.", name);
			_mode = 1;
		}
		if (_mode == 1) {
			_readFile = Game->_fileManager->openFile(_filename);
			if (!_readFile) {
				//script->runtimeError("File.%s: Error opening file '%s' for reading.", Name, _filename);
				close();
			} else _textMode = strcmp(name, "OpenAsText") == 0;
		} else {
			if (strcmp(name, "OpenAsText") == 0) {
				if (_mode == 2) _writeFile = fopen(_filename, "w+");
				else _writeFile = fopen(_filename, "a+");
			} else {
				if (_mode == 2) _writeFile = fopen(_filename, "wb+");
				else _writeFile = fopen(_filename, "ab+");
			}

			if (!_writeFile) {
				//script->runtimeError("File.%s: Error opening file '%s' for writing.", Name, _filename);
				close();
			} else _textMode = strcmp(name, "OpenAsText") == 0;
		}

		if (_readFile || _writeFile) stack->pushBool(true);
		else stack->pushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Close
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Close") == 0) {
		stack->correctParams(0);
		close();
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetPosition") == 0) {
		stack->correctParams(1);
		if (_mode == 0) {
			script->runtimeError("File.%s: File is not open", name);
			stack->pushBool(false);
		} else {
			int Pos = stack->pop()->getInt();
			stack->pushBool(setPos(Pos));
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Delete
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Delete") == 0) {
		stack->correctParams(0);
		close();
		stack->pushBool(CBPlatform::DeleteFile(_filename) != false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Copy
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Copy") == 0) {
		stack->correctParams(2);
		const char *Dest = stack->pop()->getString();
		bool Overwrite = stack->pop()->getBool(true);

		close();
		stack->pushBool(CBPlatform::CopyFile(_filename, Dest, !Overwrite) != false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadLine
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadLine") == 0) {
		stack->correctParams(0);
		if (!_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open in text mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		uint32 bufSize = FILE_BUFFER_SIZE;
		byte *buf = (byte *)malloc(bufSize);
		uint32 counter = 0;
		byte b;
		bool foundNewLine = false;
		HRESULT ret = E_FAIL;
		do {
			ret = _readFile->read(&b, 1);
			if (ret != 1) break;

			if (counter > bufSize) {
				buf = (byte *)realloc(buf, bufSize + FILE_BUFFER_SIZE);
				bufSize += FILE_BUFFER_SIZE;
			}
			if (b == '\n') {
				buf[counter] = '\0';
				foundNewLine = true;
				break;
			} else if (b == 0x0D) continue;
			else {
				buf[counter] = b;
				counter++;
			}
		} while (SUCCEEDED(ret));

		if (counter > bufSize) {
			buf = (byte *)realloc(buf, bufSize + FILE_BUFFER_SIZE);
			bufSize += FILE_BUFFER_SIZE;
		}
		buf[counter] = '\0';

		if (!foundNewLine && counter == 0) stack->pushNULL();
		else stack->pushString((char *)buf);

		free(buf);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadText") == 0) {
		stack->correctParams(1);
		int textLen = stack->pop()->getInt();

		if (!_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open in text mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		uint32 bufSize = FILE_BUFFER_SIZE;
		byte *buf = (byte *)malloc(bufSize);
		uint32 counter = 0;
		byte b;

		HRESULT ret = E_FAIL;
		while (counter < textLen) {
			ret = _readFile->read(&b, 1);
			if (ret != 1) break;

			if (counter > bufSize) {
				buf = (byte *)realloc(buf, bufSize + FILE_BUFFER_SIZE);
				bufSize += FILE_BUFFER_SIZE;
			}
			if (b == 0x0D) continue;
			else {
				buf[counter] = b;
				counter++;
			}
		}

		if (counter > bufSize) {
			buf = (byte *)realloc(buf, bufSize + FILE_BUFFER_SIZE);
			bufSize += FILE_BUFFER_SIZE;
		}
		buf[counter] = '\0';

		if (textLen > 0 && counter == 0) stack->pushNULL();
		else stack->pushString((char *)buf);

		free(buf);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteLine / WriteText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteLine") == 0 || strcmp(name, "WriteText") == 0) {
		stack->correctParams(1);
		const char *line = stack->pop()->getString();
		if (!_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in text mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		if (strcmp(name, "WriteLine") == 0)
			fprintf((FILE *)_writeFile, "%s\n", line);
		else
			fprintf((FILE *)_writeFile, "%s", line);

		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// ReadBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadBool") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		bool val;
		if (_readFile->read(&val, sizeof(bool)) == sizeof(bool)) stack->pushBool(val);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadByte") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		byte val;
		if (_readFile->read(&val, sizeof(byte)) == sizeof(byte)) stack->pushInt(val);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadShort") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		short val;
		if (_readFile->read(&val, sizeof(short)) == sizeof(short)) stack->pushInt(65536 + val);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadInt / ReadLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadInt") == 0 || strcmp(name, "ReadLong") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		int val;
		if (_readFile->read(&val, sizeof(int)) == sizeof(int)) stack->pushInt(val);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadFloat") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		float val;
		if (_readFile->read(&val, sizeof(float)) == sizeof(float)) stack->pushFloat(val);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadDouble") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		double val;
		if (_readFile->read(&val, sizeof(double)) == sizeof(double)) stack->pushFloat(val);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadString") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return S_OK;
		}
		uint32 size;
		if (_readFile->read(&size, sizeof(uint32)) == sizeof(uint32)) {
			byte *str = new byte[size + 1];
			if (str) {
				if (_readFile->read(str, size) == size) {
					str[size] = '\0';
					stack->pushString((char *)str);
				}
				delete [] str;
			} else stack->pushNULL();
		} else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteBool") == 0) {
		stack->correctParams(1);
		bool val = stack->pop()->getBool();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		fwrite(&val, sizeof(val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteByte") == 0) {
		stack->correctParams(1);
		byte val = stack->pop()->getInt();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		fwrite(&val, sizeof(val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteShort") == 0) {
		stack->correctParams(1);
		short val = stack->pop()->getInt();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		fwrite(&val, sizeof(val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteInt / WriteLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteInt") == 0 || strcmp(name, "WriteLong") == 0) {
		stack->correctParams(1);
		int val = stack->pop()->getInt();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		fwrite(&val, sizeof(val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteFloat") == 0) {
		stack->correctParams(1);
		float val = stack->pop()->getFloat();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		fwrite(&val, sizeof(val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteDouble") == 0) {
		stack->correctParams(1);
		double val = stack->pop()->getFloat();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}
		fwrite(&val, sizeof(val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteString") == 0) {
		stack->correctParams(1);
		const char *val = stack->pop()->getString();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return S_OK;
		}

		uint32 size = strlen(val);
		fwrite(&size, sizeof(size), 1, (FILE *)_writeFile);
		fwrite(val, size, 1, (FILE *)_writeFile);

		stack->pushBool(true);

		return S_OK;
	}


	else return CBScriptable::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXFile::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("file");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Filename (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Filename") == 0) {
		_scValue->setString(_filename);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Position (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Position") == 0) {
		_scValue->setInt(getPos());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Length (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Length") == 0) {
		_scValue->setInt(getLength());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TextMode") == 0) {
		_scValue->setBool(_textMode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccessMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccessMode") == 0) {
		_scValue->setInt(_mode);
		return _scValue;
	}

	else return CBScriptable::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXFile::scSetProperty(const char *name, CScValue *value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Length
	//////////////////////////////////////////////////////////////////////////
	if(strcmp(name, "Length")==0){
	    int OrigLength = _length;
	    _length = max(value->getInt(0), 0);

	    char PropName[20];
	    if(_length < OrigLength){
	        for(int i=_length; i<OrigLength; i++){
	            sprintf(PropName, "%d", i);
	            _values->DeleteProp(PropName);
	        }
	    }
	    return S_OK;
	}
	else*/ return CBScriptable::scSetProperty(name, value);
}

//////////////////////////////////////////////////////////////////////////
uint32 CSXFile::getPos() {
	if (_mode == 1 && _readFile)
		return _readFile->pos();
	else if ((_mode == 2 || _mode == 3) && _writeFile) return ftell((FILE *)_writeFile);
	else return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CSXFile::setPos(uint32 pos, TSeek origin) {
	if (_mode == 1 && _readFile)
		return _readFile->seek(pos, origin);
	else if ((_mode == 2 || _mode == 3) && _writeFile) return fseek((FILE *)_writeFile, pos, (int)origin) == 0;
	else return false;
}

//////////////////////////////////////////////////////////////////////////
uint32 CSXFile::getLength() {
	if (_mode == 1 && _readFile)
		return _readFile->size();
	else if ((_mode == 2 || _mode == 3) && _writeFile) {
		uint32 CurrentPos = ftell((FILE *)_writeFile);
		fseek((FILE *)_writeFile, 0, SEEK_END);
		int Ret = ftell((FILE *)_writeFile);
		fseek((FILE *)_writeFile, CurrentPos, SEEK_SET);
		return Ret;
	} else return 0;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CSXFile::persist(CBPersistMgr *persistMgr) {

	CBScriptable::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_filename));
	persistMgr->transfer(TMEMBER(_mode));
	persistMgr->transfer(TMEMBER(_textMode));

	uint32 pos = 0;
	if (persistMgr->_saving) {
		pos = getPos();
		persistMgr->transfer(TMEMBER(pos));
	} else {
		persistMgr->transfer(TMEMBER(pos));

		// try to re-open file if needed
		_writeFile = NULL;
		_readFile = NULL;

		if (_mode != 0) {
			// open for reading
			if (_mode == 1) {
				_readFile = Game->_fileManager->openFile(_filename);
				if (!_readFile)
					close();
			}
			// open for writing / appending
			else {
				if (_textMode) {
					if (_mode == 2)
						_writeFile = fopen(_filename, "w+");
					else
						_writeFile = fopen(_filename, "a+");
				} else {
					if (_mode == 2)
						_writeFile = fopen(_filename, "wb+");
					else
						_writeFile = fopen(_filename, "ab+");
				}
				if (_writeFile)
					close();
			}
			setPos(pos);
		}
	}

	return S_OK;
}

} // end of namespace WinterMute
