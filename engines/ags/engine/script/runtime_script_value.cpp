/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ags/shared/script/cc_common.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/engine/ac/dynobj/cc_script_object.h"
#include "ags/shared/util/memory.h"

namespace AGS3 {

using namespace AGS::Shared;

//
// NOTE to future optimizers: I am using 'this' ptr here to better
// distinguish Runtime Values.
//

uint8_t RuntimeScriptValue::ReadByte() const {
	switch (this->Type) {
	case kScValStackPtr:
	case kScValGlobalVar:
		if (RValue->Type == kScValData) {
			return *(uint8_t *)(GetRValuePtrWithOffset());
		} else {
			return static_cast<uint8_t>(RValue->IValue);
		}
	case kScValStaticArray:
	case kScValScriptObject:
		return this->ObjMgr->ReadInt8(this->Ptr, this->IValue);
	default:
		return *((uint8_t *)this->GetPtrWithOffset());
	}
}

int16_t RuntimeScriptValue::ReadInt16() const {
	switch (this->Type) {
	case kScValStackPtr:
		if (RValue->Type == kScValData) {
			return *(int16_t *)(GetRValuePtrWithOffset());
		} else {
			return static_cast<int16_t>(RValue->IValue);
		}
	case kScValGlobalVar:
		if (RValue->Type == kScValData) {
			return Memory::ReadInt16LE(GetRValuePtrWithOffset());
		} else {
			return static_cast<int16_t>(RValue->IValue);
		}
	case kScValStaticArray:
	case kScValScriptObject:
		return this->ObjMgr->ReadInt16(this->Ptr, this->IValue);

	default:
		return *((int16_t *)this->GetPtrWithOffset());
	}
}

int32_t RuntimeScriptValue::ReadInt32() const {
	switch (this->Type) {
	case kScValStackPtr:
		if (RValue->Type == kScValData) {
			return *(int32_t *)(GetRValuePtrWithOffset());
		} else {
			return static_cast<int32_t>(RValue->IValue);
		}
	case kScValGlobalVar:
		if (RValue->Type == kScValData) {
			return Memory::ReadInt32LE(GetRValuePtrWithOffset());
		} else {
			return static_cast<uint32_t>(RValue->IValue);
		}
	case kScValStaticArray:
	case kScValScriptObject:
		return this->ObjMgr->ReadInt32(this->Ptr, this->IValue);
	default:
		return *((int32_t *)this->GetPtrWithOffset());
	}
}

void RuntimeScriptValue::WriteByte(uint8_t val) {
	switch (this->Type) {
	case kScValStackPtr:
	case kScValGlobalVar:
		if (RValue->Type == kScValData) {
			*(uint8_t *)(GetRValuePtrWithOffset()) = val;
		} else {
			RValue->SetUInt8(val); // set RValue as int
		}
		break;
	case kScValStaticArray:
	case kScValScriptObject:
		this->ObjMgr->WriteInt8(this->Ptr, this->IValue, val);
		break;
	default:
		*((uint8_t *)this->GetPtrWithOffset()) = val;
		break;
	}
}

void RuntimeScriptValue::WriteInt16(int16_t val) {
	switch (this->Type) {
	case kScValStackPtr:
		if (RValue->Type == kScValData) {
			*(int16_t *)(GetRValuePtrWithOffset()) = val;
		} else {
			RValue->SetInt16(val); // set RValue as int
		}
		break;
	case kScValGlobalVar:
		if (RValue->Type == kScValData) {
			Memory::WriteInt16LE(GetRValuePtrWithOffset(), val);
		} else {
			RValue->SetInt16(val); // set RValue as int
		}
		break;
	case kScValStaticArray:
	case kScValScriptObject:
		this->ObjMgr->WriteInt16(this->Ptr, this->IValue, val);
		break;
	default:
		*((int16_t *)this->GetPtrWithOffset()) = val;
		break;
	}
}

void RuntimeScriptValue::WriteInt32(int32_t val) {
	switch (this->Type) {
	case kScValStackPtr:
		if (RValue->Type == kScValData) {
			*(int32_t *)(GetRValuePtrWithOffset()) = val;
		} else {
			RValue->SetInt32(val); // set RValue as int
		}
		break;
	case kScValGlobalVar:
		if (RValue->Type == kScValData) {
			Memory::WriteInt32LE(GetRValuePtrWithOffset(), val);
		} else {
			RValue->SetInt32(val); // set RValue as int
		}
		break;
	case kScValStaticArray:
	case kScValScriptObject:
		this->ObjMgr->WriteInt32(this->Ptr, this->IValue, val);
		break;
	default:
		*((int32_t *)this->GetPtrWithOffset()) = val;
		break;
	}
}

RuntimeScriptValue &RuntimeScriptValue::DirectPtr() {
	if (Type == kScValGlobalVar || Type == kScValStackPtr) {
		int ival = IValue;
		*this = *RValue;
		IValue += ival;
	}

	if (Ptr) {
		if (Type == kScValScriptObject)
			Ptr = ObjMgr->GetFieldPtr(Ptr, IValue);
		else
			Ptr = PtrU8 + IValue;
		IValue = 0;
	}
	return *this;
}

RuntimeScriptValue &RuntimeScriptValue::DirectPtrObj() {
	if (Type == kScValGlobalVar || Type == kScValStackPtr)
		*this = *RValue;
	return *this;
}

void *RuntimeScriptValue::GetDirectPtr() const {
	const RuntimeScriptValue *temp_val = this;
	int ival = temp_val->IValue;
	if (temp_val->Type == kScValGlobalVar || temp_val->Type == kScValStackPtr) {
		temp_val = temp_val->RValue;
		ival += temp_val->IValue;
	}
	if (temp_val->Type == kScValScriptObject)
		return temp_val->ObjMgr->GetFieldPtr(temp_val->Ptr, ival);
	else
		return temp_val->PtrU8 + ival;
}

} // namespace AGS3
