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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TRECISION_ACTOR_H
#define TRECISION_ACTOR_H

#include "trecision/trecision.h"

namespace Trecision {
class Actor {
private:
	TrecisionEngine *_vm;

public:
	Actor(TrecisionEngine *vm);
	~Actor();

	SVertex *_characterArea; // TODO: Make it private

	SVertex  *_vertex;
	SFace    *_face;
	SLight   *_light;
	SCamera  *_camera;
	STexture *_texture;

	uint16 _textureMat[256][91];
	int16  _textureCoord[MAXFACE][3][2];

	uint32 _vertexNum;
	uint32 _faceNum;
	uint32 _lightNum;
	int _matNum;

	float _px, _py, _pz;
	float _dx, _dz;
	float _theta;
	int   _lim[6];

	int _curFrame;
	int _curAction;

	void readModel(const char *filename);
	void syncGameStream(Common::Serializer &ser);
	void actorDoAction(int action);
	void actorStop();
	void read3D(Common::SeekableReadStream *ff);
};


; // end of class

} // end of namespace
#endif

