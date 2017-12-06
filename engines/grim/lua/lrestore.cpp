#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp

#include "common/endian.h"
#include "common/debug.h"

#include "engines/grim/savegame.h"

#include "engines/grim/lua/ltask.h"
#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/ldo.h"
#include "engines/grim/lua/ltm.h"
#include "engines/grim/lua/ltable.h"
#include "engines/grim/lua/lvm.h"
#include "engines/grim/lua/lopcodes.h"
#include "engines/grim/lua/lstring.h"
#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/lua.h"

namespace Grim {

static void restoreObjectValue(TObject *object, SaveGame *savedState) {
	object->ttype = (lua_Type)savedState->readLESint32();

	switch (object->ttype) {
		case LUA_T_NUMBER:
		case LUA_T_TASK:
			{
				object->value.n = savedState->readFloat();
			}
			break;
		case LUA_T_NIL:
			{
				object->value.ts = nullptr;
			}
			break;
		case LUA_T_ARRAY:
			{
				PointerId ptr;
				ptr.id = savedState->readLEUint64();
				object->value.a = (Hash *)makePointerFromId(ptr);
			}
			break;
		case LUA_T_USERDATA:
			{
				object->value.ud.id = savedState->readLESint32();
				object->value.ud.tag = savedState->readLESint32();
				if (savedState->saveMinorVersion() == 0) {
					savedState->readLEUint32();
					savedState->readLEUint32();
				}
			}
			break;
		case LUA_T_STRING:
			{
				PointerId ptr;
				ptr.id = savedState->readLEUint64();
				object->value.ts = (TaggedString *)makePointerFromId(ptr);
			}
			break;
		case LUA_T_PROTO:
		case LUA_T_PMARK:
			{
				PointerId ptr;
				ptr.id = savedState->readLEUint64();
				object->value.tf = (TProtoFunc *)makePointerFromId(ptr);
			}
			break;
		case LUA_T_CPROTO:
		case LUA_T_CMARK:
			{
				PointerId ptr;
				ptr.id = savedState->readLEUint64();

				// WORKAROUND: C++ forbids casting from a pointer-to-function to a
				// pointer-to-object. We use a union to work around that.
				union {
					void *objPtr;
					lua_CFunction funcPtr;
				} ptrUnion;

				ptrUnion.objPtr = makePointerFromId(ptr);
				object->value.f = ptrUnion.funcPtr;
			}
			break;
		case LUA_T_CLOSURE:
		case LUA_T_CLMARK:
			{
				PointerId ptr;
				ptr.id = savedState->readLEUint64();
				object->value.cl = (Closure *)makePointerFromId(ptr);
			}
			break;
		case LUA_T_LINE:
			{
				object->value.i = savedState->readLESint32();
			}
			break;
		default:
			PointerId ptr;
			ptr.id = savedState->readLEUint64();
			object->value.ts = (TaggedString *)makePointerFromId(ptr);
	}
}

struct ArrayIDObj {
	void *object;
	PointerId idObj;
};

static int sortCallback(const void *id1, const void *id2) {
	if (((const ArrayIDObj *)id1)->idObj.id > ((const ArrayIDObj *)id2)->idObj.id) {
		return 1;
	} else if (((const ArrayIDObj *)id1)->idObj.id < ((const ArrayIDObj *)id2)->idObj.id) {
		return -1;
	} else {
		return 0;
	}
}

int32 arrayHashTablesCount = 0;
int32 arrayProtoFuncsCount = 0;
int32 arrayClosuresCount = 0;
int32 arrayStringsCount = 0;
ArrayIDObj *arrayStrings = nullptr;
ArrayIDObj *arrayHashTables = nullptr;
ArrayIDObj *arrayClosures = nullptr;
ArrayIDObj *arrayProtoFuncs = nullptr;
static bool arraysAllreadySort = false;

static void recreateObj(TObject *obj) {
	if (obj->ttype == LUA_T_CPROTO) {
		uintptr id = ((uintptr)(obj->value.f)) >> 16;
		luaL_libList *list = list_of_libs;
		while (list) {
			if (id == 0)
				break;
			id--;
			list = list->next;
		}

		int32 numberFunc = (uintptr)(obj->value.f) & 0xffff;
		if (list && id == 0 && numberFunc < list->number) {
			obj->value.f = list->list[numberFunc].func;
		} else {
			obj->value.f = nullptr;
			assert(obj->value.f);
		}
	} else if (obj->ttype == LUA_T_NIL || obj->ttype == LUA_T_LINE || obj->ttype == LUA_T_NUMBER ||
			obj->ttype == LUA_T_TASK || obj->ttype == LUA_T_USERDATA) {
		return;
	} else {
		if (obj->value.i == 0)
			return;

		if (!arraysAllreadySort) {
			arraysAllreadySort = true;
			qsort(arrayHashTables, arrayHashTablesCount, sizeof(ArrayIDObj), sortCallback);
			qsort(arrayProtoFuncs, arrayProtoFuncsCount, sizeof(ArrayIDObj), sortCallback);
			qsort(arrayClosures, arrayClosuresCount, sizeof(ArrayIDObj), sortCallback);
			qsort(arrayStrings, arrayStringsCount, sizeof(ArrayIDObj), sortCallback);
		}

		ArrayIDObj *found;
		ArrayIDObj tmpId;
		tmpId.object = nullptr;

		switch (obj->ttype) {
		case LUA_T_PMARK:
			tmpId.idObj = makeIdFromPointer(obj->value.tf);
			found = (ArrayIDObj *)bsearch(&tmpId, arrayProtoFuncs, arrayProtoFuncsCount, sizeof(ArrayIDObj), sortCallback);
			assert(found);
			obj->value.tf = (TProtoFunc *)found->object;
			break;
		case LUA_T_PROTO:
			tmpId.idObj = makeIdFromPointer(obj->value.tf);
			found = (ArrayIDObj *)bsearch(&tmpId, arrayProtoFuncs, arrayProtoFuncsCount, sizeof(ArrayIDObj), sortCallback);
			assert(found);
			obj->value.tf = (TProtoFunc *)found->object;
			break;
		case LUA_T_CLOSURE:
			tmpId.idObj = makeIdFromPointer(obj->value.cl);
			found = (ArrayIDObj *)bsearch(&tmpId, arrayClosures, arrayClosuresCount, sizeof(ArrayIDObj), sortCallback);
			assert(found);
			obj->value.cl = (Closure *)found->object;
			break;
		case LUA_T_ARRAY:
			tmpId.idObj = makeIdFromPointer(obj->value.a);
			found = (ArrayIDObj *)bsearch(&tmpId, arrayHashTables, arrayHashTablesCount, sizeof(ArrayIDObj), sortCallback);
			assert(found);
			obj->value.a = (Hash *)found->object;
			break;
		case LUA_T_STRING:
			tmpId.idObj = makeIdFromPointer(obj->value.ts);
			found = (ArrayIDObj *)bsearch(&tmpId, arrayStrings, arrayStringsCount, sizeof(ArrayIDObj), sortCallback);
			assert(found);
			obj->value.ts = (TaggedString *)found->object;
			break;
		default:
			obj->value.i = 0;
			obj->value.ts = nullptr;
			return;
		}
	}
}

void lua_Restore(SaveGame *savedState) {
	savedState->beginSection('LUAS');

	lua_close();
	lua_rootState = lua_state = luaM_new(LState);
	lua_stateinit(lua_state);
	lua_resetglobals();

	arrayStringsCount = savedState->readLESint32();
	arrayClosuresCount = savedState->readLESint32();
	arrayHashTablesCount = savedState->readLESint32();
	arrayProtoFuncsCount = savedState->readLESint32();
	int32 rootGlobalCount = savedState->readLESint32();

	arrayStrings = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayStringsCount);
	ArrayIDObj *arraysObj = arrayStrings;
	int32 maxStringsLength;
	maxStringsLength = savedState->readLESint32();
	char *tempStringBuffer = (char *)luaM_malloc(maxStringsLength + 1); // add extra char for 0 terminate string

	//printf("1: %d\n", g_grim->_savedState->getBufferPos());

	int32 i;
	for (i = 0; i < arrayStringsCount; i++) {
		arraysObj->idObj.id = savedState->readLEUint64();
		int32 constIndex = savedState->readLESint32();

		TaggedString *tempString = nullptr;
		if (constIndex != -1) {
			TObject obj;
			restoreObjectValue(&obj, savedState);
			int32 length = savedState->readLESint32();
			savedState->read(tempStringBuffer, length);
			tempStringBuffer[length] = '\0';
			tempString = luaS_new(tempStringBuffer);
			tempString->globalval = obj;
		}
		assert(tempString);
		tempString->constindex = constIndex;
		arraysObj->object = tempString;
		arraysObj++;
	}
	luaM_free(tempStringBuffer);

	//printf("2: %d\n", g_grim->_savedState->getBufferPos());

	int32 l;
	Closure *tempClosure;
	GCnode *prevClosure = &rootcl;
	arraysObj = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayClosuresCount);
	arrayClosures = arraysObj;
	for (i = 0; i < arrayClosuresCount; i++) {
		arraysObj->idObj.id = savedState->readLEUint64();
		int32 countElements = savedState->readLESint32();
		tempClosure = (Closure *)luaM_malloc((countElements * sizeof(TObject)) + sizeof(Closure));
		luaO_insertlist(prevClosure, (GCnode *)tempClosure);
		prevClosure = (GCnode *)tempClosure;

		tempClosure->nelems = countElements;
		for (l = 0; l <= tempClosure->nelems; l++) {
			restoreObjectValue(&tempClosure->consts[l], savedState);
		}
		arraysObj->object = tempClosure;
		arraysObj++;
	}

	Hash *tempHash;
	GCnode *prevHash = &roottable;
	arraysObj = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayHashTablesCount);
	arrayHashTables = arraysObj;
	for (i = 0; i < arrayHashTablesCount; i++) {
		arraysObj->idObj.id = savedState->readLEUint64();
		tempHash = luaM_new(Hash);
		tempHash->nhash = savedState->readLESint32();
		tempHash->nuse = savedState->readLESint32();
		tempHash->htag = savedState->readLESint32();
		tempHash->node = hashnodecreate(tempHash->nhash);
		luaO_insertlist(prevHash, (GCnode *)tempHash);
		prevHash = (GCnode *)tempHash;

		for (l = 0; l < tempHash->nuse; l++) {
			restoreObjectValue(&tempHash->node[l].ref, savedState);
			restoreObjectValue(&tempHash->node[l].val, savedState);
		}
		arraysObj->object = tempHash;
		arraysObj++;
	}

	TProtoFunc *tempProtoFunc;
	GCnode *oldProto = &rootproto;
	arrayProtoFuncs = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayProtoFuncsCount);
	arraysObj = arrayProtoFuncs;
	for (i = 0; i < arrayProtoFuncsCount; i++) {
		arraysObj->idObj.id = savedState->readLEUint64();
		tempProtoFunc = luaM_new(TProtoFunc);
		luaO_insertlist(oldProto, (GCnode *)tempProtoFunc);
		oldProto = (GCnode *)tempProtoFunc;
		PointerId ptr;
		ptr.id = savedState->readLEUint64();
		tempProtoFunc->fileName = (TaggedString *)makePointerFromId(ptr);
		tempProtoFunc->lineDefined = savedState->readLESint32();
		tempProtoFunc->nconsts = savedState->readLESint32();
		if (tempProtoFunc->nconsts > 0) {
			tempProtoFunc->consts = (TObject *)luaM_malloc(tempProtoFunc->nconsts * sizeof(TObject));
		} else {
			tempProtoFunc->consts = nullptr;
		}

		for (l = 0; l < tempProtoFunc->nconsts; l++) {
			restoreObjectValue(&tempProtoFunc->consts[l], savedState);
		}

		int32 countVariables = savedState->readLESint32();
		if (countVariables) {
			tempProtoFunc->locvars = (LocVar *)luaM_malloc(countVariables * sizeof(LocVar));
		} else {
			tempProtoFunc->locvars = nullptr;
		}

		for (l = 0; l < countVariables; l++) {
			ptr.id = savedState->readLEUint64();
			tempProtoFunc->locvars[l].varname = (TaggedString *)makePointerFromId(ptr);
			tempProtoFunc->locvars[l].line = savedState->readLESint32();
		}

		int32 codeSize = savedState->readLESint32();
		tempProtoFunc->code = (byte *)luaM_malloc(codeSize);
		savedState->read(tempProtoFunc->code, codeSize);
		arraysObj->object = tempProtoFunc;
		arraysObj++;
	}

	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tempStringTable = &string_root[i];
		for (l = 0; l < tempStringTable->size; l++) {
			TaggedString *tempString = tempStringTable->hash[l];
			if (tempString && tempString->constindex != -1 && tempString != &EMPTY) {
				recreateObj(&tempString->globalval);
			}
		}
	}

	tempProtoFunc = (TProtoFunc *)rootproto.next;
	while (tempProtoFunc) {
		TObject tempObj;
		tempObj.value.ts = (TaggedString *)tempProtoFunc->fileName;
		tempObj.ttype = LUA_T_STRING;
		recreateObj(&tempObj);
		tempProtoFunc->fileName = (TaggedString *)tempObj.value.ts;
		for (i = 0; i < tempProtoFunc->nconsts; i++) {
			recreateObj(&tempProtoFunc->consts[i]);
		}

		if (tempProtoFunc->locvars) {
			for (i = 0; tempProtoFunc->locvars[i].line != -1; i++) {
				TObject tempObj2;
				tempObj2.value.ts = tempProtoFunc->locvars[i].varname;
				tempObj2.ttype = LUA_T_STRING;
				recreateObj(&tempObj2);
				tempProtoFunc->locvars[i].varname = (TaggedString *)tempObj2.value.ts;
			}
		}
		tempProtoFunc = (TProtoFunc *)tempProtoFunc->head.next;
	}

	tempHash = (Hash *)roottable.next;
	while (tempHash) {
		for (i = 0; i < tempHash->nuse; i++) {
			recreateObj(&tempHash->node[i].ref);
			recreateObj(&tempHash->node[i].val);
		}
		Node *oldNode = tempHash->node;
		tempHash->node = hashnodecreate(tempHash->nhash);
		for (i = 0; i < tempHash->nuse; i++) {
			Node *newNode = oldNode + i;
			if (newNode->ref.ttype != LUA_T_NIL && newNode->val.ttype != LUA_T_NIL) {
				*node(tempHash, present(tempHash, &newNode->ref)) = *newNode;
			}
		}
		luaM_free(oldNode);
		tempHash = (Hash *)tempHash->head.next;
	}

	tempClosure = (Closure *)rootcl.next;
	while (tempClosure) {
		for (i = 0; i <= tempClosure->nelems; i++) {
			recreateObj(&tempClosure->consts[i]);
		}
		tempClosure = (Closure *)tempClosure->head.next;
	}

	TaggedString *tempListString = (TaggedString *)&(rootglobal);
	for (i = 0; i < rootGlobalCount; i++) {
		TObject tempObj;
		TaggedString *tempString = nullptr;
		tempObj.ttype = LUA_T_STRING;
		PointerId ptr;
		ptr.id = savedState->readLEUint64();
		tempObj.value.ts = (TaggedString *)makePointerFromId(ptr);
		recreateObj(&tempObj);
 		tempString = (TaggedString *)tempObj.value.ts;
		assert(tempString);
		tempListString->head.next = (GCnode *)tempString;
		tempListString = tempString;
	}
	tempListString->head.next = nullptr;

	restoreObjectValue(&errorim, savedState);
	recreateObj(&errorim);

	IMtable_size = savedState->readLESint32();
	if (IMtable_size > 0) {
		IMtable = (IM *)luaM_malloc(IMtable_size * sizeof(IM));
		for (i = 0; i < IMtable_size; i++) {
			IM *im = &IMtable[i];
			for (l = 0; l < IM_N; l++) {
				restoreObjectValue(&im->int_method[l], savedState);
				recreateObj(&im->int_method[l]);
			}
		}
	} else {
		IMtable = nullptr;
	}

	last_tag = savedState->readLESint32();
	refSize = savedState->readLESint32();
	if (refSize > 0) {
		refArray = (ref *)luaM_malloc(refSize * sizeof(ref));
		for (i = 0; i < refSize; i++) {
			restoreObjectValue(&refArray[i].o, savedState);
			recreateObj(&refArray[i].o);
			refArray[i].status = (Status)savedState->readLESint32();
		}
	} else {
		refArray = nullptr;
	}

	GCthreshold = savedState->readLESint32();
	nblocks = savedState->readLESint32();

	Mbuffsize = savedState->readLESint32();
	Mbuffer = (char *)luaM_malloc(Mbuffsize);
	savedState->read(Mbuffer, Mbuffsize);
	int32 MbaseOffset;
	MbaseOffset = savedState->readLESint32();
	Mbuffbase = MbaseOffset + Mbuffer;
	Mbuffnext = savedState->readLESint32();

	globalTaskSerialId = savedState->readLESint32();

	int32 countStates = savedState->readLESint32();
	int32 currentState = savedState->readLESint32();

	LState *state = lua_rootState;
	for (l = 0; l < countStates; l++) {
		if (l == 0)
			state = lua_rootState;
		else {
			LState *s = luaM_new(LState);
			lua_stateinit(s);
			state->next = s;
			s->prev = state;
			state = s;
		}
		int32 countTasks = savedState->readLESint32();
		if (countTasks) {
			lua_Task *task = nullptr;
			for (i = 0; i < countTasks; i++) {
				if (i == 0) {
					task = state->task = luaM_new(lua_Task);
					lua_taskinit(task, nullptr, 0, 0);
				} else {
					lua_Task *t = luaM_new(lua_Task);
					lua_taskinit(t, nullptr, 0, 0);
					task->next = t;
					task = t;
				}

				task->S = &state->stack;

				TObject tempObj;
				tempObj.ttype = LUA_T_CLOSURE;
				PointerId ptr;
				ptr.id = savedState->readLEUint64();
				tempObj.value.cl = (Closure *)makePointerFromId(ptr);
				recreateObj(&tempObj);
				task->cl = (Closure *)tempObj.value.cl;
				tempObj.ttype = LUA_T_PROTO;
				ptr.id = savedState->readLEUint64();
				tempObj.value.tf = (TProtoFunc *)makePointerFromId(ptr);
				recreateObj(&tempObj);
				task->tf = (TProtoFunc *)tempObj.value.tf;

				task->base = savedState->readLESint32();
				task->some_base = savedState->readLESint32();
				task->some_results = savedState->readLESint32();
				task->some_flag = savedState->readBool();
				int32 pcOffset = savedState->readLESint32();
				task->pc = task->tf->code + pcOffset;
				task->aux = savedState->readLESint32();
				task->consts = task->tf->consts;
			}
		} else {
			state->task = nullptr;
		}
		int32 n = savedState->readLESint32();
		if (n < 0) {
			state->some_task = nullptr;
		} else {
			state->some_task = state->task;
			for (; n; n--)
				state->some_task = state->some_task->next;
		}

		state->updated = savedState->readBool();

		byte pauseState = savedState->readByte();
		state->all_paused = pauseState & LUA_SG_ALL_PAUSED;
		state->paused = (pauseState & LUA_SG_PAUSED) ? true : false;

		state->state_counter1 = savedState->readLESint32();
		state->state_counter2 = savedState->readLESint32();

		int32 stackLastSize = savedState->readLESint32();
		if (state->stack.stack)
			luaM_free(state->stack.stack);

		state->stack.stack = (TObject *)luaM_malloc(stackLastSize * sizeof(TObject));
		state->stack.last = state->stack.stack + stackLastSize - 1;

		int32 stackTopSize = savedState->readLESint32();
		state->stack.top = state->stack.stack + stackTopSize;
		for (i = 0; i < stackTopSize; i++) {
			restoreObjectValue(&state->stack.stack[i], savedState);
			recreateObj(&state->stack.stack[i]);
		}

		state->Cstack.base = savedState->readLESint32();
		state->Cstack.lua2C = savedState->readLESint32();
		state->Cstack.num = savedState->readLESint32();

		state->numCblocks = savedState->readLESint32();
		for (i = 0; i < state->numCblocks; i++) {
			state->Cblocks[i].base = savedState->readLESint32();
			state->Cblocks[i].lua2C = savedState->readLESint32();
			state->Cblocks[i].num = savedState->readLESint32();
		}

		if (savedState->saveMinorVersion() >= 3) {
			state->sleepFor = savedState->readLEUint32();
		}
		state->id = savedState->readLEUint32();
		restoreObjectValue(&state->taskFunc, savedState);
		if (state->taskFunc.ttype == LUA_T_PROTO || state->taskFunc.ttype == LUA_T_CPROTO)
			recreateObj(&state->taskFunc);
	}

	for (; currentState; currentState--)
		lua_state = lua_state->next;

	arraysAllreadySort = false;
	arrayStringsCount = 0;
	arrayHashTablesCount = 0;
	arrayClosuresCount = 0;
	arrayProtoFuncsCount = 0;
	luaM_free(arrayClosures);
	luaM_free(arrayStrings);
	luaM_free(arrayHashTables);
	luaM_free(arrayProtoFuncs);
	arrayHashTables = nullptr;
	arrayClosures = nullptr;
	arrayProtoFuncs = nullptr;
	arrayStrings = nullptr;

	savedState->endSection();
}

} // end of namespace Grim
