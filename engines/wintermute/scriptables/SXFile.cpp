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

#include "engines/wintermute/SysClassRegistry.h"
#include "engines/wintermute/SysClass.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/utils.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BFile.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/scriptables/SXFile.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CSXFile, false)

//////////////////////////////////////////////////////////////////////////
CSXFile::CSXFile(CBGame *inGame, CScStack *Stack): CBScriptable(inGame) {
	Stack->CorrectParams(1);
	CScValue *Val = Stack->Pop();

	_filename = NULL;
	if (!Val->IsNULL()) CBUtils::SetString(&_filename, Val->GetString());

	_readFile = NULL;
	_writeFile = NULL;

	_mode = 0;
	_textMode = false;
}


//////////////////////////////////////////////////////////////////////////
CSXFile::~CSXFile() {
	Cleanup();
}

//////////////////////////////////////////////////////////////////////////
void CSXFile::Cleanup() {
	delete[] _filename;
	_filename = NULL;
	Close();
}


//////////////////////////////////////////////////////////////////////////
void CSXFile::Close() {
	if (_readFile) {
		Game->_fileManager->CloseFile(_readFile);
		_readFile = NULL;
	}
	if (_writeFile) {
		fclose(_writeFile);
		_writeFile = NULL;
	}
	_mode = 0;
	_textMode = false;
}

//////////////////////////////////////////////////////////////////////////
char *CSXFile::ScToString() {
	if (_filename) return _filename;
	else return "[file object]";
}

#define FILE_BUFFER_SIZE 32768
//////////////////////////////////////////////////////////////////////////
HRESULT CSXFile::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SetFilename
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetFilename") == 0) {
		Stack->CorrectParams(1);
		const char *Filename = Stack->Pop()->GetString();
		Cleanup();
		CBUtils::SetString(&_filename, Filename);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OpenAsText / OpenAsBinary
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "OpenAsText") == 0 || strcmp(Name, "OpenAsBinary") == 0) {
		Stack->CorrectParams(1);
		Close();
		_mode = Stack->Pop()->GetInt(1);
		if (_mode < 1 || _mode > 3) {
			Script->RuntimeError("File.%s: invalid access mode. Setting read mode.", Name);
			_mode = 1;
		}
		if (_mode == 1) {
			_readFile = Game->_fileManager->OpenFile(_filename);
			if (!_readFile) {
				//Script->RuntimeError("File.%s: Error opening file '%s' for reading.", Name, _filename);
				Close();
			} else _textMode = strcmp(Name, "OpenAsText") == 0;
		} else {
			if (strcmp(Name, "OpenAsText") == 0) {
				if (_mode == 2) _writeFile = fopen(_filename, "w+");
				else _writeFile = fopen(_filename, "a+");
			} else {
				if (_mode == 2) _writeFile = fopen(_filename, "wb+");
				else _writeFile = fopen(_filename, "ab+");
			}

			if (!_writeFile) {
				//Script->RuntimeError("File.%s: Error opening file '%s' for writing.", Name, _filename);
				Close();
			} else _textMode = strcmp(Name, "OpenAsText") == 0;
		}

		if (_readFile || _writeFile) Stack->PushBool(true);
		else Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Close
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Close") == 0) {
		Stack->CorrectParams(0);
		Close();
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetPosition") == 0) {
		Stack->CorrectParams(1);
		if (_mode == 0) {
			Script->RuntimeError("File.%s: File is not open", Name);
			Stack->PushBool(false);
		} else {
			int Pos = Stack->Pop()->GetInt();
			Stack->PushBool(SetPos(Pos));
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Delete
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Delete") == 0) {
		Stack->CorrectParams(0);
		Close();
		Stack->PushBool(CBPlatform::DeleteFile(_filename) != FALSE);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Copy
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Copy") == 0) {
		Stack->CorrectParams(2);
		const char *Dest = Stack->Pop()->GetString();
		bool Overwrite = Stack->Pop()->GetBool(true);

		Close();
		Stack->PushBool(CBPlatform::CopyFile(_filename, Dest, !Overwrite) != FALSE);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadLine
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadLine") == 0) {
		Stack->CorrectParams(0);
		if (!_textMode || !_readFile) {
			Script->RuntimeError("File.%s: File must be open in text mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		uint32 BufSize = FILE_BUFFER_SIZE;
		byte *Buf = (byte  *)malloc(BufSize);
		uint32 Counter = 0;
		byte b;
		bool FoundNewLine = false;
		HRESULT Ret = E_FAIL;
		do {
			Ret = _readFile->Read(&b, 1);
			if (FAILED(Ret)) break;

			if (Counter > BufSize) {
				Buf = (byte  *)realloc(Buf, BufSize + FILE_BUFFER_SIZE);
				BufSize += FILE_BUFFER_SIZE;
			}
			if (b == '\n') {
				Buf[Counter] = '\0';
				FoundNewLine = true;
				break;
			} else if (b == 0x0D) continue;
			else {
				Buf[Counter] = b;
				Counter++;
			}
		} while (SUCCEEDED(Ret));

		if (Counter > BufSize) {
			Buf = (byte  *)realloc(Buf, BufSize + FILE_BUFFER_SIZE);
			BufSize += FILE_BUFFER_SIZE;
		}
		Buf[Counter] = '\0';

		if (!FoundNewLine && Counter == 0) Stack->PushNULL();
		else Stack->PushString((char *)Buf);

		free(Buf);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadText") == 0) {
		Stack->CorrectParams(1);
		int TextLen = Stack->Pop()->GetInt();

		if (!_textMode || !_readFile) {
			Script->RuntimeError("File.%s: File must be open in text mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		uint32 BufSize = FILE_BUFFER_SIZE;
		byte *Buf = (byte  *)malloc(BufSize);
		uint32 Counter = 0;
		byte b;

		HRESULT Ret = E_FAIL;
		while (Counter < TextLen) {
			Ret = _readFile->Read(&b, 1);
			if (FAILED(Ret)) break;

			if (Counter > BufSize) {
				Buf = (byte  *)realloc(Buf, BufSize + FILE_BUFFER_SIZE);
				BufSize += FILE_BUFFER_SIZE;
			}
			if (b == 0x0D) continue;
			else {
				Buf[Counter] = b;
				Counter++;
			}
		}

		if (Counter > BufSize) {
			Buf = (byte  *)realloc(Buf, BufSize + FILE_BUFFER_SIZE);
			BufSize += FILE_BUFFER_SIZE;
		}
		Buf[Counter] = '\0';

		if (TextLen > 0 && Counter == 0) Stack->PushNULL();
		else Stack->PushString((char *)Buf);

		free(Buf);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteLine / WriteText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteLine") == 0 || strcmp(Name, "WriteText") == 0) {
		Stack->CorrectParams(1);
		const char *Line = Stack->Pop()->GetString();
		if (!_textMode || !_writeFile) {
			Script->RuntimeError("File.%s: File must be open for writing in text mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		if (strcmp(Name, "WriteLine") == 0)
			fprintf(_writeFile, "%s\n", Line);
		else
			fprintf(_writeFile, "%s", Line);

		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// ReadBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadBool") == 0) {
		Stack->CorrectParams(0);
		if (_textMode || !_readFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		bool Val;
		if (SUCCEEDED(_readFile->Read(&Val, sizeof(bool)))) Stack->PushBool(Val);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadByte") == 0) {
		Stack->CorrectParams(0);
		if (_textMode || !_readFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		byte Val;
		if (SUCCEEDED(_readFile->Read(&Val, sizeof(byte )))) Stack->PushInt(Val);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadShort") == 0) {
		Stack->CorrectParams(0);
		if (_textMode || !_readFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		short Val;
		if (SUCCEEDED(_readFile->Read(&Val, sizeof(short)))) Stack->PushInt(65536 + Val);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadInt / ReadLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadInt") == 0 || strcmp(Name, "ReadLong") == 0) {
		Stack->CorrectParams(0);
		if (_textMode || !_readFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		int Val;
		if (SUCCEEDED(_readFile->Read(&Val, sizeof(int)))) Stack->PushInt(Val);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadFloat") == 0) {
		Stack->CorrectParams(0);
		if (_textMode || !_readFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		float Val;
		if (SUCCEEDED(_readFile->Read(&Val, sizeof(float)))) Stack->PushFloat(Val);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadDouble") == 0) {
		Stack->CorrectParams(0);
		if (_textMode || !_readFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		double Val;
		if (SUCCEEDED(_readFile->Read(&Val, sizeof(double)))) Stack->PushFloat(Val);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadString") == 0) {
		Stack->CorrectParams(0);
		if (_textMode || !_readFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		uint32 Size;
		if (SUCCEEDED(_readFile->Read(&Size, sizeof(uint32)))) {
			byte *Str = new byte[Size + 1];
			if (Str) {
				if (SUCCEEDED(_readFile->Read(Str, Size))) {
					Str[Size] = '\0';
					Stack->PushString((char *)Str);
				}
				delete [] Str;
			} else Stack->PushNULL();
		} else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteBool") == 0) {
		Stack->CorrectParams(1);
		bool Val = Stack->Pop()->GetBool();

		if (_textMode || !_writeFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, _writeFile);
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteByte") == 0) {
		Stack->CorrectParams(1);
		byte Val = Stack->Pop()->GetInt();

		if (_textMode || !_writeFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, _writeFile);
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteShort") == 0) {
		Stack->CorrectParams(1);
		short Val = Stack->Pop()->GetInt();

		if (_textMode || !_writeFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, _writeFile);
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteInt / WriteLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteInt") == 0 || strcmp(Name, "WriteLong") == 0) {
		Stack->CorrectParams(1);
		int Val = Stack->Pop()->GetInt();

		if (_textMode || !_writeFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, _writeFile);
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteFloat") == 0) {
		Stack->CorrectParams(1);
		float Val = Stack->Pop()->GetFloat();

		if (_textMode || !_writeFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, _writeFile);
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteDouble") == 0) {
		Stack->CorrectParams(1);
		double Val = Stack->Pop()->GetFloat();

		if (_textMode || !_writeFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, _writeFile);
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteString") == 0) {
		Stack->CorrectParams(1);
		const char *Val = Stack->Pop()->GetString();

		if (_textMode || !_writeFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}

		uint32 Size = strlen(Val);
		fwrite(&Size, sizeof(Size), 1, _writeFile);
		fwrite(Val, Size, 1, _writeFile);

		Stack->PushBool(true);

		return S_OK;
	}


	else return CBScriptable::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXFile::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("file");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Filename (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Filename") == 0) {
		_scValue->SetString(_filename);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Position (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Position") == 0) {
		_scValue->SetInt(GetPos());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Length (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Length") == 0) {
		_scValue->SetInt(GetLength());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TextMode") == 0) {
		_scValue->SetBool(_textMode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccessMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccessMode") == 0) {
		_scValue->SetInt(_mode);
		return _scValue;
	}

	else return CBScriptable::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXFile::ScSetProperty(const char *Name, CScValue *Value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Length
	//////////////////////////////////////////////////////////////////////////
	if(strcmp(Name, "Length")==0){
	    int OrigLength = _length;
	    _length = max(Value->GetInt(0), 0);

	    char PropName[20];
	    if(_length < OrigLength){
	        for(int i=_length; i<OrigLength; i++){
	            sprintf(PropName, "%d", i);
	            _values->DeleteProp(PropName);
	        }
	    }
	    return S_OK;
	}
	else*/ return CBScriptable::ScSetProperty(Name, Value);
}

//////////////////////////////////////////////////////////////////////////
uint32 CSXFile::GetPos() {
	if (_mode == 1 && _readFile) return _readFile->GetPos();
	else if ((_mode == 2 || _mode == 3) && _writeFile) return ftell(_writeFile);
	else return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CSXFile::SetPos(uint32 Pos, TSeek Origin) {
	if (_mode == 1 && _readFile) return SUCCEEDED(_readFile->Seek(Pos, Origin));
	else if ((_mode == 2 || _mode == 3) && _writeFile) return fseek(_writeFile, Pos, (int)Origin) == 0;
	else return false;
}

//////////////////////////////////////////////////////////////////////////
uint32 CSXFile::GetLength() {
	if (_mode == 1 && _readFile) return _readFile->GetSize();
	else if ((_mode == 2 || _mode == 3) && _writeFile) {
		uint32 CurrentPos = ftell(_writeFile);
		fseek(_writeFile, 0, SEEK_END);
		int Ret = ftell(_writeFile);
		fseek(_writeFile, CurrentPos, SEEK_SET);
		return Ret;
	} else return 0;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CSXFile::Persist(CBPersistMgr *PersistMgr) {

	CBScriptable::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_filename));
	PersistMgr->Transfer(TMEMBER(_mode));
	PersistMgr->Transfer(TMEMBER(_textMode));

	uint32 Pos = 0;
	if (PersistMgr->_saving) {
		Pos = GetPos();
		PersistMgr->Transfer(TMEMBER(Pos));
	} else {
		PersistMgr->Transfer(TMEMBER(Pos));

		// try to re-open file if needed
		_writeFile = NULL;
		_readFile = NULL;

		if (_mode != 0) {
			// open for reading
			if (_mode == 1) {
				_readFile = Game->_fileManager->OpenFile(_filename);
				if (!_readFile) Close();
			}
			// open for writing / appending
			else {
				if (_textMode) {
					if (_mode == 2) _writeFile = fopen(_filename, "w+");
					else _writeFile = fopen(_filename, "a+");
				} else {
					if (_mode == 2) _writeFile = fopen(_filename, "wb+");
					else _writeFile = fopen(_filename, "ab+");
				}
				if (_writeFile) Close();
			}
			SetPos(Pos);
		}
	}

	return S_OK;
}

} // end of namespace WinterMute
