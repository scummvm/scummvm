#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "ltask.h"
#include "lauxlib.h"
#include "lmem.h"
#include "ldo.h"
#include "ltm.h"
#include "ltable.h"
#include "lvm.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lua.h"

SaveRestoreCallback restoreCallback = NULL;

struct ArrayIDObj {
	void *object;
	unsigned int idObj;
};

static int sortCallback(const void *id1, const void *id2) {
	if (((ArrayIDObj *)id1)->idObj > ((ArrayIDObj *)id2)->idObj) {
		return 1;
	} else if (((ArrayIDObj *)id1)->idObj < ((ArrayIDObj *)id2)->idObj) {
		return -1;
	} else {
		return 0;
	}
}

int arrayHashTablesCount = 0;
int arrayProtoFuncsCount = 0;
int arrayClosuresCount = 0;
int arrayStringsCount = 0;
ArrayIDObj *arrayStrings = NULL;
ArrayIDObj *arrayHashTables = NULL;
ArrayIDObj *arrayClosures = NULL;
ArrayIDObj *arrayProtoFuncs = NULL;
static bool arraysAllreadySort = false;

static void recreateObj(TObject *obj) {
	if (obj->ttype == LUA_T_CPROTO) {
		unsigned long some = ((unsigned long)(obj->value.f)) >> 16;
		luaL_libList *list = list_of_libs;
		while (list != NULL) {
			if (some == 0)
				break;
			some--;
			list = list->next;
		}

		long numberFunc = (long)(obj->value.f) & 0xffff;
		if ((list != NULL) && (some == 0) && (numberFunc < list->number)) {
			obj->value.f = list->list[numberFunc].func;
		} else {
			obj->value.f = NULL;
			assert(obj->value.f);
		}
	} else if ((obj->ttype == LUA_T_NIL) || (obj->ttype == LUA_T_LINE) || (obj->ttype == LUA_T_NUMBER) ) {
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

		tmpId.idObj = obj->value.i;
		tmpId.object = NULL;
		obj->value.i = 0;

		switch (obj->ttype) {
		case LUA_T_PMARK:
			found = (ArrayIDObj *)bsearch(&tmpId, arrayProtoFuncs, arrayProtoFuncsCount, sizeof(ArrayIDObj), sortCallback);
			break;
		case LUA_T_USERDATA:
			found = (ArrayIDObj *)bsearch(&tmpId, arrayStrings, arrayStringsCount, sizeof(ArrayIDObj), sortCallback);
			break;
		case LUA_T_PROTO:
			found = (ArrayIDObj *)bsearch(&tmpId, arrayProtoFuncs, arrayProtoFuncsCount, sizeof(ArrayIDObj), sortCallback);
			break;
		case LUA_T_CLOSURE:
			found = (ArrayIDObj *)bsearch(&tmpId, arrayClosures, arrayClosuresCount, sizeof(ArrayIDObj), sortCallback);
			break;
		case LUA_T_ARRAY:
			found = (ArrayIDObj *)bsearch(&tmpId, arrayHashTables, arrayHashTablesCount, sizeof(ArrayIDObj), sortCallback);
			break;
		case LUA_T_STRING:
			found = (ArrayIDObj *)bsearch(&tmpId, arrayStrings, arrayStringsCount, sizeof(ArrayIDObj), sortCallback);
			break;
		default:
			return;
		}

		obj->value.i = (long)found->object;
	}
}

void lua_Restore(SaveRestoreFunc restoreFunc) {
	printf("lua_Restore() started.\n");

	lua_close();
	L = luaM_new(lua_State);
	lua_resetglobals();

	restoreFunc(&arrayStringsCount, sizeof(int));
	restoreFunc(&arrayClosuresCount, sizeof(int));
	restoreFunc(&arrayHashTablesCount, sizeof(int));
	restoreFunc(&arrayProtoFuncsCount, sizeof(int));
	int rootGlobalCount;
	restoreFunc(&rootGlobalCount, sizeof(int));

	arrayStrings = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayStringsCount);
	ArrayIDObj *arraysObj = arrayStrings;
	int maxStringsLength;
	restoreFunc(&maxStringsLength, sizeof(int));
	char *tempStringBuffer = (char *)luaM_malloc(maxStringsLength);

	int i;
	for (i = 0; i < arrayStringsCount; i++) {
		restoreFunc(&arraysObj->idObj, sizeof(unsigned int));
		int constIndex;
		restoreFunc(&constIndex, sizeof(int));
		lua_Type tag;
		restoreFunc(&tag, sizeof(int));
		void *value;
		restoreFunc(&value, sizeof(void *));

		TaggedString *tempString;
		if (constIndex != -1) {
			long length;
			restoreFunc(&length, sizeof(long));
			restoreFunc(tempStringBuffer, length);
			tempString = luaS_newlstr(tempStringBuffer, length);
			tempString->u.s.globalval.ttype = tag;
			tempString->u.s.globalval.value.ts = (TaggedString *)value;
		} else {
			if (tag == 0)
				tempString = luaS_createudata(value, LUA_ANYTAG);
			else
				tempString = luaS_createudata(value, tag);
			if (restoreCallback != NULL) {
				tempString->u.s.globalval.value.ts = (TaggedString *)restoreCallback(tempString->u.s.globalval.ttype, (long)tempString->u.s.globalval.value.ts, restoreFunc);
			}
		}
		tempString->constindex = constIndex;
		arraysObj->object = tempString;
		arraysObj++;
	}
	luaM_free(tempStringBuffer);

	int l;
	Closure *tempClosure;
	arraysObj = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayClosuresCount);
	arrayClosures = arraysObj;
	for (i = 0; i < arrayClosuresCount; i++) {
		restoreFunc(&arraysObj->idObj, sizeof(unsigned int));
		int countElements;
		restoreFunc(&countElements, sizeof(int));
		tempClosure = (Closure *)luaM_malloc((countElements * sizeof(TObject)) + sizeof(Closure));
		luaO_insertlist(&L->rootcl, (GCnode *)tempClosure);

		tempClosure->nelems = countElements;
		for (l = 0; l <= tempClosure->nelems; l++) {
			restoreFunc(&tempClosure->consts[l].ttype, sizeof(lua_Type));
			restoreFunc(&tempClosure->consts[l].value, sizeof(Value));
		}
		arraysObj->object = tempClosure;
		arraysObj++;
	}
	
	Hash *tempHash;
	arraysObj = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayHashTablesCount);
	arrayHashTables = arraysObj;
	for (i = 0; i < arrayHashTablesCount; i++) {
		restoreFunc(&arraysObj->idObj, sizeof(unsigned int));
		tempHash = luaM_new(Hash);
		restoreFunc(&tempHash->nhash, sizeof(int));
		restoreFunc(&tempHash->nuse, sizeof(int));
		restoreFunc(&tempHash->htag, sizeof(int));
		tempHash->node = hashnodecreate(tempHash->nhash);
		luaO_insertlist(&L->roottable, (GCnode *)tempHash);

		for (l = 0; l < tempHash->nuse; l++) {
			restoreFunc(&tempHash->node[l].ref.ttype, sizeof(lua_Type));
			restoreFunc(&tempHash->node[l].ref.value, sizeof(Value));
			restoreFunc(&tempHash->node[l].val.ttype, sizeof(lua_Type));
			restoreFunc(&tempHash->node[l].val.value, sizeof(Value));
		}
		arraysObj->object = tempHash;
		arraysObj++;
	}

	TProtoFunc *tempProtoFunc;
	arrayProtoFuncs = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayProtoFuncsCount);
	arraysObj = arrayProtoFuncs;
	for (i = 0; i < arrayProtoFuncsCount; i++) {
		restoreFunc(&arraysObj->idObj, sizeof(unsigned int));
		tempProtoFunc = luaM_new(TProtoFunc);
		luaO_insertlist(&L->rootproto, (GCnode *)tempProtoFunc);
		restoreFunc(&tempProtoFunc->fileName, sizeof(TaggedString *));
		restoreFunc(&tempProtoFunc->lineDefined, sizeof(int));
		restoreFunc(&tempProtoFunc->nconsts, sizeof(int));
		tempProtoFunc->consts = (TObject *)luaM_malloc(tempProtoFunc->nconsts * sizeof(TObject));

		for (l = 0; l < tempProtoFunc->nconsts; l++) {
			restoreFunc(&tempProtoFunc->consts[l].ttype, sizeof(lua_Type));
			restoreFunc(&tempProtoFunc->consts[l].value, sizeof(Value));
		}

		int countVariables;
		restoreFunc(&countVariables, sizeof(int));
		if (countVariables != 0) {
			tempProtoFunc->locvars = (LocVar *)luaM_malloc(countVariables * sizeof(LocVar));
		} else {
			tempProtoFunc->locvars = NULL;
		}

		for (l = 0; l < countVariables; l++) {
			restoreFunc(&tempProtoFunc->locvars[l].varname, sizeof(TaggedString *));
			restoreFunc(&tempProtoFunc->locvars[l].line, sizeof(int));
		}

		int codeSize;
		restoreFunc(&codeSize, sizeof(int));
		tempProtoFunc->code = (lua_Byte *)luaM_malloc(codeSize);
		restoreFunc(tempProtoFunc->code, codeSize);
		arraysObj->object = tempProtoFunc;
		arraysObj++;
	}

	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tempStringTable = &L->string_root[i];
		for (l = 0; l < tempStringTable->size; l++) {
			TaggedString *tempString = tempStringTable->hash[l];
			if ((tempString != NULL) && (tempString->constindex != -1) && (tempString != &EMPTY)) {
				recreateObj(&tempString->u.s.globalval);
			}
		}
	}

	tempProtoFunc = (TProtoFunc *)L->rootproto.next;
	while (tempProtoFunc != NULL) {
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

	tempHash = (Hash *)L->roottable.next;
	while (tempHash != NULL) {
		for (i = 0; i < tempHash->nuse; i++) {
			recreateObj(&tempHash->node[i].ref);
			recreateObj(&tempHash->node[i].val);
		}
		Node *oldNode = tempHash->node;
		tempHash->node = hashnodecreate(tempHash->nhash);
		for (i = 0; i < tempHash->nuse; i++) {
			Node *newNode = oldNode + i;
			if ((newNode->val.ttype != LUA_T_NIL) && (newNode->ref.ttype != LUA_T_NIL)) {
				*node(tempHash, present(tempHash, &newNode->ref)) = *newNode;
			}
		}
		luaM_free(oldNode);
		tempHash = (Hash *)tempHash->head.next;
	}

	tempClosure = (Closure *)L->rootcl.next;
	while (tempClosure != NULL) {
		for (i = 0; i <= tempClosure->nelems; i++) {
			recreateObj(&tempClosure->consts[i]);
		}
		tempClosure = (Closure *)tempClosure->head.next;
	}

	TaggedString *tempListString = (TaggedString *)&(L->rootglobal);
	for (i = 0; i < rootGlobalCount; i++) {
		TObject tempObj;
		TaggedString *tempString = NULL;
		tempObj.ttype = LUA_T_STRING;
		restoreFunc(&tempObj.value, sizeof(TaggedString *));
		recreateObj(&tempObj);
 		tempString = (TaggedString *)tempObj.value.ts;
		assert(tempString);
		tempListString->head.next = (GCnode *)tempString;
		tempListString = tempString;
	}
	tempListString->head.next = NULL;

	restoreFunc(&L->errorim.ttype, sizeof(lua_Type));
	restoreFunc(&L->errorim.value, sizeof(Value));
	recreateObj(&L->errorim);

	restoreFunc(&L->IMtable_size, sizeof(int));
	L->IMtable = (IM *)luaM_malloc(L->IMtable_size * sizeof(IM));
	for (i = 0; i < L->IMtable_size; i++) {
		IM *im = &L->IMtable[i];
		for (l = 0; l < IM_N; l++) {
			restoreFunc(&im->int_method[l].ttype, sizeof(lua_Type));
			restoreFunc(&im->int_method[l].value, sizeof(Value));
			recreateObj(&im->int_method[l]);
		}
	}

	restoreFunc(&L->last_tag, sizeof(int));
	restoreFunc(&L->refSize, sizeof(int));
	L->refArray = (ref *)luaM_malloc(L->refSize * sizeof(ref));
	for (i = 0; i < L->refSize; i++) {
		restoreFunc(&L->refArray[i].o.ttype, sizeof(lua_Type));
		restoreFunc(&L->refArray[i].o.value, sizeof(Value));
		recreateObj(&L->refArray[i].o);
		restoreFunc(&L->refArray[i].status, sizeof(Status));
	}

	restoreFunc(&L->GCthreshold, sizeof(unsigned long));
	restoreFunc(&L->nblocks, sizeof(unsigned long));

	restoreFunc(&L->Mbuffsize, sizeof(int));
	L->Mbuffer = (char *)luaM_malloc(L->Mbuffsize);
	restoreFunc(L->Mbuffer, L->Mbuffsize);
	int MbaseOffset;
	restoreFunc(&MbaseOffset, sizeof(int));
	L->Mbuffbase = MbaseOffset + L->Mbuffer;
	restoreFunc(&L->Mbuffnext, sizeof(int));

	restoreFunc(&globalTaskSerialId, sizeof(int));

	int countTasks;
	lua_Task *tempTask = NULL;
	restoreFunc(&countTasks, sizeof(int));
	lua_Task *prevTask = L->root_task;
	for (l = 0; l < countTasks; l++) {
		tempTask = luaM_new(lua_Task);
		memset(tempTask, 0, sizeof(lua_Task));
		prevTask->next = tempTask;
		prevTask = tempTask;

		int stackLastSize;
		restoreFunc(&stackLastSize, sizeof(int));
		tempTask->stack.stack = (TObject *)luaM_malloc(stackLastSize * sizeof(TObject));
		tempTask->stack.last = tempTask->stack.stack + stackLastSize - 1;

		int stackTopSize;
		restoreFunc(&stackTopSize, sizeof(int));
		tempTask->stack.top = tempTask->stack.stack + stackTopSize;
		for (i = 0; i < stackTopSize; i++) {
			restoreFunc(&tempTask->stack.stack[i].ttype, sizeof(lua_Type));
			restoreFunc(&tempTask->stack.stack[i].value, sizeof(Value));
			recreateObj(&tempTask->stack.stack[i]);
		}

		restoreFunc(&tempTask->Cstack.base, sizeof(StkId));
		restoreFunc(&tempTask->Cstack.lua2C, sizeof(StkId));
		restoreFunc(&tempTask->Cstack.num, sizeof(int));

		restoreFunc(&tempTask->numCblocks, sizeof(int));
		for (i = 0; i < tempTask->numCblocks; i++) {
			restoreFunc(&tempTask->Cblocks[i].base,	sizeof(StkId));
			restoreFunc(&tempTask->Cblocks[i].lua2C, sizeof(StkId));
			restoreFunc(&tempTask->Cblocks[i].num, sizeof(int));
		}

		int pcOffset, taskCi;
		restoreFunc(&tempTask->base_ci_size, sizeof(int));
		tempTask->base_ci = (CallInfo *)luaM_malloc(tempTask->base_ci_size * sizeof(CallInfo));
		memset(tempTask->base_ci, 0, sizeof(CallInfo) * tempTask->base_ci_size);
		CallInfo *tempCi = tempTask->base_ci;
		int countCi = tempTask->base_ci_size / sizeof(CallInfo);
		for (i = 0; i < countCi; i++) {
			TObject tempObj;
			tempObj.ttype = LUA_T_CLOSURE;
			restoreFunc(&tempObj.value, sizeof(Closure *));
			recreateObj(&tempObj);
			tempCi->c = (Closure *)tempObj.value.cl;
			tempObj.ttype = LUA_T_PROTO;
			restoreFunc(&tempObj.value, sizeof(TProtoFunc *));
			recreateObj(&tempObj);
			tempCi->tf = (TProtoFunc *)tempObj.value.tf;

			restoreFunc(&pcOffset, sizeof(int));
			if (pcOffset != 0)
				tempCi->pc = tempCi->tf->code + pcOffset;
			else
				tempCi->pc = NULL;

			restoreFunc(&tempCi->base, sizeof(StkId));
			restoreFunc(&tempCi->nResults, sizeof(int));
			tempCi++;
		}
		restoreFunc(&taskCi, sizeof(int));
		tempTask->ci = tempTask->base_ci + taskCi;
		tempTask->end_ci = tempTask->base_ci + countCi;

		int Mbasepos;
		restoreFunc(&Mbasepos, sizeof(int));
		tempTask->Mbuffbase = Mbasepos + tempTask->Mbuffer;
		restoreFunc(&tempTask->Mbuffnext, sizeof(int));

		restoreFunc(&tempTask->Tstate, sizeof(TaskState));
		restoreFunc(&tempTask->id, sizeof(int));
	}
	L->last_task = tempTask;

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
