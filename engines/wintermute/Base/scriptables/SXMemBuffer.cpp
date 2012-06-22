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

#include "engines/wintermute/Base/BScriptable.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/SXMemBuffer.h"
#include "common/file.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CSXMemBuffer, false)

CBScriptable *makeSXMemBuffer(CBGame *inGame, CScStack *stack) {
	return new CSXMemBuffer(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
CSXMemBuffer::CSXMemBuffer(CBGame *inGame, CScStack *Stack): CBScriptable(inGame) {
	Stack->CorrectParams(1);
	_buffer = NULL;
	_size = 0;

	int NewSize = Stack->Pop()->GetInt();
	Resize(MAX(0, NewSize));
}

//////////////////////////////////////////////////////////////////////////
CSXMemBuffer::CSXMemBuffer(CBGame *inGame, void *Buffer): CBScriptable(inGame) {
	_size = NULL;
	_buffer = Buffer;
}


//////////////////////////////////////////////////////////////////////////
CSXMemBuffer::~CSXMemBuffer() {
	Cleanup();
}

//////////////////////////////////////////////////////////////////////////
void *CSXMemBuffer::ScToMemBuffer() {
	return _buffer;
}

//////////////////////////////////////////////////////////////////////////
void CSXMemBuffer::Cleanup() {
	if (_size) free(_buffer);
	_buffer = NULL;
	_size = 0;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CSXMemBuffer::Resize(int NewSize) {
	int OldSize = _size;

	if (_size == 0) {
		_buffer = malloc(NewSize);
		if (_buffer) _size = NewSize;
	} else {
		void *NewBuf = realloc(_buffer, NewSize);
		if (!NewBuf) {
			if (NewSize == 0) {
				_buffer = NewBuf;
				_size = NewSize;
			} else return E_FAIL;
		} else {
			_buffer = NewBuf;
			_size = NewSize;
		}
	}

	if (_buffer && _size > OldSize) {
		memset((byte *)_buffer + OldSize, 0, _size - OldSize);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CSXMemBuffer::CheckBounds(CScScript *Script, int Start, int Length) {
	if (_buffer == NULL) {
		Script->RuntimeError("Cannot use Set/Get methods on an uninitialized memory buffer");
		return false;
	}
	if (_size == 0) return true;

	if (Start < 0 || Length == 0 || Start + Length > _size) {
		Script->RuntimeError("Set/Get method call is out of bounds");
		return false;
	} else return true;
}

//////////////////////////////////////////////////////////////////////////
const char *CSXMemBuffer::ScToString() {
	return "[membuffer object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXMemBuffer::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SetSize
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetSize") == 0) {
		Stack->CorrectParams(1);
		int NewSize = Stack->Pop()->GetInt();
		NewSize = MAX(0, NewSize);
		if (SUCCEEDED(Resize(NewSize))) Stack->PushBool(true);
		else Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetBool") == 0) {
		Stack->CorrectParams(1);
		int Start = Stack->Pop()->GetInt();
		if (!CheckBounds(Script, Start, sizeof(bool))) Stack->PushNULL();
		else Stack->PushBool(*(bool *)((byte *)_buffer + Start));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetByte") == 0) {
		Stack->CorrectParams(1);
		int Start = Stack->Pop()->GetInt();
		if (!CheckBounds(Script, Start, sizeof(byte))) Stack->PushNULL();
		else Stack->PushInt(*(byte *)((byte *)_buffer + Start));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetShort") == 0) {
		Stack->CorrectParams(1);
		int Start = Stack->Pop()->GetInt();
		if (!CheckBounds(Script, Start, sizeof(short))) Stack->PushNULL();
		else Stack->PushInt(65536 + * (short *)((byte *)_buffer + Start));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInt / GetLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetInt") == 0 || strcmp(Name, "GetLong") == 0) {
		Stack->CorrectParams(1);
		int Start = Stack->Pop()->GetInt();
		if (!CheckBounds(Script, Start, sizeof(int))) Stack->PushNULL();
		else Stack->PushInt(*(int *)((byte *)_buffer + Start));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetFloat") == 0) {
		Stack->CorrectParams(1);
		int Start = Stack->Pop()->GetInt();
		if (!CheckBounds(Script, Start, sizeof(float))) Stack->PushNULL();
		else Stack->PushFloat(*(float *)((byte *)_buffer + Start));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetDouble") == 0) {
		Stack->CorrectParams(1);
		int Start = Stack->Pop()->GetInt();
		if (!CheckBounds(Script, Start, sizeof(double))) Stack->PushNULL();
		else Stack->PushFloat(*(double *)((byte *)_buffer + Start));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetString") == 0) {
		Stack->CorrectParams(2);
		int Start = Stack->Pop()->GetInt();
		int Length = Stack->Pop()->GetInt();

		// find end of string
		if (Length == 0 && Start >= 0 && Start < _size) {
			for (int i = Start; i < _size; i++) {
				if (((char *)_buffer)[i] == '\0') {
					Length = i - Start;
					break;
				}
			}
		}

		if (!CheckBounds(Script, Start, Length)) Stack->PushNULL();
		else {
			char *Str = new char[Length + 1];
			strncpy(Str, (const char *)_buffer + Start, Length);
			Str[Length] = '\0';
			Stack->PushString(Str);
			delete [] Str;
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetPointer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetPointer") == 0) {
		Stack->CorrectParams(1);
		int Start = Stack->Pop()->GetInt();
		if (!CheckBounds(Script, Start, sizeof(void *))) Stack->PushNULL();
		else {
			void *Pointer = *(void **)((byte *)_buffer + Start);
			CSXMemBuffer *Buf = new CSXMemBuffer(Game, Pointer);
			Stack->PushNative(Buf, false);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetBool") == 0) {
		Stack->CorrectParams(2);
		int Start = Stack->Pop()->GetInt();
		bool Val = Stack->Pop()->GetBool();

		if (!CheckBounds(Script, Start, sizeof(bool))) Stack->PushBool(false);
		else {
			*(bool *)((byte *)_buffer + Start) = Val;
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetByte") == 0) {
		Stack->CorrectParams(2);
		int Start = Stack->Pop()->GetInt();
		byte Val = (byte)Stack->Pop()->GetInt();

		if (!CheckBounds(Script, Start, sizeof(byte))) Stack->PushBool(false);
		else {
			*(byte *)((byte *)_buffer + Start) = Val;
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetShort") == 0) {
		Stack->CorrectParams(2);
		int Start = Stack->Pop()->GetInt();
		short Val = (short)Stack->Pop()->GetInt();

		if (!CheckBounds(Script, Start, sizeof(short))) Stack->PushBool(false);
		else {
			*(short *)((byte *)_buffer + Start) = Val;
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetInt / SetLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetInt") == 0 || strcmp(Name, "SetLong") == 0) {
		Stack->CorrectParams(2);
		int Start = Stack->Pop()->GetInt();
		int Val = Stack->Pop()->GetInt();

		if (!CheckBounds(Script, Start, sizeof(int))) Stack->PushBool(false);
		else {
			*(int *)((byte *)_buffer + Start) = Val;
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetFloat") == 0) {
		Stack->CorrectParams(2);
		int Start = Stack->Pop()->GetInt();
		float Val = (float)Stack->Pop()->GetFloat();

		if (!CheckBounds(Script, Start, sizeof(float))) Stack->PushBool(false);
		else {
			*(float *)((byte *)_buffer + Start) = Val;
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetDouble") == 0) {
		Stack->CorrectParams(2);
		int Start = Stack->Pop()->GetInt();
		double Val = Stack->Pop()->GetFloat();

		if (!CheckBounds(Script, Start, sizeof(double))) Stack->PushBool(false);
		else {
			*(double *)((byte *)_buffer + Start) = Val;
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetString") == 0) {
		Stack->CorrectParams(2);
		int Start = Stack->Pop()->GetInt();
		const char *Val = Stack->Pop()->GetString();

		if (!CheckBounds(Script, Start, strlen(Val) + 1)) Stack->PushBool(false);
		else {
			memcpy((byte *)_buffer + Start, Val, strlen(Val) + 1);
			Stack->PushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPointer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetPointer") == 0) {
		Stack->CorrectParams(2);
		int Start = Stack->Pop()->GetInt();
		/* CScValue *Val = */ Stack->Pop();

		if (!CheckBounds(Script, Start, sizeof(void *))) Stack->PushBool(false);
		else {
			/*
			int Pointer = (int)Val->GetMemBuffer();
			memcpy((byte *)_buffer+Start, &Pointer, sizeof(void*));
			Stack->PushBool(true);
			*/
			// TODO fix
			Stack->PushBool(false);

		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DEBUG_Dump
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DEBUG_Dump") == 0) {
		Stack->CorrectParams(0);
		if (_buffer && _size) {
			warning("SXMemBuffer::ScCallMethod - DEBUG_Dump");
			Common::DumpFile f;
			f.open("buffer.bin");
			f.write(_buffer, _size);
			f.close();
		}
		Stack->PushNULL();
		return S_OK;
	}

	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXMemBuffer::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("membuffer");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Size (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Size") == 0) {
		_scValue->SetInt(_size);
		return _scValue;
	}

	else return CBScriptable::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXMemBuffer::ScSetProperty(const char *Name, CScValue *Value) {
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
HRESULT CSXMemBuffer::Persist(CBPersistMgr *persistMgr) {

	CBScriptable::Persist(persistMgr);

	persistMgr->transfer(TMEMBER(_size));

	if (persistMgr->_saving) {
		if (_size > 0) persistMgr->putBytes((byte *)_buffer, _size);
	} else {
		if (_size > 0) {
			_buffer = malloc(_size);
			persistMgr->getBytes((byte *)_buffer, _size);
		} else _buffer = NULL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CSXMemBuffer::ScCompare(CBScriptable *Val) {
	if (_buffer == Val->ScToMemBuffer()) return 0;
	else return 1;
}

} // end of namespace WinterMute
