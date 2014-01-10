
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
#include "engines/grim/lua/lua.h"

namespace Grim {

PointerId makeIdFromPointer(void *ptr) {
	PointerId pointer;

#ifdef SCUMM_64BITS
	uint64 v = (uint64)ptr;
	pointer.low = v & 0xffffffff;
	pointer.hi = v >> 32;
#else
	pointer.low = (uint32)ptr;
	pointer.hi = 0;
#endif

	return pointer;
}

void *makePointerFromId(PointerId ptr) {
	void *pointer;

#ifdef SCUMM_64BITS
	uint64 v = ptr.low | ((uint64)ptr.hi << 32);
	pointer = (void *)v;
#else
	pointer = (void *)ptr.low;
#endif

	return pointer;
}

static void saveObjectValue(TObject *object, SaveGame *savedState) {
	savedState->writeLESint32(object->ttype);

	switch (object->ttype) {
		case LUA_T_CPROTO:
		case LUA_T_CMARK:
			{
				luaL_libList *list = list_of_libs;
				int32 idObj = 0;
				while (list) {
					for (int32 l = 0; l < list->number; l++) {
						if (list->list[l].func == object->value.f) {
							idObj = (idObj << 16) | l;
							savedState->writeLESint32(idObj);
							savedState->writeLESint32(0);
							return;
						}
					}
					list = list->next;
					idObj++;
				}
				assert(0);
				break;
			}
		case LUA_T_NUMBER:
		case LUA_T_TASK:
			{
				savedState->writeFloat(object->value.n);
			}
			break;
		case LUA_T_NIL:
			break;
		case LUA_T_ARRAY:
			{
				savedState->writeLEUint32(makeIdFromPointer(object->value.a).low);
				savedState->writeLEUint32(makeIdFromPointer(object->value.a).hi);
			}
			break;
		case LUA_T_USERDATA:
			{
				savedState->writeLESint32(object->value.ud.id);
				savedState->writeLESint32(object->value.ud.tag);
			}
			break;
		case LUA_T_STRING:
			{
				savedState->writeLEUint32(makeIdFromPointer(object->value.ts).low);
				savedState->writeLEUint32(makeIdFromPointer(object->value.ts).hi);
			}
			break;
		case LUA_T_PROTO:
		case LUA_T_PMARK:
			{
				savedState->writeLEUint32(makeIdFromPointer(object->value.tf).low);
				savedState->writeLEUint32(makeIdFromPointer(object->value.tf).hi);
			}
			break;
		case LUA_T_CLOSURE:
		case LUA_T_CLMARK:
			{
				savedState->writeLEUint32(makeIdFromPointer(object->value.cl).low);
				savedState->writeLEUint32(makeIdFromPointer(object->value.cl).hi);
			}
			break;
		case LUA_T_LINE:
			{
				savedState->writeLESint32(object->value.i);
			}
			break;
		default:
			savedState->writeLEUint32(makeIdFromPointer(object->value.ts).low);
			savedState->writeLEUint32(makeIdFromPointer(object->value.ts).hi);
	}
}

static int32 opcodeSizeTable[] = {
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
	savedState->beginSection('LUAS');

	lua_collectgarbage(0);
	int32 i, l;
	int32 countElements = 0;
	int32 maxStringLength = 0;


	// Check for max length for strings and count them
	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tempStringTable = &string_root[i];
		for (l = 0; l < tempStringTable->size; l++) {
			if (tempStringTable->hash[l] && tempStringTable->hash[l] != &EMPTY) {
				countElements++;
				if (tempStringTable->hash[l]->constindex != -1) {
					int len = strlen(tempStringTable->hash[l]->str);
					if (maxStringLength < len) {
						maxStringLength = len;
					}
				}
			}
		}
	}
	// save number of strings
	savedState->writeLESint32(countElements);


	// save number of closures
	countElements = 0;
	GCnode *tempNode;
	tempNode = rootcl.next;
	while (tempNode) {
		countElements++;
		tempNode = tempNode->next;
	}
	savedState->writeLESint32(countElements);

	// save number of tables
	countElements = 0;
	tempNode = roottable.next;
	while (tempNode) {
		countElements++;
		tempNode = tempNode->next;
	}
	savedState->writeLESint32(countElements);

	// save number of prototypes
	countElements = 0;
	tempNode = rootproto.next;
	while (tempNode) {
		countElements++;
		tempNode = tempNode->next;
	}
	savedState->writeLESint32(countElements);

	// save number of global strings
	countElements = 0;
	tempNode = rootglobal.next;
	while (tempNode) {
		countElements++;
		tempNode = tempNode->next;
	}
	savedState->writeLESint32(countElements);

	// save maximum length for string
	savedState->writeLESint32(maxStringLength);

	//printf("1: %d\n", g_grim->_savedState->getBufferPos());

	// save hash tables for strings and user data
	TaggedString *tempString;
	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tempStringTable = &string_root[i];
		for (l = 0; l < tempStringTable->size; l++) {
			if (tempStringTable->hash[l] && tempStringTable->hash[l] != &EMPTY) {
				tempString = tempStringTable->hash[l];
				savedState->writeLEUint32(makeIdFromPointer(tempString).low);
				savedState->writeLEUint32(makeIdFromPointer(tempString).hi);
				savedState->writeLESint32(tempString->constindex);
				if (tempString->constindex != -1) {
					saveObjectValue(&tempString->globalval, savedState);
					int len = strlen(tempString->str);
					savedState->writeLESint32(len);
					savedState->write(tempString->str, len);
				}
			}
		}
	}

	//printf("2: %d\n", g_grim->_savedState->getBufferPos());

	Closure *tempClosure = (Closure *)rootcl.next;
	while (tempClosure) {
		savedState->writeLEUint32(makeIdFromPointer(tempClosure).low);
		savedState->writeLEUint32(makeIdFromPointer(tempClosure).hi);
		savedState->writeLESint32(tempClosure->nelems);
		for (i = 0; i <= tempClosure->nelems; i++) {
			saveObjectValue(&tempClosure->consts[i], savedState);
		}
		tempClosure = (Closure *)tempClosure->head.next;
	}

	Hash *tempHash = (Hash *)roottable.next;
	while (tempHash) {
		savedState->writeLEUint32(makeIdFromPointer(tempHash).low);
		savedState->writeLEUint32(makeIdFromPointer(tempHash).hi);
		savedState->writeLESint32(tempHash->nhash);
		int32 countUsedHash = 0;
		for (i = 0; i < tempHash->nhash; i++) {
			Node *newNode = &tempHash->node[i];
			if (newNode->ref.ttype != LUA_T_NIL && newNode->val.ttype != LUA_T_NIL) {
				countUsedHash++;
			}
		}
		savedState->writeLESint32(countUsedHash);
		savedState->writeLESint32(tempHash->htag);
		for (i = 0; i < tempHash->nhash; i++) {
			Node *newNode = &tempHash->node[i];
			if (newNode->ref.ttype != LUA_T_NIL && newNode->val.ttype != LUA_T_NIL) {
				saveObjectValue(&tempHash->node[i].ref, savedState);
				saveObjectValue(&tempHash->node[i].val, savedState);
			}
		}
		tempHash = (Hash *)tempHash->head.next;
	}

	TProtoFunc *tempProtoFunc = (TProtoFunc *)rootproto.next;
	while (tempProtoFunc) {
		savedState->writeLEUint32(makeIdFromPointer(tempProtoFunc).low);
		savedState->writeLEUint32(makeIdFromPointer(tempProtoFunc).hi);
		savedState->writeLEUint32(makeIdFromPointer(tempProtoFunc->fileName).low);
		savedState->writeLEUint32(makeIdFromPointer(tempProtoFunc->fileName).hi);
		savedState->writeLESint32(tempProtoFunc->lineDefined);
		savedState->writeLESint32(tempProtoFunc->nconsts);
		for (i = 0; i < tempProtoFunc->nconsts; i++) {
			saveObjectValue(&tempProtoFunc->consts[i], savedState);
		}
		int32 countVariables = 0;
		if (tempProtoFunc->locvars) {
			for (; tempProtoFunc->locvars[countVariables++].line != -1;) { }
		}

		savedState->writeLESint32(countVariables);
		for (i = 0; i < countVariables; i++) {
			savedState->writeLEUint32(makeIdFromPointer(tempProtoFunc->locvars[i].varname).low);
			savedState->writeLEUint32(makeIdFromPointer(tempProtoFunc->locvars[i].varname).hi);
			savedState->writeLESint32(tempProtoFunc->locvars[i].line);
		}

		byte *codePtr = tempProtoFunc->code + 2;
		byte *tmpPtr = codePtr;
		int32 opcodeId;
		do {
			opcodeId = *tmpPtr;
			tmpPtr += opcodeSizeTable[opcodeId];
		} while (opcodeId != ENDCODE);
		int32 codeSize = (tmpPtr - codePtr) + 2;
		savedState->writeLESint32(codeSize);
		savedState->write(tempProtoFunc->code, codeSize);
		tempProtoFunc = (TProtoFunc *)tempProtoFunc->head.next;
	}

	tempString = (TaggedString *)rootglobal.next;
	while (tempString) {
		savedState->writeLEUint32(makeIdFromPointer(tempString).low);
		savedState->writeLEUint32(makeIdFromPointer(tempString).hi);
		tempString = (TaggedString *)tempString->head.next;
	}

	saveObjectValue(&errorim, savedState);

	IM *tempIm = IMtable;
	savedState->writeLESint32(IMtable_size);
	for (i = 0; i < IMtable_size; i++) {
		for (l = 0; l < IM_N; l++) {
			saveObjectValue(&tempIm->int_method[l], savedState);
		}
		tempIm++;
	}

	savedState->writeLESint32(last_tag);
	savedState->writeLESint32(refSize);
	for (i = 0 ; i < refSize; i++) {
		saveObjectValue(&refArray[i].o, savedState);
		savedState->writeLESint32(refArray[i].status);
	}

	savedState->writeLESint32(GCthreshold);
	savedState->writeLESint32(nblocks);

	savedState->writeLESint32(Mbuffsize);
	savedState->write(Mbuffer, Mbuffsize);
	int32 MbaseOffset = Mbuffbase - Mbuffer;
	savedState->writeLESint32(MbaseOffset);
	savedState->writeLESint32(Mbuffnext);

	savedState->writeLESint32(globalTaskSerialId);

	int32 countStates = 0, currentState = 0;
	LState *state = lua_rootState;
	while (state) {
		if (lua_state == state)
			currentState = countStates;
		countStates++;
		state = state->next;
	}
	savedState->writeLESint32(countStates);
	savedState->writeLESint32(currentState);

	state = lua_rootState;
	while (state) {
		lua_Task *task = state->task;
		int32 countTasks = 0, n = -1;
		while (task) {
			if (state->some_task && state->some_task == task)
				n = countTasks;
			countTasks++;
			task = task->next;
		}
		savedState->writeLESint32(countTasks);
		task = state->task;
		while (task) {
			savedState->writeLEUint32(makeIdFromPointer(task->cl).low);
			savedState->writeLEUint32(makeIdFromPointer(task->cl).hi);
			savedState->writeLEUint32(makeIdFromPointer(task->tf).low);
			savedState->writeLEUint32(makeIdFromPointer(task->tf).hi);
			savedState->writeLESint32(task->base);
			savedState->writeLESint32(task->some_base);
			savedState->writeLESint32(task->some_results);
			savedState->writeBool(task->some_flag);
			int32 pcOffset = task->pc - task->tf->code;
			savedState->writeLESint32(pcOffset);
			savedState->writeLESint32(task->aux);
			task = task->next;
		}

		savedState->writeLESint32(n);

		savedState->writeBool(state->updated);

		byte pauseState = 0;
		pauseState = state->all_paused & LUA_SG_ALL_PAUSED;
		pauseState |= state->paused ? LUA_SG_PAUSED : 0;
		savedState->writeByte(pauseState);

		savedState->writeLESint32(state->state_counter1);
		savedState->writeLESint32(state->state_counter2);

		int32 stackLastSize = (state->stack.last - state->stack.stack) + 1;
		savedState->writeLESint32(stackLastSize);
		int32 stackTopSize = state->stack.top - state->stack.stack;
		savedState->writeLESint32(stackTopSize);
		for (i = 0; i < stackTopSize; i++) {
			saveObjectValue(&state->stack.stack[i], savedState);
		}

		savedState->writeLESint32(state->Cstack.base);
		savedState->writeLESint32(state->Cstack.lua2C);
		savedState->writeLESint32(state->Cstack.num);

		savedState->writeLESint32(state->numCblocks);
		for (i = 0; i < state->numCblocks; i++) {
			savedState->writeLESint32(state->Cblocks[i].base);
			savedState->writeLESint32(state->Cblocks[i].lua2C);
			savedState->writeLESint32(state->Cblocks[i].num);
		}

		savedState->writeLEUint32(state->sleepFor);
		savedState->writeLEUint32(state->id);
		saveObjectValue(&state->taskFunc, savedState);

		state = state->next;
	}

	savedState->endSection();
}

} // end of namespace Grim
