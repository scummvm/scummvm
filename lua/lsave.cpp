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

SaveRestoreCallback saveCallback = NULL;

static void saveObjectValue(TObject *object, SaveRestoreFunc saveFunc) {
	saveFunc(&object->ttype, sizeof(lua_Type));
	if (object->ttype == LUA_T_CPROTO) {
		luaL_libList *list = list_of_libs;
		unsigned int idObj = 0;
		while (list != NULL) {
			for (int l = 0; l < list->number; l++) {
				if (list->list[l].func == object->value.f) {
					idObj = (idObj << 16) | l;
					saveFunc(&idObj, sizeof(unsigned int));
					return;
				}
			}
			list = list->next;
			idObj++;
		}
		assert(0);
	} else {
		saveFunc(&object->value, sizeof(Value));
	}
}

static int opcodeSizeTable[] = {
	1, 2, 1, 2, 1, 1, 1, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 1,
	1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 3,
	1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1,
	3, 2, 1, 1, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1,
	1, 1, 1, 3, 1, 2, 3, 2, 4, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 1, 1,
	3, 2, 2, 2, 2, 3, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 3, 2, 1, 1,
	1, 1, 1, 1, 1, 1, 3, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2,
	1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 3,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 1, 1, 3, 2, 1, 1, 1, 1, 1,
	1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 2, 3, 2, 4, 2, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 2, 3, 2, 3,
	2, 3, 2, 3, 2, 3, 2, 1, 1, 3, 2, 2, 2, 2, 3, 2, 1, 1
};

void lua_Save(SaveRestoreFunc saveFunc) {
	printf("lua_Save() started.\n");

	lua_collectgarbage(0);
	int i, l;
	int countElements = 0;
	int maxStringLength = 0;
	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tempStringTable = &L->string_root[i];
		for (int l = 0; l < tempStringTable->size; l++) {
			if ((tempStringTable->hash[l] != NULL) && (tempStringTable->hash[l] != &EMPTY)) {
				countElements++;
				if (tempStringTable->hash[l]->constindex != -1) {
					if (maxStringLength < tempStringTable->hash[l]->u.s.len) {
						maxStringLength = tempStringTable->hash[l]->u.s.len;
					}
				}
			}
		}
	}

	saveFunc(&countElements, sizeof(int));
	countElements = 0;

	GCnode *tempNode;
	tempNode = L->rootcl.next;
	while (tempNode != NULL) {
		countElements++;
		tempNode = tempNode->next;
	}
	saveFunc(&countElements, sizeof(int));
	countElements = 0;

	tempNode = L->roottable.next;
	while (tempNode != NULL) {
		countElements++;
		tempNode = tempNode->next;
	}
	saveFunc(&countElements, sizeof(int));
	countElements = 0;

	tempNode = L->rootproto.next;
	while (tempNode != NULL) {
		countElements++;
		tempNode = tempNode->next;
	}
	saveFunc(&countElements, sizeof(int));
	countElements = 0;

	tempNode = L->rootglobal.next;
	while (tempNode != NULL) {
		countElements++;
		tempNode = tempNode->next;
	}
	saveFunc(&countElements, sizeof(int));

	saveFunc(&maxStringLength, sizeof(int));

	TaggedString *tempString;
	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tempStringTable = &L->string_root[i];
		for (l = 0; l < tempStringTable->size; l++) {
			if ((tempStringTable->hash[l] != NULL) && (tempStringTable->hash[l] != &EMPTY)) {
				tempString = tempStringTable->hash[l];
				saveFunc(&tempString, sizeof(TaggedString *));
				saveFunc(&tempString->constindex, sizeof(int));
				if (tempString->constindex != -1) {
					saveObjectValue(&tempString->u.s.globalval, saveFunc);
					saveFunc(&tempString->u.s.len, sizeof(long));
					saveFunc(tempString->str, tempString->u.s.len);
				}  else {
					if (saveCallback != NULL) {
						tempString->u.s.globalval.value.ts = (TaggedString *)saveCallback(tempString->u.s.globalval.ttype, (long)tempString->u.s.globalval.value.ts, saveFunc);
					}
					saveObjectValue(&tempString->u.s.globalval, saveFunc);
				}
			}
		}
	}
	
	Closure *tempClosure = (Closure *)L->rootcl.next;
	while (tempClosure != NULL) {
		saveFunc(&tempClosure, sizeof(Closure *));
		saveFunc(&tempClosure->nelems, sizeof(int));
		for(i = 0; i <= tempClosure->nelems; i++) {
			saveObjectValue(&tempClosure->consts[i], saveFunc);
		}
		tempClosure = (Closure *)tempClosure->head.next;
	}

	Hash *tempHash = (Hash *)L->roottable.next;
	while (tempHash != NULL) {
		saveFunc(&tempHash, sizeof(Hash *));
		saveFunc(&tempHash->nhash, sizeof(unsigned int));
		int countUsedHash = 0;
		for(i = 0; i < tempHash->nhash; i++) {
			Node *newNode = &tempHash->node[i];
			if ((newNode->ref.ttype != LUA_T_NIL) && (newNode->val.ttype != LUA_T_NIL)) {
				countUsedHash++;
			}
		}
		saveFunc(&countUsedHash, sizeof(int));
		saveFunc(&tempHash->htag, sizeof(int));
		for (i = 0; i < tempHash->nhash; i++) {
			Node *newNode = &tempHash->node[i];
			if ((newNode->val.ttype != LUA_T_NIL) && (newNode->ref.ttype != LUA_T_NIL)) {
				saveObjectValue(&tempHash->node[i].ref, saveFunc);
				saveObjectValue(&tempHash->node[i].val, saveFunc);
			}
		}
		tempHash = (Hash *)tempHash->head.next;
	}

	TProtoFunc *tempProtoFunc = (TProtoFunc *)L->rootproto.next;
	while (tempProtoFunc != NULL) {
		saveFunc(&tempProtoFunc, sizeof(TProtoFunc *));
		saveFunc(&tempProtoFunc->fileName, sizeof(TaggedString *));
		saveFunc(&tempProtoFunc->lineDefined, sizeof(unsigned int));
		saveFunc(&tempProtoFunc->nconsts, sizeof(unsigned int));
		for (i = 0; i < tempProtoFunc->nconsts; i++) {
			saveObjectValue(&tempProtoFunc->consts[i], saveFunc);
		}
		int countVariables = 0;
		if (tempProtoFunc->locvars) {
			for (; tempProtoFunc->locvars[countVariables++].line != -1;) { }
		}

		saveFunc(&countVariables, sizeof(int));
		for (i = 0; i < countVariables; i++) {
			saveFunc(&tempProtoFunc->locvars[i].varname, sizeof(TaggedString *));
			saveFunc(&tempProtoFunc->locvars[i].line, sizeof(int));
		}

		Byte *codePtr = tempProtoFunc->code + 2;
		Byte *tmpPtr = codePtr;
		int opcodeId;
		do {
			opcodeId = *tmpPtr;
			tmpPtr += opcodeSizeTable[opcodeId];
		} while (opcodeId != ENDCODE);
		int codeSize = (tmpPtr - codePtr) + 2;
		saveFunc(&codeSize, sizeof(int));
		saveFunc(tempProtoFunc->code, codeSize);
		tempProtoFunc = (TProtoFunc *)tempProtoFunc->head.next;
	}

	tempString = (TaggedString *)L->rootglobal.next;
	while (tempString != NULL) {
		saveFunc(&tempString, sizeof(TaggedString *));
		tempString = (TaggedString *)tempString->head.next;
	}

	saveObjectValue(&L->errorim, saveFunc);

	IM *tempIm = L->IMtable;
	saveFunc(&L->IMtable_size, sizeof(int));
	for (i = 0; i < L->IMtable_size; i++) {
		for (l = 0; l < IM_N; l++) {
			saveObjectValue(&tempIm->int_method[l], saveFunc);
		}
		tempIm++;
	}

	saveFunc(&L->last_tag, sizeof(int));
	saveFunc(&L->refSize, sizeof(int));
	for (i = 0 ; i < L->refSize; i++) {
		saveObjectValue(&L->refArray[i].o, saveFunc);
		saveFunc(&L->refArray[i].status, sizeof(Status));
	}

	saveFunc(&L->GCthreshold, sizeof(unsigned long));
	saveFunc(&L->nblocks, sizeof(unsigned long));

	saveFunc(&L->Mbuffsize, sizeof(int));
	saveFunc(L->Mbuffer, L->Mbuffsize);
	int MbaseOffset = L->Mbuffbase - L->Mbuffer;
	saveFunc(&MbaseOffset, sizeof(int));
	saveFunc(&L->Mbuffnext, sizeof(int));

	saveFunc(&globalTaskSerialId, sizeof(int));

	int countTasks = 0;
	lua_Task *tempTask = L->root_task->next;
	while (tempTask != NULL) {
		countTasks++;
		tempTask = tempTask->next;
	}
	saveFunc(&countTasks, sizeof(int));

	tempTask = L->root_task->next;
	while (tempTask != NULL) {
		int stackLastSize = (tempTask->stack.last - tempTask->stack.stack) + 1;
		saveFunc(&stackLastSize, sizeof(int));
		int stackTopSize = tempTask->stack.top - tempTask->stack.stack;
		saveFunc(&stackTopSize, sizeof(int));
		for (i = 0; i < stackTopSize; i++) {
			saveObjectValue(&tempTask->stack.stack[i], saveFunc);
		}

		saveFunc(&tempTask->Cstack.base, sizeof(StkId));
		saveFunc(&tempTask->Cstack.lua2C, sizeof(StkId));
		saveFunc(&tempTask->Cstack.num, sizeof(int));

		saveFunc(&tempTask->numCblocks, sizeof(int));
		for (i = 0; i < tempTask->numCblocks; i++) {
			saveFunc(&tempTask->Cblocks[i].base, sizeof(StkId));
			saveFunc(&tempTask->Cblocks[i].lua2C, sizeof(StkId));
			saveFunc(&tempTask->Cblocks[i].num, sizeof(int));
		}

		int pcOffset, taskCi = -1;
		saveFunc(&tempTask->base_ci_size, sizeof(int));
		assert(tempTask->base_ci);
		CallInfo *tempCi = tempTask->base_ci;
		int countCi = tempTask->base_ci_size / sizeof(CallInfo);
		for (i = 0; i < countCi; i++) {
			saveFunc(&tempCi->c, sizeof(Closure *));
			saveFunc(&tempCi->tf, sizeof(TProtoFunc *));
			if ((tempCi->pc != NULL) && (tempTask->ci->tf != NULL))
				pcOffset = tempCi->pc - tempCi->tf->code;
			else
				pcOffset = 0;
			saveFunc(&pcOffset, sizeof(int));
			saveFunc(&tempCi->base, sizeof(StkId));
			saveFunc(&tempCi->nResults, sizeof(int));
			if (tempCi == tempTask->ci)
				taskCi = i;
			tempCi++;
		}
		assert(taskCi != -1);
		saveFunc(&taskCi, sizeof(int));

		MbaseOffset = tempTask->Mbuffbase - tempTask->Mbuffer;
		saveFunc(&MbaseOffset, sizeof(int));
		saveFunc(&tempTask->Mbuffnext, sizeof(int));

		saveFunc(&tempTask->Tstate, sizeof(TaskState));
		saveFunc(&tempTask->id, sizeof(int));

		tempTask = tempTask->next;
	}

	printf("lua_Save() finished.\n");
}
