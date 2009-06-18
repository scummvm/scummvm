#include "engines/grim/savegame.h"
#include "engines/grim/grim.h"

#include "common/endian.h"
#include "common/debug.h"

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

#ifdef TARGET_64BITS
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

#ifdef TARGET_64BITS
	uint64 v = ptr.low | ((uint64)ptr.hi << 32);
	pointer = (void *)v;
#else
	pointer = (void *)ptr.low;
#endif

	return pointer;
}

SaveCallback saveCallbackPtr = NULL;

static void saveObjectValue(TObject *object, SaveSint32 saveSint32, SaveUint32 saveUint32) {
	saveSint32(object->ttype);

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
							saveSint32(idObj);
							saveSint32(0);
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
				byte *udata = (byte *)(&object->value.n);
				uint32 v;
#if defined(SYSTEM_LITTLE_ENDIAN)
				byte b[4];
				b[0] = udata[3];
				b[1] = udata[2];
				b[2] = udata[1];
				b[3] = udata[0];
				v = *(uint32 *)b;
#else
				memcpy(&v, udata, 4);
#endif
				saveUint32(v);
				saveUint32(0);
			}
			break;
		case LUA_T_NIL:
			{
				saveUint32(0);
				saveUint32(0);
			}
			break;
		case LUA_T_ARRAY:
			{
				saveUint32(makeIdFromPointer(object->value.a).low);
				saveUint32(makeIdFromPointer(object->value.a).hi);
			}
			break;
		case LUA_T_USERDATA:
		case LUA_T_STRING:
			{
				saveUint32(makeIdFromPointer(object->value.ts).low);
				saveUint32(makeIdFromPointer(object->value.ts).hi);
			}
			break;
		case LUA_T_PROTO:
		case LUA_T_PMARK:
			{
				saveUint32(makeIdFromPointer(object->value.tf).low);
				saveUint32(makeIdFromPointer(object->value.tf).hi);
			}
			break;
		case LUA_T_CLOSURE:
		case LUA_T_CLMARK:
			{
				saveUint32(makeIdFromPointer(object->value.cl).low);
				saveUint32(makeIdFromPointer(object->value.cl).hi);
			}
			break;
		case LUA_T_LINE:
			{
				saveSint32(object->value.i);
				saveSint32(0);
			}
			break;
		default:
			saveUint32(makeIdFromPointer(object->value.ts).low);
			saveUint32(makeIdFromPointer(object->value.ts).hi);
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

void lua_Save(SaveStream saveStream, SaveSint32 saveSint32, SaveUint32 saveUint32) {
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
	saveSint32(countElements);


	// save number of closures
	countElements = 0;
	GCnode *tempNode;
	tempNode = rootcl.next;
	while (tempNode) {
		countElements++;
		tempNode = tempNode->next;
	}
	saveSint32(countElements);

	// save number of tables
	countElements = 0;
	tempNode = roottable.next;
	while (tempNode) {
		countElements++;
		tempNode = tempNode->next;
	}
	saveSint32(countElements);

	// save number of prototypes
	countElements = 0;
	tempNode = rootproto.next;
	while (tempNode) {
		countElements++;
		tempNode = tempNode->next;
	}
	saveSint32(countElements);

	// save number of global strings
	countElements = 0;
	tempNode = rootglobal.next;
	while (tempNode) {
		countElements++;
		tempNode = tempNode->next;
	}
	saveSint32(countElements);

	// save maximum length for string
	saveSint32(maxStringLength);

	//printf("1: %d\n", g_grim->_savedState->getBufferPos());

	// save hash tables for strings and user data
	TaggedString *tempString;
	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tempStringTable = &string_root[i];
		for (l = 0; l < tempStringTable->size; l++) {
			if (tempStringTable->hash[l] && tempStringTable->hash[l] != &EMPTY) {
				tempString = tempStringTable->hash[l];
				saveUint32(makeIdFromPointer(tempString).low);
				saveUint32(makeIdFromPointer(tempString).hi);
				saveSint32(tempString->constindex);
				if (tempString->constindex != -1) {
					saveObjectValue(&tempString->globalval, saveSint32, saveUint32);
					int len = strlen(tempString->str);
					saveSint32(len);
					saveStream(tempString->str, len);
				} else {
					if (saveCallbackPtr) {
						PointerId ptr = makeIdFromPointer(tempString->globalval.value.ts);
						ptr = saveCallbackPtr(tempString->globalval.ttype, ptr, saveSint32);
						tempString->globalval.value.ts = (TaggedString *)makePointerFromId(ptr);
					}
					saveObjectValue((TObject *)&tempString->globalval, saveSint32, saveUint32);
				}
			}
		}
	}

	//printf("2: %d\n", g_grim->_savedState->getBufferPos());

	Closure *tempClosure = (Closure *)rootcl.next;
	while (tempClosure) {
		saveUint32(makeIdFromPointer(tempClosure).low);
		saveUint32(makeIdFromPointer(tempClosure).hi);
		saveSint32(tempClosure->nelems);
		for (i = 0; i <= tempClosure->nelems; i++) {
			saveObjectValue(&tempClosure->consts[i], saveSint32, saveUint32);
		}
		tempClosure = (Closure *)tempClosure->head.next;
	}

	Hash *tempHash = (Hash *)roottable.next;
	while (tempHash) {
		saveUint32(makeIdFromPointer(tempHash).low);
		saveUint32(makeIdFromPointer(tempHash).hi);
		saveSint32(tempHash->nhash);
		int32 countUsedHash = 0;
		for (i = 0; i < tempHash->nhash; i++) {
			Node *newNode = &tempHash->node[i];
			if (newNode->ref.ttype != LUA_T_NIL && newNode->val.ttype != LUA_T_NIL) {
				countUsedHash++;
			}
		}
		saveSint32(countUsedHash);
		saveSint32(tempHash->htag);
		for (i = 0; i < tempHash->nhash; i++) {
			Node *newNode = &tempHash->node[i];
			if (newNode->ref.ttype != LUA_T_NIL && newNode->val.ttype != LUA_T_NIL) {
				saveObjectValue(&tempHash->node[i].ref, saveSint32, saveUint32);
				saveObjectValue(&tempHash->node[i].val, saveSint32, saveUint32);
			}
		}
		tempHash = (Hash *)tempHash->head.next;
	}

	TProtoFunc *tempProtoFunc = (TProtoFunc *)rootproto.next;
	while (tempProtoFunc) {
		saveUint32(makeIdFromPointer(tempProtoFunc).low);
		saveUint32(makeIdFromPointer(tempProtoFunc).hi);
		saveUint32(makeIdFromPointer(tempProtoFunc->fileName).low);
		saveUint32(makeIdFromPointer(tempProtoFunc->fileName).hi);
		saveSint32(tempProtoFunc->lineDefined);
		saveSint32(tempProtoFunc->nconsts);
		for (i = 0; i < tempProtoFunc->nconsts; i++) {
			saveObjectValue(&tempProtoFunc->consts[i], saveSint32, saveUint32);
		}
		int32 countVariables = 0;
		if (tempProtoFunc->locvars) {
			for (; tempProtoFunc->locvars[countVariables++].line != -1;) { }
		}

		saveSint32(countVariables);
		for (i = 0; i < countVariables; i++) {
			saveUint32(makeIdFromPointer(tempProtoFunc->locvars[i].varname).low);
			saveUint32(makeIdFromPointer(tempProtoFunc->locvars[i].varname).hi);
			saveSint32(tempProtoFunc->locvars[i].line);
		}

		byte *codePtr = tempProtoFunc->code + 2;
		byte *tmpPtr = codePtr;
		int32 opcodeId;
		do {
			opcodeId = *tmpPtr;
			tmpPtr += opcodeSizeTable[opcodeId];
		} while (opcodeId != ENDCODE);
		int32 codeSize = (tmpPtr - codePtr) + 2;
		saveSint32(codeSize);
		saveStream(tempProtoFunc->code, codeSize);
		tempProtoFunc = (TProtoFunc *)tempProtoFunc->head.next;
	}

	tempString = (TaggedString *)rootglobal.next;
	while (tempString) {
		saveUint32(makeIdFromPointer(tempString).low);
		saveUint32(makeIdFromPointer(tempString).hi);
		tempString = (TaggedString *)tempString->head.next;
	}

	saveObjectValue(&errorim, saveSint32, saveUint32);

	IM *tempIm = IMtable;
	saveSint32(IMtable_size);
	for (i = 0; i < IMtable_size; i++) {
		for (l = 0; l < IM_N; l++) {
			saveObjectValue(&tempIm->int_method[l], saveSint32, saveUint32);
		}
		tempIm++;
	}

	saveSint32(last_tag);
	saveSint32(refSize);
	for (i = 0 ; i < refSize; i++) {
		saveObjectValue(&refArray[i].o, saveSint32, saveUint32);
		saveSint32(refArray[i].status);
	}

	saveSint32(GCthreshold);
	saveSint32(nblocks);

	saveSint32(Mbuffsize);
	saveStream(Mbuffer, Mbuffsize);
	int32 MbaseOffset = Mbuffbase - Mbuffer;
	saveSint32(MbaseOffset);
	saveSint32(Mbuffnext);

	saveSint32(globalTaskSerialId);

	int32 countStates = 0, currentState = 0;
	LState *state = lua_rootState;
	while (state) {
		if (lua_rootState == state)
			currentState = countStates;
		countStates++;
		state = state->next;
	}
	saveSint32(countStates);
	saveSint32(currentState);

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
		saveSint32(countTasks);
		task = state->task;
		while (task) {
			saveUint32(makeIdFromPointer(task->cl).low);
			saveUint32(makeIdFromPointer(task->cl).hi);
			saveUint32(makeIdFromPointer(task->tf).low);
			saveUint32(makeIdFromPointer(task->tf).hi);
			saveSint32(task->base);
			saveSint32(task->some_base);
			saveSint32(task->some_results);
			saveSint32(task->some_flag);
			int32 pcOffset = task->pc - task->tf->code;
			saveSint32(pcOffset);
			saveSint32(task->aux);
			task = task->next;
		}

		saveSint32(n);

		saveSint32(state->flag2);
		saveSint32(state->paused);
		saveSint32(state->state_counter1);
		saveSint32(state->state_counter2);

		int32 stackLastSize = (state->stack.last - state->stack.stack) + 1;
		saveSint32(stackLastSize);
		int32 stackTopSize = state->stack.top - state->stack.stack;
		saveSint32(stackTopSize);
		for (i = 0; i < stackTopSize; i++) {
			saveObjectValue(&state->stack.stack[i], saveSint32, saveUint32);
		}

		saveSint32(state->Cstack.base);
		saveSint32(state->Cstack.lua2C);
		saveSint32(state->Cstack.num);

		saveSint32(state->numCblocks);
		for (i = 0; i < state->numCblocks; i++) {
			saveSint32(state->Cblocks[i].base);
			saveSint32(state->Cblocks[i].lua2C);
			saveSint32(state->Cblocks[i].num);
		}

		saveSint32(state->id);
		saveObjectValue(&state->taskFunc, saveSint32, saveUint32);

		state = state->next;
	}
}

} // end of namespace Grim
