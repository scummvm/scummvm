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
/*	
	printf("lua_Save() started.\n");

	lua_collectgarbage(0);
	int32 i, l;
	int32 countElements = 0;
	int32 maxStringLength = 0;


	// Check for max length for strings and count them
	for (i = 0; i < NUM_HASHS; i++) {
		stringtable *tempStringTable = &lua_state->string_root[i];
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
	tempNode = lua_state->rootcl.next;
	while (tempNode) {
		countElements++;
		tempNode = tempNode->next;
	}
	saveSint32(countElements);

	// save number of tables
	countElements = 0;
	tempNode = lua_state->roottable.next;
	while (tempNode) {
		countElements++;
		tempNode = tempNode->next;
	}
	saveSint32(countElements);

	// save number of prototypes
	countElements = 0;
	tempNode = lua_state->rootproto.next;
	while (tempNode) {
		countElements++;
		tempNode = tempNode->next;
	}
	saveSint32(countElements);

	// save number of global strings
	countElements = 0;
	tempNode = lua_state->rootglobal.next;
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
		stringtable *tempStringTable = &lua_state->string_root[i];
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

	Closure *tempClosure = (Closure *)lua_state->rootcl.next;
	while (tempClosure) {
		saveUint32(makeIdFromPointer(tempClosure).low);
		saveUint32(makeIdFromPointer(tempClosure).hi);
		saveSint32(tempClosure->nelems);
		for (i = 0; i <= tempClosure->nelems; i++) {
			saveObjectValue(&tempClosure->consts[i], saveSint32, saveUint32);
		}
		tempClosure = (Closure *)tempClosure->head.next;
	}

	Hash *tempHash = (Hash *)lua_state->roottable.next;
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

	TProtoFunc *tempProtoFunc = (TProtoFunc *)lua_state->rootproto.next;
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

	tempString = (TaggedString *)lua_state->rootglobal.next;
	while (tempString) {
		saveUint32(makeIdFromPointer(tempString).low);
		saveUint32(makeIdFromPointer(tempString).hi);
		tempString = (TaggedString *)tempString->head.next;
	}

	saveObjectValue(&lua_state->errorim, saveSint32, saveUint32);

	IM *tempIm = lua_state->IMtable;
	saveSint32(lua_state->IMtable_size);
	for (i = 0; i < lua_state->IMtable_size; i++) {
		for (l = 0; l < IM_N; l++) {
			saveObjectValue(&tempIm->int_method[l], saveSint32, saveUint32);
		}
		tempIm++;
	}

	saveSint32(lua_state->last_tag);
	saveSint32(lua_state->refSize);
	for (i = 0 ; i < lua_state->refSize; i++) {
		saveObjectValue(&lua_state->refArray[i].o, saveSint32, saveUint32);
		saveSint32(lua_state->refArray[i].status);
	}

	saveSint32(lua_state->GCthreshold);
	saveSint32(lua_state->nblocks);

	saveSint32(lua_state->Mbuffsize);
	saveStream(lua_state->Mbuffer, lua_state->Mbuffsize);
	int32 MbaseOffset = lua_state->Mbuffbase - lua_state->Mbuffer;
	saveSint32(MbaseOffset);
	saveSint32(lua_state->Mbuffnext);

	saveSint32(globalTaskSerialId);

	int32 countTasks = 0;
	lua_Task *tempTask = lua_state->root_task->next;
	while (tempTask) {
		countTasks++;
		tempTask = tempTask->next;
	}
	saveSint32(countTasks);

	tempTask = lua_state->root_task->next;
	while (tempTask) {
		int32 stackLastSize = (tempTask->stack.last - tempTask->stack.stack) + 1;
		saveSint32(stackLastSize);
		int32 stackTopSize = tempTask->stack.top - tempTask->stack.stack;
		saveSint32(stackTopSize);
		for (i = 0; i < stackTopSize; i++) {
			saveObjectValue(&tempTask->stack.stack[i], saveSint32, saveUint32);
		}

		saveSint32(tempTask->Cstack.base);
		saveSint32(tempTask->Cstack.lua2C);
		saveSint32(tempTask->Cstack.num);

		saveSint32(tempTask->numCblocks);
		for (i = 0; i < tempTask->numCblocks; i++) {
			saveSint32(tempTask->Cblocks[i].base);
			saveSint32(tempTask->Cblocks[i].lua2C);
			saveSint32(tempTask->Cblocks[i].num);
		}

		int32 pcOffset, taskCi = -1;
		saveSint32(tempTask->base_ci_size);
		assert(tempTask->base_ci);
		CallInfo *tempCi = tempTask->base_ci;
		int32 countCi = tempTask->base_ci_size / sizeof(CallInfo);
		for (i = 0; i < countCi; i++) {
			saveUint32(makeIdFromPointer(tempCi->c).low);
			saveUint32(makeIdFromPointer(tempCi->c).hi);
			saveUint32(makeIdFromPointer(tempCi->tf).low);
			saveUint32(makeIdFromPointer(tempCi->tf).hi);
			if (tempCi->pc && tempTask->ci->tf)
				pcOffset = tempCi->pc - tempCi->tf->code;
			else
				pcOffset = 0;
			saveSint32(pcOffset);
			saveSint32(tempCi->base);
			saveSint32(tempCi->nResults);
			if (tempCi == tempTask->ci)
				taskCi = i;
			tempCi++;
		}
		assert(taskCi != -1);
		saveSint32(taskCi);

		MbaseOffset = tempTask->Mbuffbase - tempTask->Mbuffer;
		saveSint32(MbaseOffset);
		saveSint32(tempTask->Mbuffnext);

		saveSint32(tempTask->Tstate);
		saveSint32(tempTask->id);

		tempTask = tempTask->next;
	}

	printf("lua_Save() finished.\n");
*/
}

} // end of namespace Grim
