/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/trecision.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"

#include <common/file.h>
#include <common/str.h>

namespace Trecision {

// da cancellare
int VertexNum, FaceNum, MatNum, LightNum;

#define MAXLIGHT	40
#define MAXMAT		20

#define SOUND_OFF		0
#define SOUND_ON		1

int MaxMemory = 0;
// VIDEO
uint16 *Video, VideoPitch;
uint16 *Video2;
// GAME POINTER
uint16 *ImagePointer;
uint16 *SmackImagePointer;
uint16 *ObjPointers[MAXOBJINROOM];
uint8 *MaskPointers[MAXOBJINROOM];
uint8 *SoundPointer[MAXSOUNDSINROOM];
uint8 *_actionPointer[MAXACTIONFRAMESINROOM];		// puntatore progressivo ai frame
uint16 _actionPosition[MAXACTIONINROOM];			// Starting position of each action in the room
uint8 *SmackBuffer[MAXSMACK];
uint8 *SmackTempBuffer[MAXSMACK];
// DATA POINTER
uint16 *Icone;
uint8 *Font;
uint8 *IntroFont;
uint16 *Arrows;
uint8 *TextArea;
uint8 *SpeechBuf[2];
uint8 *MuLawBuf[2];
uint16 *ExtraObj2C;
uint16 *ExtraObj41D;
// 3D AREA
int16  *ZBuffer;
uint8 *_characterArea;
uint8 *TextureArea;
uint8 *BaseHeadTexture;
uint8 AddObjectMaterial;
// MEMORY
uint32 GameBytePointer;
uint32 GameWordPointer;
uint32 TotalMemory;
// VESA
uint8 CVP;
uint16 VesaGranularity;
int32  CurRoomMaxX = 640L;
// DTEXT
int8 DTextLines[MAXDTEXTLINES][MAXDTEXTCHARS];
// SCROLL
int32 VirtualPageLen = 640L;
int32 CurScrollPageDx = 0L;
int32 VideoScrollPageDx = 0L;
// 3D
struct SLight  VLight[MAXLIGHT];
struct SCamera FCamera;
struct STexture FTexture[MAXMAT];
// ANIMATION
uint8 *MemoryArea;
uint32 CurBufferSize;
// SOUND
uint8 SoundSystemActive;
uint8 SpeechTrackEnabled = 0;
uint8 *SoundStartBuffer;
// Temporary variables
FILE  *fhi;
LPFILEHANDLE ff;
char UStr[140];
int32  hh;
// MOUSE
struct SDText TheString;
struct SDText OldString;
int8   MouseONOFF = 1;
uint8  TextStatus;
// AOT.CFG
char  CurCDSet = 1;
// ALTERNATIVE SCROLLING
int   ScrollBottle;
short LeftArrow;
short RightArrow;
// FILEREF
FILEENTRY FileRef[MAXFILEREF];
int NumFileRef;

// info for Toc BmData
struct SBmInfo {
	uint16 px, py, dx, dy;
} BmInfo;

#define NL_REQUIREDMEMORY	6100000L
/*-----------------17/02/95 10.19-------------------
						openSys
--------------------------------------------------*/
void openSys() {
	Common::File testCD;
	for (CurCDSet = 1; CurCDSet <= 3; CurCDSet++) {
		Common::String filename = Common::String::format("NlAnim.cd%c", CurCDSet + '0');
		if (testCD.open(filename)) {
			testCD.close();
			break;
		}
	}

	SoundSystemActive = SOUND_OFF;

	StartSoundSystem();

	CurBufferSize = NL_REQUIREDMEMORY;
	MemoryArea = (uint8 *)malloc(CurBufferSize);

	memset(MemoryArea, 0, CurBufferSize);
	Video2 = (uint16 *)MemoryArea;

	TotalMemory = CurBufferSize;

	OpenVideo();
}
/*-----------------13/09/95 11.59-------------------
					OpenVideo
--------------------------------------------------*/
void OpenVideo() {
	GameBytePointer = 0;
	GameWordPointer = 0;

	sprintf(UStr, "%sData\\NlData.cd0", g_vm->_gamePath);
	//	sprintf( UStr, "Data\\NlData.cd0" );
	FastFileInit(UStr);
	sprintf(UStr, "%sData\\NlSpeech.cd0", g_vm->_gamePath);
	//	sprintf( UStr, "Data\\NlSpeech.cd0" );
	SpeechFileInit(UStr);
	sprintf(UStr, "%sData\\NlAnim.cd%c", g_vm->_gamePath, CurCDSet + '0');
	//	sprintf( UStr, "Data\\NlAnim.cd%c", CurCDSet+'0' );
	AnimFileInit(UStr);

	Video2 = (uint16 *)MemoryArea + 2000000L;

	Font = (uint8 *)MemoryArea + GameBytePointer;
	ff = FastFileOpen("NlFont.fnt");
	GameBytePointer += FastFileRead(ff, (void *)Font, FastFileLen(ff));
	FastFileClose(ff);

	IntroFont = (uint8 *)MemoryArea + GameBytePointer;
	ff = FastFileOpen("NlIntro.fnt");
	GameBytePointer += FastFileRead(ff, (void *)IntroFont, FastFileLen(ff));
	FastFileClose(ff);

	Arrows = (uint16 *)(MemoryArea + GameBytePointer);
	ff = FastFileOpen("frecc.bm");
	GameBytePointer += FastFileRead(ff, (void *)Arrows, FastFileLen(ff));
	FastFileClose(ff);
	UpdatePixelFormat(Arrows, 64000);

	Icone = (uint16 *)(MemoryArea + GameBytePointer);
	wordset(Icone, 0, ICONDX * ICONDY);
	GameBytePointer += (ICONDX * ICONDY * 2);
	ff = FastFileOpen("icone.bm");
	GameBytePointer += FastFileRead(ff, (void *)(Icone + ICONDX * ICONDY), FastFileLen(ff));
	FastFileClose(ff);
	UpdatePixelFormat(Icone + ICONDX * ICONDY, 500000);
	GameBytePointer += (ICONDX * ICONDY * 2 * INVICONNUM);

	//
	TextureArea = (uint8 *)MemoryArea + GameBytePointer;
	BaseHeadTexture = (uint8 *)(TextureArea);

	ff = FastFileOpen("textur.bm");
	GameBytePointer += FastFileRead(ff, TextureArea, FastFileLen(ff));
	FastFileClose(ff);

	// head
	hh = 0;
	FTexture[hh]._dx = 300 / 2;
	FTexture[hh]._dy = 208 / 2;
	FTexture[hh]._angle = 0;
	FTexture[hh]._texture = TextureArea;
	FTexture[hh]._palette = NULL;
	FTexture[hh]._flag = TEXTUREACTIVE + TEXTURECYLIND;

	// corpo
	hh = 1;
	FTexture[hh]._dx = 300;
	FTexture[hh]._dy = 300;
	FTexture[hh]._angle = 0;
	FTexture[hh]._texture = FTexture[0]._texture + (300 * 208) / 4;
	FTexture[hh]._palette = NULL;
	FTexture[hh]._flag = TEXTUREACTIVE + TEXTURECYLIND;

	// braccia
	hh = 2;
	FTexture[hh]._dx = 300;
	FTexture[hh]._dy = 150;
	FTexture[hh]._angle = 0;
	FTexture[hh]._texture = FTexture[1]._texture + 300 * 300;
	FTexture[hh]._palette = NULL;
	FTexture[hh]._flag = TEXTUREACTIVE + TEXTURECYLIND;

	_characterArea = (uint8 *)(MemoryArea + GameBytePointer);
	GameBytePointer += ReadActor("jm.om", (uint8 *)_characterArea);

	_actor._vertexNum = VertexNum;
	_actor._faceNum = FaceNum;
	_actor._light = (struct SLight *)&VLight;
	_actor._lightNum = LightNum;
	_actor._camera = (struct SCamera *)&FCamera;
	_actor._texture = (struct STexture *)&FTexture[0];

	TextArea = (uint8 *)(MemoryArea + GameBytePointer);
	GameBytePointer += MAXTEXTAREA;

	// icone area
	SmackBuffer[2] = (uint8 *)(MemoryArea + GameBytePointer);
	GameBytePointer += ICONDX * ICONDY;
	// zbuffer
	ZBuffer = (int16 *)(MemoryArea + GameBytePointer);
	GameBytePointer += (ZBUFFERSIZE);
	for (int c = 0; c < (ZBUFFERSIZE / 2); c++)
		ZBuffer[c] = 0x7FFF;
	// CDBuffer
	SpeechBuf[0] = (uint8 *)(MemoryArea + GameBytePointer);
	SpeechBuf[1] = (uint8 *)(MemoryArea + GameBytePointer);
	ExtraObj2C = (uint16 *)SpeechBuf[0]; // for room 2C
	GameBytePointer += SPEECHSIZE;
	// omino e full motions area
	SmackBuffer[1] = (uint8 *)(MemoryArea + GameBytePointer);
	ExtraObj41D = (uint16 *)SmackBuffer[1]; // for room 41D
	GameBytePointer += SCREENLEN * AREA;
	// omino buffer
	SmackTempBuffer[1] = (uint8 *)(MemoryArea + GameBytePointer);
	GameBytePointer += SMKANBUFFER;
	// background buffer
	SmackTempBuffer[0] = (uint8 *)(MemoryArea + GameBytePointer);
	GameBytePointer += SMKBKGBUFFER;
	// icone buffer
	SmackTempBuffer[2] = (uint8 *)(MemoryArea + GameBytePointer);
	GameBytePointer += SMKICONBUFFER;
	// background area
	SmackBuffer[0] = (uint8 *)(MemoryArea + GameBytePointer);
	GameBytePointer += SCREENLEN * AREA;
	// SmackImagePointer
	SmackImagePointer = (uint16 *)(MemoryArea + GameBytePointer);
	GameBytePointer += SCREENLEN * AREA * 2;

	Video2 = (uint16 *)(MemoryArea + GameBytePointer);

	if (!SemMouseEnabled)
		Mouse(2); // SPEGNI

	wordset(Video2, 0, 1280L * 480L);
	ShowScreen(0, 0, 640, 480);

	memset(OldObjStatus, 0, MAXOBJINROOM);
	memset(VideoObjStatus, 0, MAXOBJINROOM);
}

/*-----------------13/09/95 11.59-------------------
					ReadActor
--------------------------------------------------*/
uint32 ReadActor(const char *filename, uint8 *Area) {
	int32 ActionNum;

	extern uint16 _textureMat[256][91];
	extern int16  _textureCoord[MAXFACE][3][2];

	UpdatePixelFormat((uint16 *)_textureMat, 256 * 91);

	ff = FastFileOpen(filename);
	if (ff == NULL)
		CloseSys(g_vm->_sysSentence[1]);

	FastFileRead(ff, &ActionNum, 4);

	FastFileRead(ff, &VertexNum, 4);
	_actor._vertexNum = VertexNum;

	_actor._vertex = (struct SVertex *)(Area);
	_characterArea = Area;

	int32 Read = FastFileRead(ff, _actor._vertex, sizeof(struct SVertex) * VertexNum * ActionNum);

	FastFileRead(ff, &FaceNum, 4);
	_actor._faceNum = FaceNum;

	Area += Read;
	_actor._face = (struct SFace *)(Area);
	Read += FastFileRead(ff, _actor._face, sizeof(struct SFace) * FaceNum);
	FastFileClose(ff);

	ff = FastFileOpen("mat.tex");
	FastFileRead(ff, _textureMat, 2 * 91 * 256);
	UpdatePixelFormat((uint16 *)_textureMat, 91 * 256);
	FastFileRead(ff, _textureCoord, 2 * MAXFACE * 3 * 2);
	FastFileRead(ff, _actor._face, sizeof(struct SFace)*FaceNum);
	FastFileClose(ff);

	_actor._curFrame  = 0;
	_actor._curAction = hSTAND;

	// fixup Microprose head correction
	{
#define P1	306
#define P2	348
#define P3	288
		extern float _vertsCorr[104][3];
		extern int _vertsCorrList[84];
		double v1[3], v2[3], v[3], q[3], m1[3][3], m2[3][3], s;
		int c, d, f;

		v1[0] = _actor._vertex[P2]._x - _actor._vertex[P1]._x;
		v1[1] = _actor._vertex[P2]._y - _actor._vertex[P1]._y;
		v1[2] = _actor._vertex[P2]._z - _actor._vertex[P1]._z;
		s = sqrt(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]);
		v1[0] /= s;
		v1[1] /= s;
		v1[2] /= s;

		v2[0] = _actor._vertex[P3]._x - _actor._vertex[P1]._x;
		v2[1] = _actor._vertex[P3]._y - _actor._vertex[P1]._y;
		v2[2] = _actor._vertex[P3]._z - _actor._vertex[P1]._z;
		s = sqrt(v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2]);
		v2[0] /= s;
		v2[1] /= s;
		v2[2] /= s;

		m1[1][0] = v2[1] * v1[2] - v1[1] * v2[2];
		m1[1][1] = v2[2] * v1[0] - v1[2] * v2[0];
		m1[1][2] = v2[0] * v1[1] - v1[0] * v2[1];
		s = sqrt(m1[1][0] * m1[1][0] + m1[1][1] * m1[1][1] + m1[1][2] * m1[1][2]);
		m1[1][0] /= s;
		m1[1][1] /= s;
		m1[1][2] /= s;

		m1[2][0] = m1[1][1] * v1[2] - v1[1] * m1[1][2];
		m1[2][1] = m1[1][2] * v1[0] - v1[2] * m1[1][0];
		m1[2][2] = m1[1][0] * v1[1] - v1[0] * m1[1][1];
		s = sqrt(m1[2][0] * m1[2][0] + m1[2][1] * m1[2][1] + m1[2][2] * m1[2][2]);
		m1[2][0] /= s;
		m1[2][1] /= s;
		m1[2][2] /= s;

		m1[0][0] = v1[0];
		m1[0][1] = v1[1];
		m1[0][2] = v1[2];

		for (int b = 0; b < ActionNum; b++) {
			struct SVertex *sv = (struct SVertex *)(_actor._vertex + b * VertexNum);

			v1[0] = sv[P2]._x - sv[P1]._x;
			v1[1] = sv[P2]._y - sv[P1]._y;
			v1[2] = sv[P2]._z - sv[P1]._z;
			s = sqrt(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]);
			v1[0] /= s;
			v1[1] /= s;
			v1[2] /= s;

			v2[0] = sv[P3]._x - sv[P1]._x;
			v2[1] = sv[P3]._y - sv[P1]._y;
			v2[2] = sv[P3]._z - sv[P1]._z;
			s = sqrt(v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2]);
			v2[0] /= s;
			v2[1] /= s;
			v2[2] /= s;

			m2[1][0] = v2[1] * v1[2] - v1[1] * v2[2];
			m2[1][1] = v2[2] * v1[0] - v1[2] * v2[0];
			m2[1][2] = v2[0] * v1[1] - v1[0] * v2[1];
			s = sqrt(m2[1][0] * m2[1][0] + m2[1][1] * m2[1][1] + m2[1][2] * m2[1][2]);
			m2[1][0] /= s;
			m2[1][1] /= s;
			m2[1][2] /= s;

			m2[2][0] = m2[1][1] * v1[2] - v1[1] * m2[1][2];
			m2[2][1] = m2[1][2] * v1[0] - v1[2] * m2[1][0];
			m2[2][2] = m2[1][0] * v1[1] - v1[0] * m2[1][1];
			s = sqrt(m2[2][0] * m2[2][0] + m2[2][1] * m2[2][1] + m2[2][2] * m2[2][2]);
			m2[2][0] /= s;
			m2[2][1] /= s;
			m2[2][2] /= s;

			m2[0][0] = v1[0];
			m2[0][1] = v1[1];
			m2[0][2] = v1[2];

			v2[0] = sv[P1]._x;
			v2[1] = sv[P1]._y;
			v2[2] = sv[P1]._z;

			v1[0] = _actor._vertex[P1]._x;
			v1[1] = _actor._vertex[P1]._y;
			v1[2] = _actor._vertex[P1]._z;

			for (int e = 279; e < 383; e++) {
				for (f = 0; f < 84; f++)
					if (_vertsCorrList[f] == e)
						break;
				if (f == 84)
					continue;

				v[0] = _vertsCorr[e - 279][0];
				v[1] = _vertsCorr[e - 279][2];
				v[2] = _vertsCorr[e - 279][1];

				q[0] = 0.0;
				q[1] = 0.0;
				q[2] = 0.0;
				for (d = 0; d < 3; d++)
					for (c = 0; c < 3; c++)
						q[c] += m1[c][d] * v[d];
				v[0] = 0.0;
				v[1] = 0.0;
				v[2] = 0.0;
				for (d = 0; d < 3; d++)
					for (c = 0; c < 3; c++)
						v[c] += m2[d][c] * q[d];

				if (b < 42) {
					sv[e]._x += _vertsCorr[e - 279][0];
					sv[e]._y += _vertsCorr[e - 279][2];
					sv[e]._z += _vertsCorr[e - 279][1];
				} else {
					sv[e]._x += v[0];
					sv[e]._y += v[1];
					sv[e]._z += v[2];
				}
			}
		}
	}

	return (Read + 8 + 4);
}

/* -----------------27/06/97 17.52-------------------
					actionInRoom
 --------------------------------------------------*/
int actionInRoom(int curA) {
	int b;

	for (b = 0; b < MAXACTIONINROOM; b++) {
		if (g_vm->_room[g_vm->_curRoom]._actions[b] == curA)
			break;
	}

	if (b >= MAXACTIONINROOM) {
		warning("Action %d not found in room %d", curA, g_vm->_curRoom);
		return (0) ;
	}

	return b;
}

int RoomReady = 0;
/*-----------------17/02/95 10.19-------------------
						ReadLoc
--------------------------------------------------*/
void ReadLoc() {
	extern uint16 _playingAnims[];

	RoomReady = 0;
	if ((g_vm->_curRoom == r11) && !(g_vm->_room[r11]._flag & OBJFLAG_DONE))
		SemShowHomo = 1;

	SoundFadOut();

	memset(Video2, 0, TotalMemory - GameBytePointer);

	GameWordPointer = (CurRoomMaxX * MAXY);           // space for Video2

	sprintf(UStr, "%s.cr", g_vm->_room[g_vm->_curRoom]._baseName);
	ImagePointer = (uint16 *)Video2 + GameWordPointer - 4;

	GameWordPointer += (DecCR(UStr, (uint8 *)ImagePointer, (uint8 *)Video2) + 1) / 2;
	memcpy(&BmInfo, (struct SBmInfo *)ImagePointer, sizeof(struct SBmInfo));
	ImagePointer += 4;
	UpdatePixelFormat(ImagePointer, BmInfo.dx * BmInfo.dy);

	ReadObj();
	if ((g_vm->_room[g_vm->_curRoom]._sounds[0] != 0))
		ReadSounds();

	sprintf(UStr, "%s.3d", g_vm->_room[g_vm->_curRoom]._baseName);
	_actionPointer[0] = (uint8 *)(Video2 + GameWordPointer);
	GameWordPointer += read3D(UStr) / 2;

	sprintf(UStr, "act\\%s.act", g_vm->_room[g_vm->_curRoom]._baseName);

	wordset(Video2, 0, CurRoomMaxX * MAXY);
	MCopy(Video2 + TOP * CurRoomMaxX, ImagePointer, CurRoomMaxX * AREA);

	g_vm->_curSortTableNum = 0;
	memset(OldObjStatus, 0, MAXOBJINROOM);
	memset(VideoObjStatus, 0, MAXOBJINROOM);

	RegenRoom();

	if (g_vm->_room[g_vm->_curRoom]._bkgAnim) {
		wordcopy(SmackImagePointer, ImagePointer, MAXX * AREA);
		StartSmackAnim(g_vm->_room[g_vm->_curRoom]._bkgAnim);
	} else
		StopSmackAnim(_playingAnims[0]);

	InitAtFrameHandler(g_vm->_room[g_vm->_curRoom]._bkgAnim, 0);

	if (MaxMemory < (GameWordPointer * 2 + GameBytePointer))
		MaxMemory = (GameWordPointer * 2 + GameBytePointer);
}

/*-----------------10/12/95 14.59-------------------
					TendIn
--------------------------------------------------*/
void TendIn() {
	TextStatus = TEXT_OFF;

	if (g_vm->_curRoom == rINTRO) {
		PlayDialog(dFLOG);
		return ;
	}

	SemPaintHomo = 1;
	WaitSoundFadEnd();
	PaintScreen(1);

	ShowScreen(0, 0, MAXX, MAXY);
	RoomReady = 1;
//	for(int a=0; a<MAXY; a++ )
//		VCopy(a*VirtualPageLen+VideoScrollPageDx,Video2+a*CurRoomMaxX+CurScrollPageDx,VirtualPageLen);
//	UnlockVideo();
}

/*-----------------17/02/95 10.20-------------------
						ReadObj
--------------------------------------------------*/
void ReadObj() {
	if (!g_vm->_room[g_vm->_curRoom]._object[0])
		return;

	uint16 *o = (uint16 *)ImagePointer + BmInfo.dx * BmInfo.dy;
	uint32 b = 0;
	for (uint16 a = 0; a < MAXOBJINROOM; a++) {
		uint16 c = g_vm->_room[g_vm->_curRoom]._object[a];
		if (!c)
			break;

		if ((g_vm->_curRoom == r41D) && (a == 89))
			break;

		if ((g_vm->_curRoom == r2C) && (a == 20))
			break;

		if (g_vm->_obj[c]._mode & OBJMODE_FULL) {
			wordcopy(&BmInfo, o + b, 4);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			ObjPointers[a] = (uint16 *)(o + b);
			UpdatePixelFormat(ObjPointers[a], (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy));
			b += (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy);
		}

		if ((g_vm->_obj[c]._mode & OBJMODE_MASK)) {
			wordcopy(&BmInfo, o + b, 4);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			uint32 *p = (uint32 *)(o + b);
			ObjPointers[a] = (uint16 *)p + 2;
			UpdatePixelFormat(ObjPointers[a], *p);

			b += (p[0]);
			b += 2;

			p = (uint32 *)(o + b);
			MaskPointers[a] = (uint8 *)p + 4;
			b += (*p / 2);
			b += 2;
		}
	}
}

/*-----------------17/02/95 10.20-------------------
						ReadObj
--------------------------------------------------*/
void ReadExtraObj2C() {
	if (!g_vm->_room[g_vm->_curRoom]._object[32])
		return;
	
	uint16 *o = (uint16 *)ExtraObj2C;
	ff = FastFileOpen("2C2.bm");
	FastFileRead(ff, ExtraObj2C, FastFileLen(ff));
	FastFileClose(ff);

	uint32 b = 0;
	for (uint16 a = 20; a < MAXOBJINROOM; a++) {
		uint16 c = g_vm->_room[g_vm->_curRoom]._object[a];
		if (!c)
			break;

		if (g_vm->_obj[c]._mode & OBJMODE_FULL) {
			wordcopy(&BmInfo, o + b, 4);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			ObjPointers[a] = (uint16 *)(o + b);
			UpdatePixelFormat(ObjPointers[a], (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy));
			b += (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy);
		}

		if ((g_vm->_obj[c]._mode & OBJMODE_MASK)) {
			wordcopy(&BmInfo, o + b, 4);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			uint32 *p = (uint32 *)(o + b);
			ObjPointers[a] = (uint16 *)p + 2;
			UpdatePixelFormat(ObjPointers[a], *p);

			b += (p[0]);
			b += 2;

			p = (uint32 *)(o + b);
			MaskPointers[a] = (uint8 *)p + 4;
			b += (*p / 2);
			b += 2;
		}
	}
}

/*-----------------17/02/95 10.20-------------------
						ReadObj
--------------------------------------------------*/
void ReadExtraObj41D() {
	if (!g_vm->_room[g_vm->_curRoom]._object[32])
		return;

	uint16 *o = (uint16 *)ExtraObj41D;
	ff = FastFileOpen("41D2.bm");
	FastFileRead(ff, ExtraObj41D, FastFileLen(ff));
	FastFileClose(ff);

	uint32 b = 0;
	for (uint16 a = 89; a < MAXOBJINROOM; a++) {
		uint16 c = g_vm->_room[g_vm->_curRoom]._object[a];
		if (!c)
			break;

		if (g_vm->_obj[c]._mode & OBJMODE_FULL) {
			wordcopy(&BmInfo, o + b, 4);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			ObjPointers[a] = (uint16 *)(o + b);
			UpdatePixelFormat(ObjPointers[a], (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy));
			b += (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy);
		}

		if ((g_vm->_obj[c]._mode & OBJMODE_MASK)) {
			wordcopy(&BmInfo, o + b, 4);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			uint32 *p = (uint32 *)(o + b);
			ObjPointers[a] = (uint16 *)p + 2;
			UpdatePixelFormat(ObjPointers[a], *p);

			b += (p[0]);
			b += 2;

			p = (uint32 *)(o + b);
			MaskPointers[a] = (uint8 *)p + 4;
			b += (*p / 2);
			b += 2;
		}
	}
}

/*-----------------12/12/95 11.39-------------------
					ReadSounds
--------------------------------------------------*/
void ReadSounds() {
	if (!SoundSystemActive)
		return;

	for (uint16 a = 0; a < MAXSOUNDSINROOM; a++) {
		uint16 b = g_vm->_room[g_vm->_curRoom]._sounds[a];

		if (b == 0)
			break;

		SoundPointer[a] = (uint8 *)(Video2 + GameWordPointer);

		sprintf(UStr, "%s", GSample[b]._name);
		if (!scumm_stricmp(UStr, "RUOTE2C.WAV")) break;

		ff = FastFileOpen(UStr);
		if (ff == NULL)
			CloseSys(g_vm->_sysSentence[1]);
		int len = FastFileRead(ff, SoundPointer[a], FastFileLen(ff));
		FastFileClose(ff);
		if (LoadAudioWav(b, SoundPointer[a], len))
			len *= 2;

		GameWordPointer += (len + 1) / 2;

		if (GSample[b]._flag & SOUNDFLAG_SBACK)
			SoundFadIn(b);
		else if (GSample[b]._flag & SOUNDFLAG_SON)
			NLPlaySound(b);
	}
}

/*------------------------------------------------
                    RegenRoom
--------------------------------------------------*/
void RegenRoom() {
	for (uint16 a = 0; a < MAXOBJINROOM; a++) {
		if (g_vm->_room[g_vm->_curRoom]._object[a] == 0)
			break;

		int status = (g_vm->_obj[g_vm->_room[g_vm->_curRoom]._object[a]]._mode & OBJMODE_OBJSTATUS);

		if (status) {
			if ((!OldObjStatus[a]) && (g_vm->_obj[g_vm->_room[g_vm->_curRoom]._object[a]]._mode & (OBJMODE_MASK | OBJMODE_FULL))) {
				OldObjStatus[a] = 1;
			}
		} else if ((OldObjStatus[a]) && (g_vm->_obj[g_vm->_room[g_vm->_curRoom]._object[a]]._mode & (OBJMODE_MASK | OBJMODE_FULL)))
			OldObjStatus[a] = 0;
	}
}

/*------------------------------------------------
                  PaintRegenRoom
--------------------------------------------------*/
void PaintRegenRoom() {
	for (uint16 a = 0; a < MAXOBJINROOM; a++) {
		if ((OldObjStatus[a]) && (!VideoObjStatus[a])) {
			SortTable[g_vm->_curSortTableNum]._index = g_vm->_room[g_vm->_curRoom]._object[a];
			SortTable[g_vm->_curSortTableNum]._roomIndex = a;
			SortTable[g_vm->_curSortTableNum]._remove = false;
			SortTable[g_vm->_curSortTableNum]._curFrame = 0;
			SortTable[g_vm->_curSortTableNum]._typology = TYPO_BMP;
			VideoObjStatus[a] = 1;
			g_vm->_curSortTableNum++;
		} else if ((!OldObjStatus[a]) && (VideoObjStatus[a])) {
			SortTable[g_vm->_curSortTableNum]._index = g_vm->_room[g_vm->_curRoom]._object[a];
			SortTable[g_vm->_curSortTableNum]._roomIndex = a;
			SortTable[g_vm->_curSortTableNum]._remove = true;
			SortTable[g_vm->_curSortTableNum]._curFrame = 0;
			SortTable[g_vm->_curSortTableNum]._typology = TYPO_BMP;
			VideoObjStatus[a] = 0;
			g_vm->_curSortTableNum++;
		}

	}
	memcpy(VideoObjStatus, OldObjStatus, MAXOBJINROOM);
}
/*-----------------16/05/95 11.03-------------------
                              DrawObj
--------------------------------------------------*/
void DrawObj(struct SDObj d) {
	uint16 b;

	for (uint16 a = 0; a < 4; a++) {
		if (d.l[a] > (CurRoomMaxX)) {
			//printf("%u %u %u %u\n",d.l[0],d.l[1],d.l[2],d.l[3]);
			return;
		}
	}

	uint16 *buf = d.buf;
	if (d.flag & DRAWMASK) {
		uint8 *mask = d.mask;

		if (d.flag & COPYTORAM) {
			for (b = d.y; b < (d.y + d.dy); b++) {
				uint16 Sco = 0;
				uint16 c = 0;
				while (Sco < d.dx) {
					if (c == 0) {                 /* salta */
						Sco += *mask;
						mask++;

						c = 1;
					} else {                 // copia
						uint16 Now = *mask;

						if ((Now != 0) && (b >= (d.y + d.l[1])) && (b < (d.y + d.l[3]))) {
							if ((Sco >= d.l[0]) && ((Sco + Now) < d.l[2]))
								MCopy(Video2 + (b * CurRoomMaxX) + Sco + d.x, buf, Now);

							else if ((Sco < d.l[0]) && ((Sco + Now) < d.l[2]) && ((Sco + Now) >= d.l[0]))
								MCopy(Video2 + (b * CurRoomMaxX) + d.l[0] + d.x, buf + d.l[0] - Sco, (Now + Sco - d.l[0]));

							else if ((Sco >= d.l[0]) && ((Sco + Now) >= d.l[2]) && (Sco < d.l[2]))
								MCopy(Video2 + (b * CurRoomMaxX) + Sco + d.x, buf, (d.l[2] - Sco));

							else if ((Sco < d.l[0]) && ((Sco + Now) >= d.l[2]))
								MCopy(Video2 + (b * CurRoomMaxX) + d.l[0] + d.x, buf + d.l[0] - Sco, (d.l[2] - d.l[0]));
						}
						Sco += *mask;
						buf += *mask++;
						c = 0;
					}
				}
			}
		}

		if (d.flag & COPYTOVIDEO)
			CloseSys(g_vm->_sysSentence[5]);
	} else {
		if (d.flag & COPYTORAM) {
			for (b = d.l[1]; b < d.l[3]; b++) {
				MCopy(Video2 + (d.y + b)*CurRoomMaxX + (d.x + d.l[0]),
					  buf + (b * d.dx) + d.l[0], (d.l[2] - d.l[0]));
			}
		}

		if (d.flag & COPYTOVIDEO) {
			for (b = d.l[1]; b < d.l[3]; b++) {
				VCopy((d.y + b)*VirtualPageLen + (d.x + d.l[0]),
					  buf + (b * d.dx) + d.l[0], d.l[2] - d.l[0]);
			}

			UnlockVideo();
		}
	}
}

/*-----------------27/02/95 22.16-------------------
                    RegenInventory()
--------------------------------------------------*/
void RegenInventory(uint8 StartIcon, uint8 StartLine) {
	if (StartLine > ICONDY)
		StartLine = ICONDY;

	for (uint16 b = 0; b < ICONDY; b++)
		wordset(Video2 + (FIRSTLINE + b)*CurRoomMaxX + CurScrollPageDx, 0, SCREENLEN);

	for (uint16 a = 0; a < ICONSHOWN; a++) {
		if ((g_vm->_inventory[a + StartIcon] >= LASTICON) /*|| ( _inventory[a+StartIcon] == iEMPTYSLOT )*/) {
			for (uint16 b = 0; b < (ICONDY - StartLine); b++)
				MCopy(Video2 + (FIRSTLINE + b)*CurRoomMaxX + a * (ICONDX) + ICONMARGSX + CurScrollPageDx,
				      Icone + (g_vm->_inventory[a + StartIcon] - LASTICON + READICON + 1) * ICONDX * ICONDY + (b + StartLine) * ICONDX, ICONDX);
		} else if (g_vm->_inventory[a + StartIcon] != g_vm->_lightIcon) {
			for (uint16 b = 0; b < (ICONDY - StartLine); b++)
				MCopy(Video2 + (FIRSTLINE + b)*CurRoomMaxX + a * (ICONDX) + ICONMARGSX + CurScrollPageDx,
				      Icone + g_vm->_inventory[a + StartIcon] * ICONDX * ICONDY + (b + StartLine) * ICONDX, ICONDX);
		}
	}

	// frecce
	if (StartIcon != 0) {							// COPIA LA SINISTRA
		LeftArrow = ICONMARGSX * ICONDY * 3;
		for (uint16 b = 0; b < (ICONDY - StartLine); b++) {
			MCopy(Video2 + (FIRSTLINE + b)*CurRoomMaxX + CurScrollPageDx,
				  Arrows + LeftArrow + (b + StartLine)*ICONMARGSX, ICONMARGSX);
		}
	}

	if ((StartIcon + ICONSHOWN) < g_vm->_inventorySize) { // COPIA LA DESTRA
		RightArrow = ICONMARGDX * ICONDY * 2;
		for (uint16 b = 0; b < (ICONDY - StartLine); b++) {
			MCopy(Video2 + (FIRSTLINE + b)*CurRoomMaxX + CurScrollPageDx + SCREENLEN - ICONMARGDX,
				  Arrows + RightArrow + ICONMARGSX * ICONDY * 2 + (b + StartLine)*ICONMARGSX, ICONMARGSX);
		}
	}
	LeftArrow = 0;
	RightArrow = 0;

	//RegenSmackIcon( StartIcon, 2 );

	VMouseCopy();
	for (uint16 a = 0; a < ICONDY; a++) {
		VCopy((FIRSTLINE + a) * VirtualPageLen + VideoScrollPageDx,
		      Video2 + (FIRSTLINE + a) * CurRoomMaxX + CurScrollPageDx, SCREENLEN);
	}
	VMouseRestore();
	//VMouseON();
	UnlockVideo();
}

/*-----------------16/05/95 22.08-------------------
     RGBColor - Torna un rgb a 8 bit da un _color hi
--------------------------------------------------*/
void  RGBColor(uint16 a, uint8 *r, uint8 *g, uint8 *b) {

	*r = (uint8)(((uint16)((uint16)a >> 10L) & 0x1F) << 3);
	*g = (uint8)(((uint16)((uint16)a >> 5L) & 0x1F) << 3);
	*b = (uint8)(((uint16)((uint16)a) & 0x1F) << 3);
}

/*-----------------16/05/95 22.08-------------------
     ColorRGB - Torna il _color hi a un rgb a 8 bit
--------------------------------------------------*/
uint16 RGB2Color(uint8 r, uint8 g, uint8 b) {
	r >>= 3;
	g >>= 3;
	b >>= 3;

	int16 a = (int16)(((uint16)(b & 0x1F) & 0x1F) +
					  ((uint16)((uint16)(g & 0x1F) << 5L) & 0x3E0) +
					  ((uint16)((uint16)(r & 0x1F) << 10L) & 0x7C00)
					 );

	return a;
}

/* -----------------20/11/97 16.59-------------------
 * 					ULaw2Linear
 * --------------------------------------------------*/
uint16 ULaw2Linear(uint8 ulawbyte) {
	static short exp_lut[8] = {0, 132, 396, 924, 1980, 4092, 8316, 16764};

	ulawbyte = ~ulawbyte;
	short sign = (ulawbyte & 0x80);
	short exponent = (ulawbyte >> 4) & 0x07;
	short mantissa = ulawbyte & 0x0F;
	short sample = exp_lut[exponent] + (mantissa << (exponent + 3));
	if (sign != 0)
		sample = -sample;

	return sample;
}

} // End of namespace Trecision
