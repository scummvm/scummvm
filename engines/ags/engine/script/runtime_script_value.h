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

//=============================================================================
//
// Runtime script value struct
//
//=============================================================================

#ifndef AGS_ENGINE_SCRIPT_RUNTIME_SCRIPT_VALUE_H
#define AGS_ENGINE_SCRIPT_RUNTIME_SCRIPT_VALUE_H

#include "ags/engine/ac/dynobj/cc_script_object.h"
#include "ags/engine/ac/dynobj/cc_static_array.h"
#include "ags/engine/script/script_api.h"
#include "ags/shared/util/memory.h"

#include "ags/plugins/plugin_base.h"

namespace AGS3 {

enum ScriptValueType {
	kScValUndefined,    // to detect errors
	kScValInteger,      // as strictly 32-bit integer (for integer math)
	kScValFloat,        // as float (for floating point math), 32-bit
	kScValPluginArg,    // an 32-bit value, passed to a script function when called
						// directly by plugin; is allowed to represent object pointer
	kScValStackPtr,     // as a pointer to stack entry
	kScValData,         // as a container for randomly sized data (usually array)
	kScValGlobalVar,    // as a pointer to script variable; used only for global vars,
						// as pointer to local vars must have StackPtr type so that the
						// stack allocation could work
	kScValStringLiteral,// as a pointer to literal string (array of chars)
	kScValStaticArray,  // as a pointer to static global array (of static or dynamic objects)
	kScValScriptObject, // as a pointer to managed script object
	kScValPluginObject, // as a pointer to object managed by plugin (similar to
						// kScValScriptObject, but has backward-compatible limitations)
	kScValStaticFunction,// as a pointer to static function
	kScValPluginFunction,// temporary workaround for plugins (unsafe function ptr)
	kScValObjectFunction,// as a pointer to object member function, gets object pointer as
						 // first parameter
	kScValCodePtr       // as a pointer to element in byte-code array
};

struct RuntimeScriptValue {
public:
	RuntimeScriptValue() {
		Type = kScValUndefined;
		IValue = 0;
		Ptr = nullptr;
		MgrPtr = nullptr;
		Size = 0;
	}

	RuntimeScriptValue(int32_t val) {
		Type = kScValInteger;
		IValue = val;
		Ptr = nullptr;
		MgrPtr = nullptr;
		Size = 4;
	}

	ScriptValueType Type;
	Common::String methodName;
	// The 32-bit value used for integer/float math and for storing
	// variable/element offset relative to object (and array) address
	union {
		int32_t     IValue; // access Value as int32 type
		float       FValue; // access Value as float type
	};
	// Pointer is used for storing... pointers - to objects, arrays,
	// functions and stack entries (other RSV)
	union {
		void	*Ptr;	// generic data pointer
		uint8_t *PtrU8;	// byte buffer pointer
		char	*CStr;	// char buffer pointer
		RuntimeScriptValue *RValue;// access ptr as a pointer to Runtime Value
		ScriptAPIFunction *SPfn;  // access ptr as a pointer to Script API Static Function
		ScriptAPIObjectFunction *ObjPfn; // access ptr as a pointer to Script API Object Function
	};
	// TODO: separation to Ptr and MgrPtr is only needed so far as there's
	// a separation between Script*, Dynamic* and game entity classes.
	// Once those classes are merged, it will no longer be needed.
	union {
		void			 *MgrPtr;  // generic object manager pointer
		IScriptObject	 *ObjMgr;  // script object manager
		CCStaticArray	 *ArrMgr;  // static array manager
	};
	// The "real" size of data, either one stored in I/FValue,
	// or the one referenced by Ptr. Used for calculating stack
	// offsets.
	// Original AGS scripts always assumed pointer is 32-bit.
	// Therefore for stored pointers Size is always 4 both for x32
	// and x64 builds, so that the script is interpreted correctly.
	int             Size;

	inline bool IsValid() const {
		return Type != kScValUndefined;
	}

	inline bool IsNull() const {
		return Ptr == nullptr && IValue == 0;
	}

	inline bool GetAsBool() const {
		return !IsNull();
	}

	inline void *GetPtrWithOffset() const {
		return PtrU8 + IValue;
	}

	inline void *GetRValuePtrWithOffset() const {
		return static_cast<uint8_t *>(RValue->GetPtrWithOffset()) + this->IValue;
	}

	inline RuntimeScriptValue &Invalidate() {
		*this = RuntimeScriptValue();
		return *this;
	}

	inline RuntimeScriptValue &SetUInt8(uint8_t val) {
		Type = kScValInteger;
		methodName.clear();
		IValue = val;
		Ptr = nullptr;
		MgrPtr = nullptr;
		Size = 1;
		return *this;
	}

	inline RuntimeScriptValue &SetInt16(int16_t val) {
		Type = kScValInteger;
		methodName.clear();
		IValue = val;
		Ptr = nullptr;
		MgrPtr = nullptr;
		Size = 2;
		return *this;
	}

	inline RuntimeScriptValue &SetInt32(int32_t val) {
		Type = kScValInteger;
		methodName.clear();
		IValue = val;
		Ptr = nullptr;
		MgrPtr = nullptr;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue &SetFloat(float val) {
		Type = kScValFloat;
		methodName.clear();
		FValue = val;
		Ptr = nullptr;
		MgrPtr = nullptr;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue &SetInt32AsBool(bool val) {
		return SetInt32(val ? 1 : 0);
	}

	inline RuntimeScriptValue &SetFloatAsBool(bool val) {
		return SetFloat(val ? 1.0F : 0.0F);
	}

	inline RuntimeScriptValue &SetPluginArgument(int32_t val) {
		Type = kScValPluginArg;
		methodName.clear();
		IValue = val;
		Ptr = nullptr;
		MgrPtr = nullptr;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue &SetStackPtr(RuntimeScriptValue *stack_entry) {
		Type = kScValStackPtr;
		methodName.clear();
		IValue = 0;
		RValue = stack_entry;
		MgrPtr = nullptr;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue &SetData(void *data, int size) {
		Type = kScValData;
		methodName.clear();
		IValue = 0;
		Ptr = data;
		MgrPtr = nullptr;
		Size = size;
		return *this;
	}

	inline RuntimeScriptValue &SetGlobalVar(RuntimeScriptValue *glvar_value) {
		Type = kScValGlobalVar;
		methodName.clear();
		IValue = 0;
		RValue = glvar_value;
		MgrPtr = nullptr;
		Size = 4;
		return *this;
	}

	// TODO: size?
	inline RuntimeScriptValue &SetStringLiteral(const char *str) {
		Type = kScValStringLiteral;
		methodName.clear();
		IValue = 0;
		Ptr = const_cast<char *>(str);
		MgrPtr = nullptr;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue &SetStaticArray(void *object, CCStaticArray *manager) {
		Type = kScValStaticArray;
		methodName.clear();
		IValue = 0;
		Ptr = object;
		ArrMgr = manager;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue &SetScriptObject(void *object, IScriptObject *manager) {
		Type = kScValScriptObject;
		methodName.clear();
		IValue = 0;
		Ptr = object;
		ObjMgr = manager;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue &SetPluginObject(void *object, IScriptObject *manager) {
		Type = kScValPluginObject;
		methodName.clear();
		IValue = 0;
		Ptr = object;
		ObjMgr = manager;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue &SetScriptObject(ScriptValueType type, void *object, IScriptObject *manager) {
		Type = type;
		IValue = 0;
		Ptr = object;
		ObjMgr = manager;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue &SetStaticFunction(ScriptAPIFunction *pfn) {
		Type = kScValStaticFunction;
		methodName.clear();
		IValue = 0;
		SPfn = pfn;
		MgrPtr = nullptr;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue &SetPluginMethod(Plugins::ScriptContainer *sc, const Common::String &method) {
		Type = kScValPluginFunction;
		methodName = method;
		Ptr = sc;
		MgrPtr = nullptr;
		IValue = 0;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue &SetObjectFunction(ScriptAPIObjectFunction *pfn) {
		Type = kScValObjectFunction;
		methodName.clear();
		IValue = 0;
		ObjPfn = pfn;
		MgrPtr = nullptr;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue &SetCodePtr(void *ptr) {
		Type = kScValCodePtr;
		methodName.clear();
		IValue = 0;
		Ptr = ptr;
		MgrPtr = nullptr;
		Size = 4;
		return *this;
	}

	inline RuntimeScriptValue operator !() const {
		return RuntimeScriptValue().SetInt32AsBool(!GetAsBool());
	}

	inline bool operator ==(const RuntimeScriptValue &rval) const {
		if (rval.Type == kScValPluginFunction) {
			assert(!rval.methodName.empty());
			return (Type == kScValPluginFunction) && (rval.methodName == methodName);
		}

		return ((intptr_t)Ptr + (intptr_t)IValue) == ((intptr_t)rval.Ptr + (intptr_t)rval.IValue);
	}
	inline bool operator !=(const RuntimeScriptValue &rval) const {
		return !(*this == rval);
	}

	// FIXME: find out all certain cases when we are reading a pointer and store it
	// as 32-bit value here. There should be a solution to distinct these cases and
	// store value differently, otherwise it won't work for 64-bit build.
	inline RuntimeScriptValue ReadValue() const {
		switch (this->Type) {
		case kScValStackPtr: {
			// FIXME: join the kScValStackPtr with kScValData using some flag?
			switch (RValue->Type) {
			case kScValData:
				// read from the stack memory buffer
				return RuntimeScriptValue().SetInt32(*(int32_t *)(GetRValuePtrWithOffset()));
			default:
				// return the stack entry itself
				return *RValue;
			}
		}
		case kScValGlobalVar: {
			// FIXME: join the kScValGlobalVar with kScValData using some flag?
			switch (RValue->Type) {
			case kScValData:
				// read from the global memory buffer
				return RuntimeScriptValue().SetInt32(AGS::Shared::Memory::ReadInt32LE(GetRValuePtrWithOffset()));
			default:
				// return the gvar entry itself
				return *RValue;
			}
		}
		case kScValStaticArray:
		case kScValScriptObject:
			return RuntimeScriptValue().SetInt32(this->ObjMgr->ReadInt32(this->Ptr, this->IValue));
		default:
			return RuntimeScriptValue().SetInt32(*(int32_t *)this->GetPtrWithOffset());
		}
	}

	// Notice, that there are only two valid cases when a pointer may be written:
	// when the destination is a stack entry or global variable of free type
	// (not kScValData type).
	// In any other case, only the numeric value (integer/float) will be written.
	inline void WriteValue(const RuntimeScriptValue &rval) {
		switch (this->Type) {
		case kScValStackPtr: {
			// FIXME: join the kScValStackPtr with kScValData using some flag?
			switch (RValue->Type) {
			case kScValData:
				// write into the stack memory buffer
				*(int32_t *)(GetRValuePtrWithOffset()) = rval.IValue;
				break;
			default:
				// write into the stack entry
				*RValue = rval;
				// On stack we assume each item has at least 4 bytes (with exception
				// of arrays - kScValData). This is why we fixup the size in case
				// the assigned value is less (char, int16).
				RValue->Size = 4;
				break;
			}
			break;
		}
		case kScValGlobalVar: {
			// FIXME: join the kScValGlobalVar with kScValData using some flag?
			switch (RValue->Type) {
			case kScValData:
				// write into the global memory buffer
				AGS::Shared::Memory::WriteInt32LE(GetRValuePtrWithOffset(), rval.IValue);
				break;
			default:
				// write into the gvar entry
				*RValue = rval;
				break;
			}
			break;
		}
		case kScValStaticArray:
		case kScValScriptObject: {
			this->ObjMgr->WriteInt32(this->Ptr, this->IValue, rval.IValue);
			break;
		}
		default: {
			*((int32_t *)this->GetPtrWithOffset()) = rval.IValue;
			break;
		}
		}
	}

	Plugins::PluginMethod pluginMethod() const {
		return Plugins::PluginMethod((Plugins::PluginBase *)Ptr, methodName);
	}

	// Helper functions for reading or writing values from/to
	// object, referenced by this Runtime Value.
	// Copy implementation depends on value type.
	uint8_t     ReadByte() const;
	int16_t     ReadInt16() const;
	int32_t     ReadInt32() const;
	void        WriteByte(uint8_t val);
	void        WriteInt16(int16_t val);
	void        WriteInt32(int32_t val);

	// Convert to most simple pointer type by resolving RValue ptrs and applying offsets;
	// non pointer types are left unmodified
	RuntimeScriptValue &DirectPtr();
	// Similar to above, a slightly speed-optimised version for situations when we can
	// tell for certain that we are expecting a pointer to the object and not its (first) field.
	RuntimeScriptValue &DirectPtrObj();
	// Resolve and return direct pointer to the referenced data; non pointer types return IValue
	void *GetDirectPtr() const;
};

} // namespace AGS3

#endif
