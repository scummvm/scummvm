/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm.h"
#include "sound.h"

struct SaveGameHeader {
	uint32 type;
	uint32 size;
	uint32 ver;
	char name[32];
};

#define CURRENT_VER 7

bool Scumm::saveState(int slot, bool compat) {
	char filename[256];
	FILE *out;
	SaveGameHeader hdr;
	Serializer ser;

	makeSavegameName(filename, slot, compat);
	
	out = fopen(filename,"wb");
	if (out==NULL)
		return false;

	memcpy(hdr.name, _saveLoadName, sizeof(hdr.name));
		
	hdr.type = MKID('SCVM');
	hdr.size = 0;
	hdr.ver = CURRENT_VER;
	
	fwrite(&hdr, sizeof(hdr), 1, out);

	ser._saveLoadStream = out;
	ser._saveOrLoad = true;
	saveOrLoad(&ser);

	fclose(out);
	debug(1,"State saved as '%s'", filename);
	return true;
}

bool Scumm::loadState(int slot, bool compat) {
	char filename[256];
 	FILE *out;
	int i,j;
	SaveGameHeader hdr;
	Serializer ser;
	int sb,sh;
	SoundEngine *se;

	makeSavegameName(filename, slot, compat);
	out = fopen(filename,"rb");

	if (out==NULL)
		return false;

	fread(&hdr, sizeof(hdr), 1, out);
	if (hdr.type != MKID('SCVM')) {
		warning("Invalid savegame '%s'", filename);
		fclose(out);
		return false;
	}

	if (hdr.ver != CURRENT_VER) {
		warning("Invalid version of '%s'", filename);
		fclose(out);
		return false;
	}

	memcpy(_saveLoadName, hdr.name, sizeof(hdr.name));

	pauseSounds(true);

	CHECK_HEAP

	openRoom(-1);
	memset(_inventory, 0, sizeof(_inventory[0])*_numInventory);

	/* Nuke all resources */
	for (i=rtFirst; i<=rtLast; i++)
		if (i!=rtTemp && i!=rtBuffer)
			for(j=0; j<res.num[i]; j++) {
				nukeResource(i,j);
				res.flags[i][j] = 0;
			}

	initScummVars();
	
	ser._saveLoadStream = out;
	ser._saveOrLoad = false;
	saveOrLoad(&ser);
	fclose(out);

	sb = _screenB;
	sh = _screenH;

	gdi._mask_left = -1;
	
	initScreens(0, 0, 320, 200);
	_screenEffectFlag = 1;
	unkVirtScreen4(129);

	initScreens(0, sb, 320, sh);

	_completeScreenRedraw = true;
	setDirtyColors(0,255);

	_lastCodePtr = NULL;
 
	_drawObjectQueNr = 0;
	_verbMouseOver = 0;

#if defined(FULL_THROTTLE)
	cameraMoved();
#endif

	initBGBuffers();

	CHECK_HEAP

	debug(1,"State loaded from '%s'", filename);


	pauseSounds(false);

	return true;
}

void Scumm::makeSavegameName(char *out, int slot, bool compatible) {
	sprintf(out, "%s.%c%.2d", _exe_name, compatible ? 'c': 's', slot);
}

bool Scumm::getSavegameName(int slot, char *desc) {
	char filename[256];
	FILE *out;
	SaveGameHeader hdr;
	bool result;
	int len;

	makeSavegameName(filename, slot, false);
	out = fopen(filename,"rb");	
	if (out==NULL) {
		strcpy(desc,"");
		return false;
	}
	len = fread(&hdr, sizeof(hdr), 1, out);
	fclose(out);

	if (len!=1 || hdr.type != MKID('SCVM')) {
		strcpy(desc, "Invalid savegame");
		return false;
	}
	
	if (hdr.ver != CURRENT_VER) {
		strcpy(desc, "Invalid version");
		return false;
	}
	
	memcpy(desc, hdr.name, sizeof(hdr.name));
	desc[sizeof(hdr.name)-1] = 0;
	return true;
}

#define OFFS(type,item) ((int)(&((type*)0)->item))
#define SIZE(type,item) sizeof(((type*)0)->item)
#define MKLINE(type,item,saveas) {OFFS(type,item),saveas,SIZE(type,item)}
#define MKARRAY(type,item,saveas,num) {OFFS(type,item),128|saveas,SIZE(type,item)}, {num,0,0}
#define MKEND() {0xFFFF,0xFF,0xFF}

void Scumm::saveOrLoad(Serializer *s) {
	const SaveLoadEntry objectEntries[] = {
		MKLINE(ObjectData,offs_obim_to_room,sleUint32),
		MKLINE(ObjectData,offs_obcd_to_room,sleUint32),
		MKLINE(ObjectData,walk_x,sleUint16),
		MKLINE(ObjectData,walk_y,sleUint16),
		MKLINE(ObjectData,obj_nr,sleUint16),
		MKLINE(ObjectData,x_pos,sleInt16),
		MKLINE(ObjectData,y_pos,sleInt16),
		MKLINE(ObjectData,width,sleUint16),
		MKLINE(ObjectData,height,sleUint16),
		MKLINE(ObjectData,actordir,sleByte),
		MKLINE(ObjectData,parentstate,sleByte),
		MKLINE(ObjectData,parent,sleByte),
		MKLINE(ObjectData,state,sleByte),
		MKLINE(ObjectData,fl_object_index,sleByte),
		MKEND()
	};

	const SaveLoadEntry actorEntries[] = {
		MKLINE(Actor,x,sleInt16),
		MKLINE(Actor,y,sleInt16),
		MKLINE(Actor,top,sleInt16),
		MKLINE(Actor,bottom,sleInt16),
		MKLINE(Actor,elevation,sleInt16),
		MKLINE(Actor,width,sleUint16),
		MKLINE(Actor,facing,sleUint16),
		MKLINE(Actor,costume,sleUint16),
		MKLINE(Actor,room,sleByte),
		MKLINE(Actor,talkColor,sleByte),
		MKLINE(Actor,scalex,sleByte),
		MKLINE(Actor,scaley,sleByte),
		MKLINE(Actor,charset,sleByte),
		MKARRAY(Actor,sound[0],sleByte, 8),
		MKARRAY(Actor,animVariable[0],sleUint16, 8),
		MKLINE(Actor,newDirection,sleUint16),
		MKLINE(Actor,moving,sleByte),
		MKLINE(Actor,ignoreBoxes,sleByte),
		MKLINE(Actor,forceClip,sleByte),
		MKLINE(Actor,initFrame,sleByte),
		MKLINE(Actor,walkFrame,sleByte),
		MKLINE(Actor,standFrame,sleByte),
		MKLINE(Actor,talkFrame1,sleByte),
		MKLINE(Actor,talkFrame2,sleByte),
		MKLINE(Actor,speedx,sleUint16),
		MKLINE(Actor,speedy,sleUint16),
		MKLINE(Actor,cost.animCounter1,sleUint16),
		MKLINE(Actor,cost.animCounter2,sleByte),
		MKARRAY(Actor,palette[0],sleByte,64),
		MKLINE(Actor,mask,sleByte),
		MKLINE(Actor,shadow_mode,sleByte),
		MKLINE(Actor,visible,sleByte),
		MKLINE(Actor,frame,sleByte),
		MKLINE(Actor,animSpeed,sleByte),
		MKLINE(Actor,animProgress,sleByte),
		MKLINE(Actor,walkbox,sleByte),
		MKLINE(Actor,needRedraw,sleByte),
		MKLINE(Actor,needBgReset,sleByte),
		MKLINE(Actor,costumeNeedsInit,sleByte),

		MKLINE(Actor,new_1,sleInt16),
		MKLINE(Actor,new_2,sleInt16),
		MKLINE(Actor,new_3,sleByte),

		MKLINE(Actor,layer,sleByte),

		MKLINE(Actor,talk_script,sleUint16),
		MKLINE(Actor,walk_script,sleUint16),

		MKLINE(Actor,walkdata.destx,sleInt16),
		MKLINE(Actor,walkdata.desty,sleInt16),
		MKLINE(Actor,walkdata.destbox,sleByte),
		MKLINE(Actor,walkdata.destdir,sleUint16),
		MKLINE(Actor,walkdata.curbox,sleByte),
		MKLINE(Actor,walkdata.x,sleInt16),
		MKLINE(Actor,walkdata.y,sleInt16),
		MKLINE(Actor,walkdata.newx,sleInt16),
		MKLINE(Actor,walkdata.newy,sleInt16),
		MKLINE(Actor,walkdata.XYFactor,sleInt32),
		MKLINE(Actor,walkdata.YXFactor,sleInt32),
		MKLINE(Actor,walkdata.xfrac,sleUint16),
		MKLINE(Actor,walkdata.yfrac,sleUint16),

		MKARRAY(Actor,cost.active[0],sleByte,16),
		MKLINE(Actor,cost.stopped,sleUint16),
		MKARRAY(Actor,cost.curpos[0],sleUint16,16),
		MKARRAY(Actor,cost.start[0],sleUint16,16),
		MKARRAY(Actor,cost.end[0],sleUint16,16),
		MKARRAY(Actor,cost.frame[0],sleUint16,16),
		MKEND()
	};

	const SaveLoadEntry verbEntries[] = {
		MKLINE(VerbSlot,x,sleInt16),
		MKLINE(VerbSlot,y,sleInt16),
		MKLINE(VerbSlot,right,sleInt16),
		MKLINE(VerbSlot,bottom,sleInt16),
		MKLINE(VerbSlot,oldleft,sleInt16),
		MKLINE(VerbSlot,oldtop,sleInt16),
		MKLINE(VerbSlot,oldright,sleInt16),
		MKLINE(VerbSlot,oldbottom,sleInt16),
		MKLINE(VerbSlot,verbid,sleByte),
		MKLINE(VerbSlot,color,sleByte),
		MKLINE(VerbSlot,hicolor,sleByte),
		MKLINE(VerbSlot,dimcolor,sleByte),
		MKLINE(VerbSlot,bkcolor,sleByte),
		MKLINE(VerbSlot,type,sleByte),
		MKLINE(VerbSlot,charset_nr,sleByte),
		MKLINE(VerbSlot,curmode,sleByte),
		MKLINE(VerbSlot,saveid,sleByte),
		MKLINE(VerbSlot,key,sleByte),
		MKLINE(VerbSlot,center,sleByte),
		MKLINE(VerbSlot,field_1B,sleByte),
		MKLINE(VerbSlot,imgindex,sleUint16),
		MKEND()
	};
	
	const SaveLoadEntry mainEntries[] = {
		MKLINE(Scumm,_scrWidth,sleUint16),
		MKLINE(Scumm,_scrHeight,sleUint16),
		MKLINE(Scumm,_ENCD_offs,sleUint32),
		MKLINE(Scumm,_EXCD_offs,sleUint32),
		MKLINE(Scumm,_IM00_offs,sleUint32),
		MKLINE(Scumm,_CLUT_offs,sleUint32),
		MKLINE(Scumm,_EPAL_offs,sleUint32),
		MKLINE(Scumm,_PALS_offs,sleUint32),
		MKLINE(Scumm,_curPalIndex,sleByte),
		MKLINE(Scumm,_currentRoom,sleByte),
		MKLINE(Scumm,_roomResource,sleByte),
		MKLINE(Scumm,_numObjectsInRoom,sleByte),
		MKLINE(Scumm,_currentScript,sleByte),
		MKARRAY(Scumm,_localScriptList[0],sleUint32,NUM_LOCALSCRIPT),
		MKARRAY(Scumm,vm.localvar[0][0],sleUint16,NUM_SCRIPT_SLOT*17),
		MKARRAY(Scumm,_resourceMapper[0],sleByte,128),
		MKARRAY(Scumm,charset._colorMap[0],sleByte,16),
		MKARRAY(Scumm,_charsetData[0][0],sleByte,10*16),
		MKLINE(Scumm,_curExecScript,sleUint16),

#if defined(FULL_THROTTLE)
		MKLINE(Scumm,camera._dest.x,sleInt16),
		MKLINE(Scumm,camera._dest.y,sleInt16),
		MKLINE(Scumm,camera._cur.x,sleInt16),
		MKLINE(Scumm,camera._cur.y,sleInt16),
		MKLINE(Scumm,camera._last.x,sleInt16),
		MKLINE(Scumm,camera._last.y,sleInt16),
		MKLINE(Scumm,camera._accel.x,sleInt16),
		MKLINE(Scumm,camera._accel.y,sleInt16),
		MKLINE(Scumm,camera._follows,sleByte),
		MKLINE(Scumm,camera._movingToActor,sleUint16),
#else
		MKLINE(Scumm,camera._dest.x,sleInt16),
		MKLINE(Scumm,camera._cur.x,sleInt16),
		MKLINE(Scumm,camera._last.x,sleInt16),
		MKLINE(Scumm,_screenStartStrip,sleInt16),
		MKLINE(Scumm,_screenEndStrip,sleInt16),
		MKLINE(Scumm,camera._mode,sleByte),
		MKLINE(Scumm,camera._follows,sleByte),
		MKLINE(Scumm,camera._leftTrigger,sleInt16),
		MKLINE(Scumm,camera._rightTrigger,sleInt16),
		MKLINE(Scumm,camera._movingToActor,sleUint16),
#endif
		MKLINE(Scumm,_actorToPrintStrFor,sleByte),
		MKLINE(Scumm,_charsetColor,sleByte),
		/* XXX Convert into word next time format changes */
		MKLINE(Scumm,charset._bufPos,sleByte),
		MKLINE(Scumm,_haveMsg,sleByte),
		MKLINE(Scumm,_useTalkAnims,sleByte),

		MKLINE(Scumm,_talkDelay,sleInt16),
		MKLINE(Scumm,_defaultTalkDelay,sleInt16),
		MKLINE(Scumm,_numInMsgStack,sleInt16),
		MKLINE(Scumm,_sentenceNum,sleByte),

		MKLINE(Scumm,vm.cutSceneStackPointer,sleByte),
		MKARRAY(Scumm,vm.cutScenePtr[0],sleUint32,5),
		MKARRAY(Scumm,vm.cutSceneScript[0],sleByte,5),
		MKARRAY(Scumm,vm.cutSceneData[0],sleInt16,5),
		MKLINE(Scumm,vm.cutSceneScriptIndex,sleInt16),
		
		/* nest */
		MKLINE(Scumm,_numNestedScripts,sleByte),
		MKLINE(Scumm,_userPut,sleByte),
		MKLINE(Scumm,_cursorState,sleByte),
		MKLINE(Scumm,gdi._cursorActive,sleByte),
		MKLINE(Scumm,gdi._currentCursor,sleByte),

		MKLINE(Scumm,_doEffect,sleByte),
		MKLINE(Scumm,_switchRoomEffect,sleByte),
		MKLINE(Scumm,_newEffect,sleByte),
		MKLINE(Scumm,_switchRoomEffect2,sleByte),
		MKLINE(Scumm,_BgNeedsRedraw,sleByte),

		MKARRAY(Scumm,gfxUsageBits[0],sleUint32,200),
		MKLINE(Scumm,gdi._transparency,sleByte),
		MKARRAY(Scumm,_currentPalette[0],sleByte,768),
		/* virtscr */

		MKARRAY(Scumm,charset._buffer[0],sleByte,256),

		MKLINE(Scumm,_egoPositioned,sleByte),

		MKARRAY(Scumm,gdi._imgBufOffs[0],sleUint16,4),
		MKLINE(Scumm,gdi._numZBuffer,sleByte),

		MKLINE(Scumm,_screenEffectFlag,sleByte),

		MKLINE(Scumm,_randSeed1,sleUint32),
		MKLINE(Scumm,_randSeed2,sleUint32),

		/* XXX: next time the save game format changes,
		 * convert _shakeEnabled to boolean and add a _shakeFrame field */
		MKLINE(Scumm,_shakeEnabled,sleInt16),

		MKLINE(Scumm,_keepText,sleByte),

		MKLINE(Scumm,_screenB,sleUint16),
		MKLINE(Scumm,_screenH,sleUint16),

		MKEND()
	};

	const SaveLoadEntry scriptSlotEntries[] = {
		MKLINE(ScriptSlot,offs,sleUint32),
		MKLINE(ScriptSlot,delay,sleInt32),
		MKLINE(ScriptSlot,number,sleUint16),
		MKLINE(ScriptSlot,newfield,sleUint16),
		MKLINE(ScriptSlot,status,sleByte),
		MKLINE(ScriptSlot,where,sleByte),
		MKLINE(ScriptSlot,unk1,sleByte),
		MKLINE(ScriptSlot,unk2,sleByte),
		MKLINE(ScriptSlot,freezeCount,sleByte),
		MKLINE(ScriptSlot,didexec,sleByte),
		MKLINE(ScriptSlot,cutsceneOverride,sleByte),
		MKLINE(ScriptSlot,unk5,sleByte),
		MKEND()
	};

	const SaveLoadEntry nestedScriptEntries[] = {
		MKLINE(NestedScript,number,sleUint16),
		MKLINE(NestedScript,where,sleByte),
		MKLINE(NestedScript,slot,sleByte),
		MKEND()
	};

	const SaveLoadEntry sentenceTabEntries[] = {
		MKLINE(SentenceTab,unk5,sleUint8),
		MKLINE(SentenceTab,unk2,sleUint8),
		MKLINE(SentenceTab,unk4,sleUint16),
		MKLINE(SentenceTab,unk3,sleUint16),
		MKLINE(SentenceTab,unk,sleUint8),
		MKEND()
	};

	const SaveLoadEntry stringTabEntries[] = {
		MKLINE(StringTab,xpos,sleInt16),
		MKLINE(StringTab,t_xpos,sleInt16),
		MKLINE(StringTab,ypos,sleInt16),
		MKLINE(StringTab,t_ypos,sleInt16),
		MKLINE(StringTab,right,sleInt16),
		MKLINE(StringTab,t_right,sleInt16),
		MKLINE(StringTab,color,sleInt8),
		MKLINE(StringTab,t_color,sleInt8),
		MKLINE(StringTab,charset,sleInt8),
		MKLINE(StringTab,t_charset,sleInt8),
		MKLINE(StringTab,center,sleByte),
		MKLINE(StringTab,t_center,sleByte),
		MKLINE(StringTab,overhead,sleByte),
		MKLINE(StringTab,t_overhead,sleByte),
		MKLINE(StringTab,no_talk_anim,sleByte),
		MKLINE(StringTab,t_no_talk_anim,sleByte),
		MKEND()
	};

	const SaveLoadEntry colorCycleEntries[] = {
		MKLINE(ColorCycle,delay,sleUint16),
		MKLINE(ColorCycle,counter,sleUint16),
		MKLINE(ColorCycle,flags,sleUint16),
		MKLINE(ColorCycle,start,sleByte),
		MKLINE(ColorCycle,end,sleByte),
		MKEND()
	};

	int i,j;
	
	s->saveLoadEntries(this,mainEntries);

	s->saveLoadArrayOf(actor, NUM_ACTORS, sizeof(actor[0]), actorEntries);
	s->saveLoadArrayOf(vm.slot, NUM_SCRIPT_SLOT, sizeof(vm.slot[0]), scriptSlotEntries);
	s->saveLoadArrayOf(_objs, _numLocalObjects, sizeof(_objs[0]), objectEntries);
	s->saveLoadArrayOf(_verbs, _numVerbs, sizeof(_verbs[0]), verbEntries);
	s->saveLoadArrayOf(vm.nest, 16, sizeof(vm.nest[0]), nestedScriptEntries);
	s->saveLoadArrayOf(sentence, 6, sizeof(sentence[0]), sentenceTabEntries);
	s->saveLoadArrayOf(string, 6, sizeof(string[0]), stringTabEntries);
	s->saveLoadArrayOf(_colorCycle, 16, sizeof(_colorCycle[0]), colorCycleEntries);

	for (i=rtFirst; i<=rtLast; i++)
		if (res.mode[i]==0)
			for(j=1; j<res.num[i]; j++)
				saveLoadResource(s,i,j);

	s->saveLoadArrayOf(_objectOwnerTable, _numGlobalObjects, sizeof(_objectOwnerTable[0]), sleByte);
	s->saveLoadArrayOf(_objectStateTable, _numGlobalObjects, sizeof(_objectStateTable[0]), sleByte);
	if (_objectRoomTable)
		s->saveLoadArrayOf(_objectRoomTable, _numGlobalObjects, sizeof(_objectRoomTable[0]), sleByte);

	if (_shadowPaletteSize)
		s->saveLoadArrayOf(_shadowPalette, _shadowPaletteSize, 1, sleByte);

	s->saveLoadArrayOf(_classData, _numGlobalObjects, sizeof(_classData[0]), sleUint32);
	s->saveLoadArrayOf(_vars, _numVariables, sizeof(_vars[0]), sleInt16);
	s->saveLoadArrayOf(_bitVars, _numBitVariables>>3, 1, sleByte);

	/* Save or load a list of the locked objects */
	if (s->isSaving()) {
		for (i=rtFirst; i<=rtLast; i++)
			for(j=1; j<res.num[i]; j++) {
				if (res.flags[i][j]&RF_LOCK) {
					s->saveByte(i);
					s->saveWord(j);
				}
			}
		s->saveByte(0xFF);
	} else {
		int r;
		while ((r = s->loadByte()) != 0xFF) {
			res.flags[r][s->loadWord()] |= RF_LOCK;
		}
	}

	if (_soundEngine)
		((SoundEngine*)_soundEngine)->save_or_load(s);

}

void Scumm::saveLoadResource(Serializer *ser, int type, int index) {
	byte *ptr;
	uint32 size,sizele;
	byte flag;

	/* don't save/load these resource types */
	if (/*type==rtFlObject ||*/ type==rtTemp || type==rtBuffer || res.mode[type])
		return;

	if (ser->isSaving()) {
		ptr = res.address[type][index];
		if (ptr==NULL) {
			ser->saveUint32(0);
			return;
		}

		size = ((MemBlkHeader*)ptr)->size;

		ser->saveUint32(size);
		ser->saveLoadBytes(ptr+sizeof(MemBlkHeader),size);

		if (type==rtInventory) {
			ser->saveWord(_inventory[index]);
		}
	} else {
		size = ser->loadUint32();
		if (size) {
			createResource(type, index, size);
			ser->saveLoadBytes(getResourceAddress(type, index), size);
			if (type==rtInventory) {
				_inventory[index] = ser->loadWord();
			}
		}
	}
}

void Serializer::saveLoadBytes(void *b, int len) {
	if (_saveOrLoad)
		fwrite(b, 1, len, _saveLoadStream);
	else
		fread(b, 1, len, _saveLoadStream);
}

void Serializer::saveUint32(uint32 d) {
	uint32 e = FROM_LE_32(d);
	saveLoadBytes(&e,4);
}

void Serializer::saveWord(uint16 d) {
	uint16 e = FROM_LE_16(d);
	saveLoadBytes(&e,2);
}

void Serializer::saveByte(byte b) {
	saveLoadBytes(&b,1);
}

uint32 Serializer::loadUint32() {
	uint32 e;
	saveLoadBytes(&e,4);
	return FROM_LE_32(e);
}

uint16 Serializer::loadWord() {
	uint16 e;
	saveLoadBytes(&e,2);
	return FROM_LE_16(e);
}

byte Serializer::loadByte() {
	byte e;
	saveLoadBytes(&e,1);
	return e;
}

void Serializer::saveLoadArrayOf(void *b, int len, int datasize, byte filetype) {
	byte *at = (byte*)b;
	uint32 data;

	/* speed up byte arrays */
	if (datasize==1 && filetype==sleByte) {
		saveLoadBytes(b, len);
		return;
	}

	while (--len>=0) {
		if (_saveOrLoad) {
			/* saving */
			if (datasize==1) {
				data = *(byte*)at;
				at += 1;
			} else if (datasize==2) {
				data = *(uint16*)at;
				at += 2;
			} else if (datasize==4) {
				data = *(uint32*)at;
				at += 4;
			} else {
				error("saveLoadArrayOf: invalid size %d", datasize);
			}
			switch(filetype) {
			case sleByte: saveByte(data); break;
			case sleUint16:
			case sleInt16:saveWord(data); break;
			case sleInt32:
			case sleUint32:saveUint32(data); break;
			default:
				error("saveLoadArrayOf: invalid filetype %d", filetype);
			}
		} else {
			/* loading */
			switch(filetype) {
			case sleByte: data = loadByte(); break;
			case sleUint16: data = loadWord(); break;
			case sleInt16: data = (int16)loadWord(); break;
			case sleUint32: data = loadUint32(); break;
			case sleInt32: data = (int32)loadUint32(); break;
			default:
				error("saveLoadArrayOf: invalid filetype %d", filetype);
			}
			if (datasize==1) {
				*(byte*)at = data;
				at += 1;
			} else if (datasize==2) {
				*(uint16*)at = data;
				at += 2;
			} else if (datasize==4) {
				*(uint32*)at = data;
				at += 4;
			} else {
				error("saveLoadArrayOf: invalid size %d", datasize);
			}
		}
	}
}

void Serializer::saveLoadArrayOf(void *b, int num, int datasize, const SaveLoadEntry *sle) {
	byte *data = (byte*)b;

	while (--num>=0) {
		saveLoadEntries(data, sle);		
		data += datasize;
	}
}


void Serializer::saveLoadEntries(void *d, const SaveLoadEntry *sle) {
	int replen;
	byte type;
	byte *at;
	int size;
	int value;
	int num;
	void *ptr;
	
	while(sle->offs != 0xFFFF) {
		at = (byte*)d + sle->offs;
		size = sle->size;
		type = sle->type;
		
		if (size==0xFF) {
			if (_saveOrLoad) {
				/* save reference */
				ptr = *((void**)at);
				saveWord(ptr ? ((*_save_ref)(_ref_me, type, ptr ) + 1) : 0);
			} else {
				/* load reference */
				num = loadWord();
				*((void**)at) = num ? (*_load_ref)(_ref_me, type, num-1) : NULL;
			}
		} else {
			replen = 1;
			if (type&128) {
				sle++;
				replen = sle->offs;
				type&=~128;
			}
			saveLoadArrayOf(at, replen, size, type);
		}
		sle++;
	}
}

