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
CSXMemBuffer::CSXMemBuffer(CBGame *inGame, CScStack *stack): CBScriptable(inGame) {
	stack->correctParams(1);
	_buffer = NULL;
	_size = 0;

	int NewSize = stack->pop()->getInt();
	Resize(MAX(0, NewSize));
}

//////////////////////////////////////////////////////////////////////////
CSXMemBuffer::CSXMemBuffer(CBGame *inGame, void *Buffer): CBScriptable(inGame) {
	_size = 0;
	_buffer = Buffer;
}


//////////////////////////////////////////////////////////////////////////
CSXMemBuffer::~CSXMemBuffer() {
	cleanup();
}

//////////////////////////////////////////////////////////////////////////
void *CSXMemBuffer::scToMemBuffer() {
	return _buffer;
}

//////////////////////////////////////////////////////////////////////////
void CSXMemBuffer::cleanup() {
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
bool CSXMemBuffer::CheckBounds(CScScript *script, int start, int length) {
	if (_buffer == NULL) {
		script->runtimeError("Cannot use Set/Get methods on an uninitialized memory buffer");
		return false;
	}
	if (_size == 0) return true;

	if (start < 0 || length == 0 || start + length > _size) {
		script->runtimeError("Set/Get method call is out of bounds");
		return false;
	} else return true;
}

//////////////////////////////////////////////////////////////////////////
const char *CSXMemBuffer::scToString() {
	return "[membuffer object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXMemBuffer::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetSize
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetSize") == 0) {
		stack->correctParams(1);
		int NewSize = stack->pop()->getInt();
		NewSize = MAX(0, NewSize);
		if (SUCCEEDED(Resize(NewSize))) stack->pushBool(true);
		else stack->pushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetBool") == 0) {
		stack->correctParams(1);
		int Start = stack->pop()->getInt();
		if (!CheckBounds(script, Start, sizeof(bool))) stack->pushNULL();
		else stack->pushBool(*(bool *)((byte *)_buffer + Start));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetByte") == 0) {
		stack->correctParams(1);
		int Start = stack->pop()->getInt();
		if (!CheckBounds(script, Start, sizeof(byte))) stack->pushNULL();
		else stack->pushInt(*(byte *)((byte *)_buffer + Start));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetShort") == 0) {
		stack->correctParams(1);
		int Start = stack->pop()->getInt();
		if (!CheckBounds(script, Start, sizeof(short))) stack->pushNULL();
		else stack->pushInt(65536 + * (short *)((byte *)_buffer + Start));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInt / GetLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetInt") == 0 || strcmp(name, "GetLong") == 0) {
		stack->correctParams(1);
		int Start = stack->pop()->getInt();
		if (!CheckBounds(script, Start, sizeof(int))) stack->pushNULL();
		else stack->pushInt(*(int *)((byte *)_buffer + Start));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFloat") == 0) {
		stack->correctParams(1);
		int Start = stack->pop()->getInt();
		if (!CheckBounds(script, Start, sizeof(float))) stack->pushNULL();
		else stack->pushFloat(*(float *)((byte *)_buffer + Start));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetDouble") == 0) {
		stack->correctParams(1);
		int Start = stack->pop()->getInt();
		if (!CheckBounds(script, Start, sizeof(double))) stack->pushNULL();
		else stack->pushFloat(*(double *)((byte *)_buffer + Start));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetString") == 0) {
		stack->correctParams(2);
		int Start = stack->pop()->getInt();
		int Length = stack->pop()->getInt();

		// find end of string
		if (Length == 0 && Start >= 0 && Start < _size) {
			for (int i = Start; i < _size; i++) {
				if (((char *)_buffer)[i] == '\0') {
					Length = i - Start;
					break;
				}
			}
		}

		if (!CheckBounds(script, Start, Length)) stack->pushNULL();
		else {
			char *Str = new char[Length + 1];
			strncpy(Str, (const char *)_buffer + Start, Length);
			Str[Length] = '\0';
			stack->pushString(Str);
			delete [] Str;
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetPointer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetPointer") == 0) {
		stack->correctParams(1);
		int Start = stack->pop()->getInt();
		if (!CheckBounds(script, Start, sizeof(void *))) stack->pushNULL();
		else {
			void *Pointer = *(void **)((byte *)_buffer + Start);
			CSXMemBuffer *Buf = new CSXMemBuffer(Game, Pointer);
			stack->pushNative(Buf, false);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetBool") == 0) {
		stack->correctParams(2);
		int Start = stack->pop()->getInt();
		bool Val = stack->pop()->getBool();

		if (!CheckBounds(script, Start, sizeof(bool))) stack->pushBool(false);
		else {
			*(bool *)((byte *)_buffer + Start) = Val;
			stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetByte") == 0) {
		stack->correctParams(2);
		int Start = stack->pop()->getInt();
		byte Val = (byte)stack->pop()->getInt();

		if (!CheckBounds(script, Start, sizeof(byte))) stack->pushBool(false);
		else {
			*(byte *)((byte *)_buffer + Start) = Val;
			stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetShort") == 0) {
		stack->correctParams(2);
		int Start = stack->pop()->getInt();
		short Val = (short)stack->pop()->getInt();

		if (!CheckBounds(script, Start, sizeof(short))) stack->pushBool(false);
		else {
			*(short *)((byte *)_buffer + Start) = Val;
			stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetInt / SetLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetInt") == 0 || strcmp(name, "SetLong") == 0) {
		stack->correctParams(2);
		int Start = stack->pop()->getInt();
		int Val = stack->pop()->getInt();

		if (!CheckBounds(script, Start, sizeof(int))) stack->pushBool(false);
		else {
			*(int *)((byte *)_buffer + Start) = Val;
			stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetFloat") == 0) {
		stack->correctParams(2);
		int Start = stack->pop()->getInt();
		float Val = (float)stack->pop()->getFloat();

		if (!CheckBounds(script, Start, sizeof(float))) stack->pushBool(false);
		else {
			*(float *)((byte *)_buffer + Start) = Val;
			stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetDouble") == 0) {
		stack->correctParams(2);
		int Start = stack->pop()->getInt();
		double Val = stack->pop()->getFloat();

		if (!CheckBounds(script, Start, sizeof(double))) stack->pushBool(false);
		else {
			*(double *)((byte *)_buffer + Start) = Val;
			stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetString") == 0) {
		stack->correctParams(2);
		int Start = stack->pop()->getInt();
		const char *Val = stack->pop()->getString();

		if (!CheckBounds(script, Start, strlen(Val) + 1)) stack->pushBool(false);
		else {
			memcpy((byte *)_buffer + Start, Val, strlen(Val) + 1);
			stack->pushBool(true);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPointer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetPointer") == 0) {
		stack->correctParams(2);
		int Start = stack->pop()->getInt();
		/* CScValue *Val = */ stack->pop();

		if (!CheckBounds(script, Start, sizeof(void *))) stack->pushBool(false);
		else {
			/*
			int Pointer = (int)Val->getMemBuffer();
			memcpy((byte *)_buffer+Start, &Pointer, sizeof(void*));
			stack->pushBool(true);
			*/
			// TODO fix
			stack->pushBool(false);

		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DEBUG_Dump
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DEBUG_Dump") == 0) {
		stack->correctParams(0);
		if (_buffer && _size) {
			warning("SXMemBuffer::ScCallMethod - DEBUG_Dump");
			Common::DumpFile f;
			f.open("buffer.bin");
			f.write(_buffer, _size);
			f.close();
		}
		stack->pushNULL();
		return S_OK;
	}

	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXMemBuffer::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("membuffer");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Size (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Size") == 0) {
		_scValue->setInt(_size);
		return _scValue;
	}

	else return CBScriptable::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXMemBuffer::scSetProperty(const char *name, CScValue *value) {
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
HRESULT CSXMemBuffer::persist(CBPersistMgr *persistMgr) {

	CBScriptable::persist(persistMgr);

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
int CSXMemBuffer::scCompare(CBScriptable *Val) {
	if (_buffer == Val->scToMemBuffer()) return 0;
	else return 1;
}

} // end of namespace WinterMute
