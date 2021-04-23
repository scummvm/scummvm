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

#include <common/file.h>

#include "common/system.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "trecision/dialog.h"
#include "trecision/trecision.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/graphics.h"
#include "trecision/sound.h"
#include "trecision/video.h"

namespace Trecision {

#define MAXMAT		20

// GAME POINTER
uint16 _actionPosition[MAXACTIONINROOM];			// Starting position of each action in the room
// DATA POINTER
uint8 *TextArea;
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

// info for Toc BmData
struct SBmInfo {
	uint16 px, py, dx, dy;

	void read(uint16 *buf) {
		px = *buf++;
		py = *buf++;
		dx = *buf++;
		dy = *buf++;
	}

	void read(Common::SeekableReadStream *stream) {
		px = stream->readUint16LE();
		py = stream->readUint16LE();
		dx = stream->readUint16LE();
		dy = stream->readUint16LE();
	}
} BmInfo;

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
	g_vm->_actor->readActor("jm.om");

	g_vm->_actor->_light = (SLight *)&VLight;
	g_vm->_actor->_camera = (SCamera *)&FCamera;
	g_vm->_actor->_texture = (STexture *)&FTexture[0];

	TextArea = new uint8[MAXTEXTAREA];

	// zbuffer
	g_vm->_zBuffer = new int16[ZBUFFERSIZE / 2];
	for (int c = 0; c < ZBUFFERSIZE / 2; ++c)
		g_vm->_zBuffer[c] = 0x7FFF;

	g_vm->_extraRoomObject = nullptr;

	g_vm->_screenBuffer = new uint16[MAXX * MAXY];
	memset(g_vm->_screenBuffer, 0, MAXX * MAXY * 2);

	g_vm->_graphicsMgr->clearScreen();

	g_vm->hideCursor();

	for (int i = 0; i < MAXOBJINROOM; ++i) {
		OldObjStatus[i] = false;
		VideoObjStatus[i] = false;
	}
}

static const float _vertsCorr[104][3] = {
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.000001f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	-0.061717f,	0.833191f,		0.000000f,	-0.120163f,	0.330445f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	-0.432022f,	0.216004f,		0.000000f,	-0.030041f,	0.360489f,
	0.310895f,	0.000000f,	0.000000f,		0.312943f,	0.000000f,	0.000000f,
	0.114858f,	0.000000f,	0.000000f,		0.000000f,	1.051431f,	0.300415f,
	0.000000f,	0.000000f,	0.246856f,		0.000000f,	0.120163f,	0.480652f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.180247f,	0.600815f,		0.000000f,	0.000000f,	0.000000f,
	0.530074f,	0.041892f,	0.670273f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.060081f,	0.540726f,		0.000000f,	-0.318127f,	-0.249817f,
	0.000000f,	0.180244f,	0.540741f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	-0.922172f,	0.201188f,		0.000000f,	-0.442684f,	-0.328400f,
	0.353384f,	1.047291f,	-1.005401f,		0.000000f,	-0.646931f,	-0.933030f,
	0.000000f,	2.283107f,	-0.420562f,		0.412281f,	-1.633775f,	-1.193909f,
	0.312389f,	0.000000f,	0.000000f,		0.000000f,	0.020947f,	-0.083786f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	-1.021390f,	-1.141556f,
	0.000000f,	0.020946f,	-0.146637f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.020946f,	-0.146637f,		0.000000f,	0.020947f,	-0.146637f,
	0.000000f,	0.020946f,	-0.083786f,		0.000000f,	0.020946f,	-0.125687f,
	0.000000f,	0.020947f,	-0.146637f,		0.000000f,	0.020947f,	-0.125687f,
	0.000000f,	0.020946f,	-0.083786f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.020947f,	-0.125687f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.020947f,	-0.125686f,		0.000000f,	0.020946f,	-0.125687f,
	0.000000f,	0.020946f,	-0.083786f,		0.000000f,	0.020946f,	-0.146637f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	-0.061717f,	0.833191f,		0.000000f,	-0.090122f,	0.330460f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	-0.432022f,	0.185150f,
	-0.310895f,	0.000000f,	0.000000f,		-0.312943f,	0.000001f,	0.000000f,
	-0.114858f,	0.000000f,	0.000000f,		0.000000f,	1.051431f,	0.270371f,
	0.000000f,	-0.030858f,	0.246856f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f,
	-0.647869f,	0.041892f,	0.628372f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	-0.442684f,	-0.328400f,		-0.294485f,	1.026345f,	-1.005401f,
	-0.353383f,	-1.633775f,	-1.214859f,		-0.312389f,	0.000000f,	0.000000f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.020947f,	-0.146637f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.020946f,	-0.146637f,
	0.000000f,	0.020946f,	-0.083786f,		0.000000f,	0.020947f,	-0.146637f,
	0.000000f,	0.020947f,	-0.125687f,		0.000000f,	0.020947f,	-0.083786f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.020947f,	-0.125687f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.020946f,	-0.125687f,
	0.000000f,	0.020946f,	-0.146637f,		0.000000f,	0.000000f,	0.000000f,
	0.000000f,	0.000000f,	0.000000f,		0.000000f,	0.000000f,	0.000000f
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

void SActor::readActor(const char *filename) {
	_vm->_graphicsMgr->updatePixelFormat((uint16 *)_textureMat, 256 * 91);

	Common::SeekableReadStream *ff = g_vm->_dataFile.createReadStreamForMember(filename);
	if (ff == nullptr)
		error("readActor - Error opening file %s", filename);

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
		error("readActor - Error opening file mat.tex");

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

int actionInRoom(int curA) {
	for (int b = 0; b < MAXACTIONINROOM; b++) {
		if (g_vm->_room[g_vm->_curRoom]._actions[b] == curA)
			return b;
	}

	warning("Action %d not found in room %d", curA, g_vm->_curRoom);
	return 0;
}

void ReadLoc() {
	if (g_vm->_curRoom == kRoom11 && !(g_vm->_room[kRoom11]._flag & kObjFlagDone))
		g_vm->_flagShowCharacter = true;

	g_vm->_soundMgr->fadeOut();

	memset(g_vm->_screenBuffer, 0, MAXX * MAXY * 2);

	Common::String filename = Common::String::format("%s.cr", g_vm->_room[g_vm->_curRoom]._baseName);
	Common::SeekableReadStream *picFile = g_vm->_dataFile.createReadStreamForCompressedMember(filename);

	BmInfo.read(picFile);

	g_vm->_graphicsMgr->loadBackground(picFile, BmInfo.dx, BmInfo.dy);
	ReadObj(picFile);

	g_vm->_soundMgr->stopAll();

	if (g_vm->_room[g_vm->_curRoom]._sounds[0] != 0)
		g_vm->_soundMgr->loadRoomSounds();

	Common::String fname = Common::String::format("%s.3d", g_vm->_room[g_vm->_curRoom]._baseName);
	read3D(fname);

	g_vm->_graphicsMgr->resetScreenBuffer();

	g_vm->_curSortTableNum = 0;
	for (int i = 0; i < MAXOBJINROOM; ++i) {
		OldObjStatus[i] = false;
		VideoObjStatus[i] = false;
	}

	RegenRoom();

	if (g_vm->_room[g_vm->_curRoom]._bkgAnim) {
		g_vm->_animMgr->startSmkAnim(g_vm->_room[g_vm->_curRoom]._bkgAnim);
	} else
		g_vm->_animMgr->smkStop(kSmackerBackground);

	InitAtFrameHandler(g_vm->_room[g_vm->_curRoom]._bkgAnim, 0);
}

void TendIn() {
	TextStatus = TEXT_OFF;

	if (g_vm->_curRoom == kRoomIntro) {
		g_vm->_dialogMgr->playDialog(dFLOG);
		return;
	}

	g_vm->_flagPaintCharacter = true;
	g_vm->_soundMgr->waitEndFading();
	PaintScreen(1);

	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, MAXY);
}

void readObject(Common::SeekableReadStream *stream, uint16 objIndex, uint16 roomObjIndex) {
	SObject *obj = &g_vm->_obj[roomObjIndex];

	if (obj->_mode & OBJMODE_FULL) {
		BmInfo.read(stream);

		obj->_px = BmInfo.px;
		obj->_py = BmInfo.py;
		obj->_dx = BmInfo.dx;
		obj->_dy = BmInfo.dy;

		uint32 size = obj->_dx * obj->_dy;
		delete[] g_vm->ObjPointers[objIndex];
		g_vm->ObjPointers[objIndex] = new uint16[size];
		for (uint32 i = 0; i < size; ++i)
			g_vm->ObjPointers[objIndex][i] = stream->readUint16LE();

		g_vm->_graphicsMgr->updatePixelFormat(g_vm->ObjPointers[objIndex], size);
	}

	if (obj->_mode & OBJMODE_MASK) {
		BmInfo.read(stream);

		obj->_px = BmInfo.px;
		obj->_py = BmInfo.py;
		obj->_dx = BmInfo.dx;
		obj->_dy = BmInfo.dy;

		uint32 size = stream->readUint32LE();
		delete[] g_vm->ObjPointers[objIndex];
		g_vm->ObjPointers[objIndex] = new uint16[size];
		for (uint32 i = 0; i < size; ++i)
			g_vm->ObjPointers[objIndex][i] = stream->readUint16LE();

		g_vm->_graphicsMgr->updatePixelFormat(g_vm->ObjPointers[objIndex], size);

		size = stream->readUint32LE();
		delete[] g_vm->MaskPointers[objIndex];
		g_vm->MaskPointers[objIndex] = new uint8[size];
		for (uint32 i = 0; i < size; ++i)
			g_vm->MaskPointers[objIndex][i] = (uint8)stream->readByte();
	}
}

void ReadObj(Common::SeekableReadStream *stream) {
	if (!g_vm->_room[g_vm->_curRoom]._object[0])
		return;

	for (uint16 objIndex = 0; objIndex < MAXOBJINROOM; objIndex++) {
		uint16 roomObjIndex = g_vm->_room[g_vm->_curRoom]._object[objIndex];
		if (!roomObjIndex)
			break;

		if (g_vm->_curRoom == kRoom41D && objIndex == PATCHOBJ_ROOM41D)
			break;

		if (g_vm->_curRoom == kRoom2C && objIndex == PATCHOBJ_ROOM2C)
			break;

		readObject(stream, objIndex, roomObjIndex);
	}
}

void ReadExtraObj2C() {
	if (!g_vm->_room[g_vm->_curRoom]._object[32])
		return;

	Common::SeekableReadStream *ff = g_vm->_dataFile.createReadStreamForMember("2c2.bm");

	for (uint16 objIndex = PATCHOBJ_ROOM2C; objIndex < MAXOBJINROOM; objIndex++) {
		uint16 roomObjIndex = g_vm->_room[g_vm->_curRoom]._object[objIndex];
		if (!roomObjIndex)
			break;

		readObject(ff, objIndex, roomObjIndex);
	}

	delete ff;
}

void ReadExtraObj41D() {
	if (!g_vm->_room[g_vm->_curRoom]._object[32])
		return;

	Common::SeekableReadStream *ff = g_vm->_dataFile.createReadStreamForMember("41d2.bm");
	for (uint16 objIndex = PATCHOBJ_ROOM41D; objIndex < MAXOBJINROOM; objIndex++) {
		uint16 roomObjIndex = g_vm->_room[g_vm->_curRoom]._object[objIndex];
		if (!roomObjIndex)
			break;

		readObject(ff, objIndex, roomObjIndex);
	}
	delete ff;
}

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

void PaintRegenRoom() {
	for (uint16 a = 0; a < MAXOBJINROOM; a++) {
		if (OldObjStatus[a] != VideoObjStatus[a]) {
			SortTable[g_vm->_curSortTableNum]._index = g_vm->_room[g_vm->_curRoom]._object[a];
			SortTable[g_vm->_curSortTableNum]._roomIndex = a;
			SortTable[g_vm->_curSortTableNum]._remove = VideoObjStatus[a];
			SortTable[g_vm->_curSortTableNum]._curFrame = 0;
			SortTable[g_vm->_curSortTableNum]._isBitmap = true;
			VideoObjStatus[a] = OldObjStatus[a];
			g_vm->_curSortTableNum++;
		}
	}
}

} // End of namespace Trecision
