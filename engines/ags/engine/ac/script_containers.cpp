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
// Containers script API.
//
//=============================================================================

#include "ags/shared/ac/common.h" // quit
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/dynobj/cc_dynamic_array.h"
#include "ags/engine/ac/dynobj/cc_script_object.h"
#include "ags/engine/ac/dynobj/script_dict.h"
#include "ags/engine/ac/dynobj/script_set.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/shared/util/bbop.h"
#include "ags/globals.h"

namespace AGS3 {

//=============================================================================
//
// Dictionary of strings script API.
//
//=============================================================================

ScriptDictBase *Dict_CreateImpl(bool sorted, bool case_sensitive) {
	ScriptDictBase *dic;
	if (sorted) {
		if (case_sensitive)
			dic = new ScriptDict();
		else
			dic = new ScriptDictCI();
	} else {
		if (case_sensitive)
			dic = new ScriptHashDict();
		else
			dic = new ScriptHashDictCI();
	}
	return dic;
}

ScriptDictBase *Dict_Create(bool sorted, bool case_sensitive) {
	ScriptDictBase *dic = Dict_CreateImpl(sorted, case_sensitive);
	ccRegisterManagedObject(dic, dic);
	return dic;
}

// TODO: we need memory streams
ScriptDictBase *Dict_Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) {
	if (data_sz < sizeof(int32_t) * 2)
		quit("Dict_Unserialize: not enough data."); // TODO: don't quit, return error
	const int sorted = in->ReadInt32();
	const int cs = in->ReadInt32();
	ScriptDictBase *dic = Dict_CreateImpl(sorted != 0, cs != 0);
	dic->Unserialize(index, in, data_sz - sizeof(int32_t) * 2);
	return dic;
}

void Dict_Clear(ScriptDictBase *dic) {
	dic->Clear();
}

bool Dict_Contains(ScriptDictBase *dic, const char *key) {
	return dic->Contains(key);
}

const char *Dict_Get(ScriptDictBase *dic, const char *key) {
	auto *str = dic->Get(key);
	return str ? CreateNewScriptString(str) : nullptr;
}

bool Dict_Remove(ScriptDictBase *dic, const char *key) {
	return dic->Remove(key);
}

bool Dict_Set(ScriptDictBase *dic, const char *key, const char *value) {
	return dic->Set(key, value);
}

int Dict_GetCompareStyle(ScriptDictBase *dic) {
	return dic->IsCaseSensitive() ? 1 : 0;
}

int Dict_GetSortStyle(ScriptDictBase *dic) {
	return dic->IsSorted() ? 1 : 0;
}

int Dict_GetItemCount(ScriptDictBase *dic) {
	return dic->GetItemCount();
}

void *Dict_GetKeysAsArray(ScriptDictBase *dic) {
	std::vector<const char *> items;
	dic->GetKeys(items);
	if (items.size() == 0)
		return nullptr;
	DynObjectRef arr = DynamicArrayHelpers::CreateStringArray(items);
	return arr.Obj;
}

void *Dict_GetValuesAsArray(ScriptDictBase *dic) {
	std::vector<const char *> items;
	dic->GetValues(items);
	if (items.size() == 0)
		return nullptr;
	DynObjectRef arr = DynamicArrayHelpers::CreateStringArray(items);
	return arr.Obj;
}

RuntimeScriptValue Sc_Dict_Create(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PBOOL2(ScriptDictBase, Dict_Create);
}

RuntimeScriptValue Sc_Dict_Clear(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptDictBase, Dict_Clear);
}

RuntimeScriptValue Sc_Dict_Contains(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ(ScriptDictBase, Dict_Contains, const char);
}

RuntimeScriptValue Sc_Dict_Get(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_POBJ(ScriptDictBase, const char, _GP(myScriptStringImpl), Dict_Get, const char);
}

RuntimeScriptValue Sc_Dict_Remove(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ(ScriptDictBase, Dict_Remove, const char);
}

RuntimeScriptValue Sc_Dict_Set(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ2(ScriptDictBase, Dict_Set, const char, const char);
}

RuntimeScriptValue Sc_Dict_GetCompareStyle(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDictBase, Dict_GetCompareStyle);
}

RuntimeScriptValue Sc_Dict_GetSortStyle(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDictBase, Dict_GetSortStyle);
}

RuntimeScriptValue Sc_Dict_GetItemCount(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDictBase, Dict_GetItemCount);
}

RuntimeScriptValue Sc_Dict_GetKeysAsArray(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(ScriptDictBase, void, _GP(globalDynamicArray), Dict_GetKeysAsArray);
}

RuntimeScriptValue Sc_Dict_GetValuesAsArray(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(ScriptDictBase, void, _GP(globalDynamicArray), Dict_GetValuesAsArray);
}

//=============================================================================
//
// Set of strings script API.
//
//=============================================================================

ScriptSetBase *Set_CreateImpl(bool sorted, bool case_sensitive) {
	ScriptSetBase *set;
	if (sorted) {
		if (case_sensitive)
			set = new ScriptSet();
		else
			set = new ScriptSetCI();
	} else {
		if (case_sensitive)
			set = new ScriptHashSet();
		else
			set = new ScriptHashSetCI();
	}
	return set;
}

ScriptSetBase *Set_Create(bool sorted, bool case_sensitive) {
	ScriptSetBase *set = Set_CreateImpl(sorted, case_sensitive);
	ccRegisterManagedObject(set, set);
	return set;
}

// TODO: we need memory streams
ScriptSetBase *Set_Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) {
	if (data_sz < sizeof(int32_t) * 2)
		quit("Set_Unserialize: not enough data."); // TODO: don't quit, return error
	const int sorted = in->ReadInt32();
	const int cs = in->ReadInt32();
	ScriptSetBase *set = Set_CreateImpl(sorted != 0, cs != 0);
	set->Unserialize(index, in, data_sz - sizeof(int32_t) * 2);
	return set;
}

bool Set_Add(ScriptSetBase *set, const char *item) {
	return set->Add(item);
}

void Set_Clear(ScriptSetBase *set) {
	set->Clear();
}

bool Set_Contains(ScriptSetBase *set, const char *item) {
	return set->Contains(item);
}

bool Set_Remove(ScriptSetBase *set, const char *item) {
	return set->Remove(item);
}

int Set_GetCompareStyle(ScriptSetBase *set) {
	return set->IsCaseSensitive() ? 1 : 0;
}

int Set_GetSortStyle(ScriptSetBase *set) {
	return set->IsSorted() ? 1 : 0;
}

int Set_GetItemCount(ScriptSetBase *set) {
	return set->GetItemCount();
}

void *Set_GetItemsAsArray(ScriptSetBase *set) {
	std::vector<const char *> items;
	set->GetItems(items);
	if (items.size() == 0)
		return nullptr;
	DynObjectRef arr = DynamicArrayHelpers::CreateStringArray(items);
	return arr.Obj;
}

RuntimeScriptValue Sc_Set_Create(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PBOOL2(ScriptSetBase, Set_Create);
}

RuntimeScriptValue Sc_Set_Add(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ(ScriptSetBase, Set_Add, const char);
}

RuntimeScriptValue Sc_Set_Clear(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptSetBase, Set_Clear);
}

RuntimeScriptValue Sc_Set_Contains(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ(ScriptSetBase, Set_Contains, const char);
}

RuntimeScriptValue Sc_Set_Remove(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL_POBJ(ScriptSetBase, Set_Remove, const char);
}

RuntimeScriptValue Sc_Set_GetCompareStyle(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptSetBase, Set_GetCompareStyle);
}

RuntimeScriptValue Sc_Set_GetSortStyle(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptSetBase, Set_GetSortStyle);
}

RuntimeScriptValue Sc_Set_GetItemCount(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptSetBase, Set_GetItemCount);
}

RuntimeScriptValue Sc_Set_GetItemsAsArray(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(ScriptSetBase, void, _GP(globalDynamicArray), Set_GetItemsAsArray);
}

void RegisterContainerAPI() {
	ScFnRegister container_api[] = {
		// Dictionary
		{"Dictionary::Create", API_FN_PAIR(Dict_Create)},
		{"Dictionary::Clear", API_FN_PAIR(Dict_Clear)},
		{"Dictionary::Contains", API_FN_PAIR(Dict_Contains)},
		{"Dictionary::Get", API_FN_PAIR(Dict_Get)},
		{"Dictionary::Remove", API_FN_PAIR(Dict_Remove)},
		{"Dictionary::Set", API_FN_PAIR(Dict_Set)},
		{"Dictionary::get_CompareStyle", API_FN_PAIR(Dict_GetCompareStyle)},
		{"Dictionary::get_SortStyle", API_FN_PAIR(Dict_GetSortStyle)},
		{"Dictionary::get_ItemCount", API_FN_PAIR(Dict_GetItemCount)},
		{"Dictionary::GetKeysAsArray", API_FN_PAIR(Dict_GetKeysAsArray)},
		{"Dictionary::GetValuesAsArray", API_FN_PAIR(Dict_GetValuesAsArray)},
		// Set
		{"Set::Create", API_FN_PAIR(Set_Create)},
		{"Set::Add", API_FN_PAIR(Set_Add)},
		{"Set::Clear", API_FN_PAIR(Set_Clear)},
		{"Set::Contains", API_FN_PAIR(Set_Contains)},
		{"Set::Remove", API_FN_PAIR(Set_Remove)},
		{"Set::get_CompareStyle", API_FN_PAIR(Set_GetCompareStyle)},
		{"Set::get_SortStyle", API_FN_PAIR(Set_GetSortStyle)},
		{"Set::get_ItemCount", API_FN_PAIR(Set_GetItemCount)},
		{"Set::GetItemsAsArray", API_FN_PAIR(Set_GetItemsAsArray)},
	};

	ccAddExternalFunctions361(container_api);
}

} // namespace AGS3
