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

#include "twine/renderer/renderer.h"
#include "common/util.h"
#include "twine/menu/interface.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/shadeangletab.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/grid.h"
#include "twine/scene/movements.h"
#include "twine/shared.h"
#include "twine/twine.h"

namespace TwinE {

#define RENDERTYPE_DRAWLINE 0
#define RENDERTYPE_DRAWPOLYGON 1
#define RENDERTYPE_DRAWSPHERE 2

Renderer::Renderer(TwinEEngine *engine) : _engine(engine) {
}

Renderer::~Renderer() {
	free(_tabVerticG);
	free(_tabVerticD);
	free(_tabCoulG);
	free(_tabCoulD);
	free(_taby0);
	free(_taby1);
}

void Renderer::init(int32 w, int32 h) {
	size_t size = _engine->height() * sizeof(int16);

	_tabVerticG = (int16 *)malloc(size);
	memset(_tabVerticG, 0, size);
	_tabVerticD = (int16 *)malloc(size);
	memset(_tabVerticD, 0, size);
	_tabCoulG = (int16 *)malloc(size);
	memset(_tabCoulG, 0, size);
	_tabCoulD = (int16 *)malloc(size);
	memset(_tabCoulD, 0, size);
	_taby0 = (int16 *)malloc(size);
	memset(_taby0, 0, size);
	_taby1 = (int16 *)malloc(size);
	memset(_taby1, 0, size);

	_tabx0 = _tabCoulG;
	_tabx1 = _tabCoulD;
}

void Renderer::projIso(IVec3 &pos, int32 x, int32 y, int32 z) {
	pos.x = (int16)((((x - z) * 24) / ISO_SCALE) + _projectionCenter.x);
	pos.y = (int16)(((((x + z) * 12) - (y * 30)) / ISO_SCALE) + _projectionCenter.y);
	pos.z = 0;
}

IVec3 Renderer::projectPoint(int32 cX, int32 cY, int32 cZ) { // ProjettePoint
	IVec3 pos;
	if (_typeProj == TYPE_ISO) {
		projIso(pos, cX, cY, cZ);
		return pos;
	}

	if (_cameraRot.z - cZ < 0) {
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;
		return pos;
	}

	cX -= _cameraRot.x;
	cY -= _cameraRot.y;
	cZ = _cameraRot.z - cZ;

	int32 posZ = cZ + _kFactor;
	if (posZ <= 0) {
		posZ = 0x7FFF;
	}

	pos.x = (cX * _lFactorX) / posZ + _projectionCenter.x;
	pos.y = (-cY * _lFactorY) / posZ + _projectionCenter.y;
	pos.z = posZ;
	return pos;
}

void Renderer::setProjection(int32 x, int32 y, int32 kfact, int32 lfactx, int32 lfacty) {
	_projectionCenter.x = x;
	_projectionCenter.y = y;

	_kFactor = kfact;
	_lFactorX = lfactx;
	_lFactorY = lfacty;

	_typeProj = TYPE_3D;
}

void Renderer::setPosCamera(int32 x, int32 y, int32 z) {
	_cameraPos.x = x;
	_cameraPos.y = y;
	_cameraPos.z = z;
}

void Renderer::setIsoProjection(int32 x, int32 y, int32 scale) {
	_projectionCenter.x = x;
	_projectionCenter.y = y;
	_projectionCenter.z = scale; // not used - IsoScale is always 512

	_typeProj = TYPE_ISO;
}

void Renderer::flipMatrix() { // FlipMatrice
	SWAP(_matrixWorld.row1.y, _matrixWorld.row2.x);
	SWAP(_matrixWorld.row1.z, _matrixWorld.row3.x);
	SWAP(_matrixWorld.row2.z, _matrixWorld.row3.y);
}

IVec3 Renderer::setInverseAngleCamera(int32 alpha, int32 beta, int32 gamma) {
	setAngleCamera(alpha, beta, gamma);
	flipMatrix();
	_cameraRot = longWorldRot(_cameraPos.x, _cameraPos.y, _cameraPos.z);
	return _cameraRot;
}

IVec3 Renderer::setAngleCamera(int32 alpha, int32 beta, int32 gamma) {
	const int32 cAlpha = ClampAngle(alpha);
	const int32 cAlpha2 = ClampAngle(alpha + LBAAngles::ANGLE_90);
	const int32 nSin = sinTab[cAlpha];
	const int32 nCos = sinTab[cAlpha2];
	const int32 cGamma = ClampAngle(gamma);
	const int32 cGamma2 = ClampAngle(gamma + LBAAngles::ANGLE_90);
	int32 nSin2 = sinTab[cGamma];
	int32 nCos2 = sinTab[cGamma2];

	_matrixWorld.row1.x = nCos2;
	_matrixWorld.row1.y = -nSin2;
	_matrixWorld.row2.x = (nSin2 * nCos) >> 14;
	_matrixWorld.row2.y = (nCos2 * nCos) >> 14;
	_matrixWorld.row3.x = (nSin2 * nSin) >> 14;
	_matrixWorld.row3.y = (nCos2 * nSin) >> 14;

	const int32 cBeta = ClampAngle(beta);
	const int32 cBeta2 = ClampAngle(beta + LBAAngles::ANGLE_90);
	nSin2 = sinTab[cBeta];
	nCos2 = sinTab[cBeta2];

	int32 h = _matrixWorld.row1.x;
	_matrixWorld.row1.x = (nCos2 * h) >> 14;
	_matrixWorld.row1.z = (nSin2 * h) >> 14;

	h = _matrixWorld.row2.x;
	_matrixWorld.row2.x = ((nCos2 * h) + (nSin2 * nSin)) >> 14;
	_matrixWorld.row2.z = ((nSin2 * h) - (nCos2 * nSin)) >> 14;

	h = _matrixWorld.row3.x;
	_matrixWorld.row3.x = ((nCos2 * h) - (nSin2 * nCos)) >> 14;
	_matrixWorld.row3.z = ((nCos2 * nCos) + (nSin2 * h)) >> 14;

	_cameraRot = longWorldRot(_cameraPos.x, _cameraPos.y, _cameraPos.z);

	return _cameraRot;
}

IVec3 Renderer::worldRotatePoint(const IVec3& vec) {
	const int32 vx = (_matrixWorld.row1.x * vec.x + _matrixWorld.row1.y * vec.y + _matrixWorld.row1.z * vec.z) / SCENE_SIZE_HALF;
	const int32 vy = (_matrixWorld.row2.x * vec.x + _matrixWorld.row2.y * vec.y + _matrixWorld.row2.z * vec.z) / SCENE_SIZE_HALF;
	const int32 vz = (_matrixWorld.row3.x * vec.x + _matrixWorld.row3.y * vec.y + _matrixWorld.row3.z * vec.z) / SCENE_SIZE_HALF;
	return IVec3(vx, vy, vz);
}

IVec3 Renderer::longWorldRot(int32 x, int32 y, int32 z) {
	const int64 vx = ((int64)_matrixWorld.row1.x * (int64)x + (int64)_matrixWorld.row1.y * (int64)y + (int64)_matrixWorld.row1.z * (int64)z) / SCENE_SIZE_HALF;
	const int64 vy = ((int64)_matrixWorld.row2.x * (int64)x + (int64)_matrixWorld.row2.y * (int64)y + (int64)_matrixWorld.row2.z * (int64)z) / SCENE_SIZE_HALF;
	const int64 vz = ((int64)_matrixWorld.row3.x * (int64)x + (int64)_matrixWorld.row3.y * (int64)y + (int64)_matrixWorld.row3.z * (int64)z) / SCENE_SIZE_HALF;
	return IVec3((int32)vx, (int32)vy, (int32)vz);
}

IVec3 Renderer::longInverseRot(int32 x, int32 y, int32 z) {
	const int64 vx = ((int64)_matrixWorld.row1.x * (int64)x + (int64)_matrixWorld.row2.x * (int64)y + (int64)_matrixWorld.row3.x * (int64)z) / SCENE_SIZE_HALF;
	const int64 vy = ((int64)_matrixWorld.row1.y * (int64)x + (int64)_matrixWorld.row2.y * (int64)y + (int64)_matrixWorld.row3.y * (int64)z) / SCENE_SIZE_HALF;
	const int64 vz = ((int64)_matrixWorld.row1.z * (int64)x + (int64)_matrixWorld.row2.z * (int64)y + (int64)_matrixWorld.row3.z * (int64)z) / SCENE_SIZE_HALF;
	return IVec3((int32)vx, (int32)vy, (int32)vz);
}

IVec3 Renderer::rot(const IMatrix3x3 &matrix, int32 x, int32 y, int32 z) {
	const int32 vx = (matrix.row1.x * x + matrix.row1.y * y + matrix.row1.z * z) / SCENE_SIZE_HALF;
	const int32 vy = (matrix.row2.x * x + matrix.row2.y * y + matrix.row2.z * z) / SCENE_SIZE_HALF;
	const int32 vz = (matrix.row3.x * x + matrix.row3.y * y + matrix.row3.z * z) / SCENE_SIZE_HALF;
	return IVec3(vx, vy, vz);
}

void Renderer::setFollowCamera(int32 targetX, int32 targetY, int32 targetZ, int32 cameraAlpha, int32 cameraBeta, int32 cameraGamma, int32 cameraZoom) {
	_cameraPos.x = targetX;
	_cameraPos.y = targetY;
	_cameraPos.z = targetZ;

	setAngleCamera(cameraAlpha, cameraBeta, cameraGamma);
	_cameraRot.z += cameraZoom;

	_cameraPos = longInverseRot(_cameraRot.x, _cameraRot.y, _cameraRot.z);
}

IVec2 Renderer::rotate(int32 side, int32 forward, int32 angle) const {
	if (angle) {
		const int32 nSin = sinTab[ClampAngle(angle)];
		const int32 nCos = sinTab[ClampAngle((angle + LBAAngles::ANGLE_90))];

		const int32 x0 = ((side * nCos) + (forward * nSin)) >> 14;
		const int32 y0 = ((forward * nCos) - (side * nSin)) >> 14;
		return IVec2(x0, y0);
	}
	return IVec2(side, forward);
}

void Renderer::rotMatIndex2(IMatrix3x3 *pDest, const IMatrix3x3 *pSrc, const IVec3 &angleVec) {
	IMatrix3x3 tmp;
	const int32 lAlpha = angleVec.x;
	const int32 lBeta = angleVec.y;
	const int32 lGamma = angleVec.z;

	if (lAlpha) {
		int32 nSin = sinTab[ClampAngle(lAlpha)];
		int32 nCos = sinTab[ClampAngle(lAlpha + LBAAngles::ANGLE_90)];

		pDest->row1.x = pSrc->row1.x;
		pDest->row2.x = pSrc->row2.x;
		pDest->row3.x = pSrc->row3.x;

		pDest->row1.y = (pSrc->row1.z * nSin + pSrc->row1.y * nCos) / SCENE_SIZE_HALF;
		pDest->row1.z = (pSrc->row1.z * nCos - pSrc->row1.y * nSin) / SCENE_SIZE_HALF;
		pDest->row2.y = (pSrc->row2.z * nSin + pSrc->row2.y * nCos) / SCENE_SIZE_HALF;
		pDest->row2.z = (pSrc->row2.z * nCos - pSrc->row2.y * nSin) / SCENE_SIZE_HALF;
		pDest->row3.y = (pSrc->row3.z * nSin + pSrc->row3.y * nCos) / SCENE_SIZE_HALF;
		pDest->row3.z = (pSrc->row3.z * nCos - pSrc->row3.y * nSin) / SCENE_SIZE_HALF;
		pSrc = pDest;
	}

	if (lGamma) {
		int32 nSin = sinTab[ClampAngle(lGamma)];
		int32 nCos = sinTab[ClampAngle(lGamma + LBAAngles::ANGLE_90)];

		tmp.row1.z = pSrc->row1.z;
		tmp.row2.z = pSrc->row2.z;
		tmp.row3.z = pSrc->row3.z;

		tmp.row1.x = (pSrc->row1.y * nSin + pSrc->row1.x * nCos) / SCENE_SIZE_HALF;
		tmp.row1.y = (pSrc->row1.y * nCos - pSrc->row1.x * nSin) / SCENE_SIZE_HALF;
		tmp.row2.x = (pSrc->row2.y * nSin + pSrc->row2.x * nCos) / SCENE_SIZE_HALF;
		tmp.row2.y = (pSrc->row2.y * nCos - pSrc->row2.x * nSin) / SCENE_SIZE_HALF;
		tmp.row3.x = (pSrc->row3.y * nSin + pSrc->row3.x * nCos) / SCENE_SIZE_HALF;
		tmp.row3.y = (pSrc->row3.y * nCos - pSrc->row3.x * nSin) / SCENE_SIZE_HALF;
	}

	if (lBeta) {
		int32 nSin = sinTab[ClampAngle(lBeta)];
		int32 nCos = sinTab[ClampAngle(lBeta + LBAAngles::ANGLE_90)];

		if (pSrc == pDest) {
			tmp.row1.x = pSrc->row1.x;
			tmp.row1.z = pSrc->row1.z;
			tmp.row2.x = pSrc->row2.x;
			tmp.row2.z = pSrc->row2.z;
			tmp.row3.x = pSrc->row3.x;
			tmp.row3.z = pSrc->row3.z;
			pSrc = &tmp;
		} else {
			pDest->row1.y = pSrc->row1.y;
			pDest->row2.y = pSrc->row2.y;
			pDest->row3.y = pSrc->row3.y;
		}

		pDest->row1.x = (pSrc->row1.x * nCos - pSrc->row1.z * nSin) / SCENE_SIZE_HALF;
		pDest->row1.z = (pSrc->row1.x * nSin + pSrc->row1.z * nCos) / SCENE_SIZE_HALF;
		pDest->row2.x = (pSrc->row2.x * nCos - pSrc->row2.z * nSin) / SCENE_SIZE_HALF;
		pDest->row2.z = (pSrc->row2.x * nSin + pSrc->row2.z * nCos) / SCENE_SIZE_HALF;
		pDest->row3.x = (pSrc->row3.x * nCos - pSrc->row3.z * nSin) / SCENE_SIZE_HALF;
		pDest->row3.z = (pSrc->row3.x * nSin + pSrc->row3.z * nCos) / SCENE_SIZE_HALF;
	} else if (pSrc != pDest) {
		*pDest = *pSrc;
	}
}

bool isPolygonVisible(const ComputedVertex *vertices) { // TestVuePoly
	const int32 a = ((int32)vertices[0].y - (int32)vertices[2].y) * ((int32)vertices[1].x - (int32)vertices[0].x);
	const int32 b = ((int32)vertices[1].y - (int32)vertices[0].y) * ((int32)vertices[0].x - (int32)vertices[2].x);
	if (a <= b) {
		return false;
	}
	return true;
}

void Renderer::rotList(const Common::Array<BodyVertex> &vertices, int32 firstPoint, int32 numPoints, I16Vec3 *destPoints, const IMatrix3x3 *rotationMatrix, const IVec3 &destPos) {
	for (int32 i = 0; i < numPoints; ++i) {
		const BodyVertex &vertex = vertices[i + firstPoint];
		destPoints->x = (int16)(((rotationMatrix->row1.x * vertex.x + rotationMatrix->row1.y * vertex.y + rotationMatrix->row1.z * vertex.z) / SCENE_SIZE_HALF) + destPos.x);
		destPoints->y = (int16)(((rotationMatrix->row2.x * vertex.x + rotationMatrix->row2.y * vertex.y + rotationMatrix->row2.z * vertex.z) / SCENE_SIZE_HALF) + destPos.y);
		destPoints->z = (int16)(((rotationMatrix->row3.x * vertex.x + rotationMatrix->row3.y * vertex.y + rotationMatrix->row3.z * vertex.z) / SCENE_SIZE_HALF) + destPos.z);

		destPoints++;
	}
}

// RotateGroupe
void Renderer::processRotatedElement(IMatrix3x3 *targetMatrix, const Common::Array<BodyVertex> &vertices, int32 alpha, int32 beta, int32 gamma, const BodyBone &bone, ModelData *modelData) {
	const int32 firstPoint = bone.firstVertex;
	const int32 numOfPoints = bone.numVertices;
	const IVec3 renderAngle(alpha, beta, gamma);

	const IMatrix3x3 *currentMatrix;
	IVec3 destPos;
	// if its the first point
	if (bone.isRoot()) {
		currentMatrix = &_matrixWorld;
	} else {
		const int32 pointIdx = bone.vertex;
		const int32 matrixIndex = bone.parent;
		assert(matrixIndex >= 0 && matrixIndex < ARRAYSIZE(_matricesTable));
		currentMatrix = &_matricesTable[matrixIndex];

		destPos = modelData->computedPoints[pointIdx];
	}

	rotMatIndex2(targetMatrix, currentMatrix, renderAngle);

	if (!numOfPoints) {
		warning("RENDER WARNING: No points in this model!");
	}

	rotList(vertices, firstPoint, numOfPoints, &modelData->computedPoints[firstPoint], targetMatrix, destPos);
}

void Renderer::transRotList(const Common::Array<BodyVertex> &vertices, int32 firstPoint, int32 numPoints, I16Vec3 *destPoints, const IMatrix3x3 *translationMatrix, const IVec3 &angleVec, const IVec3 &destPos) {
	for (int32 i = 0; i < numPoints; ++i) {
		const BodyVertex &vertex = vertices[i + firstPoint];
		const int16 tmpX = (int16)(vertex.x + angleVec.x);
		const int16 tmpY = (int16)(vertex.y + angleVec.y);
		const int16 tmpZ = (int16)(vertex.z + angleVec.z);

		destPoints->x = ((translationMatrix->row1.x * tmpX + translationMatrix->row1.y * tmpY + translationMatrix->row1.z * tmpZ) / SCENE_SIZE_HALF) + destPos.x;
		destPoints->y = ((translationMatrix->row2.x * tmpX + translationMatrix->row2.y * tmpY + translationMatrix->row2.z * tmpZ) / SCENE_SIZE_HALF) + destPos.y;
		destPoints->z = ((translationMatrix->row3.x * tmpX + translationMatrix->row3.y * tmpY + translationMatrix->row3.z * tmpZ) / SCENE_SIZE_HALF) + destPos.z;

		destPoints++;
	}
}

// TranslateGroupe
void Renderer::translateGroup(IMatrix3x3 *targetMatrix, const Common::Array<BodyVertex> &vertices, int32 rotX, int32 rotY, int32 rotZ, const BodyBone &bone, ModelData *modelData) {
	IVec3 renderAngle;
	renderAngle.x = rotX;
	renderAngle.y = rotY;
	renderAngle.z = rotZ;

	IVec3 destPos;

	if (bone.isRoot()) { // base point
		*targetMatrix = _matrixWorld;
	} else { // dependent
		const int32 pointsIdx = bone.vertex;
		destPos = modelData->computedPoints[pointsIdx];

		const int32 matrixIndex = bone.parent;
		assert(matrixIndex >= 0 && matrixIndex < ARRAYSIZE(_matricesTable));
		*targetMatrix = _matricesTable[matrixIndex];
	}

	transRotList(vertices, bone.firstVertex, bone.numVertices, &modelData->computedPoints[bone.firstVertex], targetMatrix, renderAngle, destPos);
}

void Renderer::setLightVector(int32 angleX, int32 angleY, int32 angleZ) {
	const int32 normalUnit = 64;
	const IVec3 renderAngle(angleX, angleY, angleZ);
	IMatrix3x3 rotationMatrix;
	rotMatIndex2(&rotationMatrix, &_matrixWorld, renderAngle);
	_normalLight = rot(rotationMatrix, 0, 0, normalUnit - 5);
}

int16 Renderer::leftClip(int16 polyRenderType, ComputedVertex** offTabPoly, int32 numVertices) {
	const Common::Rect &clip = _engine->_interface->_clip;
	ComputedVertex *pTabPolyClip = offTabPoly[1];
	ComputedVertex *pTabPoly = offTabPoly[0];
	int16 newNbPoints = 0;

	// invert the pointers to continue on the clipped vertices in the next method
	offTabPoly[0] = pTabPolyClip;
	offTabPoly[1] = pTabPoly;

	for (; numVertices > 0; --numVertices, pTabPoly++) {
		const ComputedVertex *p0 = pTabPoly;
		const ComputedVertex *p1 = p0 + 1;

		// clipFlag :
		// 0x00 : none clipped
		// 0x01 : point 0 clipped
		// 0x02 : point 1 clipped
		// 0x03 : both clipped
		uint8 clipFlag = (p1->x < clip.left) ? 2 : 0;

		if (p0->x < clip.left) {
			if (clipFlag) {
				continue; // both clipped, skip point 0
			}
			clipFlag |= 1;
		} else {
			// point 0 not clipped, store it
			*pTabPolyClip++ = *pTabPoly;
			++newNbPoints;
		}

		if (clipFlag) {
			// point 0 or 1 is clipped, apply clipping
			if (p1->x >= p0->x) {
				p0 = p1;
				p1 = pTabPoly;
			}

			const int32 dx = p1->x - p0->x;
			const int32 dy = p1->y - p0->y;
			const int32 dxClip = clip.left - p0->x;

			pTabPolyClip->y = (int16)(p0->y + ((dxClip * dy) / dx));
			pTabPolyClip->x = (int16)clip.left;

			if (polyRenderType >= POLYGONTYPE_GOURAUD) {
				pTabPolyClip->intensity = (int16)(p0->intensity + (((p1->intensity - p0->intensity) * dxClip) / dx));
			}

			++pTabPolyClip;
			++newNbPoints;
		}
	}

	// copy first vertex to the end
	*pTabPolyClip = *offTabPoly[0];
	return newNbPoints;
}

int16 Renderer::rightClip(int16 polyRenderType, ComputedVertex** offTabPoly, int32 numVertices) {
	const Common::Rect &clip = _engine->_interface->_clip;
	ComputedVertex *pTabPolyClip = offTabPoly[1];
	ComputedVertex *pTabPoly = offTabPoly[0];
	int16 newNbPoints = 0;

	// invert the pointers to continue on the clipped vertices in the next method
	offTabPoly[0] = pTabPolyClip;
	offTabPoly[1] = pTabPoly;

	for (; numVertices > 0; --numVertices, pTabPoly++) {
		const ComputedVertex *p0 = pTabPoly;
		const ComputedVertex *p1 = p0 + 1;

		// clipFlag :
		// 0x00 : none clipped
		// 0x01 : point 0 clipped
		// 0x02 : point 1 clipped
		// 0x03 : both clipped
		uint8 clipFlag = (p1->x > clip.right) ? 2 : 0;

		if (p0->x > clip.right) {
			if (clipFlag) {
				continue; // both clipped, skip point 0
			}
			clipFlag |= 1;
		} else {
			// point 0 not clipped, store it
			*pTabPolyClip++ = *pTabPoly;
			++newNbPoints;
		}

		if (clipFlag) {
			// point 0 or 1 is clipped, apply clipping
			if (p1->x >= p0->x) {
				p0 = p1;
				p1 = pTabPoly;
			}

			const int32 dx = p1->x - p0->x;
			const int32 dy = p1->y - p0->y;
			const int32 dxClip = clip.right - p0->x;

			pTabPolyClip->y = (int16)(p0->y + ((dxClip * dy) / dx));
			pTabPolyClip->x = (int16)clip.right;

			if (polyRenderType >= POLYGONTYPE_GOURAUD) {
				pTabPolyClip->intensity = (int16)(p0->intensity + (((p1->intensity - p0->intensity) * dxClip) / dx));
			}

			++pTabPolyClip;
			++newNbPoints;
		}
	}

	// copy first vertex to the end
	*pTabPolyClip = *offTabPoly[0];
	return newNbPoints;
}

int16 Renderer::topClip(int16 polyRenderType, ComputedVertex** offTabPoly, int32 numVertices) {
	const Common::Rect &clip = _engine->_interface->_clip;
	ComputedVertex *pTabPolyClip = offTabPoly[1];
	ComputedVertex *pTabPoly = offTabPoly[0];
	int16 newNbPoints = 0;

	// invert the pointers to continue on the clipped vertices in the next method
	offTabPoly[0] = pTabPolyClip;
	offTabPoly[1] = pTabPoly;

	for (; numVertices > 0; --numVertices, pTabPoly++) {
		const ComputedVertex *p0 = pTabPoly;
		const ComputedVertex *p1 = p0 + 1;

		// clipFlag :
		// 0x00 : none clipped
		// 0x01 : point 0 clipped
		// 0x02 : point 1 clipped
		// 0x03 : both clipped
		uint8 clipFlag = (p1->y < clip.top) ? 2 : 0;

		if (p0->y < clip.top) {
			if (clipFlag) {
				continue; // both clipped, skip point 0
			}
			clipFlag |= 1;
		} else {
			// point 0 not clipped, store it
			*pTabPolyClip++ = *pTabPoly;
			++newNbPoints;
		}

		if (clipFlag) {
			// point 0 or 1 is clipped, apply clipping
			if (p1->y >= p0->y) {
				p0 = p1;
				p1 = pTabPoly;
			}

			const int32 dx = p1->x - p0->x;
			const int32 dy = p1->y - p0->y;
			const int32 dyClip = clip.top - p0->y;

			pTabPolyClip->x = (int16)(p0->x + ((dyClip * dx) / dy));
			pTabPolyClip->y = (int16)clip.top;

			if (polyRenderType >= POLYGONTYPE_GOURAUD) {
				pTabPolyClip->intensity = (int16)(p0->intensity + (((p1->intensity - p0->intensity) * dyClip) / dy));
			}

			++pTabPolyClip;
			++newNbPoints;
		}
	}

	// copy first vertex to the end
	*pTabPolyClip = *offTabPoly[0];
	return newNbPoints;
}

int16 Renderer::bottomClip(int16 polyRenderType, ComputedVertex** offTabPoly, int32 numVertices) {
	const Common::Rect &clip = _engine->_interface->_clip;
	ComputedVertex *pTabPolyClip = offTabPoly[1];
	ComputedVertex *pTabPoly = offTabPoly[0];
	int16 newNbPoints = 0;

	// invert the pointers to continue on the clipped vertices in the next method
	offTabPoly[0] = pTabPolyClip;
	offTabPoly[1] = pTabPoly;

	for (; numVertices > 0; --numVertices, pTabPoly++) {
		const ComputedVertex *p0 = pTabPoly;
		const ComputedVertex *p1 = p0 + 1;

		// clipFlag :
		// 0x00 : none clipped
		// 0x01 : point 0 clipped
		// 0x02 : point 1 clipped
		// 0x03 : both clipped
		uint8 clipFlag = (p1->y > clip.bottom) ? 2 : 0;

		if (p0->y > clip.bottom) {
			if (clipFlag) {
				continue; // both clipped, skip point 0
			}
			clipFlag |= 1;
		} else {
			// point 0 not clipped, store it
			*pTabPolyClip++ = *pTabPoly;
			++newNbPoints;
		}

		if (clipFlag) {
			// point 0 or 1 is clipped, apply clipping
			if (p1->y >= p0->y) {
				p0 = p1;
				p1 = pTabPoly;
			}

			const int32 dx = p1->x - p0->x;
			const int32 dy = p1->y - p0->y;
			const int32 dyClip = clip.bottom - p0->y;

			pTabPolyClip->x = (int16)(p0->x + ((dyClip * dx) / dy));
			pTabPolyClip->y = (int16)clip.bottom;

			if (polyRenderType >= POLYGONTYPE_GOURAUD) {
				pTabPolyClip->intensity = (int16)(p0->intensity + (((p1->intensity - p0->intensity) * dyClip) / dy));
			}

			++pTabPolyClip;
			++newNbPoints;
		}
	}

	// copy first vertex to the end
	*pTabPolyClip = *offTabPoly[0];
	return newNbPoints;
}

int32 Renderer::computePolyMinMax(int16 polyRenderType, ComputedVertex **offTabPoly, int32 numVertices, int16 &ymin, int16 &ymax) {
	int16 xmin = SCENE_SIZE_MAX;
	int16 xmax = SCENE_SIZE_MIN;

	ymin = SCENE_SIZE_MAX;
	ymax = SCENE_SIZE_MIN;

	ComputedVertex* pTabPoly = offTabPoly[0];
	for (int32 i = 0; i < numVertices; i++) {
		if (pTabPoly[i].x < xmin) {
			xmin = pTabPoly[i].x;
		}
		if (pTabPoly[i].x > xmax) {
			xmax = pTabPoly[i].x;
		}
		if (pTabPoly[i].y < ymin) {
			ymin = pTabPoly[i].y;
		}
		if (pTabPoly[i].y > ymax) {
			ymax = pTabPoly[i].y;
		}
	}

	const Common::Rect &clip = _engine->_interface->_clip;
	// no vertices
	if (ymin > ymax || xmax < clip.left || xmin > clip.right || ymax < clip.top || ymin > clip.bottom) {
		debug(10, "Clipped %i:%i:%i:%i, clip rect(%i:%i:%i:%i)", xmin, ymin, xmax, ymax, clip.left, clip.top, clip.right, clip.bottom);
		return 0;
	}

	pTabPoly[numVertices] = *offTabPoly[0];

	bool hasBeenClipped = false;

	int32 clippedNumVertices = numVertices;
	if (xmin < clip.left) {
		clippedNumVertices = leftClip(polyRenderType, offTabPoly, clippedNumVertices);
		if (!clippedNumVertices) {
			return 0;
		}

		hasBeenClipped = true;
	}

	if (xmax > clip.right) {
		clippedNumVertices = rightClip(polyRenderType, offTabPoly, clippedNumVertices);
		if (!clippedNumVertices) {
			return 0;
		}

		hasBeenClipped = true;
	}

	if (ymin < clip.top) {
		clippedNumVertices = topClip(polyRenderType, offTabPoly, clippedNumVertices);
		if (!clippedNumVertices) {
			return 0;
		}

		hasBeenClipped = true;
	}

	if (ymax > clip.bottom) {
		clippedNumVertices = bottomClip(polyRenderType, offTabPoly, clippedNumVertices);
		if (!clippedNumVertices) {
			return 0;
		}

		hasBeenClipped = true;
	}

	if (hasBeenClipped) {
		// search the new Ymin or Ymax
		ymin = 32767;
		ymax = -32768;

		for (int32 n = 0; n < clippedNumVertices; ++n) {
			if (offTabPoly[0][n].y < ymin) {
				ymin = offTabPoly[0][n].y;
			}

			if (offTabPoly[0][n].y > ymax) {
				ymax = offTabPoly[0][n].y;
			}
		}

		if (ymin >= ymax) {
			return 0; // No valid polygon after clipping
		}
	}

	return clippedNumVertices;
}

bool Renderer::computePoly(int16 polyRenderType, const ComputedVertex *vertices, int32 numVertices, int16 &vtop, int16 &vbottom) {
	assert(numVertices < ARRAYSIZE(_clippedPolygonVertices1));
	for (int i = 0; i < numVertices; ++i) {
		_clippedPolygonVertices1[i] = vertices[i];
	}

	ComputedVertex *offTabPoly[] = {_clippedPolygonVertices1, _clippedPolygonVertices2};

	numVertices = computePolyMinMax(polyRenderType, offTabPoly, numVertices, vtop, vbottom);
	if (numVertices == 0) {
		return false;
	}

	ComputedVertex *pTabPoly = offTabPoly[0];
	ComputedVertex *p0;
	ComputedVertex *p1;
	int16 *pVertic = nullptr;
	int16 *pCoul;
	int32 incY = -1;
	int32 dx, dy, x, y, dc;
	int32 step, reminder;

	// Drawing lines between vertices
	for (; numVertices > 0; --numVertices, pTabPoly++) {
		pCoul = nullptr;
		p0 = pTabPoly;
		p1 = p0 + 1;

		dy = p1->y - p0->y;
		if (dy == 0) {
			// forget same Y points
			continue;
		} else if (dy > 0) {
			// Y therefore goes down left buffer
			if (p0->x <= p1->x) {
				incY = 1;
			} else {
				p0 = p1;
				p1 = pTabPoly;
				incY = -1;
			}

			pVertic = &_tabVerticG[p0->y];

			if (polyRenderType >= POLYGONTYPE_GOURAUD) {
				pCoul = &_tabCoulG[p0->y];
			}
		} else if (dy < 0) {
			dy = -dy;

			if (p0->x <= p1->x) {
				p0 = p1;
				p1 = pTabPoly;
				incY = 1;
			} else {
				incY = -1;
			}

			pVertic = &_tabVerticD[p0->y];

			if (polyRenderType >= POLYGONTYPE_GOURAUD) {
				pCoul = &_tabCoulD[p0->y];
			}
		}

		dx = (p1->x - p0->x) << 16;

		step = dx / dy;
		reminder = ((dx % dy) >> 1) + 0x7FFF;

		dx = step >> 16; // recovery part high division (entire)
		step &= 0xFFFF;  // preserves lower part (mantissa)
		x = p0->x;

		for (y = dy; y >= 0; --y) {
			*pVertic = (int16)x;
			pVertic += incY;
			x += dx;
			reminder += step;
			if (reminder & 0xFFFF0000) {
				x += reminder >> 16;
				reminder &= 0xFFFF;
			}
		}

		if (pCoul) {
			dc = (p1->intensity - p0->intensity) << 8;
			step = dc / dy;
			reminder = ((((dc % dy) >> 1) + 0x7F) & 0xFF) | (p0->intensity << 8);

			for (y = dy; y >= 0; --y) {
				*pCoul = (int16)reminder;
				pCoul += incY;
				reminder += step;
			}
		}
	}

	return true;
}

void Renderer::svgaPolyCopper(int16 vtop, int16 Ymax, uint16 color) const {
	const int screenWidth = _engine->width();
	int16 xMin, xMax;
	int16 y = vtop;
	byte *pDestLine = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, y);
	byte *pDest;
	int16 *pVerticG = &_tabVerticG[y];
	int16 *pVerticD = &_tabVerticD[y];
	int32 sens = 1;

	for (; y <= Ymax; y++) {
		xMin = *pVerticG++;
		xMax = *pVerticD++;
		pDest = pDestLine + xMin;

		for (; xMin <= xMax; xMin++) {
			*pDest++ = (byte)color;
		}

		color += sens;
		if (!(color & 0xF)) {
			sens = -sens;
			if (sens < 0) {
				color += sens;
			}
		}

		pDestLine += screenWidth;
	}
}

void Renderer::svgaPolyBopper(int16 vtop, int16 Ymax, uint16 color) const {
	const int screenWidth = _engine->width();
	int16 xMin, xMax;
	int16 y = vtop;
	byte *pDestLine = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, y);
	byte *pDest;
	int16 *pVerticG = &_tabVerticG[y];
	int16 *pVerticD = &_tabVerticD[y];
	int32 sens = 1;
	int32 line = 2;

	for (; y <= Ymax; y++) {
		xMin = *pVerticG++;
		xMax = *pVerticD++;
		pDest = pDestLine + xMin;

		for (; xMin <= xMax; xMin++) {
			*pDest++ = (byte)color;
		}

		line--;
		if (!line) {
			line = 2;
			color += sens;
			if (!(color & 0xF)) {
				sens = -sens;
				if (sens < 0) {
					color += sens;
				}
			}
		}

		pDestLine += screenWidth;
	}
}

void Renderer::svgaPolyTriste(int16 vtop, int16 Ymax, uint16 color) const {
	const int screenWidth = _engine->width();
	int16 xMin, xMax;
	int16 y = vtop;
	byte *pDestLine = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	byte *pDest;
	int16 *pVerticG = &_tabVerticG[y];
	int16 *pVerticD = &_tabVerticD[y];

	for (; y <= Ymax; y++) {
		xMin = *pVerticG++;
		xMax = *pVerticD++;
		pDest = pDestLine + xMin;

		for (; xMin <= xMax; xMin++) {
			*pDest++ = (byte)color;
		}

		pDestLine += screenWidth;
	}
}

#define ROL8(x,b) (byte)(((x) << (b)) | ((x) >> (8 - (b))))
#define ROL16(x, b) (((x) << (b)) | ((x) >> (16 - (b))))

void Renderer::svgaPolyTele(int16 vtop, int16 Ymax, uint16 color) const {
	const int screenWidth = _engine->width();
	int16 xMin, xMax;
	int16 y = vtop;
	int16 acc = 17371;
	byte *pDestLine = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	byte *pDest;
	int16 *pVerticG = &_tabVerticG[y];
	int16 *pVerticD = &_tabVerticD[y];
	uint16 col;

	color &= 0xFF;

	for (; y <= Ymax; y++) {
		xMin = *pVerticG++;
		xMax = *pVerticD++;
		pDest = pDestLine + xMin;
		col = xMin;

		for (; xMin <= xMax; xMin++) {
			col = ((col + acc) & 0xFF03) + (uint16)color;
			acc = ROL16(acc, 2) + 1;

			*pDest++ = (byte)col;
		}

		pDestLine += screenWidth;
	}
}

void Renderer::svgaPolyTrans(int16 vtop, int16 Ymax, uint16 color) const {
	const int screenWidth = _engine->width();
	int16 xMin, xMax;
	int16 y = vtop;
	byte *pDestLine = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	byte *pDest;
	int16 *pVerticG = &_tabVerticG[y];
	int16 *pVerticD = &_tabVerticD[y];

	color &= 0xF0;

	for (; y <= Ymax; y++) {
		xMin = *pVerticG++;
		xMax = *pVerticD++;
		pDest = pDestLine + xMin;

		for (; xMin <= xMax; xMin++) {
			*pDest = (byte)color | (*pDest & 0x0F);
			pDest++;
		}

		pDestLine += screenWidth;
	}
}

// Used e.g for the legs of the horse or the ears of most characters
void Renderer::svgaPolyTrame(int16 vtop, int16 Ymax, uint16 color) const {
	const int screenWidth = _engine->width();
	int16 xMin, xMax;
	int16 y = vtop;
	byte *pDestLine = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	byte *pDest;
	int16 *pVerticG = &_tabVerticG[y];
	int16 *pVerticD = &_tabVerticD[y];
	int32 pair = 0;

	for (; y <= Ymax; y++) {
		xMin = *pVerticG++;
		xMax = *pVerticD++;
		pDest = pDestLine + xMin;

		xMax = ((xMax - xMin) + 1) / 2;
		if (xMax > 0) {
			pair ^= 1; // paire/impair
			if ((xMin & 1) ^ pair) {
				pDest++;
			}

			for (; xMax > 0; xMax--) {
				*pDest = (byte)color;
				pDest += 2;
			}
		}

		pDestLine += screenWidth;
	}
}

void Renderer::svgaPolyGouraud(int16 vtop, int16 Ymax) const {
	const int screenWidth = _engine->width();
	int16 xMin, xMax;
	int16 y = vtop;
	int16 start, end, step;
	byte *pDestLine = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, y);
	byte *pDest;
	int16 *pVerticG = &_tabVerticG[y];
	int16 *pVerticD = &_tabVerticD[y];
	int16 *pCoulG = &_tabCoulG[y];
	int16 *pCoulD = &_tabCoulD[y];

	for (; y <= Ymax; y++) {
		xMin = *pVerticG++;
		xMax = *pVerticD++;
		start = *pCoulG++;
		end = *pCoulD++;
		pDest = pDestLine + xMin;

		xMax -= xMin;

		if (xMax == 0) {
			*pDest = (byte)((end + start) >> 9);
		} else if (xMax <= 2) {
			pDest[xMax--] = (byte)(end >> 8);
			if (xMax) {
				pDest[xMax--] = (byte)((end + start) >> 9);
			}
			*pDest = (byte)(start >> 8);
		} else {
			step = (end - start) / xMax;

			for (; xMax >= 0; xMax--) {
				*pDest++ = (byte)(start >> 8);
				start += step;
			}
		}

		pDestLine += screenWidth;
	}
}

// used for the most of the heads of the characters and the horse body
void Renderer::svgaPolyDith(int16 vtop, int16 Ymax) const {
	const int screenWidth = _engine->width();
	int16 xMin, xMax;
	int16 y = vtop;
	int16 start, end, step, delta, impair;
	byte *pDestLine = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, y);
	byte *pDest;
	int16 *pVerticG = &_tabVerticG[y];
	int16 *pVerticD = &_tabVerticD[y];
	int16 *pCoulG = &_tabCoulG[y];
	int16 *pCoulD = &_tabCoulD[y];

	for (; y <= Ymax; y++) {
		xMin = *pVerticG++;
		xMax = *pVerticD++;
		start = *pCoulG++;
		end = *pCoulD++;
		pDest = pDestLine + xMin;

		xMax -= xMin;
		delta = end - start;

		if (xMax == 0) {
			// rcr ax,1
			*pDest = (byte)(((int32)end + start) >> 9);
		} else if (xMax <= 2) {
			step = start;

			if (xMax == 2) // if( !(xMax & 1) )
			{
				delta = (delta >> 1) | (delta & 0x8000); // sar ax,1
				step &= 0xFF;
				step = start + ROL8(step, 1);
				*pDest++ = (byte)(step >> 8);
				start += delta;
			}

			step = start + (step & 0xFF);
			*pDest++ = (byte)(step >> 8);

			start += delta;
			step &= 0xFF;
			step = start + ROL8(step, 1);
			*pDest = (byte)(step >> 8);
		} else {
			delta /= xMax;
			step = start;
			impair = xMax & 1;
			xMax = (xMax + 1) >> 1;

			if (!impair) {
				step &= 0xFF;
				step = start + ROL8(step, xMax & 7);
				*pDest++ = (byte)(step >> 8);
				start += delta;
			}

			for (; xMax > 0; xMax--) {
				step &= 0xFF;
				step += start;
				*pDest++ = (byte)(step >> 8);
				start += delta;
				step &= 0xFF;
				step = start + ROL8(step, xMax & 7);
				*pDest++ = (byte)(step >> 8);
				start += delta;
			}
		}

		pDestLine += screenWidth;
	}
}

void Renderer::svgaPolyMarbre(int16 vtop, int16 Ymax, uint16 color) const {
	const int screenWidth = _engine->width();
	int16 xMin, xMax;
	int16 y = vtop;
	byte *pDestLine = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, y);
	byte *pDest;
	int16 *pVerticG = &_tabVerticG[y];
	int16 *pVerticD = &_tabVerticD[y];

	// color contains 2 colors: 0xFF start, 0xFF00 end
	uint16 start = (color & 0xFF) << 8;
	uint16 end = color & 0xFF00;
	uint16 delta = end - start + 1; // delta intensity
	int32 step, dc;

	for (; y <= Ymax; y++) {
		xMin = *pVerticG++;
		xMax = *pVerticD++;
		pDest = pDestLine + xMin;

		dc = xMax - xMin;
		if (dc == 0) {
			// just one
			*pDest++ = (byte)(end >> 8);
		} else if (dc > 0) {
			step = delta / (dc + 1);
			color = start;

			for (; xMin <= xMax; xMin++) {
				*pDest++ = (byte)(color >> 8);
				color += step;
			}
		}

		pDestLine += screenWidth;
	}
}

void Renderer::svgaPolyTriche(int16 vtop, int16 Ymax, uint16 color) const {
	const int screenWidth = _engine->width();
	int16 xMin, xMax;
	int16 y = vtop;
	byte *pDestLine = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, y);
	byte *pDest;
	int16 *pVerticG = &_tabVerticG[y];
	int16 *pVerticD = &_tabVerticD[y];
	int16 *pCoulG = &_tabCoulG[y];

	for (; y <= Ymax; y++) {
		xMin = *pVerticG++;
		xMax = *pVerticD++;
		pDest = pDestLine + xMin;

		color = (*pCoulG++) >> 8;
		for (; xMin <= xMax; xMin++) {
			*pDest++ = (byte)color;
		}

		pDestLine += screenWidth;
	}
}

void Renderer::renderPolygons(const CmdRenderPolygon &polygon, ComputedVertex *vertices) {
	int16 vtop, vbottom;
	uint8 renderType = polygon.renderType;
	if (computePoly(renderType, vertices, polygon.numVertices, vtop, vbottom)) {
		fillVertices(vtop, vbottom, renderType, polygon.colorIndex);
	}
}

void Renderer::fillVertices(int16 vtop, int16 vbottom, uint8 renderType, uint16 color) {
	switch (renderType) {
	case POLYGONTYPE_FLAT:
		svgaPolyTriste(vtop, vbottom, color);
		break;
	case POLYGONTYPE_TELE:
		if (_engine->_cfgfile.PolygonDetails == 0) {
			svgaPolyTriste(vtop, vbottom, color);
		} else {
			svgaPolyTele(vtop, vbottom, color);
		}
		break;
	case POLYGONTYPE_COPPER:
		svgaPolyCopper(vtop, vbottom, color);
		break;
	case POLYGONTYPE_BOPPER:
		svgaPolyBopper(vtop, vbottom, color);
		break;
	case POLYGONTYPE_TRANS:
		svgaPolyTrans(vtop, vbottom, color);
		break;
	case POLYGONTYPE_TRAME: // raster
		svgaPolyTrame(vtop, vbottom, color);
		break;
	case POLYGONTYPE_GOURAUD:
		if (_engine->_cfgfile.PolygonDetails == 0) {
			svgaPolyTriche(vtop, vbottom, color);
		} else {
			svgaPolyGouraud(vtop, vbottom);
		}
		break;
	case POLYGONTYPE_DITHER:
		if (_engine->_cfgfile.PolygonDetails == 0) {
			svgaPolyTriche(vtop, vbottom, color);
		} else if (_engine->_cfgfile.PolygonDetails == 1) {
			svgaPolyGouraud(vtop, vbottom);
		} else {
			svgaPolyDith(vtop, vbottom);
		}
		break;
	case POLYGONTYPE_MARBLE:
		svgaPolyMarbre(vtop, vbottom, color);
		break;
	default:
		warning("RENDER WARNING: Unsupported render type %d", renderType);
		break;
	}
}

bool Renderer::computeSphere(int32 x, int32 y, int32 radius, int &vtop, int &vbottom) {
	if (radius <= 0) {
		return false;
	}
	int16 left = (int16)(x - radius);
	int16 right = (int16)(x + radius);
	int16 bottom = (int16)(y + radius);
	int16 top = (int16)(y - radius);
	const Common::Rect &clip = _engine->_interface->_clip;
	int16 cleft = clip.left;
	int16 cright = clip.right;
	int16 ctop = clip.top;
	int16 cbottom = clip.bottom;

	if (left <= cright && right >= cleft && bottom <= cbottom && top >= ctop) {
		if (left < cleft) {
			left = cleft;
		}
		if (bottom > cbottom) {
			bottom = cbottom;
		}
		if (right > cright) {
			right = cright;
		}
		if (top < ctop) {
			top = ctop;
		}

		int32 r = 0;
		int32 acc = -radius;

		while (r <= radius) {
			int32 x1 = x - radius;
			if (x1 < cleft) {
				x1 = cleft;
			}

			int32 x2 = x + radius;
			if (x2 > cright) {
				x2 = cright;
			}

			int32 ny = y - r;
			if ((ny >= ctop) && (ny <= cbottom)) {
				_tabVerticG[ny] = (int16)x1;
				_tabVerticD[ny] = (int16)x2;
			}

			ny = y + r;
			if ((ny >= ctop) && (ny <= cbottom)) {
				_tabVerticG[ny] = (int16)x1;
				_tabVerticD[ny] = (int16)x2;
			}

			if (acc < 0) {
				acc += r;
				if (acc >= 0) {
					x1 = x - r;
					if (x1 < cleft) {
						x1 = cleft;
					}

					x2 = x + r;
					if (x2 > cright) {
						x2 = cright;
					}

					ny = y - radius;
					if ((ny >= ctop) && (ny <= cbottom)) {
						_tabVerticG[ny] = (int16)x1;
						_tabVerticD[ny] = (int16)x2;
					}

					ny = y + radius;
					if ((ny >= ctop) && (ny <= cbottom)) {
						_tabVerticG[ny] = (int16)x1;
						_tabVerticD[ny] = (int16)x2;
					}

					--radius;
					acc -= radius;
				}
			}

			++r;
		}

		vtop = top;
		vbottom = bottom;

		return true;
	}

	return false;
}

uint8 *Renderer::prepareSpheres(const Common::Array<BodySphere> &spheres, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData) {
	for (const BodySphere &sphere : spheres) {
		CmdRenderSphere *cmd = (CmdRenderSphere *)(void*)renderBufferPtr;
		cmd->color = sphere.color;
		cmd->polyRenderType = sphere.fillType;
		cmd->radius = sphere.radius;
		const int16 centerIndex = sphere.vertex;
		cmd->x = modelData->flattenPoints[centerIndex].x;
		cmd->y = modelData->flattenPoints[centerIndex].y;
		cmd->z = modelData->flattenPoints[centerIndex].z;

		(*renderCmds)->depth = modelData->flattenPoints[centerIndex].z;
		(*renderCmds)->renderType = RENDERTYPE_DRAWSPHERE;
		(*renderCmds)->dataPtr = renderBufferPtr;
		(*renderCmds)++;

		renderBufferPtr += sizeof(CmdRenderSphere);
	}
	numOfPrimitives += spheres.size();
	return renderBufferPtr;
}

uint8 *Renderer::prepareLines(const Common::Array<BodyLine> &lines, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData) {
	for (const BodyLine &line : lines) {
		CmdRenderLine *cmd = (CmdRenderLine *)(void*)renderBufferPtr;
		cmd->colorIndex = line.color;
		const int32 point1Index = line.vertex1;
		const int32 point2Index = line.vertex2;
		cmd->x1 = modelData->flattenPoints[point1Index].x;
		cmd->y1 = modelData->flattenPoints[point1Index].y;
		cmd->x2 = modelData->flattenPoints[point2Index].x;
		cmd->y2 = modelData->flattenPoints[point2Index].y;
		(*renderCmds)->depth = MAX(modelData->flattenPoints[point1Index].z, modelData->flattenPoints[point2Index].z);
		(*renderCmds)->renderType = RENDERTYPE_DRAWLINE;
		(*renderCmds)->dataPtr = renderBufferPtr;
		(*renderCmds)++;

		renderBufferPtr += sizeof(CmdRenderLine);
	}
	numOfPrimitives += lines.size();
	return renderBufferPtr;
}

uint8 *Renderer::preparePolygons(const Common::Array<BodyPolygon> &polygons, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData) {
	for (const BodyPolygon &polygon : polygons) {
		const uint8 materialType = polygon.materialType;
		const uint8 numVertices = polygon.indices.size();
		assert(numVertices <= 16);

		int16 zMax = -32000;

		CmdRenderPolygon *destinationPolygon = (CmdRenderPolygon *)(void*)renderBufferPtr;
		destinationPolygon->numVertices = numVertices;

		renderBufferPtr += sizeof(CmdRenderPolygon);

		ComputedVertex *const vertices = (ComputedVertex *)(void*)renderBufferPtr;

		renderBufferPtr += destinationPolygon->numVertices * sizeof(ComputedVertex);

		ComputedVertex *vertex = vertices;

		if (materialType >= MAT_GOURAUD) {
			destinationPolygon->renderType = polygon.materialType - (MAT_GOURAUD - MAT_FLAT);
			destinationPolygon->colorIndex = polygon.intensity;

			for (int16 idx = 0; idx < numVertices; ++idx) {
				const uint16 shadeEntry = polygon.normals[idx];
				const int16 shadeValue = polygon.intensity + modelData->normalTable[shadeEntry];
				const uint16 vertexIndex = polygon.indices[idx];
				const I16Vec3 *point = &modelData->flattenPoints[vertexIndex];

				vertex->intensity = shadeValue;
				vertex->x = point->x;
				vertex->y = point->y;
				zMax = MAX(zMax, point->z);
				++vertex;
			}
		} else {
			if (materialType >= MAT_FLAT) {
				// only 1 shade value is used
				destinationPolygon->renderType = materialType - MAT_FLAT;
				const uint16 normalIndex = polygon.normals[0];
				const int16 shadeValue = polygon.intensity + modelData->normalTable[normalIndex];
				destinationPolygon->colorIndex = shadeValue;
			} else {
				// no shade is used
				destinationPolygon->renderType = materialType;
				destinationPolygon->colorIndex = polygon.intensity;
			}

			for (int16 idx = 0; idx < numVertices; ++idx) {
				const uint16 vertexIndex = polygon.indices[idx];
				const I16Vec3 *point = &modelData->flattenPoints[vertexIndex];

				vertex->intensity = destinationPolygon->colorIndex;
				vertex->x = point->x;
				vertex->y = point->y;
				zMax = MAX<int16>(zMax, point->z);
				++vertex;
			}
		}

		if (!isPolygonVisible(vertices)) {
			renderBufferPtr = (uint8 *)destinationPolygon;
			continue;
		}

		numOfPrimitives++;

		(*renderCmds)->depth = zMax;
		(*renderCmds)->renderType = RENDERTYPE_DRAWPOLYGON;
		(*renderCmds)->dataPtr = (uint8 *)destinationPolygon;
		(*renderCmds)++;
	}

	return renderBufferPtr;
}

const Renderer::RenderCommand *Renderer::depthSortRenderCommands(int32 numOfPrimitives) {
	Common::sort(&_renderCmds[0], &_renderCmds[numOfPrimitives], [](const RenderCommand &lhs, const RenderCommand &rhs) { return lhs.depth > rhs.depth; });
	return _renderCmds;
}

bool Renderer::renderObjectIso(const BodyData &bodyData, RenderCommand **renderCmds, ModelData *modelData, Common::Rect &modelRect) {
	int32 numOfPrimitives = 0;
	uint8 *renderBufferPtr = _renderCoordinatesBuffer;
	renderBufferPtr = preparePolygons(bodyData.getPolygons(), numOfPrimitives, renderCmds, renderBufferPtr, modelData);
	renderBufferPtr = prepareLines(bodyData.getLines(), numOfPrimitives, renderCmds, renderBufferPtr, modelData);
	prepareSpheres(bodyData.getSpheres(), numOfPrimitives, renderCmds, renderBufferPtr, modelData);

	if (numOfPrimitives == 0) {
		return false;
	}
	const RenderCommand *cmds = depthSortRenderCommands(numOfPrimitives);

	int32 primitiveCounter = numOfPrimitives;

	do {
		int16 type = cmds->renderType;
		uint8 *pointer = cmds->dataPtr;

		switch (type) {
		case RENDERTYPE_DRAWLINE: {
			const CmdRenderLine *lineCoords = (const CmdRenderLine *)(const void*)pointer;
			const int32 x1 = lineCoords->x1;
			const int32 y1 = lineCoords->y1;
			const int32 x2 = lineCoords->x2;
			const int32 y2 = lineCoords->y2;
			_engine->_interface->drawLine(x1, y1, x2, y2, lineCoords->colorIndex);
			break;
		}
		case RENDERTYPE_DRAWPOLYGON: {
			const CmdRenderPolygon *header = (const CmdRenderPolygon *)(const void*)pointer;
			ComputedVertex *vertices = (ComputedVertex *)(void*)(pointer + sizeof(CmdRenderPolygon));
			renderPolygons(*header, vertices);
			break;
		}
		case RENDERTYPE_DRAWSPHERE: {
			const CmdRenderSphere *sphere = (const CmdRenderSphere *)(const void*)pointer;
			int32 radius = sphere->radius;

			if (_typeProj == TYPE_ISO) {
				// * sqrt(sx+sy) / 512 (isometric scale)
				radius = (radius * 34) / ISO_SCALE;
			} else {
				int32 delta = _kFactor + sphere->z;
				if (delta == 0) {
					break;
				}
				radius = (sphere->radius * _lFactorX) / delta;
			}

			if (sphere->x + radius > modelRect.right) {
				modelRect.right = sphere->x + radius;
			}

			if (sphere->x - radius < modelRect.left) {
				modelRect.left = sphere->x - radius;
			}

			if (sphere->y + radius > modelRect.bottom) {
				modelRect.bottom = sphere->y + radius;
			}

			if (sphere->y - radius < modelRect.top) {
				modelRect.top = sphere->y - radius;
			}

			int vtop = -1;
			int vbottom = -1;
			if (computeSphere(sphere->x, sphere->y, radius, vtop, vbottom)) {
				fillVertices(vtop, vbottom, sphere->polyRenderType, sphere->color);
			}
			break;
		}
		default:
			break;
		}

		cmds++;
	} while (--primitiveCounter);
	return true;
}

void Renderer::animModel(ModelData *modelData, const BodyData &bodyData, RenderCommand *renderCmds, const IVec3 &angleVec, const IVec3 &poswr, Common::Rect &modelRect) {
	const int32 numVertices = bodyData.getNumVertices();
	const int32 numBones = bodyData.getNumBones();

	const Common::Array<BodyVertex> &vertices = bodyData.getVertices();

	IMatrix3x3 *modelMatrix = &_matricesTable[0];

	const BodyBone &firstBone = bodyData.getBone(0);
	processRotatedElement(modelMatrix, vertices, angleVec.x, angleVec.y, angleVec.z, firstBone, modelData);

	int32 numOfPrimitives = 0;

	if (numBones - 1 != 0) {
		numOfPrimitives = numBones - 1;
		int boneIdx = 1;
		modelMatrix = &_matricesTable[boneIdx];

		do {
			const BodyBone &bone = bodyData.getBone(boneIdx);
			const BoneFrame *boneData = bodyData.getBoneState(boneIdx);

			if (boneData->type == BoneType::TYPE_ROTATE) {
				processRotatedElement(modelMatrix, vertices, boneData->x, boneData->y, boneData->z, bone, modelData);
			} else if (boneData->type == BoneType::TYPE_TRANSLATE) {
				translateGroup(modelMatrix, vertices, boneData->x, boneData->y, boneData->z, bone, modelData);
			} else if (boneData->type == BoneType::TYPE_ZOOM) {
				// unsupported type
			}

			++modelMatrix;
			++boneIdx;
		} while (--numOfPrimitives);
	}

	numOfPrimitives = numVertices;

	const I16Vec3 *pointPtr = &modelData->computedPoints[0];
	I16Vec3 *pointPtrDest = &modelData->flattenPoints[0];

	if (_typeProj == TYPE_ISO) {
		do {
			const int32 coX = pointPtr->x + poswr.x;
			const int32 coY = pointPtr->y + poswr.y;
			const int32 coZ = -(pointPtr->z + poswr.z);

			pointPtrDest->x = (int16)((coX + coZ) * 24 / ISO_SCALE + _projectionCenter.x);
			pointPtrDest->y = (int16)((((coX - coZ) * 12) - coY * 30) / ISO_SCALE + _projectionCenter.y);
			pointPtrDest->z = (int16)(coZ - coX - coY);

			if (pointPtrDest->x < modelRect.left) {
				modelRect.left = pointPtrDest->x;
			}
			if (pointPtrDest->x > modelRect.right) {
				modelRect.right = pointPtrDest->x;
			}

			if (pointPtrDest->y < modelRect.top) {
				modelRect.top = pointPtrDest->y;
			}
			if (pointPtrDest->y > modelRect.bottom) {
				modelRect.bottom = pointPtrDest->y;
			}

			pointPtr++;
			pointPtrDest++;
		} while (--numOfPrimitives);
	} else {
		do {
			int32 coZ = _kFactor - (pointPtr->z + poswr.z);
			if (coZ <= 0) {
				coZ = 0x7FFFFFFF;
			}

			int32 coX = (((pointPtr->x + poswr.x) * _lFactorX) / coZ) + _projectionCenter.x;
			if (coX > 0xFFFF) {
				coX = 0x7FFF;
			}
			pointPtrDest->x = (int16)coX;
			if (pointPtrDest->x < modelRect.left) {
				modelRect.left = pointPtrDest->x;
			}
			if (pointPtrDest->x > modelRect.right) {
				modelRect.right = pointPtrDest->x;
			}

			int32 coY = _projectionCenter.y - (((pointPtr->y + poswr.y) * _lFactorY) / coZ);
			if (coY > 0xFFFF) {
				coY = 0x7FFF;
			}
			pointPtrDest->y = (int16)coY;
			if (pointPtrDest->y < modelRect.top) {
				modelRect.top = pointPtrDest->y;
			}
			if (pointPtrDest->y > modelRect.bottom) {
				modelRect.bottom = pointPtrDest->y;
			}

			if (coZ > 0xFFFF) {
				coZ = 0x7FFF;
			}
			pointPtrDest->z = (int16)coZ;

			pointPtr++;
			pointPtrDest++;

		} while (--numOfPrimitives);
	}

	int32 numNormals = (int32)bodyData.getNormals().size();

	if (numNormals) { // process normal data
		uint16 *currentShadeDestination = (uint16 *)modelData->normalTable;
		IMatrix3x3 *lightMatrix = &_matricesTable[0];

		numOfPrimitives = numBones;

		int16 shadeIndex = 0;
		int16 boneIdx = 0;
		do { // for each element
			numNormals = bodyData.getBone(boneIdx).numNormals;

			if (numNormals) {
				const IMatrix3x3 matrix = *lightMatrix * _normalLight;

				for (int32 i = 0; i < numNormals; ++i) { // for each normal
					const BodyNormal &normalPtr = bodyData.getNormal(shadeIndex);

					const int32 x = (int32)normalPtr.x;
					const int32 y = (int32)normalPtr.y;
					const int32 z = (int32)normalPtr.z;

					int32 intensity = 0;
					intensity += matrix.row1.x * x + matrix.row1.y * y + matrix.row1.z * z;
					intensity += matrix.row2.x * x + matrix.row2.y * y + matrix.row2.z * z;
					intensity += matrix.row3.x * x + matrix.row3.y * y + matrix.row3.z * z;

					if (intensity > 0) {
						intensity >>= 14;
						intensity /= normalPtr.prenormalizedRange;
					} else {
						intensity = 0;
					}

					*currentShadeDestination++ = (uint16)intensity;
					++shadeIndex;
				};
			}

			++boneIdx;
			++lightMatrix;
		} while (--numOfPrimitives);
	}
}

bool Renderer::affObjetIso(int32 x, int32 y, int32 z, int32 alpha, int32 beta, int32 gamma, const BodyData &bodyData, Common::Rect &modelRect) {
	IVec3 renderAngle;
	renderAngle.x = alpha;
	renderAngle.y = beta;
	renderAngle.z = gamma;

	// model render size reset
	modelRect.left = SCENE_SIZE_MAX;
	modelRect.top = SCENE_SIZE_MAX;
	modelRect.right = SCENE_SIZE_MIN;
	modelRect.bottom = SCENE_SIZE_MIN;

	IVec3 poswr; // PosXWr, PosYWr, PosZWr
	if (_typeProj == TYPE_3D) {
		poswr = longWorldRot(x, y, z) - _cameraRot;
	} else {
		poswr.x = x;
		poswr.y = y;
		poswr.z = z;
	}

	if (!bodyData.isAnimated()) {
#if 0
		// TODO: fill modeldata.flattenedpoints
		// not used in original source release
		int32 numOfPrimitives = 0;
		RenderCommand* renderCmds = _renderCmds;
		return renderModelElements(numOfPrimitives, bodyData, &renderCmds, &_modelData, modelRect);
#else
		error("Unsupported unanimated model render for model index %i!", bodyData.hqrIndex());
#endif
	}
	// restart at the beginning of the renderTable
	RenderCommand *renderCmds = _renderCmds;
	if (bodyData.isAnimated()) {
		animModel(&_modelData, bodyData, renderCmds, renderAngle, poswr, modelRect);
	}
	if (!renderObjectIso(bodyData, &renderCmds, &_modelData, modelRect)) {
		modelRect.right = -1;
		modelRect.bottom = -1;
		modelRect.left = -1;
		modelRect.top = -1;
		return false;
	}
	return true;
}

void Renderer::drawObj3D(const Common::Rect &rect, int32 y, int32 angle, const BodyData &bodyData, ActorMoveStruct &move) {
	int32 boxLeft = rect.left;
	int32 boxTop = rect.top;
	int32 boxRight = rect.right;
	int32 boxBottom = rect.bottom;
	const int32 ypos = (boxBottom + boxTop) / 2;
	const int32 xpos = (boxRight + boxLeft) / 2;

	setIsoProjection(xpos, ypos, 0);
	_engine->_interface->setClip(rect);

	Common::Rect dummy;
	if (angle == -1) {
		angle = move.getRealAngle(_engine->timerRef);
		if (move.timeValue == 0) {
			_engine->_movements->initRealAngle(angle, angle - LBAAngles::ANGLE_90, LBAAngles::ANGLE_17, &move);
		}
	}
	affObjetIso(0, y, 0, LBAAngles::ANGLE_0, angle, LBAAngles::ANGLE_0, bodyData, dummy);
}

void Renderer::draw3dObject(int32 x, int32 y, const BodyData &bodyData, int32 angle, int32 cameraZoom) {
	setProjection(x, y, 128, 200, 200);
	setFollowCamera(0, 0, 0, 60, 0, 0, cameraZoom);

	Common::Rect dummy;
	affObjetIso(0, 0, 0, LBAAngles::ANGLE_0, angle, LBAAngles::ANGLE_0, bodyData, dummy);
}

void Renderer::fillHolomapTriangle(int16 *pDest, int32 x0, int32 y0, int32 x1, int32 y1) {
	uint32 dx, step, reminder;
	if (y0 > y1) {
		SWAP(x0, x1);
		SWAP(y0, y1);
	}

	y1 -= y0;
	pDest += y0;

	if (x0 <= x1) {
		dx = (x1 - x0) << 16;

		step = dx / y1;
		reminder = ((dx % y1) >> 1) + 0x7FFF;

		x1 = step >> 16;
		step &= 0xFFFF;

		for (; y1 >= 0; --y1) {
			*pDest++ = (int16)x0;
			x0 += x1;
			if (reminder & 0xFFFF0000) {
				x0 += reminder >> 16;
				reminder &= 0xFFFF;
			}
			reminder += step;
		}
	} else {
		dx = (x0 - x1) << 16;

		step = dx / y1;
		reminder = ((dx % y1) >> 1) + 0x7FFF;

		x1 = step >> 16;
		step &= 0xFFFF;

		for (; y1 >= 0; --y1) {
			*pDest++ = (int16)x0;
			x0 -= x1;
			if (reminder & 0xFFFF0000) {
				x0 += reminder >> 16;
				reminder &= 0xFFFF;
			}
			reminder -= step;
		}
	}
}

void Renderer::fillHolomapTriangles(const ComputedVertex &vertex0, const ComputedVertex &vertex1, const ComputedVertex &texCoord0, const ComputedVertex &texCoord1, int32 &lymin, int32 &lymax) {
	const int32 y0 = vertex0.y;
	const int32 y1 = vertex1.y;

	if (y0 < y1) {
		if (y0 < lymin) {
			lymin = y0;
		}
		if (y1 > lymax) {
			lymax = y1;
		}
		fillHolomapTriangle(_tabVerticG, vertex0.x, y0, vertex1.x, y1);
		fillHolomapTriangle(_tabx0, (int32)(uint16)texCoord0.x, y0, (int32)(uint16)texCoord1.x, y1);
		fillHolomapTriangle(_taby0, (int32)(uint16)texCoord0.y, y0, (int32)(uint16)texCoord1.y, y1);
	} else if (y0 > y1) {
		if (y0 > lymax) {
			lymax = y0;
		}
		if (y1 < lymin) {
			lymin = y1;
		}
		fillHolomapTriangle(_tabVerticD, vertex0.x, y0, vertex1.x, y1);
		fillHolomapTriangle(_tabx1, (int32)(uint16)texCoord0.x, y0, (int32)(uint16)texCoord1.x, y1);
		fillHolomapTriangle(_taby1, (int32)(uint16)texCoord0.y, y0, (int32)(uint16)texCoord1.y, y1);
	}
}

void Renderer::asmTexturedTriangleNoClip(const ComputedVertex vertexCoordinates[3], const ComputedVertex textureCoordinates[3], const uint8 *holomapImage, uint32 holomapImageSize) {
	int32 lymin = SCENE_SIZE_MAX;
	int32 lymax = SCENE_SIZE_MIN;
	fillHolomapTriangles(vertexCoordinates[0], vertexCoordinates[1], textureCoordinates[0], textureCoordinates[1], lymin, lymax);
	fillHolomapTriangles(vertexCoordinates[1], vertexCoordinates[2], textureCoordinates[1], textureCoordinates[2], lymin, lymax);
	fillHolomapTriangles(vertexCoordinates[2], vertexCoordinates[0], textureCoordinates[2], textureCoordinates[0], lymin, lymax);
	fillTextPolyNoClip(lymin, lymax, holomapImage, holomapImageSize);
}

void Renderer::fillTextPolyNoClip(int32 yMin, int32 yMax, const uint8 *holomapImage, uint32 holomapImageSize) {
	if (yMin < 0 || yMin >= _engine->_frontVideoBuffer.h) {
		return;
	}
	const int screenWidth = _engine->width();
	uint8 *pDestLine = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, yMin);

	const int16 *pVerticG = &_tabVerticG[yMin];
	const int16 *pVerticD = &_tabVerticD[yMin];
	const uint16 *pU0 = (const uint16 *)&_tabx0[yMin];
	const uint16 *pV0 = (const uint16 *)&_taby0[yMin];
	const uint16 *pU1 = (const uint16 *)&_tabx1[yMin];
	const uint16 *pV1 = (const uint16 *)&_taby1[yMin];
	byte *pDest;
	int32 ustep, vstep;
	int16 xMin, xMax;
	uint32 u0, v0, u1, v1, idx;
	int32 u, v;

	yMax -= yMin;

	for (; yMax >= 0; yMax--) {
		xMin = *pVerticG++;
		xMax = *pVerticD++;
		xMax -= xMin;

		u = u0 = *pU0++;
		v = v0 = *pV0++;
		u1 = *pU1++;
		v1 = *pV1++;

		if (xMax > 0) {
			pDest = pDestLine + xMin;

			ustep = ((int32)u1 - (int32)u0 + 1) / xMax;
			vstep = ((int32)v1 - (int32)v0 + 1) / xMax;

			for (; xMax > 0; xMax--) {
				idx = ((u >> 8) & 0xFF) | (v & 0xFF00); // u0&0xFF00=column*256, v0&0xFF00 = line*256
				*pDest++ = holomapImage[idx];

				u += ustep;
				v += vstep;
			}
		}

		pDestLine += screenWidth;
	}
}

} // namespace TwinE
