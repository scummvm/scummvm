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
	free(_polyTab);
	free(_colorProgressionBuffer);
}

void Renderer::init(int32 w, int32 h) {
	_polyTabSize = _engine->height() * 6;
	_polyTab = (int16 *)malloc(_polyTabSize * sizeof(int16));
	_colorProgressionBuffer = (int16 *)malloc(_polyTabSize * sizeof(int16));
	_tabVerticG = &_polyTab[_engine->height() * 0];
	_tabVerticD = &_polyTab[_engine->height() * 1];
	_tabx0 = &_polyTab[_engine->height() * 2];
	_tabx1 = &_polyTab[_engine->height() * 3];
	_taby0 = &_polyTab[_engine->height() * 4];
	_taby1 = &_polyTab[_engine->height() * 5];
}

IVec3 &Renderer::projectPositionOnScreen(int32 cX, int32 cY, int32 cZ) { // ProjettePoint
	if (_isUsingIsoProjection) {
		_projPos.x = ((cX - cZ) * 24) / ISO_SCALE + _projectionCenter.x;
		_projPos.y = (((cX + cZ) * 12) - cY * 30) / ISO_SCALE + _projectionCenter.y;
		_projPos.z = cZ - cY - cX;
		return _projPos;
	}

	if (_baseRotPos.z - cZ < 0) {
		_projPos.x = 0;
		_projPos.y = 0;
		_projPos.z = 0;
		return _projPos;
	}

	cX -= _baseRotPos.x;
	cY -= _baseRotPos.y;
	cZ = _baseRotPos.z - cZ;

	int32 posZ = cZ + _kFactor;
	if (posZ <= 0) {
		posZ = 0x7FFF;
	}

	_projPos.x = (cX * _lFactorX) / posZ + _projectionCenter.x;
	_projPos.y = (-cY * _lFactorY) / posZ + _projectionCenter.y;
	_projPos.z = posZ;
	return _projPos;
}

void Renderer::setProjection(int32 x, int32 y, int32 kfact, int32 lfactx, int32 lfacty) {
	_projectionCenter.x = x;
	_projectionCenter.y = y;

	_kFactor = kfact;
	_lFactorX = lfactx;
	_lFactorY = lfacty;

	_isUsingIsoProjection = false;
}

void Renderer::setBaseTranslation(int32 x, int32 y, int32 z) {
	_baseTransPos.x = x;
	_baseTransPos.y = y;
	_baseTransPos.z = z;
}

void Renderer::setIsoProjection(int32 x, int32 y, int32 scale) {
	_projectionCenter.x = x;
	_projectionCenter.y = y;
	_projectionCenter.z = scale; // not used - IsoScale is always 512

	_isUsingIsoProjection = true;
}

void Renderer::baseMatrixTranspose() {
	SWAP(_baseMatrix.row1.y, _baseMatrix.row2.x);
	SWAP(_baseMatrix.row1.z, _baseMatrix.row3.x);
	SWAP(_baseMatrix.row2.z, _baseMatrix.row3.y);
}

IVec3 Renderer::setAngleCamera(int32 x, int32 y, int32 z, bool transpose) {
	const double Xradians = (double)((LBAAngles::ANGLE_90 - x) % LBAAngles::ANGLE_360) * 2 * M_PI / LBAAngles::ANGLE_360;
	const double Yradians = (double)((LBAAngles::ANGLE_90 - y) % LBAAngles::ANGLE_360) * 2 * M_PI / LBAAngles::ANGLE_360;
	const double Zradians = (double)((LBAAngles::ANGLE_90 - z) % LBAAngles::ANGLE_360) * 2 * M_PI / LBAAngles::ANGLE_360;

	_baseMatrix.row1.x = (int32)(sin(Zradians) * sin(Yradians) * SCENE_SIZE_HALFF);
	_baseMatrix.row1.y = (int32)(-cos(Zradians) * SCENE_SIZE_HALFF);
	_baseMatrix.row1.z = (int32)(sin(Zradians) * cos(Yradians) * SCENE_SIZE_HALFF);
	_baseMatrix.row2.x = (int32)(cos(Zradians) * sin(Xradians) * SCENE_SIZE_HALFF);
	_baseMatrix.row2.y = (int32)(sin(Zradians) * sin(Xradians) * SCENE_SIZE_HALFF);
	_baseMatrix.row3.x = (int32)(cos(Zradians) * cos(Xradians) * SCENE_SIZE_HALFF);
	_baseMatrix.row3.y = (int32)(sin(Zradians) * cos(Xradians) * SCENE_SIZE_HALFF);

	int32 matrixElem = _baseMatrix.row2.x;

	_baseMatrix.row2.x = (int32)(sin(Yradians) * matrixElem + SCENE_SIZE_HALFF * cos(Yradians) * cos(Xradians));
	_baseMatrix.row2.z = (int32)(cos(Yradians) * matrixElem - SCENE_SIZE_HALFF * sin(Yradians) * cos(Xradians));

	matrixElem = _baseMatrix.row3.x;

	_baseMatrix.row3.x = (int32)(sin(Yradians) * matrixElem - SCENE_SIZE_HALFF * sin(Xradians) * cos(Yradians));
	_baseMatrix.row3.z = (int32)(cos(Yradians) * matrixElem + SCENE_SIZE_HALFF * sin(Xradians) * sin(Yradians));

	if (transpose) {
		baseMatrixTranspose();
	}
	_baseRotPos = getBaseRotationPosition(_baseTransPos.x, _baseTransPos.y, _baseTransPos.z);

	return _baseRotPos;
}

IVec3 Renderer::getBaseRotationPosition(int32 x, int32 y, int32 z) {
	const int32 vx = (_baseMatrix.row1.x * x + _baseMatrix.row1.y * y + _baseMatrix.row1.z * z) / SCENE_SIZE_HALF;
	const int32 vy = (_baseMatrix.row2.x * x + _baseMatrix.row2.y * y + _baseMatrix.row2.z * z) / SCENE_SIZE_HALF;
	const int32 vz = (_baseMatrix.row3.x * x + _baseMatrix.row3.y * y + _baseMatrix.row3.z * z) / SCENE_SIZE_HALF;
	return IVec3(vx, vy, vz);
}

IVec3 Renderer::getCameraAnglePositions(int32 x, int32 y, int32 z) {
	const int32 vx = (_baseMatrix.row1.x * x + _baseMatrix.row2.x * y + _baseMatrix.row3.x * z) / SCENE_SIZE_HALF;
	const int32 vy = (_baseMatrix.row1.y * x + _baseMatrix.row2.y * y + _baseMatrix.row3.y * z) / SCENE_SIZE_HALF;
	const int32 vz = (_baseMatrix.row1.z * x + _baseMatrix.row2.z * y + _baseMatrix.row3.z * z) / SCENE_SIZE_HALF;
	return IVec3(vx, vy, vz);
}

IVec3 Renderer::translateGroup(const IMatrix3x3 &matrix, int32 x, int32 y, int32 z) {
	const int32 vx = (matrix.row1.x * x + matrix.row1.y * y + matrix.row1.z * z) / SCENE_SIZE_HALF;
	const int32 vy = (matrix.row2.x * x + matrix.row2.y * y + matrix.row2.z * z) / SCENE_SIZE_HALF;
	const int32 vz = (matrix.row3.x * x + matrix.row3.y * y + matrix.row3.z * z) / SCENE_SIZE_HALF;
	return IVec3(vx, vy, vz);
}

void Renderer::setCameraAngle(int32 transPosX, int32 transPosY, int32 transPosZ, int32 rotPosX, int32 rotPosY, int32 rotPosZ, int32 param6) {
	_baseTransPos.x = transPosX;
	_baseTransPos.y = transPosY;
	_baseTransPos.z = transPosZ;

	setAngleCamera(rotPosX, rotPosY, rotPosZ);

	_baseRotPos.z += param6;

	_baseTransPos = updateCameraAnglePositions();
}

IVec3 Renderer::updateCameraAnglePositions(int zShift) {
	return getCameraAnglePositions(_baseRotPos.x, _baseRotPos.y, _baseRotPos.z + zShift);
}

IVec3 Renderer::getHolomapRotation(const int32 angleX, const int32 angleY, const int32 angleZ) const {
	int32 rotX = angleX * 2 + 1000;

	int32 rotY;
	if (angleY == LBAAngles::ANGLE_0) {
		rotY = LBAAngles::ANGLE_0;
	} else {
		rotY = -lba1ShadeAngleTable[ClampAngle(angleY)] * rotX / SCENE_SIZE_HALF;
		rotX = lba1ShadeAngleTable[ClampAngle(angleY + LBAAngles::ANGLE_90)] * rotX / SCENE_SIZE_HALF;
	}

	int32 rotZ;
	if (angleZ == LBAAngles::ANGLE_0) {
		rotZ = LBAAngles::ANGLE_0;
	} else {
		rotZ = -lba1ShadeAngleTable[ClampAngle(angleZ)] * rotX / SCENE_SIZE_HALF;
		rotX = lba1ShadeAngleTable[ClampAngle(angleZ + LBAAngles::ANGLE_90)] * rotX / SCENE_SIZE_HALF;
	}

	const int32 row1X = _baseMatrix.row1.x * rotX;
	const int32 row1Y = _baseMatrix.row1.y * rotY;
	const int32 row1Z = _baseMatrix.row1.z * rotZ;
	const int32 row2X = _baseMatrix.row2.x * rotX;
	const int32 row2Y = _baseMatrix.row2.y * rotY;
	const int32 row2Z = _baseMatrix.row2.z * rotZ;
	const int32 row3X = _baseMatrix.row3.x * rotX;
	const int32 row3Y = _baseMatrix.row3.y * rotY;
	const int32 row3Z = _baseMatrix.row3.z * rotZ;
	IVec3 vec;
	vec.x = (row1X + row1Y + row1Z) / SCENE_SIZE_HALF;
	vec.y = (row2X + row2Y + row2Z) / SCENE_SIZE_HALF;
	vec.z = (row3X + row3Y + row3Z) / SCENE_SIZE_HALF;
	return vec;
}

void Renderer::applyRotation(IMatrix3x3 *targetMatrix, const IMatrix3x3 *currentMatrix, const IVec3 &angleVec) {
	IMatrix3x3 matrix1;
	IMatrix3x3 matrix2;

	if (angleVec.x) {
		int32 angle = angleVec.x;
		int32 angleVar2 = lba1ShadeAngleTable[ClampAngle(angle)];
		angle += LBAAngles::ANGLE_90;
		int32 angleVar1 = lba1ShadeAngleTable[ClampAngle(angle)];

		matrix1.row1.x = currentMatrix->row1.x;
		matrix1.row2.x = currentMatrix->row2.x;
		matrix1.row3.x = currentMatrix->row3.x;

		matrix1.row1.y = (currentMatrix->row1.z * angleVar2 + currentMatrix->row1.y * angleVar1) / SCENE_SIZE_HALF;
		matrix1.row1.z = (currentMatrix->row1.z * angleVar1 - currentMatrix->row1.y * angleVar2) / SCENE_SIZE_HALF;
		matrix1.row2.y = (currentMatrix->row2.z * angleVar2 + currentMatrix->row2.y * angleVar1) / SCENE_SIZE_HALF;
		matrix1.row2.z = (currentMatrix->row2.z * angleVar1 - currentMatrix->row2.y * angleVar2) / SCENE_SIZE_HALF;
		matrix1.row3.y = (currentMatrix->row3.z * angleVar2 + currentMatrix->row3.y * angleVar1) / SCENE_SIZE_HALF;
		matrix1.row3.z = (currentMatrix->row3.z * angleVar1 - currentMatrix->row3.y * angleVar2) / SCENE_SIZE_HALF;
	} else {
		matrix1 = *currentMatrix;
	}

	if (angleVec.z) {
		int32 angle = angleVec.z;
		int32 angleVar2 = lba1ShadeAngleTable[ClampAngle(angle)];
		angle += LBAAngles::ANGLE_90;
		int32 angleVar1 = lba1ShadeAngleTable[ClampAngle(angle)];

		matrix2.row1.z = matrix1.row1.z;
		matrix2.row2.z = matrix1.row2.z;
		matrix2.row3.z = matrix1.row3.z;

		matrix2.row1.x = (matrix1.row1.y * angleVar2 + matrix1.row1.x * angleVar1) / SCENE_SIZE_HALF;
		matrix2.row1.y = (matrix1.row1.y * angleVar1 - matrix1.row1.x * angleVar2) / SCENE_SIZE_HALF;
		matrix2.row2.x = (matrix1.row2.y * angleVar2 + matrix1.row2.x * angleVar1) / SCENE_SIZE_HALF;
		matrix2.row2.y = (matrix1.row2.y * angleVar1 - matrix1.row2.x * angleVar2) / SCENE_SIZE_HALF;
		matrix2.row3.x = (matrix1.row3.y * angleVar2 + matrix1.row3.x * angleVar1) / SCENE_SIZE_HALF;
		matrix2.row3.y = (matrix1.row3.y * angleVar1 - matrix1.row3.x * angleVar2) / SCENE_SIZE_HALF;
	} else {
		matrix2 = matrix1;
	}

	if (angleVec.y) {
		int32 angle = angleVec.y;
		int32 angleVar2 = lba1ShadeAngleTable[ClampAngle(angle)];
		angle += LBAAngles::ANGLE_90;
		int32 angleVar1 = lba1ShadeAngleTable[ClampAngle(angle)];

		targetMatrix->row1.y = matrix2.row1.y;
		targetMatrix->row2.y = matrix2.row2.y;
		targetMatrix->row3.y = matrix2.row3.y;

		targetMatrix->row1.x = (matrix2.row1.x * angleVar1 - matrix2.row1.z * angleVar2) / SCENE_SIZE_HALF;
		targetMatrix->row1.z = (matrix2.row1.x * angleVar2 + matrix2.row1.z * angleVar1) / SCENE_SIZE_HALF;
		targetMatrix->row2.x = (matrix2.row2.x * angleVar1 - matrix2.row2.z * angleVar2) / SCENE_SIZE_HALF;
		targetMatrix->row2.z = (matrix2.row2.x * angleVar2 + matrix2.row2.z * angleVar1) / SCENE_SIZE_HALF;

		targetMatrix->row3.x = (matrix2.row3.x * angleVar1 - matrix2.row3.z * angleVar2) / SCENE_SIZE_HALF;
		targetMatrix->row3.z = (matrix2.row3.x * angleVar2 + matrix2.row3.z * angleVar1) / SCENE_SIZE_HALF;
	} else {
		*targetMatrix = matrix2;
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

void Renderer::applyPointsRotation(const Common::Array<BodyVertex> &vertices, int32 firstPoint, int32 numPoints, I16Vec3 *destPoints, const IMatrix3x3 *rotationMatrix, const IVec3 &destPos) {
	for (int32 i = 0; i < numPoints; ++i) {
		const BodyVertex &vertex = vertices[i + firstPoint];
		destPoints->x = (int16)((rotationMatrix->row1.x * vertex.x + rotationMatrix->row1.y * vertex.y + rotationMatrix->row1.z * vertex.z) / SCENE_SIZE_HALF) + destPos.x;
		destPoints->y = (int16)((rotationMatrix->row2.x * vertex.x + rotationMatrix->row2.y * vertex.y + rotationMatrix->row2.z * vertex.z) / SCENE_SIZE_HALF) + destPos.y;
		destPoints->z = (int16)((rotationMatrix->row3.x * vertex.x + rotationMatrix->row3.y * vertex.y + rotationMatrix->row3.z * vertex.z) / SCENE_SIZE_HALF) + destPos.z;

		destPoints++;
	}
}

void Renderer::processRotatedElement(IMatrix3x3 *targetMatrix, const Common::Array<BodyVertex> &vertices, int32 rotX, int32 rotY, int32 rotZ, const BodyBone &bone, ModelData *modelData) {
	const int32 firstPoint = bone.firstVertex;
	const int32 numOfPoints = bone.numVertices;
	const IVec3 renderAngle(rotX, rotY, rotZ);

	const IMatrix3x3 *currentMatrix;
	IVec3 destPos;
	// if its the first point
	if (bone.isRoot()) {
		currentMatrix = &_baseMatrix;
	} else {
		const int32 pointIdx = bone.vertex;
		const int32 matrixIndex = bone.parent;
		assert(matrixIndex >= 0 && matrixIndex < ARRAYSIZE(_matricesTable));
		currentMatrix = &_matricesTable[matrixIndex];

		destPos = modelData->computedPoints[pointIdx];
	}

	applyRotation(targetMatrix, currentMatrix, renderAngle);

	if (!numOfPoints) {
		warning("RENDER WARNING: No points in this model!");
	}

	applyPointsRotation(vertices, firstPoint, numOfPoints, &modelData->computedPoints[firstPoint], targetMatrix, destPos);
}

void Renderer::applyPointsTranslation(const Common::Array<BodyVertex> &vertices, int32 firstPoint, int32 numPoints, I16Vec3 *destPoints, const IMatrix3x3 *translationMatrix, const IVec3 &angleVec, const IVec3 &destPos) {
	for (int32 i = 0; i < numPoints; ++i) {
		const BodyVertex &vertex = vertices[i + firstPoint];
		const int32 tmpX = vertex.x + angleVec.x;
		const int32 tmpY = vertex.y + angleVec.y;
		const int32 tmpZ = vertex.z + angleVec.z;

		destPoints->x = ((translationMatrix->row1.x * tmpX + translationMatrix->row1.y * tmpY + translationMatrix->row1.z * tmpZ) / SCENE_SIZE_HALF) + destPos.x;
		destPoints->y = ((translationMatrix->row2.x * tmpX + translationMatrix->row2.y * tmpY + translationMatrix->row2.z * tmpZ) / SCENE_SIZE_HALF) + destPos.y;
		destPoints->z = ((translationMatrix->row3.x * tmpX + translationMatrix->row3.y * tmpY + translationMatrix->row3.z * tmpZ) / SCENE_SIZE_HALF) + destPos.z;

		destPoints++;
	}
}

void Renderer::processTranslatedElement(IMatrix3x3 *targetMatrix, const Common::Array<BodyVertex> &vertices, int32 rotX, int32 rotY, int32 rotZ, const BodyBone &bone, ModelData *modelData) {
	IVec3 renderAngle;
	renderAngle.x = rotX;
	renderAngle.y = rotY;
	renderAngle.z = rotZ;

	IVec3 destPos;

	if (bone.isRoot()) { // base point
		*targetMatrix = _baseMatrix;
	} else { // dependent
		const int32 pointsIdx = bone.vertex;
		destPos = modelData->computedPoints[pointsIdx];

		const int32 matrixIndex = bone.parent;
		assert(matrixIndex >= 0 && matrixIndex < ARRAYSIZE(_matricesTable));
		*targetMatrix = _matricesTable[matrixIndex];
	}

	applyPointsTranslation(vertices, bone.firstVertex, bone.numVertices, &modelData->computedPoints[bone.firstVertex], targetMatrix, renderAngle, destPos);
}

void Renderer::setLightVector(int32 angleX, int32 angleY, int32 angleZ) {
	// TODO: RECHECK THIS
	/*_cameraAngleX = angleX;
	_cameraAngleY = angleY;
	_cameraAngleZ = angleZ;*/
	const int32 normalUnit = 64;
	const IVec3 renderAngle(angleX, angleY, angleZ);
	IMatrix3x3 matrix;
	applyRotation(&matrix, &_baseMatrix, renderAngle);
	_lightNorm = translateGroup(matrix, 0, 0, normalUnit - 5);
}

static FORCEINLINE int16 clamp(int16 x, int16 a, int16 b) {
	return x < a ? a : (x > b ? b : x);
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

int32 Renderer::computePolyMinMax(int16 polyRenderType, ComputedVertex **offTabPoly, int32 numVertices) {
	const Common::Rect &clip = _engine->_interface->_clip;
	if (clip.isEmpty()) {
		return numVertices;
	}

	int32 minsx = SCENE_SIZE_MAX;
	int32 maxsx = SCENE_SIZE_MIN;
	int32 minsy = SCENE_SIZE_MAX;
	int32 maxsy = SCENE_SIZE_MIN;

	ComputedVertex* pTabPoly = offTabPoly[0];
	for (int32 i = 0; i < numVertices; i++) {
		if (pTabPoly[i].x < minsx) {
			minsx = pTabPoly[i].x;
		}
		if (pTabPoly[i].x > maxsx) {
			maxsx = pTabPoly[i].x;
		}
		if (pTabPoly[i].y < minsy) {
			minsy = pTabPoly[i].y;
		}
		if (pTabPoly[i].y > maxsy) {
			maxsy = pTabPoly[i].y;
		}
	}

	// no vertices
	if (minsy > maxsy || maxsx < clip.left || minsx > clip.right || maxsy < clip.top || minsy > clip.bottom) {
		debug(10, "Clipped %i:%i:%i:%i, clip rect(%i:%i:%i:%i)", minsx, minsy, maxsx, maxsy, clip.left, clip.top, clip.right, clip.bottom);
		return 0;
	}

	pTabPoly[numVertices] = *offTabPoly[0];

	bool hasBeenClipped = false;

	int32 clippedNumVertices = numVertices;
	if (minsx < clip.left) {
		clippedNumVertices = leftClip(polyRenderType, offTabPoly, clippedNumVertices);
		if (!clippedNumVertices) {
			return 0;
		}

		hasBeenClipped = true;
	}

	if (maxsx > clip.right) {
		clippedNumVertices = rightClip(polyRenderType, offTabPoly, clippedNumVertices);
		if (!clippedNumVertices) {
			return 0;
		}

		hasBeenClipped = true;
	}

	if (minsy < clip.top) {
		clippedNumVertices = topClip(polyRenderType, offTabPoly, clippedNumVertices);
		if (!clippedNumVertices) {
			return 0;
		}

		hasBeenClipped = true;
	}

	if (maxsy > clip.bottom) {
		clippedNumVertices = bottomClip(polyRenderType, offTabPoly, clippedNumVertices);
		if (!clippedNumVertices) {
			return 0;
		}

		hasBeenClipped = true;
	}

	if (hasBeenClipped) {
		minsy = 32767;
		maxsy = -32768;

		for (int32 i = 0; i < clippedNumVertices; i++) {
			if (offTabPoly[0][i].y < minsy) {
				minsy = offTabPoly[0][i].y;
			}

			if (offTabPoly[0][i].y > maxsy) {
				maxsy = offTabPoly[0][i].y;
			}
		}

		if (minsy >= maxsy) {
			return 0;
		}
	}

	return clippedNumVertices;
}

bool Renderer::computePoly(int16 polyRenderType, const ComputedVertex *vertices, int32 numVertices) {
	const int16 *polyTabBegin = _polyTab;
	const int16 *polyTabEnd = &_polyTab[_polyTabSize - 1];
	const int16 *colProgressBufStart = _colorProgressionBuffer;
	const int16 *colProgressBufEnd = &_colorProgressionBuffer[_polyTabSize - 1];
	const int screenHeight = _engine->height();

	assert(numVertices < ARRAYSIZE(_clippedPolygonVertices1));
	for (int i = 0; i < numVertices; ++i) {
		_clippedPolygonVertices1[i] = vertices[i];
	}

	ComputedVertex *offTabPoly[] = {_clippedPolygonVertices1, _clippedPolygonVertices2};

	numVertices = computePolyMinMax(polyRenderType, offTabPoly, numVertices);
	if (numVertices == 0) {
		return false;
	}

	const ComputedVertex *clippedVertices = offTabPoly[0];
	uint8 vertexParam1 = clippedVertices[numVertices - 1].intensity;
	int16 currentVertexX = clippedVertices[numVertices - 1].x;
	int16 currentVertexY = clippedVertices[numVertices - 1].y;

	for (int32 nVertex = 0; nVertex < numVertices; nVertex++) {
		const int16 oldVertexY = currentVertexY;
		const int16 oldVertexX = currentVertexX;
		const uint8 oldVertexParam = vertexParam1;

		vertexParam1 = clippedVertices[nVertex].intensity;
		const uint8 vertexParam2 = vertexParam1;
		currentVertexX = clippedVertices[nVertex].x;
		currentVertexY = clippedVertices[nVertex].y;

		// drawLine(oldVertexX,oldVertexY,currentVertexX,currentVertexY,255);

		if (currentVertexY == oldVertexY) {
			continue;
		}

		const int8 up = currentVertexY < oldVertexY;
		int8 direction = up ? -1 : 1;

		const int16 vsize = ABS(currentVertexY - oldVertexY);
		const int16 hsize = ABS(currentVertexX - oldVertexX);

		int16 cvalue;
		int16 cdelta;
		int16 ypos;
		float xpos;
		if (direction * oldVertexX > direction * currentVertexX) { // if we are going up right
			xpos = currentVertexX;
			ypos = currentVertexY;
			cvalue = (vertexParam2 * 256) + ((oldVertexParam - vertexParam2) * 256) % vsize;
			cdelta = ((oldVertexParam - vertexParam2) * 256) / vsize;
			direction = -direction; // we will draw by going down the tab
		} else {
			xpos = oldVertexX;
			ypos = oldVertexY;
			cvalue = (oldVertexParam * 256) + ((vertexParam2 - oldVertexParam) * 256) % vsize;
			cdelta = ((vertexParam2 - oldVertexParam) * 256) / vsize;
		}
		const int32 polyTabIndex = ypos + (up ? screenHeight : 0);
		int16 *outPtr = &_polyTab[polyTabIndex]; // outPtr is the output ptr in the renderTab

		float slope = (float)hsize / (float)vsize;
		slope = up ? -slope : slope;

		for (int16 i = 0; i <= vsize; i++) {
			if (outPtr >= polyTabBegin && outPtr <= polyTabEnd) {
				*outPtr = xpos;
			}
			outPtr += direction;
			xpos += slope;
		}

		if (polyRenderType >= POLYGONTYPE_GOURAUD) { // we must compute the color progression
			int16 *outPtr2 = &_colorProgressionBuffer[polyTabIndex];

			for (int16 i = 0; i <= vsize; i++) {
				if (outPtr2 >= colProgressBufStart && outPtr2 <= colProgressBufEnd) {
					*outPtr2 = cvalue;
				}
				outPtr2 += direction;
				cvalue += cdelta;
			}
		}
	}
	return true;
}

void Renderer::svgaPolyCopper(int vtop, int32 vsize, uint16 color) const {
	uint8 *out = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	const int16 *ptr1 = &_polyTab[vtop];
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	int32 renderLoop = vsize;
	if (vtop < 0) {
		out += screenWidth * ABS(vtop);
		renderLoop -= ABS(vtop);
	}
	if (renderLoop > screenHeight) {
		renderLoop = screenHeight;
	}
	int32 sens = 1;

	for (int32 currentLine = 0; currentLine < renderLoop; ++currentLine) {
		int16 xMin = ptr1[0];
		int16 xMax = ptr1[screenHeight];

		ptr1++;
		uint8 *pDest = out + xMin;

		for (; xMin <= xMax; xMin++) {
			*pDest++ = (uint8)color;
		}

		color += sens;
		if (!(color & 0xF)) {
			sens = -sens;
			if (sens < 0) {
				color += sens;
			}
		}
		out += screenWidth;
	}
}

void Renderer::svgaPolyBopper(int vtop, int32 vsize, uint16 color) const {
	uint8 *out = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	const int16 *ptr1 = &_polyTab[vtop];
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();
	int32 renderLoop = vsize;
	if (vtop < 0) {
		out += screenWidth * ABS(vtop);
		renderLoop -= ABS(vtop);
	}
	if (renderLoop > screenHeight) {
		renderLoop = screenHeight;
	}
	int32 sens = 1;
	int32 line = 2;
	for (int32 currentLine = 0; currentLine < renderLoop; ++currentLine) {
		int16 xMin = ptr1[0];
		int16 xMax = ptr1[screenHeight];
		ptr1++;

		uint8 *pDest = out + xMin;

		for (; xMin <= xMax; xMin++) {
			*pDest++ = (uint8)color;
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
		out += screenWidth;
	}
}

void Renderer::svgaPolyTriste(int vtop, int32 vsize, uint16 color) const {
	uint8 *out = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	const int16 *ptr1 = &_polyTab[vtop];
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();
	int32 renderLoop = vsize;
	if (vtop < 0) {
		out += screenWidth * ABS(vtop);
		renderLoop -= ABS(vtop);
	}
	if (renderLoop > screenHeight) {
		renderLoop = screenHeight;
	}
	for (int32 currentLine = 0; currentLine < renderLoop; ++currentLine) {
		const int16 start = ptr1[0];
		const int16 stop = ptr1[screenHeight];
		ptr1++;
		const int32 hsize = stop - start;

		for (int32 j = start; j <= hsize + start; j++) {
			if (j >= 0 && j < screenWidth) {
				out[j] = color;
			}
		}
		out += screenWidth;
	}
}

#define ROL16(x, b) (((x) << (b)) | ((x) >> (16 - (b))))

void Renderer::svgaPolyTele(int vtop, int32 vsize, uint16 color) const {
	uint8 *out = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	const int16 *ptr1 = &_polyTab[vtop];
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	int32 renderLoop = vsize;
	if (vtop < 0) {
		out += screenWidth * ABS(vtop);
		renderLoop -= ABS(vtop);
	}
	if (renderLoop > screenHeight) {
		renderLoop = screenHeight;
	}

	uint16 acc = 17371;
	color &= 0xFF;
	uint16 col;
	for (int32 currentLine = 0; currentLine < renderLoop; ++currentLine) {
		int16 xMin = ptr1[0];
		int16 xMax = ptr1[screenHeight];
		++ptr1;
		uint8 *pDest = out + xMin;
		col = xMin;

		for (; xMin <= xMax; xMin++) {
			col = ((col + acc) & 0xFF03) + (uint16)color;
			acc = ROL16(acc, 2) + 1;

			*pDest++ = (uint8)col;
		}
		out += screenWidth;
	}
}

void Renderer::svgaPolyTrans(int vtop, int32 vsize, uint16 color) const {
	uint8 *out = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	const int16 *ptr1 = &_polyTab[vtop];
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	do {
		int16 start = ptr1[0];
		int16 stop = ptr1[screenHeight];

		ptr1++;
		int32 hsize = stop - start;

		if (hsize >= 0) {
			hsize++;
			uint8 *out2 = start + out;
			*out2 = (*(out2)&0x0F) | color;
			out2++;
		}
		out += screenWidth;
	} while (--vsize);
}

// Used e.g for the legs of the horse or the ears of most characters
void Renderer::svgaPolyTrame(int vtop, int32 vsize, uint16 color) const {
	uint8 *out = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	const int16 *ptr1 = &_polyTab[vtop];
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	int32 renderLoop = vsize;
	if (vtop < 0) {
		out += screenWidth * ABS(vtop);
		renderLoop -= ABS(vtop);
	}
	if (renderLoop > screenHeight) {
		renderLoop = screenHeight;
	}
	int32 pair = 0;
	for (int32 currentLine = 0; currentLine < renderLoop; ++currentLine) {
		int16 start = ptr1[0];
		int16 stop = ptr1[screenHeight];
		ptr1++;
		uint8 *out2 = start + out;
		stop = ((stop - start) + 1) / 2;
		if (stop > 0) {
			pair ^= 1; // paire/impair
			if ((start & 1) ^ pair) {
				out2++;
			}

			for (; stop > 0; stop--) {
				*out2 = color;
				out2 += 2;
			}
		}

		out += screenWidth;
	}
}

void Renderer::svgaPolyGouraud(int vtop, int32 vsize) const {
	uint8 *out = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	const int16 *ptr1 = &_polyTab[vtop];
	const int16 *ptr2 = &_colorProgressionBuffer[vtop];
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();
	int32 renderLoop = vsize;
	if (vtop < 0) {
		out += screenWidth * ABS(vtop);
		renderLoop -= ABS(vtop);
	}
	if (renderLoop > screenHeight) {
		renderLoop = screenHeight;
	}
	for (int32 currentLine = 0; currentLine < renderLoop; ++currentLine) {
		uint16 startColor = ptr2[0];
		const uint16 stopColor = ptr2[screenHeight];

		int16 colorDiff = stopColor - startColor;

		const int16 stop = ptr1[screenHeight];
		const int16 start = ptr1[0];

		ptr1++;
		uint8 *out2 = start + out;
		int32 hsize = stop - start;

		ptr2++;

		if (hsize == 0) {
			if (start >= 0 && start < screenWidth) {
				*out2 = ((startColor + stopColor) / 2) / 256; // average of the 2 colors
			}
		} else if (hsize == 1) {
			if (start >= 1 && start < screenWidth - 1) {
				*(out2 + 1) = stopColor / 256;
			}

			if (start >= 0 && start < screenWidth) {
				*out2 = startColor / 256;
			}
		} else if (hsize == 2) {
			if (start >= 2 && start < screenWidth - 2) {
				*(out2 + 2) = stopColor / 256;
			}

			if (start >= 1 && start < screenWidth - 1) {
				*(out2 + 1) = ((startColor + stopColor) / 2) / 256; // average of the 2 colors
			}

			if (start >= 0 && start < screenWidth) {
				*out2 = startColor / 256;
			}
		} else if (hsize > 0) {
			int32 currentXPos = start;
			colorDiff /= hsize;
			hsize++;

			if (hsize % 2) {
				if (currentXPos >= 0 && currentXPos < screenWidth) {
					*out2 = startColor / 256;
				}
				++out2;
				++currentXPos;
				startColor += colorDiff;
			}
			hsize /= 2;

			do {
				for (int i = 0; i < 2; ++i) {
					if (currentXPos >= 0 && currentXPos < screenWidth) {
						*out2 = startColor / 256;
					}
					++out2;
					++currentXPos;
					startColor += colorDiff;
				}
			} while (--hsize);
		}
		out += screenWidth;
	}
}

// used for the most of the heads of the characters and the horse body
void Renderer::svgaPolyDith(int vtop, int32 vsize) const {
	uint8 *out = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	const int16 *ptr1 = &_polyTab[vtop];
	const int16 *ptr2 = &_colorProgressionBuffer[vtop];
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	int32 renderLoop = vsize;
	if (vtop < 0) {
		out += screenWidth * ABS(vtop);
		renderLoop -= ABS(vtop);
	}
	if (renderLoop > screenHeight) {
		renderLoop = screenHeight;
	}
	for (int32 currentLine = 0; currentLine < renderLoop; ++currentLine) {
		int16 stop = ptr1[screenHeight];
		int16 start = ptr1[0];
		ptr1++;
		int32 hsize = stop - start;
		if (hsize < 0) {
			out += screenWidth;
			continue;
		}
		uint16 startColor = ptr2[0];
		uint16 stopColor = ptr2[screenHeight];
		int32 currentXPos = start;

		uint8 *out2 = start + out;
		ptr2++;

		if (hsize == 0) {
			if (currentXPos >= 0 && currentXPos < screenWidth) {
				*out2 = (uint8)(((startColor + stopColor) / 2) / 256); // average of the 2 colors
			}
		} else {
			int16 colorSize = stopColor - startColor;
			if (hsize == 1) {
				uint16 currentColor = startColor;
				hsize++;
				hsize /= 2;

				currentColor &= 0xFF;
				currentColor += startColor;
				if (currentXPos >= 0 && currentXPos < screenWidth) {
					*out2 = currentColor / 256;
				}

				currentColor &= 0xFF;
				startColor += colorSize;
				currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
				currentColor += startColor;

				currentXPos++;
				if (currentXPos >= 0 && currentXPos < screenWidth) {
					*(out2 + 1) = currentColor / 256;
				}
			} else if (hsize == 2) {
				uint16 currentColor = startColor;
				hsize++;
				hsize /= 2;

				currentColor &= 0xFF;
				colorSize /= 2;
				currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
				currentColor += startColor;
				if (currentXPos >= 0 && currentXPos < screenWidth) {
					*out2 = currentColor / 256;
				}

				out2++;
				currentXPos++;
				startColor += colorSize;

				currentColor &= 0xFF;
				currentColor += startColor;

				if (currentXPos >= 0 && currentXPos < screenWidth) {
					*out2 = currentColor / 256;
				}

				currentColor &= 0xFF;
				startColor += colorSize;
				currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
				currentColor += startColor;

				currentXPos++;
				if (currentXPos >= 0 && currentXPos < screenWidth) {
					*(out2 + 1) = currentColor / 256;
				}
			} else {
				uint16 currentColor = startColor;
				colorSize /= hsize;
				hsize++;

				if (hsize % 2) {
					hsize /= 2;
					currentColor &= 0xFF;
					currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
					currentColor += startColor;
					if (currentXPos >= 0 && currentXPos < screenWidth) {
						*out2 = currentColor / 256;
					}
					out2++;
					currentXPos++;
				} else {
					hsize /= 2;
				}

				do {
					currentColor &= 0xFF;
					currentColor += startColor;
					if (currentXPos >= 0 && currentXPos < screenWidth) {
						*out2 = currentColor / 256;
					}
					currentXPos++;
					currentColor &= 0xFF;
					startColor += colorSize;
					currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
					currentColor += startColor;
					if (currentXPos >= 0 && currentXPos < screenWidth) {
						*(out2 + 1) = currentColor / 256;
					}
					currentXPos++;
					out2 += 2;
					startColor += colorSize;
				} while (--hsize);
			}
		}
		out += screenWidth;
	}
}

void Renderer::svgaPolyMarbre(int vtop, int32 vsize, uint16 color) const {
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	uint8 *out = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	int16 *ptr1 = &_polyTab[vtop];

	int16 xMin, xMax;
	int16 y = vtop;
	uint8 *pDestLine = out;
	uint8 *pDest;
	int16 *pVerticG = ptr1;
	int16 *pVerticD = &ptr1[screenHeight];

	uint16 start = (color & 0xFF) << 8;
	uint16 end = color & 0xFF00;
	uint16 delta = end - start + 1; // delta intensity
	int32 step, dc;

	for (; y <= vsize; y++) {
		xMin = *pVerticG++;
		xMax = *pVerticD++;
		pDest = pDestLine + xMin;

		dc = xMax - xMin;
		if (dc == 0) {
			// just one
			*pDest++ = (uint8)(end >> 8);
		} else if (dc > 0) {
			step = delta / (dc + 1);
			color = start;

			for (; xMin <= xMax; xMin++) {
				*pDest++ = (uint8)(color >> 8);
				color += step;
			}
		}

		pDestLine += screenWidth;
	}
}

void Renderer::svgaPolyTriche(int vtop, int32 vsize, uint16 color) const {
	uint8 *out = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, vtop);
	const int16 *ptr1 = &_polyTab[vtop];
	const int16 *ptr2 = &_colorProgressionBuffer[vtop];
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	int32 renderLoop = vsize;
	if (vtop < 0) {
		out += screenWidth * ABS(vtop);
		renderLoop -= ABS(vtop);
	}
	if (renderLoop > screenHeight) {
		renderLoop = screenHeight;
	}
	for (int32 currentLine = 0; currentLine < renderLoop; ++currentLine) {
		int16 xMin = MAX<int16>(0, ptr1[0]);
		const int16 xMax = MIN<int16>((int16)(screenWidth - 1), ptr1[screenHeight]);
		uint8 *pDest = out + xMin;

		color = (*ptr2++) >> 8;
		for (; xMin <= xMax; xMin++) {
			*pDest++ = color;
		}
		++ptr1;

		out += screenWidth;
	}
}

void Renderer::renderPolygons(const CmdRenderPolygon &polygon, ComputedVertex *vertices, int vtop, int vbottom) {
	if (computePoly(polygon.renderType, vertices, polygon.numVertices)) {
		const int32 vsize = vbottom - vtop + 1;
		fillVertices(vtop, vsize, polygon.renderType, polygon.colorIndex);
	}
}

void Renderer::fillVertices(int vtop, int32 vsize, uint8 renderType, uint16 color) {
	switch (renderType) {
	case POLYGONTYPE_FLAT:
		svgaPolyTriste(vtop, vsize, color);
		break;
	case POLYGONTYPE_TELE:
		if (_engine->_cfgfile.PolygonDetails == 0) {
			svgaPolyTriste(vtop, vsize, color);
		} else {
			svgaPolyTele(vtop, vsize, color);
		}
		break;
	case POLYGONTYPE_COPPER:
		svgaPolyCopper(vtop, vsize, color);
		break;
	case POLYGONTYPE_BOPPER:
		svgaPolyBopper(vtop, vsize, color);
		break;
	case POLYGONTYPE_TRANS:
		svgaPolyTrans(vtop, vsize, color);
		break;
	case POLYGONTYPE_TRAME: // raster
		svgaPolyTrame(vtop, vsize, color);
		break;
	case POLYGONTYPE_GOURAUD:
		if (_engine->_cfgfile.PolygonDetails == 0) {
			svgaPolyTriche(vtop, vsize, color);
		} else {
			svgaPolyGouraud(vtop, vsize);
		}
		break;
	case POLYGONTYPE_DITHER:
		if (_engine->_cfgfile.PolygonDetails == 0) {
			svgaPolyTriche(vtop, vsize, color);
		} else if (_engine->_cfgfile.PolygonDetails == 1) {
			svgaPolyGouraud(vtop, vsize);
		} else {
			svgaPolyDith(vtop, vsize);
		}
		break;
	case POLYGONTYPE_MARBLE:
		svgaPolyMarbre(vtop, vsize, color);
		break;
	default:
		warning("RENDER WARNING: Unsupported render type %d", renderType);
		break;
	}
}

bool Renderer::computeSphere(int32 x, int32 y, int32 radius) {
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

		int16 *start = _polyTab;
		int16 *end = &_polyTab[_engine->height()];

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
				start[ny] = (int16)x1;
				end[ny] = (int16)x2;
			}

			ny = y + r;
			if ((ny >= ctop) && (ny <= cbottom)) {
				start[ny] = (int16)x1;
				end[ny] = (int16)x2;
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
						start[ny] = (int16)x1;
						end[ny] = (int16)x2;
					}

					ny = y + radius;
					if ((ny >= ctop) && (ny <= cbottom)) {
						start[ny] = (int16)x1;
						end[ny] = (int16)x2;
					}

					--radius;
					acc -= radius;
				}
			}

			++r;
		}

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
	const int16 maxHeight = _engine->height() - 1;
	const int16 maxWidth = _engine->width() - 1;

	for (const BodyPolygon &polygon : polygons) {
		const uint8 materialType = polygon.materialType;
		const uint8 numVertices = polygon.indices.size();
		assert(numVertices <= 16);

		int16 zMax = -32000;

		CmdRenderPolygon *destinationPolygon = (CmdRenderPolygon *)(void*)renderBufferPtr;
		destinationPolygon->numVertices = numVertices;
		destinationPolygon->top = SCENE_SIZE_MAX;
		destinationPolygon->bottom = SCENE_SIZE_MIN;

		renderBufferPtr += sizeof(CmdRenderPolygon);

		ComputedVertex *const vertices = (ComputedVertex *)(void*)renderBufferPtr;
		renderBufferPtr += destinationPolygon->numVertices * sizeof(ComputedVertex);

		ComputedVertex *vertex = vertices;

		if (materialType >= MAT_GOURAUD) {
			destinationPolygon->renderType = polygon.materialType - (MAT_GOURAUD - POLYGONTYPE_GOURAUD);
			destinationPolygon->colorIndex = polygon.intensity;

			for (int16 idx = 0; idx < numVertices; ++idx) {
				const uint16 shadeEntry = polygon.normals[idx];
				const int16 shadeValue = polygon.intensity + modelData->normalTable[shadeEntry];
				const uint16 vertexIndex = polygon.indices[idx];
				const I16Vec3 *point = &modelData->flattenPoints[vertexIndex];

				vertex->intensity = shadeValue;
				vertex->x = clamp(point->x, 0, maxWidth);
				vertex->y = clamp(point->y, 0, maxHeight);
				destinationPolygon->top = MIN<int16>(destinationPolygon->top, vertex->y);
				destinationPolygon->bottom = MAX<int16>(destinationPolygon->bottom, vertex->y);
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
				vertex->x = clamp(point->x, 0, maxWidth);
				vertex->y = clamp(point->y, 0, maxHeight);
				destinationPolygon->top = MIN<int16>(destinationPolygon->top, vertex->y);
				destinationPolygon->bottom = MAX<int16>(destinationPolygon->bottom, vertex->y);
				zMax = MAX(zMax, point->z);
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

bool Renderer::renderModelElements(int32 numOfPrimitives, const BodyData &bodyData, RenderCommand **renderCmds, ModelData *modelData, Common::Rect &modelRect) {
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
			renderPolygons(*header, vertices, header->top, header->bottom);
			break;
		}
		case RENDERTYPE_DRAWSPHERE: {
			const CmdRenderSphere *sphere = (const CmdRenderSphere *)(const void*)pointer;
			int32 radius = sphere->radius;

			if (_isUsingIsoProjection) {
				// * sqrt(sx+sy) / 512 (isometric scale)
				radius = (radius * 34) / ISO_SCALE;
			} else {
				int32 delta = _kFactor + sphere->z;
				if (delta == 0) {
					break;
				}
				radius = (sphere->radius * _lFactorX) / delta;
			}

			radius += 3;

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

			radius -= 3;

			if (computeSphere(sphere->x, sphere->y, radius)) {
				const int32 vsize = 2 * radius;
				fillVertices(sphere->y - radius, vsize, sphere->polyRenderType, sphere->color);
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

bool Renderer::renderAnimatedModel(ModelData *modelData, const BodyData &bodyData, RenderCommand *renderCmds, const IVec3 &angleVec, const IVec3 &renderPos, Common::Rect &modelRect) {
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

			if (boneData->type == 0) {
				processRotatedElement(modelMatrix, vertices, boneData->x, boneData->y, boneData->z, bone, modelData);
			} else if (boneData->type == 1) {
				processTranslatedElement(modelMatrix, vertices, boneData->x, boneData->y, boneData->z, bone, modelData);
			}

			++modelMatrix;
			++boneIdx;
		} while (--numOfPrimitives);
	}

	numOfPrimitives = numVertices;

	const I16Vec3 *pointPtr = &modelData->computedPoints[0];
	I16Vec3 *pointPtrDest = &modelData->flattenPoints[0];

	if (_isUsingIsoProjection) { // use standard projection
		do {
			const int32 coX = pointPtr->x + renderPos.x;
			const int32 coY = pointPtr->y + renderPos.y;
			const int32 coZ = -(pointPtr->z + renderPos.z);

			// TODO: use projectPositionOnScreen()
			pointPtrDest->x = (coX + coZ) * 24 / ISO_SCALE + _projectionCenter.x;
			pointPtrDest->y = (((coX - coZ) * 12) - coY * 30) / ISO_SCALE + _projectionCenter.y;
			pointPtrDest->z = coZ - coX - coY;

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
			int32 coX = pointPtr->x + renderPos.x;
			int32 coY = pointPtr->y + renderPos.y;
			int32 coZ = -(pointPtr->z + renderPos.z);

			coZ += _kFactor;

			if (coZ <= 0) {
				coZ = 0x7FFFFFFF;
			}

			// X projection
			{
				coX = _projectionCenter.x + ((coX * _lFactorX) / coZ);

				if (coX > 0xFFFF) {
					coX = 0x7FFF;
				}

				pointPtrDest->x = coX;

				if (pointPtrDest->x < modelRect.left) {
					modelRect.left = pointPtrDest->x;
				}

				if (pointPtrDest->x > modelRect.right) {
					modelRect.right = pointPtrDest->x;
				}
			}

			// Y projection
			{
				coY = _projectionCenter.y + ((-coY * _lFactorY) / coZ);

				if (coY > 0xFFFF) {
					coY = 0x7FFF;
				}

				pointPtrDest->y = coY;

				if (pointPtrDest->y < modelRect.top) {
					modelRect.top = pointPtrDest->y;
				}
				if (pointPtrDest->y > modelRect.bottom) {
					modelRect.bottom = pointPtrDest->y;
				}
			}

			// Z projection
			{
				if (coZ > 0xFFFF) {
					coZ = 0x7FFF;
				}

				pointPtrDest->z = coZ;
			}

			pointPtr++;
			pointPtrDest++;

		} while (--numOfPrimitives);
	}

	int32 numNormals = bodyData.getNormals().size();

	if (numNormals) { // process normal data
		uint16 *currentShadeDestination = (uint16 *)modelData->normalTable;
		IMatrix3x3 *lightMatrix = &_matricesTable[0];

		numOfPrimitives = numBones;

		int shadeIndex = 0;
		int boneIdx = 0;
		do { // for each element
			numNormals = bodyData.getBone(boneIdx).numNormals;

			if (numNormals) {
				const IMatrix3x3 matrix = *lightMatrix * _lightNorm;

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

	return renderModelElements(numOfPrimitives, bodyData, &renderCmds, modelData, modelRect);
}

bool Renderer::renderIsoModel(int32 x, int32 y, int32 z, int32 angleX, int32 angleY, int32 angleZ, const BodyData &bodyData, Common::Rect &modelRect) {
	IVec3 renderAngle;
	renderAngle.x = angleX;
	renderAngle.y = angleY;
	renderAngle.z = angleZ;

	// model render size reset
	modelRect.left = SCENE_SIZE_MAX;
	modelRect.top = SCENE_SIZE_MAX;
	modelRect.right = SCENE_SIZE_MIN;
	modelRect.bottom = SCENE_SIZE_MIN;

	IVec3 renderPos;
	if (_isUsingIsoProjection) {
		renderPos.x = x;
		renderPos.y = y;
		renderPos.z = z;
	} else {
		renderPos = getBaseRotationPosition(x, y, z) - _baseRotPos;
	}

	if (!bodyData.isAnimated()) {
#if 0
		// TODO: fill modeldata.flattenedpoints
		int32 numOfPrimitives = 0;
		RenderCommand* renderCmds = _renderCmds;
		return renderModelElements(numOfPrimitives, bodyData, &renderCmds, &_modelData, modelRect);
#else
		error("Unsupported unanimated model render!");
#endif
	}
	// restart at the beginning of the renderTable
	if (!renderAnimatedModel(&_modelData, bodyData, _renderCmds, renderAngle, renderPos, modelRect)) {
		modelRect.right = -1;
		modelRect.bottom = -1;
		modelRect.left = -1;
		modelRect.top = -1;
		return false;
	}
	return true;
}

void Renderer::renderBehaviourModel(const Common::Rect &rect, int32 y, int32 angle, const BodyData &bodyData, ActorMoveStruct &move) {
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
		const int16 newAngle = move.getRealAngle(_engine->_lbaTime);
		if (move.numOfStep == 0) {
			_engine->_movements->initRealAngle(newAngle, newAngle - LBAAngles::ANGLE_90, LBAAngles::ANGLE_17, &move);
		}
		renderIsoModel(0, y, 0, LBAAngles::ANGLE_0, newAngle, LBAAngles::ANGLE_0, bodyData, dummy);
	} else {
		renderIsoModel(0, y, 0, LBAAngles::ANGLE_0, angle, LBAAngles::ANGLE_0, bodyData, dummy);
	}
	_engine->_interface->resetClip();
}

void Renderer::renderInventoryItem(int32 x, int32 y, const BodyData &bodyData, int32 angle, int32 param) {
	setProjection(x, y, 128, 200, 200);
	setCameraAngle(0, 0, 0, 60, 0, 0, param);

	Common::Rect dummy;
	renderIsoModel(0, 0, 0, LBAAngles::ANGLE_0, angle, LBAAngles::ANGLE_0, bodyData, dummy);
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

void Renderer::renderHolomapVertices(const ComputedVertex vertexCoordinates[3], const ComputedVertex textureCoordinates[3], uint8 *holomapImage, uint32 holomapImageSize) {
	int32 lymin = SCENE_SIZE_MAX;
	int32 lymax = SCENE_SIZE_MIN;
	fillHolomapTriangles(vertexCoordinates[0], vertexCoordinates[1], textureCoordinates[0], textureCoordinates[1], lymin, lymax);
	fillHolomapTriangles(vertexCoordinates[1], vertexCoordinates[2], textureCoordinates[1], textureCoordinates[2], lymin, lymax);
	fillHolomapTriangles(vertexCoordinates[2], vertexCoordinates[0], textureCoordinates[2], textureCoordinates[0], lymin, lymax);
	renderHolomapPolygons(lymin, lymax, holomapImage, holomapImageSize);
}

void Renderer::renderHolomapPolygons(int32 ymin, int32 ymax, uint8 *holomapImage, uint32 holomapImageSize) {
	if (ymin < 0 || ymin >= _engine->_frontVideoBuffer.h) {
		return;
	}
	uint8 *pDestLine = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(0, ymin);

	const int16 *pVerticG = _tabVerticG + ymin;
	const int16 *pVerticD = _tabVerticD + ymin;
	const uint16 *pu0 = (const uint16 *)(_tabx0 + ymin);
	const uint16 *pv0 = (const uint16 *)(_taby0 + ymin);
	const uint16 *pu1 = (const uint16 *)(_tabx1 + ymin);
	const uint16 *pv1 = (const uint16 *)(_taby1 + ymin);

	int32 yHeight = ymax - ymin;
	while (yHeight > -1) {
		int32 u;
		int32 v;
		const int16 xmin = *pVerticG++;
		const int16 xmax = *pVerticD++;
		const uint32 u0 = u = *pu0++;
		const uint32 v0 = v = *pv0++;
		const uint32 u1 = *pu1++;
		const uint32 v1 = *pv1++;
		const int16 width = xmax - xmin;
		if (width > 0) {
			uint8 *pixelBufPtr = pDestLine + xmin;

			int32 ustep = ((int32)u1 - (int32)u0 + 1) / width;
			int32 vstep = ((int32)v1 - (int32)v0 + 1) / width;

			for (int16 i = 0; i < width; ++i) {
				// u0 & 0xFF00 is the x position on the image * 256
				// v0 & 0xFF00 is the y position on the image * 256
				const uint32 idx = ((u >> 8) & 0xff) | (v & 0xff00);
				assert(idx < holomapImageSize);
				*pixelBufPtr++ = holomapImage[idx];
				u += ustep;
				v += vstep;
			}
		}
		pDestLine += _engine->_frontVideoBuffer.pitch;
		--yHeight;
	}
}

} // namespace TwinE
