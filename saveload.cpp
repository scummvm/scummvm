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
 * Change Log:
 * $Log$
 * Revision 1.2  2001/10/10 10:02:33  strigeus
 * alternative mouse cursor
 * basic save&load
 *
 * Revision 1.1.1.1  2001/10/09 14:30:13  strigeus
 *
 * initial revision
 *
 *
 */

#include "stdafx.h"
#include "scumm.h"

bool Scumm::saveState(const char *filename) {
	FILE *out = fopen(filename,"wb");
	if (out==NULL)
		return false;
	saveOrLoad(out,true);
	fclose(out);
	debug(1,"State saved as '%s'", filename);
	return true;
}

bool Scumm::loadState(const char *filename) {
	FILE *out = fopen(filename,"rb");
	int i,j;

	if (out==NULL)
		return false;
	
	checkHeap();

	openRoom(-1);
	memset(_inventory, 0, sizeof(_inventory));

	/* Nuke all resources */
	for (i=1; i<16; i++)
		if (!(i==13 || i==12 || i==10 || res.mode[i]))
			for(j=1; j<res.num[i]; j++)
				nukeResource(i,j);

	initScummVars();
	
	saveOrLoad(out,false);
	fclose(out);

	initScreens(0, _screenB, 320, _screenH);

	_completeScreenRedraw = 1;
	setDirtyColors(0,255);

	_drawObjectQueNr = 0;
	_verbMouseOver = 0;

	initBGBuffers();

	checkHeap();

	debug(1,"State loaded from '%s'", filename);

	return true;
}

#define OFFS(type,item) ((int)(&((type*)0)->item))
#define SIZE(type,item) sizeof(((type*)0)->item)

#define MKLINE(type,item,saveas) {OFFS(type,item),saveas,SIZE(type,item)}
#define MKARRAY(type,item,saveas,num) {OFFS(type,item),128|saveas,SIZE(type,item)}, {num,0,0}
#define MKEND() {0xFFFF,0xFF,0xFF}

void Scumm::saveOrLoad(FILE *inout, bool mode) {
	const SaveLoadEntry objectEntries[] = {
		MKLINE(ObjectData,offs_obim_to_room,sleUint32),
		MKLINE(ObjectData,offs_obcd_to_room,sleUint32),
		MKLINE(ObjectData,cdhd_10,sleUint16),
		MKLINE(ObjectData,cdhd_12,sleUint16),
		MKLINE(ObjectData,obj_nr,sleUint16),
		MKLINE(ObjectData,x_pos,sleByte),
		MKLINE(ObjectData,y_pos,sleByte),
		MKLINE(ObjectData,numstrips,sleByte),
		MKLINE(ObjectData,height,sleByte),
		MKLINE(ObjectData,actordir,sleByte),
		MKLINE(ObjectData,parentstate,sleByte),
		MKLINE(ObjectData,parent,sleByte),
		MKLINE(ObjectData,ownerstate,sleByte),
		MKLINE(ObjectData,fl_object_index,sleByte),
		MKLINE(ObjectData,unk_3,sleByte),
		MKEND()
	};

	const SaveLoadEntry actorEntries[] = {
		MKLINE(Actor,x,sleInt16),
		MKLINE(Actor,y,sleInt16),
		MKLINE(Actor,top,sleInt16),
		MKLINE(Actor,bottom,sleInt16),
		MKLINE(Actor,elevation,sleInt16),
		MKLINE(Actor,width,sleUint16),
		MKLINE(Actor,facing,sleByte),
		MKLINE(Actor,costume,sleByte),
		MKLINE(Actor,room,sleByte),
		MKLINE(Actor,talkColor,sleByte),
		MKLINE(Actor,scalex,sleByte),
		MKLINE(Actor,scaley,sleByte),
		MKLINE(Actor,charset,sleByte),
		MKLINE(Actor,sound,sleByte),
		MKLINE(Actor,newDirection,sleByte),
		MKLINE(Actor,moving,sleByte),
		MKLINE(Actor,ignoreBoxes,sleByte),
		MKLINE(Actor,neverZClip,sleByte),
		MKLINE(Actor,initFrame,sleByte),
		MKLINE(Actor,walkFrame,sleByte),
		MKLINE(Actor,standFrame,sleByte),
		MKLINE(Actor,talkFrame1,sleByte),
		MKLINE(Actor,talkFrame2,sleByte),
		MKLINE(Actor,speedx,sleUint16),
		MKLINE(Actor,speedy,sleUint16),
		MKLINE(Actor,cost.animCounter1,sleUint16),
		MKLINE(Actor,cost.animCounter2,sleByte),
		MKARRAY(Actor,palette[0],sleByte,32),
		MKLINE(Actor,mask,sleByte),
		MKLINE(Actor,visible,sleByte),
		MKLINE(Actor,animIndex,sleByte),
		MKLINE(Actor,animSpeed,sleByte),
		MKLINE(Actor,animProgress,sleByte),
		MKLINE(Actor,walkbox,sleByte),
		MKLINE(Actor,needRedraw,sleByte),
		MKLINE(Actor,needBgReset,sleByte),
		MKLINE(Actor,costumeNeedsInit,sleByte),

		MKLINE(Actor,walkdata.destx,sleInt16),
		MKLINE(Actor,walkdata.desty,sleInt16),
		MKLINE(Actor,walkdata.destbox,sleByte),
		MKLINE(Actor,walkdata.destdir,sleByte),
		MKLINE(Actor,walkdata.curbox,sleByte),
		MKLINE(Actor,walkdata.field_7,sleByte),
		MKLINE(Actor,walkdata.x,sleInt16),
		MKLINE(Actor,walkdata.y,sleInt16),
		MKLINE(Actor,walkdata.newx,sleInt16),
		MKLINE(Actor,walkdata.newy,sleInt16),
		MKLINE(Actor,walkdata.XYFactor,sleInt32),
		MKLINE(Actor,walkdata.YXFactor,sleInt32),
		MKLINE(Actor,walkdata.xfrac,sleUint16),
		MKLINE(Actor,walkdata.yfrac,sleUint16),

		MKLINE(Actor,cost.hdr,sleUint16),
		MKARRAY(Actor,cost.a[0],sleUint16,16),
		MKARRAY(Actor,cost.b[0],sleUint16,16),
		MKARRAY(Actor,cost.c[0],sleUint16,16),
		MKARRAY(Actor,cost.d[0],sleUint16,16),
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
		MKLINE(Scumm,_scrWidthIn8Unit,sleUint16),
		MKLINE(Scumm,_scrHeight,sleUint16),
		MKLINE(Scumm,_ENCD_offs,sleUint32),
		MKLINE(Scumm,_EXCD_offs,sleUint32),
		MKLINE(Scumm,_IM00_offs,sleUint32),
		MKLINE(Scumm,_CLUT_offs,sleUint32),
		MKLINE(Scumm,_EPAL_offs,sleUint32),
		MKLINE(Scumm,_currentRoom,sleByte),
		MKLINE(Scumm,_roomResource,sleByte),
		MKLINE(Scumm,_numObjectsInRoom,sleByte),
		MKLINE(Scumm,_currentScript,sleByte),
		MKARRAY(Scumm,_localScriptList[0],sleUint32,0x39),
		MKARRAY(Scumm,vm.vars[0],sleUint16,801),
		MKARRAY(Scumm,vm.localvar[0],sleUint16,20*17),
		MKARRAY(Scumm,vm.bitvars[0],sleByte,256),
		MKARRAY(Scumm,_resourceMapper[0],sleByte,128),
		MKARRAY(Scumm,charset._colorMap[0],sleByte,16),
		MKARRAY(Scumm,_charsetData[0][0],sleByte,10*16),
		MKLINE(Scumm,_curExecScript,sleUint16),

		MKLINE(Scumm,camera._destPos,sleInt16),
		MKLINE(Scumm,camera._curPos,sleInt16),
		MKLINE(Scumm,camera._lastPos,sleInt16),
		MKLINE(Scumm,_screenStartStrip,sleInt16),
		MKLINE(Scumm,_screenEndStrip,sleInt16),
		MKLINE(Scumm,_scummTimer,sleInt16),
		MKLINE(Scumm,camera._mode,sleByte),
		MKLINE(Scumm,camera._follows,sleByte),
		MKLINE(Scumm,camera._leftTrigger,sleInt16),
		MKLINE(Scumm,camera._rightTrigger,sleInt16),
		MKLINE(Scumm,camera._movingToActor,sleUint16),

		MKLINE(Scumm,_actorToPrintStrFor,sleByte),
		MKLINE(Scumm,_charsetColor,sleByte),
		MKLINE(Scumm,charset._bufPos,sleByte),
		MKLINE(Scumm,_haveMsg,sleByte),

		MKLINE(Scumm,_talkDelay,sleInt16),
		MKLINE(Scumm,_defaultTalkDelay,sleInt16),
		MKLINE(Scumm,_numInMsgStack,sleInt16),
		MKLINE(Scumm,_sentenceIndex,sleByte),

		MKARRAY(Scumm,_sentenceTab[0],sleByte,6),
		MKARRAY(Scumm,_sentenceTab2[0],sleByte,6),
		MKARRAY(Scumm,_sentenceTab3[0],sleUint16,6),
		MKARRAY(Scumm,_sentenceTab4[0],sleUint16,6),
		MKARRAY(Scumm,_sentenceTab5[0],sleByte,6),

		MKLINE(Scumm,vm.cutSceneStackPointer,sleByte),
		MKARRAY(Scumm,vm.cutScenePtr[0],sleUint32,5),
		MKARRAY(Scumm,vm.cutSceneScript[0],sleByte,5),
		MKARRAY(Scumm,vm.cutSceneData[0],sleInt16,5),
		MKLINE(Scumm,vm.cutSceneScriptIndex,sleInt16),
		
		/* nest */
		MKLINE(Scumm,_numNestedScripts,sleByte),
		MKLINE(Scumm,_userPut,sleByte),
		MKLINE(Scumm,_cursorState,sleByte),
		MKLINE(Scumm,gdi.unk4,sleByte),
		MKLINE(Scumm,gdi.currentCursor,sleByte),
//		MKLINE(Scumm,gdi.unk3,sleByte),

		MKLINE(Scumm,dseg_4F8A,sleUint16),
		MKLINE(Scumm,_switchRoomEffect,sleByte),
		MKLINE(Scumm,_newEffect,sleByte),
		MKLINE(Scumm,_switchRoomEffect2,sleByte),
		MKLINE(Scumm,_BgNeedsRedraw,sleByte),

		MKARRAY(Scumm,actorDrawBits[0],sleUint16,160),
		MKLINE(Scumm,gdi.transparency,sleByte),
		MKARRAY(Scumm,_currentPalette[0],sleByte,768),

		/* virtscr */

		MKARRAY(Scumm,charset._buffer[0],sleByte,256),

		MKARRAY(Scumm,textslot.x[0],sleInt16,6),
		MKARRAY(Scumm,textslot.y[0],sleInt16,6),
		MKARRAY(Scumm,textslot.center[0],sleInt16,6),
		MKARRAY(Scumm,textslot.overhead[0],sleInt16,6),
		MKARRAY(Scumm,textslot.right[0],sleInt16,6),
		MKARRAY(Scumm,textslot.color[0],sleInt16,6),
		MKARRAY(Scumm,textslot.charset[0],sleInt16,6),

		MKARRAY(Scumm,_stringXpos[0],sleInt16,6),
		MKARRAY(Scumm,_stringYpos[0],sleInt16,6),
		MKARRAY(Scumm,_stringXpos2[0],sleInt16,6),
		MKARRAY(Scumm,_stringYpos2[0],sleInt16,6),
		MKARRAY(Scumm,_stringCenter[0],sleInt16,6),
		MKARRAY(Scumm,_stringOverhead[0],sleUint16,6),
		MKARRAY(Scumm,_stringRight[0],sleUint16,6),
		MKARRAY(Scumm,_stringColor[0],sleUint16,6),
		MKARRAY(Scumm,_stringCharset[0],sleUint16,6),

		MKLINE(Scumm,charset._mask_left,sleInt16),
		MKLINE(Scumm,charset._mask_top,sleInt16),
		MKLINE(Scumm,charset._mask_right,sleInt16),
		MKLINE(Scumm,charset._mask_bottom,sleInt16),

		MKLINE(Scumm,dseg_3A76,sleUint16),

		MKARRAY(Scumm,_imgBufOffs[0],sleUint16,4),
		MKLINE(Scumm,_numZBuffer,sleUint16),

		MKLINE(Scumm,dseg_4EA0,sleUint16),
		MKLINE(Scumm,dseg_4EA0,sleUint16),

		MKLINE(Scumm,_randSeed1,sleUint32),
		MKLINE(Scumm,_randSeed2,sleUint32),

		MKLINE(Scumm,_shakeMode,sleInt16),

		MKLINE(Scumm,_keepText,sleByte),

		MKLINE(Scumm,_screenB,sleUint16),
		MKLINE(Scumm,_screenH,sleUint16),

		MKARRAY(Scumm,_colorCycleDelays[0],sleUint16,17),
		MKARRAY(Scumm,_colorCycleCounter[0],sleUint16,17),
		MKARRAY(Scumm,_colorCycleFlags[0],sleUint16,17),
		MKARRAY(Scumm,_colorCycleStart[0],sleByte,17),
		MKARRAY(Scumm,_colorCycleEnd[0],sleByte,17),
		
		MKARRAY(Scumm,cost._transEffect[0],sleByte,256),
		MKEND()
	};

	const SaveLoadEntry scriptSlotEntries[] = {
		MKLINE(ScriptSlot,offs,sleUint32),
		MKLINE(ScriptSlot,delay,sleInt32),
		MKLINE(ScriptSlot,number,sleUint16),
		MKLINE(ScriptSlot,status,sleByte),
		MKLINE(ScriptSlot,type,sleByte),
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
		MKLINE(NestedScript,type,sleByte),
		MKLINE(NestedScript,slot,sleByte),
		MKEND()
	};

	int i,j;

	_saveLoadStream = inout;
	_saveOrLoad = mode;

	saveLoadEntries(this,mainEntries);
	for (i=1; i<13; i++)
		saveLoadEntries(&actor[i],actorEntries);
	for (i=0; i<20; i++)
		saveLoadEntries(&vm.slot[i],scriptSlotEntries);
	for (i=0; i<184; i++)
		saveLoadEntries(&objs[i],objectEntries);
	for (i=0; i<102; i++)
		saveLoadEntries(&verbs[i],verbEntries);
	for (i=0; i<16; i++)
		saveLoadEntries(&vm.nest[i],nestedScriptEntries);

	for (i=1; i<16; i++)
		if (res.mode[i]==0)
			for(j=1; j<res.num[i]; j++)
				saveLoadResource(i,j);

	if (_saveOrLoad) {
		for (i=0; i<_maxNrObjects; i++) {
			saveByte(_objectFlagTable[i]);
			saveUint32(_classData[i]);
		}
	} else {
		for (i=0; i<_maxNrObjects; i++) {
			_objectFlagTable[i] = loadByte();
			_classData[i] = loadUint32();
		}
	}
}

void Scumm::saveLoadResource(int type, int index) {
	byte *ptr;
	uint32 size,sizele;

	/* don't save/load these resource types */
	if (type==13 || type==12 || type==10 || res.mode[type])
		return;

	if (_saveOrLoad) {
		ptr = res.address[type][index];
		if (ptr==NULL) {
			saveUint32(0);
			return;
		}

		size = ((ResHeader*)ptr)->size;
		
		saveUint32(size);
		saveLoadBytes(ptr+sizeof(ResHeader),size);

		if (type==5) {
			saveWord(_inventory[index]);
		}
	} else {
		size = loadUint32();
		if (size) {
			createResource(type, index, size);
			saveLoadBytes(getResourceAddress(type, index), size);
			if (type==5) {
				_inventory[index] = loadWord();
			}
		}
	}
}

void Scumm::saveLoadBytes(void *b, int len) {
	if (_saveOrLoad)
		fwrite(b, 1, len, _saveLoadStream);
	else
		fread(b, 1, len, _saveLoadStream);
}

void Scumm::saveUint32(uint32 d) {
	uint32 e = FROM_LE_32(d);
	saveLoadBytes(&e,4);
}

void Scumm::saveWord(uint16 d) {
	uint16 e = FROM_LE_16(d);
	saveLoadBytes(&e,2);
}

void Scumm::saveByte(byte b) {
	saveLoadBytes(&b,1);
}

uint32 Scumm::loadUint32() {
	uint32 e;
	saveLoadBytes(&e,4);
	return FROM_LE_32(e);
}

uint16 Scumm::loadWord() {
	uint16 e;
	saveLoadBytes(&e,2);
	return FROM_LE_16(e);
}

byte Scumm::loadByte() {
	byte e;
	saveLoadBytes(&e,1);
	return e;
}


void Scumm::saveLoadEntries(void *d, const SaveLoadEntry *sle) {
	int replen;
	byte type;
	byte *at;
	int size;
	int value;
	uint32 data;
	
	while(sle->offs != 0xFFFF) {
		at = (byte*)d + sle->offs;
		size = sle->size;
		type = sle->type;
		replen = 1;
		if (type&128) {
			sle++;
			replen = sle->offs;
			type&=~128;
		}
		sle++;

		do {
			if (_saveOrLoad) {
				/* saving */
				if (size==1) {
					data = *(byte*)at;
					at += 1;
				} else if (size==2) {
					data = *(uint16*)at;
					at += 2;
				} else if (size==4) {
					data = *(uint32*)at;
					at += 4;
				} else {
					warning("invalid size %d", size);
				}
				switch(type) {
				case sleByte: saveByte(data); break;
				case sleUint16:
				case sleInt16:saveWord(data); break;
				case sleInt32:
				case sleUint32:saveUint32(data); break;
				default:
					warning("invalid type %d", type);
				}
			} else {
				/* loading */
				switch(type) {
				case sleByte: data = loadByte(); break;
				case sleUint16: data = loadWord(); break;
				case sleInt16: data = (int16)loadWord(); break;
				case sleUint32: data = loadUint32(); break;
				case sleInt32: data = (int32)loadUint32(); break;
				default:
					warning("invalid type %d", type);
				}
				if (size==1) {
					*(byte*)at = data;
					at += 1;
				} else if (size==2) {
					*(uint16*)at = data;
					at += 2;
				} else if (size==4) {
					*(uint32*)at = data;
					at += 4;
				} else {
					warning("invalid size %d", size);
				}
			}
		} while (--replen);
	}
}

