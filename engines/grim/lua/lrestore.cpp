#include "common/endian.h"
#include "common/debug.h"

#include "engines/grim/grim.h"
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
#include "engines/grim/lua/lua.h"

namespace Grim {

RestoreCallback restoreCallbackPtr = NULL;

static void restoreObjectValue(TObject *object, RestoreSint32 restoreSint32, RestoreUint32 restoreUint32) {
	object->ttype = (lua_Type)restoreSint32();

	switch (object->ttype) {
		case LUA_T_NUMBER:
		case LUA_T_TASK:
			{
				byte *udata = (byte *)(&object->value.n);
				uint32 v = restoreUint32();
				restoreUint32();
#if defined(SYSTEM_LITTLE_ENDIAN)
				byte b[4];
				*(uint32 *)&b = v;
				udata[0] = b[3];
				udata[1] = b[2];
				udata[2] = b[1];
				udata[3] = b[0];
#else
				memcpy(&udata, &v, 4);
#endif
			}
			break;
		case LUA_T_NIL:
			{
				restoreUint32();
				restoreUint32();
				object->value.ts = NULL;
			}
			break;
		case LUA_T_ARRAY:
			{
				PointerId ptr;
				ptr.low = restoreUint32();
				ptr.hi = restoreUint32();
				object->value.a = (Hash *)makePointerFromId(ptr);
			}
			break;
		case LUA_T_USERDATA:
		case LUA_T_STRING:
			{
				PointerId ptr;
				ptr.low = restoreUint32();
				ptr.hi = restoreUint32();
				object->value.ts = (TaggedString *)makePointerFromId(ptr);
			}
			break;
		case LUA_T_PROTO:
		case LUA_T_PMARK:
			{
				PointerId ptr;
				ptr.low = restoreUint32();
				ptr.hi = restoreUint32();
				object->value.tf = (TProtoFunc *)makePointerFromId(ptr);
			}
			break;
		case LUA_T_CPROTO:
		case LUA_T_CMARK:
			{
				PointerId ptr;
				ptr.low = restoreUint32();
				ptr.hi = restoreUint32();
				object->value.f = (lua_CFunction)makePointerFromId(ptr);
			}
			break;
		case LUA_T_CLOSURE:
		case LUA_T_CLMARK:
			{
				PointerId ptr;
				ptr.low = restoreUint32();
				ptr.hi = restoreUint32();
				object->value.cl = (Closure *)makePointerFromId(ptr);
			}
			break;
		case LUA_T_LINE:
			{
				object->value.i = restoreSint32();
				restoreSint32();
			}
			break;
		default:
			PointerId ptr;
			ptr.low = restoreUint32();
			ptr.hi = restoreUint32();
			object->value.ts = (TaggedString *)makePointerFromId(ptr);
	}
}

struct ArrayIDObj {
	void *object;
	PointerId idObj;
};

static int sortCallback(const void *id1, const void *id2) {
#ifdef TARGET_64BITS
	uint64 p1 = ((ArrayIDObj *)id1)->idObj.low | ((uint64)(((ArrayIDObj *)id1)->idObj.hi)) << 32;
	uint64 p2 = ((ArrayIDObj *)id2)->idObj.low | ((uint64)(((ArrayIDObj *)id2)->idObj.hi)) << 32;
	if (p1 > p2) {
		return 1;
	} else if (p1 < p2) {
		return -1;
	} else {
		return 0;
	}
#else
	if (((ArrayIDObj *)id1)->idObj.low > ((ArrayIDObj *)id2)->idObj.low) {
		return 1;
	} else if (((ArrayIDObj *)id1)->idObj.low < ((ArrayIDObj *)id2)->idObj.low) {
		return -1;
	} else {
		return 0;
	}
#endif
}

int32 arrayHashTablesCount = 0;
int32 arrayProtoFuncsCount = 0;
int32 arrayClosuresCount = 0;
int32 arrayStringsCount = 0;
ArrayIDObj *arrayStrings = NULL;
ArrayIDObj *arrayHashTables = NULL;
ArrayIDObj *arrayClosures = NULL;
ArrayIDObj *arrayProtoFuncs = NULL;
static bool arraysAllreadySort = false;

static void recreateObj(TObject *obj) {
	if (obj->ttype == LUA_T_CPROTO) {
#ifdef TARGET_64BITS
		uint64 id = ((uint64)(obj->value.f)) >> 16;
#else
		uint32 id = ((uint32)(obj->value.f)) >> 16;
#endif
		luaL_libList *list = list_of_libs;
		while (list) {
			if (id == 0)
				break;
			id--;
			list = list->next;
		}

#ifdef TARGET_64BITS
		int32 numberFunc = (uint64)(obj->value.f) & 0xffff;
#else
		int32 numberFunc = (uint32)(obj->value.f) & 0xffff;
#endif
		if (list && id == 0 && numberFunc < list->number) {
			obj->value.f = list->list[numberFunc].func;
		} else {
			obj->value.f = NULL;
			assert(obj->value.f);
		}
	} else if (obj->ttype == LUA_T_NIL || obj->ttype == LUA_T_LINE || obj->ttype == LUA_T_NUMBER) {
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
		tmpId.object = NULL;

		switch (obj->ttype) {
		case LUA_T_PMARK:
			tmpId.idObj = makeIdFromPointer(obj->value.tf);
			found = (ArrayIDObj *)bsearch(&tmpId, arrayProtoFuncs, arrayProtoFuncsCount, sizeof(ArrayIDObj), sortCallback);
			assert(found);
			obj->value.tf = (TProtoFunc *)found->object;
			break;
		case LUA_T_USERDATA:
			tmpId.idObj = makeIdFromPointer(obj->value.ts);
			found = (ArrayIDObj *)bsearch(&tmpId, arrayStrings, arrayStringsCount, sizeof(ArrayIDObj), sortCallback);
			assert(found);
			obj->value.ts = (TaggedString *)found->object;
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
			obj->value.ts = 0;
			return;
		}
	}
}

void lua_Restore(RestoreStream restoreStream, RestoreSint32 restoreSint32, RestoreUint32 restoreUint32) {
	printf("lua_Restore() started.\n");

	lua_close();
	lua_state = luaM_new(LState);
	lua_resetglobals();

	arrayStringsCount = restoreSint32();
	arrayClosuresCount = restoreSint32();
	arrayHashTablesCount = restoreSint32();
	arrayProtoFuncsCount = restoreSint32();
	int32 rootGlobalCount = restoreSint32();

	arrayStrings = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayStringsCount);
	ArrayIDObj *arraysObj = arrayStrings;
	int32 maxStringsLength;
	maxStringsLength = restoreSint32();
	char *tempStringBuffer = (char *)luaM_malloc(maxStringsLength);

	//printf("1: %d\n", g_grim->_savedState->getBufferPos());

	int32 i;
	for (i = 0; i < arrayStringsCount; i++) {
		arraysObj->idObj.low = restoreSint32();
		arraysObj->idObj.hi = restoreSint32();
		int32 constIndex = restoreSint32();

		TaggedString *tempString;
		if (constIndex != -1) {
			TObject obj;
			restoreObjectValue(&obj, restoreSint32, restoreUint32);
			int32 length = restoreSint32();
			restoreStream(tempStringBuffer, length);
			tempString = luaS_new(tempStringBuffer);
			tempString->globalval = obj;
		} else {
			PointerId ptr;
			lua_Type tag = (lua_Type)restoreSint32();
			ptr.low = restoreUint32();
			ptr.hi = restoreUint32();
			if (tag == 0)
				tempString = luaS_createudata((void *)makePointerFromId(ptr), LUA_ANYTAG);
			else
				tempString = luaS_createudata((void *)makePointerFromId(ptr), tag);
			if (restoreCallbackPtr) {
				PointerId ptr = makeIdFromPointer(tempString->globalval.value.ts);
				ptr = restoreCallbackPtr(tempString->globalval.ttype, ptr, restoreSint32);
				tempString->globalval.value.ts = (TaggedString *)makePointerFromId(ptr);
			}
		}
		tempString->constindex = constIndex;
		arraysObj->object = tempString;
		arraysObj++;
	}
	luaM_free(tempStringBuffer);

	//printf("2: %d\n", g_grim->_savedState->getBufferPos());

	int32 l;
	Closure *tempClosure;
	arraysObj = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayClosuresCount);
	arrayClosures = arraysObj;
	for (i = 0; i < arrayClosuresCount; i++) {
		arraysObj->idObj.low = restoreSint32();
		arraysObj->idObj.hi = restoreSint32();
		int32 countElements = restoreSint32();
		tempClosure = (Closure *)luaM_malloc((countElements * sizeof(TObject)) + sizeof(Closure));
		luaO_insertlist(&lua_state->rootcl, (GCnode *)tempClosure);

		tempClosure->nelems = countElements;
		for (l = 0; l <= tempClosure->nelems; l++) {
			restoreObjectValue(&tempClosure->consts[l], restoreSint32, restoreUint32);
		}
		arraysObj->object = tempClosure;
		arraysObj++;
	}

	Hash *tempHash;
	arraysObj = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayHashTablesCount);
	arrayHashTables = arraysObj;
	for (i = 0; i < arrayHashTablesCount; i++) {
		arraysObj->idObj.low = restoreSint32();
		arraysObj->idObj.hi = restoreSint32();
		tempHash = luaM_new(Hash);
		tempHash->nhash = restoreSint32();
		tempHash->nuse = restoreSint32();
		tempHash->htag = restoreSint32();
		tempHash->node = hashnodecreate(tempHash->nhash);
		luaO_insertlist(&lua_state->roottable, (GCnode *)tempHash);

		for (l = 0; l < tempHash->nuse; l++) {
			restoreObjectValue(&tempHash->node[l].ref, restoreSint32, restoreUint32);
			restoreObjectValue(&tempHash->node[l].val, restoreSint32, restoreUint32);
		}
		arraysObj->object = tempHash;
		arraysObj++;
	}

	TProtoFunc *tempProtoFunc;
	arrayProtoFuncs = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayProtoFuncsCount);
	arraysObj = arrayProtoFuncs;
	for (i = 0; i < arrayProtoFuncsCount; i++) {
		arraysObj->idObj.low = restoreSint32();
		arraysObj->idObj.hi = restoreSint32();
		tempProtoFunc = luaM_new(TProtoFunc);
		luaO_insertlist(&lua_state->rootproto, (GCnode *)tempProtoFunc);
		PointerId ptr;
		ptr.low = restoreSint32();
		ptr.hi = restoreSint32();
		tempProtoFunc->fileName = (TaggedString *)makePointerFromId(ptr);
		tempProtoFunc->lineDefined = restoreSint32();
		tempProtoFunc->nconsts = restoreSint32();
		tempProtoFunc->consts = (TObject *)luaM_malloc(tempProtoFunc->nconsts * sizeof(TObject));

		for (l = 0; l < tempProtoFunc->nconsts; l++) {
			restoreObjectValue(&tempProtoFunc->consts[l], restoreSint32, restoreUint32);
		}

		int32 countVariables = restoreSint32();
		if (countVariables) {
			tempProtoFunc->locvars = (LocVar *)luaM_malloc(countVariables * sizeof(LocVar));
		} else {
			tempProtoFunc->locvars = NULL;
		}

		for (l = 0; l < countVariables; l++) {
			PointerId ptr;
			ptr.low = restoreSint32();
			ptr.hi = restoreSint32();
			tempProtoFunc->locvars[l].varname = (TaggedString *)makePointerFromId(ptr);
			tempProtoFunc->locvars[l].line = restoreSint32();
		}

		int32 codeSize = restoreSint32();
		tempProtoFunc->code = (byte *)luaM_malloc(codeSize);
		restoreStream(tempProtoFunc->code, codeSize);
		arraysObj->object = tempProtoFunc;
		arraysObj++;
	}

	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tempStringTable = &lua_state->string_root[i];
		for (l = 0; l < tempStringTable->size; l++) {
			TaggedString *tempString = tempStringTable->hash[l];
			if (tempString && tempString->constindex != -1 && tempString != &EMPTY) {
				recreateObj(&tempString->globalval);
			}
		}
	}

	tempProtoFunc = (TProtoFunc *)lua_state->rootproto.next;
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
			i = 0;
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

	tempHash = (Hash *)lua_state->roottable.next;
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

	tempClosure = (Closure *)lua_state->rootcl.next;
	while (tempClosure) {
		for (i = 0; i <= tempClosure->nelems; i++) {
			recreateObj(&tempClosure->consts[i]);
		}
		tempClosure = (Closure *)tempClosure->head.next;
	}

	TaggedString *tempListString = (TaggedString *)&(lua_state->rootglobal);
	for (i = 0; i < rootGlobalCount; i++) {
		TObject tempObj;
		TaggedString *tempString = NULL;
		tempObj.ttype = LUA_T_STRING;
		PointerId ptr;
		ptr.low = restoreSint32();
		ptr.hi = restoreSint32();
		tempObj.value.ts = (TaggedString *)makePointerFromId(ptr);
		recreateObj(&tempObj);
 		tempString = (TaggedString *)tempObj.value.ts;
		assert(tempString);
		tempListString->head.next = (GCnode *)tempString;
		tempListString = tempString;
	}
	tempListString->head.next = NULL;

	restoreObjectValue(&lua_state->errorim, restoreSint32, restoreUint32);
	recreateObj(&lua_state->errorim);

	lua_state->IMtable_size = restoreSint32();
	lua_state->IMtable = (IM *)luaM_malloc(lua_state->IMtable_size * sizeof(IM));
	for (i = 0; i < lua_state->IMtable_size; i++) {
		IM *im = &lua_state->IMtable[i];
		for (l = 0; l < IM_N; l++) {
			restoreObjectValue(&im->int_method[l], restoreSint32, restoreUint32);
			recreateObj(&im->int_method[l]);
		}
	}

	lua_state->last_tag = restoreSint32();
	lua_state->refSize = restoreSint32();
	lua_state->refArray = (ref *)luaM_malloc(lua_state->refSize * sizeof(ref));
	for (i = 0; i < lua_state->refSize; i++) {
		restoreObjectValue(&lua_state->refArray[i].o, restoreSint32, restoreUint32);
		recreateObj(&lua_state->refArray[i].o);
		lua_state->refArray[i].status = (Status)restoreSint32();
	}

	lua_state->GCthreshold = restoreSint32();
	lua_state->nblocks = restoreSint32();

	lua_state->Mbuffsize = restoreSint32();
	lua_state->Mbuffer = (char *)luaM_malloc(lua_state->Mbuffsize);
	restoreStream(lua_state->Mbuffer, lua_state->Mbuffsize);
	int32 MbaseOffset;
	MbaseOffset = restoreSint32();
	lua_state->Mbuffbase = MbaseOffset + lua_state->Mbuffer;
	lua_state->Mbuffnext = restoreSint32();

	globalTaskSerialId = restoreSint32();

	lua_Task *tempTask = NULL;
	lua_Task *prevTask = lua_state->root_task;
	int32 countTasks = restoreSint32();
	for (l = 0; l < countTasks; l++) {
		tempTask = luaM_new(lua_Task);
		memset(tempTask, 0, sizeof(lua_Task));
		prevTask->next = tempTask;
		prevTask = tempTask;

		int32 stackLastSize = restoreSint32();
		tempTask->stack.stack = (TObject *)luaM_malloc(stackLastSize * sizeof(TObject));
		tempTask->stack.last = tempTask->stack.stack + stackLastSize - 1;

		int32 stackTopSize = restoreSint32();
		tempTask->stack.top = tempTask->stack.stack + stackTopSize;
		for (i = 0; i < stackTopSize; i++) {
			restoreObjectValue(&tempTask->stack.stack[i], restoreSint32, restoreUint32);
			recreateObj(&tempTask->stack.stack[i]);
		}

		tempTask->Cstack.base = restoreSint32();
		tempTask->Cstack.lua2C = restoreSint32();
		tempTask->Cstack.num = restoreSint32();

		tempTask->numCblocks = restoreSint32();
		for (i = 0; i < tempTask->numCblocks; i++) {
			tempTask->Cblocks[i].base = restoreSint32();
			tempTask->Cblocks[i].lua2C = restoreSint32();
			tempTask->Cblocks[i].num = restoreSint32();
		}

		int32 pcOffset, taskCi;
		tempTask->base_ci_size = restoreSint32();
		tempTask->base_ci = (CallInfo *)luaM_malloc(tempTask->base_ci_size * sizeof(CallInfo));
		memset(tempTask->base_ci, 0, sizeof(CallInfo) * tempTask->base_ci_size);
		CallInfo *tempCi = tempTask->base_ci;
		int32 countCi = tempTask->base_ci_size / sizeof(CallInfo);
		for (i = 0; i < countCi; i++) {
			TObject tempObj;
			tempObj.ttype = LUA_T_CLOSURE;
			PointerId ptr;
			ptr.low = restoreUint32();
			ptr.hi = restoreUint32();
			tempObj.value.cl = (Closure *)makePointerFromId(ptr);
			recreateObj(&tempObj);
			tempCi->c = (Closure *)tempObj.value.cl;
			tempObj.ttype = LUA_T_PROTO;
			ptr.low = restoreUint32();
			ptr.hi = restoreUint32();
			tempObj.value.tf = (TProtoFunc *)makePointerFromId(ptr);
			recreateObj(&tempObj);
			tempCi->tf = (TProtoFunc *)tempObj.value.tf;

			pcOffset = restoreSint32();
			if (pcOffset)
				tempCi->pc = tempCi->tf->code + pcOffset;
			else
				tempCi->pc = NULL;

			tempCi->base = restoreSint32();
			tempCi->nResults = restoreSint32();
			tempCi++;
		}
		taskCi = restoreSint32();
		tempTask->ci = tempTask->base_ci + taskCi;
		tempTask->end_ci = tempTask->base_ci + countCi;

		int32 Mbasepos = restoreSint32();
		tempTask->Mbuffbase = Mbasepos + tempTask->Mbuffer;
		tempTask->Mbuffnext = restoreSint32();

		tempTask->Tstate = (TaskState)restoreSint32();
		tempTask->id = restoreSint32();
	}
	lua_state->last_task = tempTask;

	arraysAllreadySort = false;
	arrayStringsCount = 0;
	arrayHashTablesCount = 0;
	arrayClosuresCount = 0;
	arrayProtoFuncsCount = 0;
	luaM_free(arrayClosures);
	luaM_free(arrayStrings);
	luaM_free(arrayHashTables);
	luaM_free(arrayProtoFuncs);
	arrayHashTables = NULL;
	arrayClosures = NULL;
	arrayProtoFuncs = NULL;
	arrayStrings = NULL;

	printf("lua_Restore() finished.\n");
}

} // end of namespace Grim
