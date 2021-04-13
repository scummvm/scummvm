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

#include "common/system.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "trecision/trecision.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/graphics.h"
#include "trecision/video.h"

namespace Trecision {

#define MAXMAT		20

// GAME POINTER
uint16 *ImagePointer;
uint16 *ObjPointers[MAXOBJINROOM];
uint8 *MaskPointers[MAXOBJINROOM];
uint8 *_actionPointer[MAXACTIONFRAMESINROOM];		// puntatore progressivo ai frame
uint16 _actionPosition[MAXACTIONINROOM];			// Starting position of each action in the room
// DATA POINTER
uint8 *TextArea;
uint8 SpeechBuf[SPEECHSIZE];
// DTEXT
int8 DTextLines[MAXDTEXTLINES][MAXDTEXTCHARS];
// 3D
SLight  VLight[MAXLIGHT];
SCamera FCamera;
STexture FTexture[MAXMAT];
int32  hh;
// MOUSE
SDText curString;
SDText oldString;
uint8  TextStatus;
// FILEREF
SFileEntry FileRef[MAXFILEREF];

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

/*-----------------17/02/95 10.19-------------------
						openSys
--------------------------------------------------*/
void openSys() {
	// head
	hh = 0;
	FTexture[hh]._dx = 300 / 2;
	FTexture[hh]._dy = 208 / 2;
	FTexture[hh]._angle = 0;
	FTexture[hh]._texture = g_vm->_textureArea;
	FTexture[hh]._flag = TEXTUREACTIVE + TEXTURECYLIND;

	// body
	hh = 1;
	FTexture[hh]._dx = 300;
	FTexture[hh]._dy = 300;
	FTexture[hh]._angle = 0;
	FTexture[hh]._texture = FTexture[0]._texture + (300 * 208) / 4;
	FTexture[hh]._flag = TEXTUREACTIVE + TEXTURECYLIND;

	// arms
	hh = 2;
	FTexture[hh]._dx = 300;
	FTexture[hh]._dy = 150;
	FTexture[hh]._angle = 0;
	FTexture[hh]._texture = FTexture[1]._texture + 300 * 300;
	FTexture[hh]._flag = TEXTUREACTIVE + TEXTURECYLIND;

	delete g_vm->_actor;
	g_vm->_actor = new SActor(g_vm);
	g_vm->_actor->ReadActor("jm.om");

	g_vm->_actor->_light = (SLight *)&VLight;
	g_vm->_actor->_camera = (SCamera *)&FCamera;
	g_vm->_actor->_texture = (STexture *)&FTexture[0];

	TextArea = new uint8[MAXTEXTAREA];

	// zbuffer
	g_vm->_zBuffer = new int16[ZBUFFERSIZE / 2];
	for (int c = 0; c < ZBUFFERSIZE / 2; ++c)
		g_vm->_zBuffer[c] = 0x7FFF;

	g_vm->_extraRoomObject = nullptr;

	g_vm->_smackImageBuffer = new uint16[MAXX * AREA];
	memset(g_vm->_smackImageBuffer, 0, MAXX * AREA * 2);

	g_vm->_screenBuffer = new uint16[MAXX * MAXY];
	memset(g_vm->_screenBuffer, 0, MAXX * MAXY * 2);

	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, MAXY);

	ImagePointer = new uint16[MAXX * MAXY * 2];

	g_vm->hideCursor();

	for (int i = 0; i < MAXOBJINROOM; ++i) {
		OldObjStatus[i] = false;
		VideoObjStatus[i] = false;
	}
}

static const float _vertsCorr[104][3] = {
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	0.000001,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	-0.061717,	0.833191,		0.000000,	-0.120163,	0.330445,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	-0.432022,	0.216004,		0.000000,	-0.030041,	0.360489,
	0.310895,	0.000000,	0.000000,		0.312943,	0.000000,	0.000000,
	0.114858,	0.000000,	0.000000,		0.000000,	1.051431,	0.300415,
	0.000000,	0.000000,	0.246856,		0.000000,	0.120163,	0.480652,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	0.180247,	0.600815,		0.000000,	0.000000,	0.000000,
	0.530074,	0.041892,	0.670273,		0.000000,	0.000000,	0.000000,
	0.000000,	0.060081,	0.540726,		0.000000,	-0.318127,	-0.249817,
	0.000000,	0.180244,	0.540741,		0.000000,	0.000000,	0.000000,
	0.000000,	-0.922172,	0.201188,		0.000000,	-0.442684,	-0.328400,
	0.353384,	1.047291,	-1.005401,		0.000000,	-0.646931,	-0.933030,
	0.000000,	2.283107,	-0.420562,		0.412281,	-1.633775,	-1.193909,
	0.312389,	0.000000,	0.000000,		0.000000,	0.020947,	-0.083786,
	0.000000,	0.000000,	0.000000,		0.000000,	-1.021390,	-1.141556,
	0.000000,	0.020946,	-0.146637,		0.000000,	0.000000,	0.000000,
	0.000000,	0.020946,	-0.146637,		0.000000,	0.020947,	-0.146637,
	0.000000,	0.020946,	-0.083786,		0.000000,	0.020946,	-0.125687,
	0.000000,	0.020947,	-0.146637,		0.000000,	0.020947,	-0.125687,
	0.000000,	0.020946,	-0.083786,		0.000000,	0.000000,	0.000000,
	0.000000,	0.020947,	-0.125687,		0.000000,	0.000000,	0.000000,
	0.000000,	0.020947,	-0.125686,		0.000000,	0.020946,	-0.125687,
	0.000000,	0.020946,	-0.083786,		0.000000,	0.020946,	-0.146637,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	0.000000,	-0.061717,	0.833191,		0.000000,	-0.090122,	0.330460,
	0.000000,	0.000000,	0.000000,		0.000000,	-0.432022,	0.185150,
	-0.310895,	0.000000,	0.000000,		-0.312943,	0.000001,	0.000000,
	-0.114858,	0.000000,	0.000000,		0.000000,	1.051431,	0.270371,
	0.000000,	-0.030858,	0.246856,		0.000000,	0.000000,	0.000000,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
	-0.647869,	0.041892,	0.628372,		0.000000,	0.000000,	0.000000,
	0.000000,	-0.442684,	-0.328400,		-0.294485,	1.026345,	-1.005401,
	-0.353383,	-1.633775,	-1.214859,		-0.312389,	0.000000,	0.000000,
	0.000000,	0.000000,	0.000000,		0.000000,	0.020947,	-0.146637,
	0.000000,	0.000000,	0.000000,		0.000000,	0.020946,	-0.146637,
	0.000000,	0.020946,	-0.083786,		0.000000,	0.020947,	-0.146637,
	0.000000,	0.020947,	-0.125687,		0.000000,	0.020947,	-0.083786,
	0.000000,	0.000000,	0.000000,		0.000000,	0.020947,	-0.125687,
	0.000000,	0.000000,	0.000000,		0.000000,	0.020946,	-0.125687,
	0.000000,	0.020946,	-0.146637,		0.000000,	0.000000,	0.000000,
	0.000000,	0.000000,	0.000000,		0.000000,	0.000000,	0.000000,
};

static const int _vertsCorrList[84] = {
	289, 290, 293, 294, 295, 296, 297, 298,
	299, 300, 300, 302, 303, 304, 305, 305,
	307, 307, 309, 310, 311, 312, 313, 314,
	315, 316, 317, 318, 319, 320, 321, 322,
	323, 324, 325, 326, 327, 328, 329, 330,
	331, 332, 333, 334, 335, 336, 337, 338,
	339, 340, 341, 349, 350, 352, 353, 354,
	355, 356, 357, 358, 359, 360, 361, 362,
	363, 364, 365, 366, 367, 368, 369, 370,
	371, 372, 373, 374, 375, 376, 377, 378,
	379, 380, 381, 382
};

/*-----------------13/09/95 11.59-------------------
					ReadActor
--------------------------------------------------*/
void SActor::ReadActor(const char *filename) {
	_vm->_graphicsMgr->updatePixelFormat((uint16 *)_textureMat, 256 * 91);

	Common::SeekableReadStream *ff = g_vm->_dataFile.createReadStreamForMember(filename);
	if (ff == nullptr)
		error("ReadActor - Error opening file %s", filename);

	int32 ActionNum = ff->readSint32LE();
	_vertexNum = ff->readSint32LE();

	_characterArea = new SVertex[_vertexNum * ActionNum];
	for (int i = 0; i < _vertexNum * ActionNum; ++i) {
		_characterArea[i]._x = ff->readFloatLE();
		_characterArea[i]._y = ff->readFloatLE();
		_characterArea[i]._z = ff->readFloatLE();
		_characterArea[i]._nx = ff->readFloatLE();
		_characterArea[i]._ny = ff->readFloatLE();
		_characterArea[i]._nz = ff->readFloatLE();
	}
	_vertex = _characterArea;
	_faceNum = ff->readUint32LE();
	delete ff;

	ff = g_vm->_dataFile.createReadStreamForMember("mat.tex");
	if (ff == nullptr)
		error("ReadActor - Error opening file mat.tex");

	for (int i = 0; i < 256; ++i) {
		for (int j = 0; j < 91; ++j)
			_textureMat[i][j] = ff->readUint16LE();
	}

	g_vm->_graphicsMgr->updatePixelFormat((uint16 *)_textureMat, 91 * 256);

	for (int i = 0; i < MAXFACE; ++i) {
		for (int j = 0; j < 3; ++j) {
			_textureCoord[i][j][0] = ff->readSint16LE();
			_textureCoord[i][j][1] = ff->readSint16LE();
		}
	}

	_face = new SFace[_faceNum];
	for (int i = 0; i < _faceNum; ++i) {
		_face[i]._a = ff->readSint16LE();
		_face[i]._b = ff->readSint16LE();
		_face[i]._c = ff->readSint16LE();
		_face[i]._mat = ff->readSint16LE();
	}

	delete ff;

	_curFrame = 0;
	_curAction = hSTAND;

	// fixup Microprose head correction
#define P1	306
#define P2	348
#define P3	288
	double v1[3], v2[3], v[3], q[3], m1[3][3], m2[3][3], s;
	int c, d, f;

	v1[0] = _vertex[P2]._x - _vertex[P1]._x;
	v1[1] = _vertex[P2]._y - _vertex[P1]._y;
	v1[2] = _vertex[P2]._z - _vertex[P1]._z;
	s = sqrt(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]);
	v1[0] /= s;
	v1[1] /= s;
	v1[2] /= s;

	v2[0] = _vertex[P3]._x - _vertex[P1]._x;
	v2[1] = _vertex[P3]._y - _vertex[P1]._y;
	v2[2] = _vertex[P3]._z - _vertex[P1]._z;
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
		SVertex *sv = &_vertex[b * _vertexNum];

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

		v1[0] = _vertex[P1]._x;
		v1[1] = _vertex[P1]._y;
		v1[2] = _vertex[P1]._z;

		for (int e = 279; e < 383; e++) {
			for (f = 0; f < 84; f++) {
				if (_vertsCorrList[f] == e)
					break;
			}
			if (f == 84)
				continue;

			v[0] = _vertsCorr[e - 279][0];
			v[1] = _vertsCorr[e - 279][2];
			v[2] = _vertsCorr[e - 279][1];

			q[0] = 0.0;
			q[1] = 0.0;
			q[2] = 0.0;
			for (d = 0; d < 3; d++) {
				for (c = 0; c < 3; c++)
					q[c] += m1[c][d] * v[d];
			}
			v[0] = 0.0;
			v[1] = 0.0;
			v[2] = 0.0;
			for (d = 0; d < 3; d++) {
				for (c = 0; c < 3; c++)
					v[c] += m2[d][c] * q[d];
			}

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

/* -----------------27/06/97 17.52-------------------
					actionInRoom
 --------------------------------------------------*/
int actionInRoom(int curA) {
	for (int b = 0; b < MAXACTIONINROOM; b++) {
		if (g_vm->_room[g_vm->_curRoom]._actions[b] == curA)
			return b;
	}

	warning("Action %d not found in room %d", curA, g_vm->_curRoom);
	return 0;
}

/*-----------------17/02/95 10.19-------------------
						ReadLoc
--------------------------------------------------*/
void ReadLoc() {
	if (g_vm->_curRoom == r11 && !(g_vm->_room[r11]._flag & OBJFLAG_DONE))
		g_vm->_flagShowCharacter = true;

	SoundFadOut();

	memset(g_vm->_screenBuffer, 0, MAXX * MAXY * 2);

	Common::String filename = Common::String::format("%s.cr", g_vm->_room[g_vm->_curRoom]._baseName);

	uint32 dataLength = (DecCR(filename, (uint8 *)(ImagePointer - 4), (uint8 *)g_vm->_screenBuffer) + 1) / 2;
	memcpy(&BmInfo, (SBmInfo *)(ImagePointer - 4), sizeof(SBmInfo));
	g_vm->_graphicsMgr->updatePixelFormat(ImagePointer, BmInfo.dx * BmInfo.dy);

	ReadObj();

	SoundStopAll();

	if (g_vm->_room[g_vm->_curRoom]._sounds[0] != 0)
		ReadSounds();

	_actionPointer[0] = (uint8 *)ImagePointer + dataLength;
	Common::String fname = Common::String::format("%s.3d", g_vm->_room[g_vm->_curRoom]._baseName);
	read3D(fname);

	memset(g_vm->_screenBuffer, 0, MAXX * MAXY * 2);
	memcpy(g_vm->_screenBuffer + TOP * MAXX, ImagePointer, MAXX * AREA * 2);

	g_vm->_curSortTableNum = 0;
	for (int i = 0; i < MAXOBJINROOM; ++i) {
		OldObjStatus[i] = false;
		VideoObjStatus[i] = false;
	}

	RegenRoom();

	if (g_vm->_room[g_vm->_curRoom]._bkgAnim) {
		memcpy(g_vm->_smackImageBuffer, ImagePointer, MAXX * AREA * 2);
		g_vm->_animMgr->startSmkAnim(g_vm->_room[g_vm->_curRoom]._bkgAnim);
	} else
		g_vm->_animMgr->stopSmkAnim(g_vm->_animMgr->_playingAnims[kSmackerBackground]);

	InitAtFrameHandler(g_vm->_room[g_vm->_curRoom]._bkgAnim, 0);
}

/*-----------------10/12/95 14.59-------------------
					TendIn
--------------------------------------------------*/
void TendIn() {
	TextStatus = TEXT_OFF;

	if (g_vm->_curRoom == rINTRO) {
		PlayDialog(dFLOG);
		return;
	}

	g_vm->_flagPaintCharacter = true;
	WaitSoundFadEnd();
	PaintScreen(1);

	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, MAXY);
}

/*-----------------17/02/95 10.20-------------------
						ReadObj
--------------------------------------------------*/
void ReadObj() {
	if (!g_vm->_room[g_vm->_curRoom]._object[0])
		return;

	uint16 *objBuffer = (uint16 *)ImagePointer + BmInfo.dx * BmInfo.dy;
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
			BmInfo.read(objBuffer + b);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			ObjPointers[a] = (uint16 *)(objBuffer + b);
			g_vm->_graphicsMgr->updatePixelFormat(ObjPointers[a], (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy));
			b += (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy);
		}

		if (g_vm->_obj[c]._mode & OBJMODE_MASK) {
			BmInfo.read(objBuffer + b);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			uint32 *p = (uint32 *)(objBuffer + b);
			ObjPointers[a] = (uint16 *)p + 2;
			g_vm->_graphicsMgr->updatePixelFormat(ObjPointers[a], *p);

			b += p[0];
			b += 2;

			p = (uint32 *)(objBuffer + b);
			MaskPointers[a] = (uint8 *)p + 4;
			b += (*p / 2);
			b += 2;
		}
	}
}

/*------------------------------------------------
				ReadExtraObj2C()
--------------------------------------------------*/
void ReadExtraObj2C() {
	if (!g_vm->_room[g_vm->_curRoom]._object[32])
		return;
	
	Common::SeekableReadStream *ff = g_vm->_dataFile.createReadStreamForMember("2c2.bm");
	uint16 *objBuffer = g_vm->_extraRoomObject = new uint16[ff->size() / 2];
	for (int i = 0; i < ff->size() / 2; i++) {
		objBuffer[i] = ff->readUint16LE();
	}
	delete ff;

	uint32 b = 0;
	for (uint16 a = 20; a < MAXOBJINROOM; a++) {
		uint16 c = g_vm->_room[g_vm->_curRoom]._object[a];
		if (!c)
			break;

		if (g_vm->_obj[c]._mode & OBJMODE_FULL) {
			BmInfo.read(objBuffer + b);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			ObjPointers[a] = (uint16 *)(objBuffer + b);
			g_vm->_graphicsMgr->updatePixelFormat(ObjPointers[a], (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy));
			b += (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy);
		}

		if (g_vm->_obj[c]._mode & OBJMODE_MASK) {
			BmInfo.read(objBuffer + b);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			uint32 *p = (uint32 *)(objBuffer + b);
			ObjPointers[a] = (uint16 *)p + 2;
			g_vm->_graphicsMgr->updatePixelFormat(ObjPointers[a], *p);

			b += p[0];
			b += 2;

			p = (uint32 *)(objBuffer + b);
			MaskPointers[a] = (uint8 *)p + 4;
			b += (*p / 2);
			b += 2;
		}
	}
}

/*------------------------------------------------
					ReadExtraObj41D
--------------------------------------------------*/
void ReadExtraObj41D() {
	if (!g_vm->_room[g_vm->_curRoom]._object[32])
		return;

	Common::SeekableReadStream *ff = g_vm->_dataFile.createReadStreamForMember("41d2.bm");
	uint16 *objBuffer = g_vm->_extraRoomObject = new uint16[ff->size() / 2];
	for (int i = 0; i < ff->size() / 2; i++) {
		objBuffer[i] = ff->readUint16LE();
	}
	delete ff;

	uint32 b = 0;
	for (uint16 a = 89; a < MAXOBJINROOM; a++) {
		uint16 c = g_vm->_room[g_vm->_curRoom]._object[a];
		if (!c)
			break;

		if (g_vm->_obj[c]._mode & OBJMODE_FULL) {
			BmInfo.read(objBuffer + b);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			ObjPointers[a] = (uint16 *)(objBuffer + b);
			g_vm->_graphicsMgr->updatePixelFormat(ObjPointers[a], (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy));
			b += (g_vm->_obj[c]._dx * g_vm->_obj[c]._dy);
		}

		if ((g_vm->_obj[c]._mode & OBJMODE_MASK)) {
			BmInfo.read(objBuffer + b);
			b += 4;
			g_vm->_obj[c]._px = BmInfo.px;
			g_vm->_obj[c]._py = BmInfo.py;
			g_vm->_obj[c]._dx = BmInfo.dx;
			g_vm->_obj[c]._dy = BmInfo.dy;

			uint32 *p = (uint32 *)(objBuffer + b);
			ObjPointers[a] = (uint16 *)p + 2;
			g_vm->_graphicsMgr->updatePixelFormat(ObjPointers[a], *p);

			b += p[0];
			b += 2;

			p = (uint32 *)(objBuffer + b);
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
	for (uint16 a = 0; a < MAXSOUNDSINROOM; a++) {
		uint16 b = g_vm->_room[g_vm->_curRoom]._sounds[a];

		if (b == 0)
			break;

		if (!scumm_stricmp(GSample[b]._name, "RUOTE2C.WAV"))
			break;

		LoadAudioWav(b, GSample[b]._name);

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
	if (d.l.left > MAXX || d.l.top > MAXX || d.l.right > MAXX || d.l.bottom > MAXX)
		return;
	
	uint16 *buf = d.buf;
	if (d.flag & DRAWMASK) {
		uint8 *mask = d.mask;

		if (d.flag & COPYTORAM) {
			for (uint16 b = d.y; b < (d.y + d.dy); b++) {
				uint16 Sco = 0;
				uint16 c = 0;
				while (Sco < d.dx) {
					if (c == 0) {            // jump
						Sco += *mask;
						mask++;

						c = 1;
					} else {                 // copy
						uint16 maskOffset = *mask;

						if ((maskOffset != 0) && (b >= (d.y + d.l.top)) && (b < (d.y + d.l.bottom))) {
							if ((Sco >= d.l.left) && ((Sco + maskOffset) < d.l.right))
								memcpy(g_vm->_screenBuffer + (b * MAXX) + Sco + d.x, buf, maskOffset * 2);

							else if ((Sco < d.l.left) && ((Sco + maskOffset) < d.l.right) && ((Sco + maskOffset) >= d.l.left))
								memcpy(g_vm->_screenBuffer + (b * MAXX) + d.l.left + d.x, buf + d.l.left - Sco, (maskOffset + Sco - d.l.left) * 2);

							else if ((Sco >= d.l.left) && ((Sco + maskOffset) >= d.l.right) && (Sco < d.l.right))
								memcpy(g_vm->_screenBuffer + (b * MAXX) + Sco + d.x, buf, (d.l.right - Sco) * 2);

							else if ((Sco < d.l.left) && ((Sco + maskOffset) >= d.l.right))
								memcpy(g_vm->_screenBuffer + (b * MAXX) + d.l.left + d.x, buf + d.l.left - Sco, (d.l.right - d.l.left) * 2);
						}
						Sco += *mask;
						buf += *mask++;
						c = 0;
					}
				}
			}
		}
	} else if (d.flag & COPYTORAM) {
		for (uint16 b = d.l.top; b < d.l.bottom; b++) {
			memcpy(g_vm->_screenBuffer + (d.y + b) * MAXX + (d.x + d.l.left),
				  buf + (b * d.dx) + d.l.left, (d.l.right - d.l.left) * 2);
		}
	}
}

} // End of namespace Trecision
