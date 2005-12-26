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

static void saveObjectValue(TObject *object, SaveGame *savedState) {
	int length;
	
	// Note: Now stores the length of the object value
	// ("unsigned int" and "Value" are different lengths)
	savedState->writeBlock(&object->ttype, sizeof(lua_Type));
	if (object->ttype == LUA_T_CPROTO) {
		luaL_libList *list = list_of_libs;
		unsigned long idObj = 0;
		while (list != NULL) {
			for (int l = 0; l < list->number; l++) {
				if (list->list[l].func == object->value.f) {
					idObj = (idObj << 16) | l;
					length = sizeof(unsigned long);
					savedState->writeBlock(&length, sizeof(int));
					savedState->writeBlock(&idObj, length);
					return;
				}
			}
			list = list->next;
			idObj++;
		}
		assert(0);
	} else {
		length = sizeof(Value);
		savedState->writeBlock(&length, sizeof(int));
		savedState->writeBlock(&object->value, length);
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

void lua_Save(SaveGame *savedState) {
	printf("lua_Save() started.\n");

	savedState->beginSection('LUAS');
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

	savedState->writeBlock(&countElements, sizeof(int));
	countElements = 0;

	GCnode *tempNode;
	tempNode = L->rootcl.next;
	while (tempNode != NULL) {
		countElements++;
		tempNode = tempNode->next;
	}
	savedState->writeBlock(&countElements, sizeof(int));
	countElements = 0;

	tempNode = L->roottable.next;
	while (tempNode != NULL) {
		countElements++;
		tempNode = tempNode->next;
	}
	savedState->writeBlock(&countElements, sizeof(int));
	countElements = 0;

	tempNode = L->rootproto.next;
	while (tempNode != NULL) {
		countElements++;
		tempNode = tempNode->next;
	}
	savedState->writeBlock(&countElements, sizeof(int));
	countElements = 0;

	tempNode = L->rootglobal.next;
	while (tempNode != NULL) {
		countElements++;
		tempNode = tempNode->next;
	}
	savedState->writeBlock(&countElements, sizeof(int));

	savedState->writeBlock(&maxStringLength, sizeof(int));

	TaggedString *tempString;
	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tempStringTable = &L->string_root[i];
		for (l = 0; l < tempStringTable->size; l++) {
			if ((tempStringTable->hash[l] != NULL) && (tempStringTable->hash[l] != &EMPTY)) {
				tempString = tempStringTable->hash[l];
				// Save the string object
				savedState->writeBlock(&tempString, sizeof(TaggedString *));
				// Save the constant index
				savedState->writeBlock(&tempString->constindex, sizeof(int));
				if (tempString->constindex != -1) {
					// Save the object value
					saveObjectValue(&tempString->u.s.globalval, savedState);
					// Save the string length
					savedState->writeBlock(&tempString->u.s.len, sizeof(long));
					// Save the string value
					if (tempString->u.s.len != 0)
						savedState->writeBlock(tempString->str, tempString->u.s.len);
				}  else {
					if (saveCallback != NULL) {
						tempString->u.s.globalval.value.ts = (TaggedString *)saveCallback(tempString->u.s.globalval.ttype, (long)tempString->u.s.globalval.value.ts, savedState);
					}
					// Save the object value
					saveObjectValue(&tempString->u.s.globalval, savedState);
				}
			}
		}
	}
	
	Closure *tempClosure = (Closure *)L->rootcl.next;
	while (tempClosure != NULL) {
		savedState->writeBlock(&tempClosure, sizeof(Closure *));
		savedState->writeBlock(&tempClosure->nelems, sizeof(int));
		for(i = 0; i <= tempClosure->nelems; i++) {
			saveObjectValue(&tempClosure->consts[i], savedState);
		}
		tempClosure = (Closure *)tempClosure->head.next;
	}

	Hash *tempHash = (Hash *)L->roottable.next;
	while (tempHash != NULL) {
		savedState->writeBlock(&tempHash, sizeof(Hash *));
		savedState->writeBlock(&tempHash->nhash, sizeof(unsigned int));
		int countUsedHash = 0;
		for(i = 0; i < tempHash->nhash; i++) {
			Node *newNode = &tempHash->node[i];
			if ((newNode->ref.ttype != LUA_T_NIL) && (newNode->val.ttype != LUA_T_NIL)) {
				countUsedHash++;
			}
		}
		savedState->writeBlock(&countUsedHash, sizeof(int));
		savedState->writeBlock(&tempHash->htag, sizeof(int));
		for (i = 0; i < tempHash->nhash; i++) {
			Node *newNode = &tempHash->node[i];
			if ((newNode->val.ttype != LUA_T_NIL) && (newNode->ref.ttype != LUA_T_NIL)) {
				saveObjectValue(&tempHash->node[i].ref, savedState);
				saveObjectValue(&tempHash->node[i].val, savedState);
			}
		}
		tempHash = (Hash *)tempHash->head.next;
	}

	TProtoFunc *tempProtoFunc = (TProtoFunc *)L->rootproto.next;
	while (tempProtoFunc != NULL) {
		savedState->writeBlock(&tempProtoFunc, sizeof(TProtoFunc *));
		savedState->writeBlock(&tempProtoFunc->fileName, sizeof(TaggedString *));
		savedState->writeBlock(&tempProtoFunc->lineDefined, sizeof(unsigned int));
		savedState->writeBlock(&tempProtoFunc->nconsts, sizeof(unsigned int));
		for (i = 0; i < tempProtoFunc->nconsts; i++) {
			saveObjectValue(&tempProtoFunc->consts[i], savedState);
		}
		int countVariables = 0;
		if (tempProtoFunc->locvars) {
			for (; tempProtoFunc->locvars[countVariables++].line != -1;) { }
		}

		savedState->writeBlock(&countVariables, sizeof(int));
		for (i = 0; i < countVariables; i++) {
			savedState->writeBlock(&tempProtoFunc->locvars[i].varname, sizeof(TaggedString *));
			savedState->writeBlock(&tempProtoFunc->locvars[i].line, sizeof(int));
		}

		Byte *codePtr = tempProtoFunc->code + 2;
		Byte *tmpPtr = codePtr;
		int opcodeId;
		do {
			opcodeId = *tmpPtr;
			tmpPtr += opcodeSizeTable[opcodeId];
		} while (opcodeId != ENDCODE);
		int codeSize = (tmpPtr - codePtr) + 2;
		savedState->writeBlock(&codeSize, sizeof(int));
		savedState->writeBlock(tempProtoFunc->code, codeSize);
		tempProtoFunc = (TProtoFunc *)tempProtoFunc->head.next;
	}

	tempString = (TaggedString *)L->rootglobal.next;
	while (tempString != NULL) {
		savedState->writeBlock(&tempString, sizeof(TaggedString *));
		tempString = (TaggedString *)tempString->head.next;
	}

	saveObjectValue(&L->errorim, savedState);

	IM *tempIm = L->IMtable;
	savedState->writeBlock(&L->IMtable_size, sizeof(int));
	for (i = 0; i < L->IMtable_size; i++) {
		for (l = 0; l < IM_N; l++) {
			saveObjectValue(&tempIm->int_method[l], savedState);
		}
		tempIm++;
	}

	savedState->writeBlock(&L->last_tag, sizeof(int));
	savedState->writeBlock(&L->refSize, sizeof(int));
	for (i = 0 ; i < L->refSize; i++) {
		saveObjectValue(&L->refArray[i].o, savedState);
		savedState->writeBlock(&L->refArray[i].status, sizeof(Status));
	}

	savedState->writeBlock(&L->GCthreshold, sizeof(unsigned long));
	savedState->writeBlock(&L->nblocks, sizeof(unsigned long));

	savedState->writeBlock(&L->Mbuffsize, sizeof(int));
	savedState->writeBlock(L->Mbuffer, L->Mbuffsize);
	int MbaseOffset = L->Mbuffbase - L->Mbuffer;
	savedState->writeBlock(&MbaseOffset, sizeof(int));
	savedState->writeBlock(&L->Mbuffnext, sizeof(int));

	savedState->writeBlock(&globalTaskSerialId, sizeof(int));

	int countTasks = 0;
	lua_Task *tempTask = L->root_task->next;
	while (tempTask != NULL) {
		countTasks++;
		tempTask = tempTask->next;
	}
	savedState->writeBlock(&countTasks, sizeof(int));

	tempTask = L->root_task->next;
	while (tempTask != NULL) {
		int stackLastSize = (tempTask->stack.last - tempTask->stack.stack) + 1;
		savedState->writeBlock(&stackLastSize, sizeof(int));
		int stackTopSize = tempTask->stack.top - tempTask->stack.stack;
		savedState->writeBlock(&stackTopSize, sizeof(int));
		for (i = 0; i < stackTopSize; i++) {
			saveObjectValue(&tempTask->stack.stack[i], savedState);
		}

		savedState->writeBlock(&tempTask->Cstack.base, sizeof(StkId));
		savedState->writeBlock(&tempTask->Cstack.lua2C, sizeof(StkId));
		savedState->writeBlock(&tempTask->Cstack.num, sizeof(int));

		savedState->writeBlock(&tempTask->numCblocks, sizeof(int));
		for (i = 0; i < tempTask->numCblocks; i++) {
			savedState->writeBlock(&tempTask->Cblocks[i].base, sizeof(StkId));
			savedState->writeBlock(&tempTask->Cblocks[i].lua2C, sizeof(StkId));
			savedState->writeBlock(&tempTask->Cblocks[i].num, sizeof(int));
		}

		int pcOffset, taskCi = -1;
		savedState->writeBlock(&tempTask->base_ci_size, sizeof(int));
		assert(tempTask->base_ci);
		CallInfo *tempCi = tempTask->base_ci;
		int countCi = tempTask->base_ci_size / sizeof(CallInfo);
		for (i = 0; i < countCi; i++) {
			savedState->writeBlock(&tempCi->c, sizeof(Closure *));
			savedState->writeBlock(&tempCi->tf, sizeof(TProtoFunc *));
			if ((tempCi->pc != NULL) && (tempTask->ci->tf != NULL))
				pcOffset = tempCi->pc - tempCi->tf->code;
			else
				pcOffset = 0;
			savedState->writeBlock(&pcOffset, sizeof(int));
			savedState->writeBlock(&tempCi->base, sizeof(StkId));
			savedState->writeBlock(&tempCi->nResults, sizeof(int));
			if (tempCi == tempTask->ci)
				taskCi = i;
			tempCi++;
		}
		assert(taskCi != -1);
		savedState->writeBlock(&taskCi, sizeof(int));

		MbaseOffset = tempTask->Mbuffbase - tempTask->Mbuffer;
		savedState->writeBlock(&MbaseOffset, sizeof(int));
		savedState->writeBlock(&tempTask->Mbuffnext, sizeof(int));

		savedState->writeBlock(&tempTask->Tstate, sizeof(TaskState));
		savedState->writeBlock(&tempTask->id, sizeof(int));

		tempTask = tempTask->next;
	}

	savedState->endSection();
	printf("lua_Save() finished.\n");
}
