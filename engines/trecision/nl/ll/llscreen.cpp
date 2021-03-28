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

#include "common/scummsys.h"
#include "trecision/trecision.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"

#include "common/file.h"
#include "common/str.h"
#include "trecision/graphics.h"
#include "trecision/video.h"

namespace Trecision {

// da cancellare
int VertexNum, FaceNum, MatNum, LightNum;

#define MAXLIGHT	40
#define MAXMAT		20

int MaxMemory = 0;
// GAME POINTER
uint16 *ImagePointer;
uint16 *SmackImagePointer;
uint16 *ObjPointers[MAXOBJINROOM];
uint8 *MaskPointers[MAXOBJINROOM];
uint8 *SoundPointer[MAXSOUNDSINROOM];
uint8 *_actionPointer[MAXACTIONFRAMESINROOM];		// puntatore progressivo ai frame
uint16 _actionPosition[MAXACTIONINROOM];			// Starting position of each action in the room
// DATA POINTER
uint8 *TextArea;
uint8 *SpeechBuf[2];
uint16 *ExtraObj2C;
uint16 *ExtraObj41D;
// 3D AREA
uint8 *_characterArea;
// MEMORY
uint32 GameBytePointer;
uint32 GameWordPointer;
uint32 TotalMemory;
// VESA
int32  CurRoomMaxX = 640L;
// DTEXT
int8 DTextLines[MAXDTEXTLINES][MAXDTEXTCHARS];
// SCROLL
int32 VirtualPageLen = 640L;
int32 CurScrollPageDx = 0L;
int32 VideoScrollPageDx = 0L;
// 3D
SLight  VLight[MAXLIGHT];
SCamera FCamera;
STexture FTexture[MAXMAT];
// ANIMATION
uint8 *MemoryArea;
uint32 CurBufferSize;
// SOUND
bool SoundSystemActive;
bool SpeechTrackEnabled = false;
uint8 *SoundStartBuffer;
// Temporary variables
Common::SeekableReadStream *ff;
int32  hh;
// MOUSE
SDText curString;
SDText oldString;
uint8  TextStatus;
// AOT.CFG
char  CurCDSet = 1;
// FILEREF
SFileEntry FileRef[MAXFILEREF];
int NumFileRef;

// info for Toc BmData
struct SBmInfo {
	uint16 px, py, dx, dy;

	void read(uint16 *buf) {
		px = *buf++;
		py = *buf++;
		dx = *buf++;
		dy = *buf++;
	}
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

	SoundSystemActive = false;

	StartSoundSystem();

	CurBufferSize = NL_REQUIREDMEMORY;
	MemoryArea = (uint8 *)malloc(CurBufferSize);

	memset(MemoryArea, 0, CurBufferSize);
	g_vm->_video2 = (uint16 *)MemoryArea;

	TotalMemory = CurBufferSize;

	OpenVideo();
}
/*-----------------13/09/95 11.59-------------------
					OpenVideo
--------------------------------------------------*/
void OpenVideo() {
	GameBytePointer = 0;
	GameWordPointer = 0;

	FastFileInit("NlData.cd0");
	SpeechFileInit("NlSpeech.cd0");
	Common::String filename = Common::String::format("NlAnim.cd%c", CurCDSet + '0');
	AnimFileInit(filename);

	g_vm->_video2 = (uint16 *)MemoryArea + 2000000L;

	ff = FastFileOpen("NlFont.fnt");
	g_vm->Font = new uint8[ff->size()];
	ff->read(g_vm->Font, ff->size());
	FastFileClose(ff);

	ff = FastFileOpen("frecc.bm");
	int size = ceil(ff->size() / 2.0);
	g_vm->Arrows = new uint16[size];
	for (int i = 0; i < size; ++i)
		g_vm->Arrows[i] = ff->readUint16LE();
	FastFileClose(ff);
	g_vm->_graphicsMgr->updatePixelFormat(g_vm->Arrows, size);

	ff = FastFileOpen("icone.bm");
	size = ceil(ff->size() / 2.0);
	int iconSize = ICONDX * ICONDY;
	int arraySize = size + iconSize * (INVICONNUM + 1);
	g_vm->Icone = new uint16[arraySize];
	for (int i = 0; i < arraySize; ++i)
		g_vm->Icone[i] = 0;
	for (int i = 0; i < size; ++i)
		g_vm->Icone[iconSize + i] = ff->readUint16LE();
	FastFileClose(ff);
	g_vm->_graphicsMgr->updatePixelFormat(&g_vm->Icone[iconSize], size);

	//
	ff = FastFileOpen("textur.bm");
	size = ff->size();
	g_vm->TextureArea = new uint8[size];
	ff->read(g_vm->TextureArea, size);
	FastFileClose(ff);

	// head
	hh = 0;
	FTexture[hh]._dx = 300 / 2;
	FTexture[hh]._dy = 208 / 2;
	FTexture[hh]._angle = 0;
	FTexture[hh]._texture = g_vm->TextureArea;
	FTexture[hh]._palette = nullptr;
	FTexture[hh]._flag = TEXTUREACTIVE + TEXTURECYLIND;

	// body
	hh = 1;
	FTexture[hh]._dx = 300;
	FTexture[hh]._dy = 300;
	FTexture[hh]._angle = 0;
	FTexture[hh]._texture = FTexture[0]._texture + (300 * 208) / 4;
	FTexture[hh]._palette = nullptr;
	FTexture[hh]._flag = TEXTUREACTIVE + TEXTURECYLIND;

	// arms
	hh = 2;
	FTexture[hh]._dx = 300;
	FTexture[hh]._dy = 150;
	FTexture[hh]._angle = 0;
	FTexture[hh]._texture = FTexture[1]._texture + 300 * 300;
	FTexture[hh]._palette = nullptr;
	FTexture[hh]._flag = TEXTUREACTIVE + TEXTURECYLIND;

	_characterArea = (uint8 *)(MemoryArea + GameBytePointer);
	GameBytePointer += ReadActor("jm.om", (uint8 *)_characterArea);

	_actor._vertexNum = VertexNum;
	_actor._faceNum = FaceNum;
	_actor._light = (SLight *)&VLight;
	_actor._lightNum = LightNum;
	_actor._camera = (SCamera *)&FCamera;
	_actor._texture = (STexture *)&FTexture[0];

	TextArea = (uint8 *)(MemoryArea + GameBytePointer);
	GameBytePointer += MAXTEXTAREA;

	// icon area
	g_vm->_animMgr->_smkBuffer[2] = (uint8 *)(MemoryArea + GameBytePointer);
	GameBytePointer += ICONDX * ICONDY;

	// zbuffer
	size = ZBUFFERSIZE / 2;
	g_vm->ZBuffer = new int16[size];
	for (int c = 0; c < size; ++c)
		g_vm->ZBuffer[c] = 0x7FFF;
	
	// CDBuffer
	SpeechBuf[0] = (uint8 *)(MemoryArea + GameBytePointer);
	SpeechBuf[1] = (uint8 *)(MemoryArea + GameBytePointer);
	ExtraObj2C = (uint16 *)SpeechBuf[0]; // for room 2C
	GameBytePointer += SPEECHSIZE;
	// omino e full motions area
	g_vm->_animMgr->_smkBuffer[1] = (uint8 *)(MemoryArea + GameBytePointer);
	ExtraObj41D = (uint16 *)g_vm->_animMgr->_smkBuffer[1]; // for room 41D
	GameBytePointer += SCREENLEN * AREA;
	// omino buffer
	GameBytePointer += SMKANBUFFER;
	// background buffer
	GameBytePointer += SMKBKGBUFFER;
	// icone buffer
	GameBytePointer += SMKICONBUFFER;
	// background area
	g_vm->_animMgr->_smkBuffer[0] = (uint8 *)(MemoryArea + GameBytePointer);
	GameBytePointer += SCREENLEN * AREA;
	// SmackImagePointer
	SmackImagePointer = (uint16 *)(MemoryArea + GameBytePointer);
	GameBytePointer += SCREENLEN * AREA * 2;

	g_vm->_video2 = (uint16 *)(MemoryArea + GameBytePointer);

	if (!g_vm->_flagMouseEnabled)
		Mouse(MCMD_OFF);

	memset(g_vm->_video2, 0, 1280L * 480L * 2);
	g_vm->_graphicsMgr->showScreen(0, 0, 640, 480);

	for (int i = 0; i < MAXOBJINROOM; ++i) {
		OldObjStatus[i] = false;
		VideoObjStatus[i] = false;
	}
}

/*-----------------13/09/95 11.59-------------------
					ReadActor
--------------------------------------------------*/
uint32 ReadActor(const char *filename, uint8 *Area) {
	int32 ActionNum;

	extern uint16 _textureMat[256][91];
	extern int16  _textureCoord[MAXFACE][3][2];

	g_vm->_graphicsMgr->updatePixelFormat((uint16 *)_textureMat, 256 * 91);

	ff = FastFileOpen(filename);
	if (ff == nullptr)
		CloseSys(g_vm->_sysText[1]);

	FastFileRead(ff, &ActionNum, 4);

	FastFileRead(ff, &VertexNum, 4);
	_actor._vertexNum = VertexNum;

	_actor._vertex = (SVertex *)(Area);
	_characterArea = Area;

	int32 Read = FastFileRead(ff, _actor._vertex, sizeof(SVertex) * VertexNum * ActionNum);

	FastFileRead(ff, &FaceNum, 4);
	_actor._faceNum = FaceNum;

	Area += Read;
	_actor._face = (SFace *)(Area);
	Read += FastFileRead(ff, _actor._face, sizeof(SFace) * FaceNum);
	FastFileClose(ff);

	ff = FastFileOpen("mat.tex");
	FastFileRead(ff, _textureMat, 2 * 91 * 256);
	g_vm->_graphicsMgr->updatePixelFormat((uint16 *)_textureMat, 91 * 256);
	FastFileRead(ff, _textureCoord, 2 * MAXFACE * 3 * 2);
	FastFileRead(ff, _actor._face, sizeof(SFace)*FaceNum);
	FastFileClose(ff);

	_actor._curFrame  = 0;
	_actor._curAction = hSTAND;

	// fixup Microprose head correction
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
		SVertex *sv = (SVertex *)(_actor._vertex + b * VertexNum);

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

/*-----------------17/02/95 10.19-------------------
						ReadLoc
--------------------------------------------------*/
void ReadLoc() {
	if ((g_vm->_curRoom == r11) && !(g_vm->_room[r11]._flag & OBJFLAG_DONE))
		FlagShowCharacter = true;

	SoundFadOut();

	memset(g_vm->_video2, 0, TotalMemory - GameBytePointer);

	GameWordPointer = (CurRoomMaxX * MAXY);           // space for _video2

	Common::String filename = Common::String::format("%s.cr", g_vm->_room[g_vm->_curRoom]._baseName);
	ImagePointer = (uint16 *)g_vm->_video2 + GameWordPointer - 4;

	GameWordPointer += (DecCR(filename, (uint8 *)ImagePointer, (uint8 *)g_vm->_video2) + 1) / 2;
	memcpy(&BmInfo, (SBmInfo *)ImagePointer, sizeof(SBmInfo));
	ImagePointer += 4;
	g_vm->_graphicsMgr->updatePixelFormat(ImagePointer, BmInfo.dx * BmInfo.dy);

	ReadObj();
	if ((g_vm->_room[g_vm->_curRoom]._sounds[0] != 0))
		ReadSounds();

	_actionPointer[0] = (uint8 *)(g_vm->_video2 + GameWordPointer);
	Common::String fname = Common::String::format("%s.3d", g_vm->_room[g_vm->_curRoom]._baseName);
	GameWordPointer += read3D(fname) / 2;

	memset(g_vm->_video2, 0, CurRoomMaxX * MAXY * 2);
	MCopy(g_vm->_video2 + TOP * CurRoomMaxX, ImagePointer, CurRoomMaxX * AREA);

	g_vm->_curSortTableNum = 0;
	for (int i = 0; i < MAXOBJINROOM; ++i) {
		OldObjStatus[i] = false;
		VideoObjStatus[i] = false;
	}

	RegenRoom();

	if (g_vm->_room[g_vm->_curRoom]._bkgAnim) {
		memcpy(SmackImagePointer, ImagePointer, MAXX * AREA * 2);
		g_vm->_animMgr->startSmkAnim(g_vm->_room[g_vm->_curRoom]._bkgAnim);
	} else
		g_vm->_animMgr->stopSmkAnim(g_vm->_animMgr->_playingAnims[0]);

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

	FlagPaintCharacter = true;
	WaitSoundFadEnd();
	PaintScreen(1);

	g_vm->_graphicsMgr->showScreen(0, 0, MAXX, MAXY);
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
			BmInfo.read(o + b);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			ObjPointers[a] = (uint16 *)(o + b);
			g_vm->_graphicsMgr->updatePixelFormat(ObjPointers[a], (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy));
			b += (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy);
		}

		if (g_vm->_obj[c]._mode & OBJMODE_MASK) {
			BmInfo.read(o + b);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			uint32 *p = (uint32 *)(o + b);
			ObjPointers[a] = (uint16 *)p + 2;
			g_vm->_graphicsMgr->updatePixelFormat(ObjPointers[a], *p);

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
			BmInfo.read(o + b);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			ObjPointers[a] = (uint16 *)(o + b);
			g_vm->_graphicsMgr->updatePixelFormat(ObjPointers[a], (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy));
			b += (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy);
		}

		if (g_vm->_obj[c]._mode & OBJMODE_MASK) {
			BmInfo.read(o + b);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			uint32 *p = (uint32 *)(o + b);
			ObjPointers[a] = (uint16 *)p + 2;
			g_vm->_graphicsMgr->updatePixelFormat(ObjPointers[a], *p);

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
			BmInfo.read(o + b);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			ObjPointers[a] = (uint16 *)(o + b);
			g_vm->_graphicsMgr->updatePixelFormat(ObjPointers[a], (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy));
			b += (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy);
		}

		if ((g_vm->_obj[c]._mode & OBJMODE_MASK)) {
			BmInfo.read(o + b);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			uint32 *p = (uint32 *)(o + b);
			ObjPointers[a] = (uint16 *)p + 2;
			g_vm->_graphicsMgr->updatePixelFormat(ObjPointers[a], *p);

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

		SoundPointer[a] = (uint8 *)(g_vm->_video2 + GameWordPointer);

		if (!scumm_stricmp(GSample[b]._name, "RUOTE2C.WAV"))
			break;

		ff = FastFileOpen(GSample[b]._name);
		if (ff == nullptr)
			CloseSys(g_vm->_sysText[1]);
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

		if (g_vm->_obj[g_vm->_room[g_vm->_curRoom]._object[a]]._mode & (OBJMODE_MASK | OBJMODE_FULL)) {
			bool status = (g_vm->_obj[g_vm->_room[g_vm->_curRoom]._object[a]]._mode & OBJMODE_OBJSTATUS);
			if (status != OldObjStatus[a])
				OldObjStatus[a] = status;
		}
	}
}

/*------------------------------------------------
                  PaintRegenRoom
--------------------------------------------------*/
void PaintRegenRoom() {
	for (uint16 a = 0; a < MAXOBJINROOM; a++) {
		if (OldObjStatus[a] != VideoObjStatus[a]) {
			SortTable[g_vm->_curSortTableNum]._index = g_vm->_room[g_vm->_curRoom]._object[a];
			SortTable[g_vm->_curSortTableNum]._roomIndex = a;
			SortTable[g_vm->_curSortTableNum]._remove = VideoObjStatus[a];
			SortTable[g_vm->_curSortTableNum]._curFrame = 0;
			SortTable[g_vm->_curSortTableNum]._typology = TYPO_BMP;
			VideoObjStatus[a] = OldObjStatus[a];
			g_vm->_curSortTableNum++;
		}
	}
}
/*-----------------16/05/95 11.03-------------------
                              DrawObj
--------------------------------------------------*/
void DrawObj(SDObj d) {
	for (uint16 a = 0; a < 4; a++) {
		if (d.l[a] > CurRoomMaxX)
			return;
	}

	uint16 *buf = d.buf;
	if (d.flag & DRAWMASK) {
		uint8 *mask = d.mask;

		if (d.flag & COPYTORAM) {
			for (uint16 b = d.y; b < (d.y + d.dy); b++) {
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
								MCopy(g_vm->_video2 + (b * CurRoomMaxX) + Sco + d.x, buf, Now);

							else if ((Sco < d.l[0]) && ((Sco + Now) < d.l[2]) && ((Sco + Now) >= d.l[0]))
								MCopy(g_vm->_video2 + (b * CurRoomMaxX) + d.l[0] + d.x, buf + d.l[0] - Sco, (Now + Sco - d.l[0]));

							else if ((Sco >= d.l[0]) && ((Sco + Now) >= d.l[2]) && (Sco < d.l[2]))
								MCopy(g_vm->_video2 + (b * CurRoomMaxX) + Sco + d.x, buf, (d.l[2] - Sco));

							else if ((Sco < d.l[0]) && ((Sco + Now) >= d.l[2]))
								MCopy(g_vm->_video2 + (b * CurRoomMaxX) + d.l[0] + d.x, buf + d.l[0] - Sco, (d.l[2] - d.l[0]));
						}
						Sco += *mask;
						buf += *mask++;
						c = 0;
					}
				}
			}
		}

		if (d.flag & COPYTOVIDEO)
			CloseSys(g_vm->_sysText[5]);
	} else {
		if (d.flag & COPYTORAM) {
			for (uint16 b = d.l[1]; b < d.l[3]; b++) {
				MCopy(g_vm->_video2 + (d.y + b) * CurRoomMaxX + (d.x + d.l[0]),
					  buf + (b * d.dx) + d.l[0], (d.l[2] - d.l[0]));
			}
		}

		if (d.flag & COPYTOVIDEO) {
			for (uint16 b = d.l[1]; b < d.l[3]; b++) {
				g_vm->_graphicsMgr->vCopy((d.y + b) * VirtualPageLen + (d.x + d.l[0]),
					  buf + (b * d.dx) + d.l[0], d.l[2] - d.l[0]);
			}

			g_vm->_graphicsMgr->unlock();
		}
	}
}

} // End of namespace Trecision
