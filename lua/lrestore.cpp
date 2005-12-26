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

void restoreObjectValue(lua_Type *objectType, void *objectValue, SaveGame *savedState) {
	int length;
	savedState->readBlock(objectType, sizeof(lua_Type));
	savedState->readBlock(&length, sizeof(int));
	savedState->readBlock(objectValue, length);
}

void lua_Restore(SaveGame *savedState) {
	printf("lua_Restore() started.\n");

	lua_close();
	L = luaM_new(lua_State);
	lua_resetglobals();

	savedState->beginSection('LUAS');
	savedState->readBlock(&arrayStringsCount, sizeof(int));
	savedState->readBlock(&arrayClosuresCount, sizeof(int));
	savedState->readBlock(&arrayHashTablesCount, sizeof(int));
	savedState->readBlock(&arrayProtoFuncsCount, sizeof(int));
	int rootGlobalCount;
	savedState->readBlock(&rootGlobalCount, sizeof(int));

	arrayStrings = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayStringsCount);
	ArrayIDObj *arraysObj = arrayStrings;
	int maxStringsLength;
	savedState->readBlock(&maxStringsLength, sizeof(int));
	char *tempStringBuffer = (char *)luaM_malloc(maxStringsLength);

	int i;
	for (i = 0; i < arrayStringsCount; i++) {
		int constIndex;
		lua_Type tag;
		void *value;
		
		// Restore the string object
		savedState->readBlock(&arraysObj->idObj, sizeof(TaggedString *));
		// Restore the constant index
		savedState->readBlock(&constIndex, sizeof(int));
		// Restore the object value
		restoreObjectValue(&tag, &value, savedState);
		
		TaggedString *tempString;
		if (constIndex != -1) {
			long length;
			// Restore the string length
			savedState->readBlock(&length, sizeof(long));
			if (length == 0) {
				tempString = luaS_newlstr("", 0);
			} else {
				// Restore the string value
				savedState->readBlock(tempStringBuffer, length);
				tempString = luaS_newlstr(tempStringBuffer, length);
			}
			tempString->u.s.globalval.ttype = tag;
			tempString->u.s.globalval.value.ts = (TaggedString *) value;
		} else {
			if (tag == 0)
				tempString = luaS_createudata(value, LUA_ANYTAG);
			else
				tempString = luaS_createudata(value, tag);
			if (restoreCallback != NULL) {
				tempString->u.s.globalval.value.ts = (TaggedString *)restoreCallback(tempString->u.s.globalval.ttype, (long)tempString->u.s.globalval.value.ts, savedState);
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
		savedState->readBlock(&arraysObj->idObj, sizeof(Closure *));
		int countElements;
		savedState->readBlock(&countElements, sizeof(int));
		tempClosure = (Closure *)luaM_malloc((countElements * sizeof(TObject)) + sizeof(Closure));
		luaO_insertlist(&L->rootcl, (GCnode *)tempClosure);

		tempClosure->nelems = countElements;
		for (l = 0; l <= tempClosure->nelems; l++) {
			restoreObjectValue(&tempClosure->consts[l].ttype, &tempClosure->consts[l].value, savedState);
		}
		arraysObj->object = tempClosure;
		arraysObj++;
	}
	
	Hash *tempHash;
	arraysObj = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayHashTablesCount);
	arrayHashTables = arraysObj;
	for (i = 0; i < arrayHashTablesCount; i++) {
		savedState->readBlock(&arraysObj->idObj, sizeof(Hash *));
		tempHash = luaM_new(Hash);
		savedState->readBlock(&tempHash->nhash, sizeof(unsigned int));
		savedState->readBlock(&tempHash->nuse, sizeof(int));
		savedState->readBlock(&tempHash->htag, sizeof(int));
		tempHash->node = hashnodecreate(tempHash->nhash);
		luaO_insertlist(&L->roottable, (GCnode *)tempHash);

		for (l = 0; l < tempHash->nuse; l++) {
			restoreObjectValue(&tempHash->node[l].ref.ttype, &tempHash->node[l].ref.value, savedState);
			restoreObjectValue(&tempHash->node[l].val.ttype, &tempHash->node[l].val.value, savedState);
		}
		arraysObj->object = tempHash;
		arraysObj++;
	}

	TProtoFunc *tempProtoFunc;
	arrayProtoFuncs = (ArrayIDObj *)luaM_malloc(sizeof(ArrayIDObj) * arrayProtoFuncsCount);
	arraysObj = arrayProtoFuncs;
	for (i = 0; i < arrayProtoFuncsCount; i++) {
		savedState->readBlock(&arraysObj->idObj, sizeof(TProtoFunc *));
		tempProtoFunc = luaM_new(TProtoFunc);
		luaO_insertlist(&L->rootproto, (GCnode *)tempProtoFunc);
		savedState->readBlock(&tempProtoFunc->fileName, sizeof(TaggedString *));
		savedState->readBlock(&tempProtoFunc->lineDefined, sizeof(unsigned int));
		savedState->readBlock(&tempProtoFunc->nconsts, sizeof(unsigned int));
		tempProtoFunc->consts = (TObject *)luaM_malloc(tempProtoFunc->nconsts * sizeof(TObject));

		for (l = 0; l < tempProtoFunc->nconsts; l++) {
			restoreObjectValue(&tempProtoFunc->consts[l].ttype, &tempProtoFunc->consts[l].value, savedState);
		}

		int countVariables;
		savedState->readBlock(&countVariables, sizeof(int));
		if (countVariables != 0) {
			tempProtoFunc->locvars = (LocVar *)luaM_malloc(countVariables * sizeof(LocVar));
		} else {
			tempProtoFunc->locvars = NULL;
		}

		for (l = 0; l < countVariables; l++) {
			savedState->readBlock(&tempProtoFunc->locvars[l].varname, sizeof(TaggedString *));
			savedState->readBlock(&tempProtoFunc->locvars[l].line, sizeof(int));
		}

		int codeSize;
		savedState->readBlock(&codeSize, sizeof(int));
		tempProtoFunc->code = (lua_Byte *)luaM_malloc(codeSize);
		savedState->readBlock(tempProtoFunc->code, codeSize);
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
		savedState->readBlock(&tempObj.value, sizeof(TaggedString *));
		recreateObj(&tempObj);
 		tempString = (TaggedString *)tempObj.value.ts;
		assert(tempString);
		tempListString->head.next = (GCnode *)tempString;
		tempListString = tempString;
	}
	tempListString->head.next = NULL;

	restoreObjectValue(&L->errorim.ttype, &L->errorim.value, savedState);
	recreateObj(&L->errorim);

	savedState->readBlock(&L->IMtable_size, sizeof(int));
	L->IMtable = (IM *)luaM_malloc(L->IMtable_size * sizeof(IM));
	for (i = 0; i < L->IMtable_size; i++) {
		IM *im = &L->IMtable[i];
		for (l = 0; l < IM_N; l++) {
			restoreObjectValue(&im->int_method[l].ttype, &im->int_method[l].value, savedState);
			recreateObj(&im->int_method[l]);
		}
	}

	savedState->readBlock(&L->last_tag, sizeof(int));
	savedState->readBlock(&L->refSize, sizeof(int));
	L->refArray = (ref *)luaM_malloc(L->refSize * sizeof(ref));
	for (i = 0; i < L->refSize; i++) {
		restoreObjectValue(&L->refArray[i].o.ttype, &L->refArray[i].o.value, savedState);
		recreateObj(&L->refArray[i].o);
		savedState->readBlock(&L->refArray[i].status, sizeof(Status));
	}

	savedState->readBlock(&L->GCthreshold, sizeof(unsigned long));
	savedState->readBlock(&L->nblocks, sizeof(unsigned long));

	savedState->readBlock(&L->Mbuffsize, sizeof(int));
	L->Mbuffer = (char *)luaM_malloc(L->Mbuffsize);
	savedState->readBlock(L->Mbuffer, L->Mbuffsize);
	int MbaseOffset;
	savedState->readBlock(&MbaseOffset, sizeof(int));
	L->Mbuffbase = MbaseOffset + L->Mbuffer;
	savedState->readBlock(&L->Mbuffnext, sizeof(int));

	savedState->readBlock(&globalTaskSerialId, sizeof(int));

	int countTasks;
	lua_Task *tempTask = NULL;
	savedState->readBlock(&countTasks, sizeof(int));
	lua_Task *prevTask = L->root_task;
	for (l = 0; l < countTasks; l++) {
		tempTask = luaM_new(lua_Task);
		memset(tempTask, 0, sizeof(lua_Task));
		prevTask->next = tempTask;
		prevTask = tempTask;

		int stackLastSize;
		savedState->readBlock(&stackLastSize, sizeof(int));
		tempTask->stack.stack = (TObject *)luaM_malloc(stackLastSize * sizeof(TObject));
		tempTask->stack.last = tempTask->stack.stack + stackLastSize - 1;

		int stackTopSize;
		savedState->readBlock(&stackTopSize, sizeof(int));
		tempTask->stack.top = tempTask->stack.stack + stackTopSize;
		for (i = 0; i < stackTopSize; i++) {
			restoreObjectValue(&tempTask->stack.stack[i].ttype, &tempTask->stack.stack[i].value, savedState);
			recreateObj(&tempTask->stack.stack[i]);
		}

		savedState->readBlock(&tempTask->Cstack.base, sizeof(StkId));
		savedState->readBlock(&tempTask->Cstack.lua2C, sizeof(StkId));
		savedState->readBlock(&tempTask->Cstack.num, sizeof(int));

		savedState->readBlock(&tempTask->numCblocks, sizeof(int));
		for (i = 0; i < tempTask->numCblocks; i++) {
			savedState->readBlock(&tempTask->Cblocks[i].base,	sizeof(StkId));
			savedState->readBlock(&tempTask->Cblocks[i].lua2C, sizeof(StkId));
			savedState->readBlock(&tempTask->Cblocks[i].num, sizeof(int));
		}

		int pcOffset, taskCi;
		savedState->readBlock(&tempTask->base_ci_size, sizeof(int));
		tempTask->base_ci = (CallInfo *)luaM_malloc(tempTask->base_ci_size * sizeof(CallInfo));
		memset(tempTask->base_ci, 0, sizeof(CallInfo) * tempTask->base_ci_size);
		CallInfo *tempCi = tempTask->base_ci;
		int countCi = tempTask->base_ci_size / sizeof(CallInfo);
		for (i = 0; i < countCi; i++) {
			TObject tempObj;
			tempObj.ttype = LUA_T_CLOSURE;
			savedState->readBlock(&tempObj.value, sizeof(Closure *));
			recreateObj(&tempObj);
			tempCi->c = (Closure *)tempObj.value.cl;
			tempObj.ttype = LUA_T_PROTO;
			savedState->readBlock(&tempObj.value, sizeof(TProtoFunc *));
			recreateObj(&tempObj);
			tempCi->tf = (TProtoFunc *)tempObj.value.tf;

			savedState->readBlock(&pcOffset, sizeof(int));
			if (pcOffset != 0)
				tempCi->pc = tempCi->tf->code + pcOffset;
			else
				tempCi->pc = NULL;

			savedState->readBlock(&tempCi->base, sizeof(StkId));
			savedState->readBlock(&tempCi->nResults, sizeof(int));
			tempCi++;
		}
		savedState->readBlock(&taskCi, sizeof(int));
		tempTask->ci = tempTask->base_ci + taskCi;
		tempTask->end_ci = tempTask->base_ci + countCi;

		int Mbasepos;
		savedState->readBlock(&Mbasepos, sizeof(int));
		tempTask->Mbuffbase = Mbasepos + tempTask->Mbuffer;
		savedState->readBlock(&tempTask->Mbuffnext, sizeof(int));

		savedState->readBlock(&tempTask->Tstate, sizeof(TaskState));
		savedState->readBlock(&tempTask->id, sizeof(int));
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

	savedState->endSection();
	printf("lua_Restore() finished.\n");
}
