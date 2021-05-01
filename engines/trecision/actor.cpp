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

#include "trecision/trecision.h"
#include "trecision/actor.h"

#include "defines.h"
#include "nl/extern.h"
#include "nl/message.h"
#include "nl/proto.h"
#include "trecision/graphics.h"

namespace Trecision {

Actor::Actor(TrecisionEngine *vm) : _vm(vm) {
	_vertex = nullptr;
	_face = nullptr;
	_light = nullptr;
	_camera = nullptr;
	_texture = nullptr;

	_vertexNum = 0;
	_faceNum = 0;
	_lightNum = 0;
	_matNum = 0;

	_px = _py = _pz = 0.0;
	_dx = _dz = 0.0;
	_theta = 0.0;

	for (uint8 i = 0; i < 6; ++i)
		_lim[i] = 0;

	_curFrame = 0;
	_curAction = 0;

	for (uint16 i = 0; i < 256; ++i) {
		for (int j = 0; j < 91; ++j)
			_textureMat[i][j] = 0;
	}

	for (uint16 i = 0; i < MAXFACE; ++i) {
		for (uint8 j = 0; j < 3; ++j) {
			_textureCoord[i][j][0] = 0;
			_textureCoord[i][j][1] = 0;
		}
	}

	_characterArea = nullptr;
}

Actor::~Actor() {
	delete[] _characterArea;
	delete[] _face;
//	delete _light;
//	delete _camera;
//	delete _texture;
}

static const float _vertsCorr[104][3] = {
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.000001f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, -0.061717f, 0.833191f, 0.000000f, -0.120163f, 0.330445f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, -0.432022f, 0.216004f, 0.000000f, -0.030041f, 0.360489f,
	0.310895f, 0.000000f, 0.000000f, 0.312943f, 0.000000f, 0.000000f,
	0.114858f, 0.000000f, 0.000000f, 0.000000f, 1.051431f, 0.300415f,
	0.000000f, 0.000000f, 0.246856f, 0.000000f, 0.120163f, 0.480652f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.180247f, 0.600815f, 0.000000f, 0.000000f, 0.000000f,
	0.530074f, 0.041892f, 0.670273f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.060081f, 0.540726f, 0.000000f, -0.318127f, -0.249817f,
	0.000000f, 0.180244f, 0.540741f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, -0.922172f, 0.201188f, 0.000000f, -0.442684f, -0.328400f,
	0.353384f, 1.047291f, -1.005401f, 0.000000f, -0.646931f, -0.933030f,
	0.000000f, 2.283107f, -0.420562f, 0.412281f, -1.633775f, -1.193909f,
	0.312389f, 0.000000f, 0.000000f, 0.000000f, 0.020947f, -0.083786f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, -1.021390f, -1.141556f,
	0.000000f, 0.020946f, -0.146637f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.020946f, -0.146637f, 0.000000f, 0.020947f, -0.146637f,
	0.000000f, 0.020946f, -0.083786f, 0.000000f, 0.020946f, -0.125687f,
	0.000000f, 0.020947f, -0.146637f, 0.000000f, 0.020947f, -0.125687f,
	0.000000f, 0.020946f, -0.083786f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.020947f, -0.125687f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.020947f, -0.125686f, 0.000000f, 0.020946f, -0.125687f,
	0.000000f, 0.020946f, -0.083786f, 0.000000f, 0.020946f, -0.146637f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, -0.061717f, 0.833191f, 0.000000f, -0.090122f, 0.330460f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, -0.432022f, 0.185150f,
	-0.310895f, 0.000000f, 0.000000f, -0.312943f, 0.000001f, 0.000000f,
	-0.114858f, 0.000000f, 0.000000f, 0.000000f, 1.051431f, 0.270371f,
	0.000000f, -0.030858f, 0.246856f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
	-0.647869f, 0.041892f, 0.628372f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, -0.442684f, -0.328400f, -0.294485f, 1.026345f, -1.005401f,
	-0.353383f, -1.633775f, -1.214859f, -0.312389f, 0.000000f, 0.000000f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020947f, -0.146637f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020946f, -0.146637f,
	0.000000f, 0.020946f, -0.083786f, 0.000000f, 0.020947f, -0.146637f,
	0.000000f, 0.020947f, -0.125687f, 0.000000f, 0.020947f, -0.083786f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020947f, -0.125687f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020946f, -0.125687f,
	0.000000f, 0.020946f, -0.146637f, 0.000000f, 0.000000f, 0.000000f,
	0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f};

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
	379, 380, 381, 382};

void Actor::readModel(const char *filename) {
	_vm->_graphicsMgr->updatePixelFormat((uint16 *)_textureMat, 256 * 91);

	Common::SeekableReadStream *ff = _vm->_dataFile.createReadStreamForMember(filename);
	if (ff == nullptr)
		error("readModel - Error opening file %s", filename);

	uint32 actionNum = ff->readUint32LE();
	_vertexNum = ff->readUint32LE();

	_characterArea = new SVertex[_vertexNum * actionNum];
	for (uint i = 0; i < _vertexNum * actionNum; ++i) {
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

	ff = _vm->_dataFile.createReadStreamForMember("mat.tex");
	if (ff == nullptr)
		error("readModel - Error opening file mat.tex");

	for (uint16 i = 0; i < 256; ++i) {
		for (uint16 j = 0; j < 91; ++j)
			_textureMat[i][j] = ff->readUint16LE();
	}

	_vm->_graphicsMgr->updatePixelFormat((uint16 *)_textureMat, 91 * 256);

	for (uint16 i = 0; i < MAXFACE; ++i) {
		for (uint16 j = 0; j < 3; ++j) {
			_textureCoord[i][j][0] = ff->readSint16LE();
			_textureCoord[i][j][1] = ff->readSint16LE();
		}
	}

	_face = new SFace[_faceNum];
	for (uint i = 0; i < _faceNum; ++i) {
		_face[i]._a = ff->readUint16LE();
		_face[i]._b = ff->readUint16LE();
		_face[i]._c = ff->readUint16LE();
		_face[i]._mat = ff->readUint16LE();
	}

	delete ff;

	_curFrame = 0;
	_curAction = hSTAND;

	// fixup Microprose head correction
#define P1 306
#define P2 348
#define P3 288
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

	for (uint b = 0; b < actionNum; b++) {
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

void Actor::syncGameStream(Common::Serializer &ser) {
	ser.syncAsFloatLE(_px);
	ser.syncAsFloatLE(_py);
	ser.syncAsFloatLE(_pz);
	ser.syncAsFloatLE(_dx);
	ser.syncAsFloatLE(_dz);
	ser.syncAsFloatLE(_theta);
}

void Actor::actorDoAction(int action) {
	if (action > hLAST)
		error("error in actorDoAction, invalid action (should be called as an animation)");

	_curStep = 1;
	float px = _px + _dx;
	float pz = _pz + _dz;
	float theta = _theta;
	int b = 0;

	_step[b]._px = px;
	_step[b]._pz = pz;
	_step[b]._dx = 0.0f;
	_step[b]._dz = 0.0f;

	_step[b]._theta = theta;
	_step[b]._curAction = hSTAND;
	_step[b]._curFrame = 0;
	_step[b]._curPanel = _curPanel;

	float t = ((270.0f - theta) * PI2) / 360.0f;
	float ox = cos(t);
	float oz = sin(t);

	SVertex *v = _characterArea;
	float firstFrame = FRAMECENTER(v);

	int len;
	int cfp = 0;
	int cur = 0;

	while (cur < action)
		cfp += _defActionLen[cur++];
	v = &_characterArea[cfp * _vertexNum];

	if (action == hWALKOUT)
		v = &_characterArea[_vertexNum];
	else if (action == hLAST)
		v = _characterArea;

	len = _defActionLen[action];

	for (b = _curStep; b < len + _curStep; b++) {
		float curLen = FRAMECENTER(v) - firstFrame;

		_step[b]._dx = curLen * ox;
		_step[b]._dz = curLen * oz;
		_step[b]._px = px;
		_step[b]._pz = pz;

		_step[b]._curAction = action;
		_step[b]._curFrame = b - _curStep;

		_step[b]._theta = theta;
		_step[b]._curPanel = _curPanel;

		v += _vertexNum;

		if (action == hLAST)
			v = _characterArea;
	}

	_step[b]._px = px;
	_step[b]._pz = pz;
	_step[b]._dx = 0.0;
	_step[b]._dz = 0.0;

	_step[b]._theta = theta;
	_step[b]._curAction = hSTAND;
	_step[b]._curFrame = 0;
	_step[b]._curPanel = _curPanel;

	_lastStep = b; // Last step

	// Starts action
	if (_vm->_obj[_vm->_curObj]._flag & kObjFlagRoomOut)
		doEvent(MC_CHARACTER, ME_CHARACTERGOTOEXIT, MP_DEFAULT, _vm->_obj[_vm->_curObj]._goRoom, 0, _vm->_obj[_vm->_curObj]._ninv, _vm->_curObj);
	else
		doEvent(MC_CHARACTER, ME_CHARACTERDOACTION, MP_DEFAULT, 0, 0, 0, 0);
}

void Actor::actorStop() {
	int b = 0;

	_step[b]._px = _px + _dx;
	_step[b]._pz = _pz + _dz;
	_step[b]._dx = 0.0;
	_step[b]._dz = 0.0;

	_step[b]._theta = _theta;
	_step[b]._curAction = hSTAND;
	_step[b]._curFrame = 0;
	_step[b]._curPanel = _curPanel;

	_characterGoToPosition = -1;

	_curStep = 0;
	_lastStep = 0;
}

void Actor::read3D(Common::SeekableReadStream *ff) {
	// read rooms and lights
	SCamera *cam = _camera;
	cam->_ex = ff->readFloatLE();
	cam->_ey = ff->readFloatLE();
	cam->_ez = ff->readFloatLE();
	for (int i = 0; i < 3; ++i)
		cam->_e1[i] = ff->readFloatLE();
	for (int i = 0; i < 3; ++i)
		cam->_e2[i] = ff->readFloatLE();
	for (int i = 0; i < 3; ++i)
		cam->_e3[i] = ff->readFloatLE();
	cam->_fovX = ff->readFloatLE();
	cam->_fovY = ff->readFloatLE();

	_lightNum = ff->readUint32LE();
	if (_lightNum > MAXLIGHT)
		error("read3D(): Too many lights");

	for (uint32 i = 0; i < g_vm->_actor->_lightNum; ++i) {
		_light[i]._x = ff->readFloatLE();
		_light[i]._y = ff->readFloatLE();
		_light[i]._z = ff->readFloatLE();
		_light[i]._dx = ff->readFloatLE();
		_light[i]._dy = ff->readFloatLE();
		_light[i]._dz = ff->readFloatLE();
		_light[i]._inr = ff->readFloatLE();
		_light[i]._outr = ff->readFloatLE();
		_light[i]._hotspot = ff->readByte();
		_light[i]._fallOff = ff->readByte();
		_light[i]._inten = ff->readSByte();
		_light[i]._position = ff->readSByte();
	}
}

} // End of namespace Trecision
