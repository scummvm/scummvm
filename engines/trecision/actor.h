/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TRECISION_ACTOR_H
#define TRECISION_ACTOR_H

#include "trecision/struct.h"
#include "common/scummsys.h"
#include "common/serializer.h"

namespace Trecision {

#define MAXLIGHT 40

class TrecisionEngine;

class Actor {
	TrecisionEngine *_vm;

	SLight _lightArea[MAXLIGHT];
	SCamera _cameraArea;
	uint8 *_textureData;
	STexture _textureArea[MAXMAT];

	void initTextures();
	void readModel(const char *filename);
	void microproseHeadFix(uint32 actionNum);

public:
	Actor(TrecisionEngine *vm);
	~Actor();

	SVertex *_characterArea;
	SVertex *_vertex;

	SFace *_face;
	SLight *_light;
	SCamera *_camera;
	STexture *_textures;

	int16  _textureCoord[MAXFACE][3][2];

	uint32 _vertexNum;
	uint32 _faceNum;
	uint32 _lightNum;

	float _px, _pz;
	float _dx, _dz;
	float _theta;

	int _area[6];

	int _curFrame;
	int _curAction;

	void syncGameStream(Common::Serializer &ser);
	void actorDoAction(int action);
	void actorStop();
	void read3D(Common::SeekableReadStreamEndian *ff);
	float frameCenter(SVertex *v);
	void updateStepSound();
	bool actorRectIsValid() const;
	Common::Rect getActorRect() const;
};

} // End of namespace Trecision
#endif

