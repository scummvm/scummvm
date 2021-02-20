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

#include "twine/renderer/renderer.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "twine/menu/interface.h"
#include "twine/menu/menu.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/shadeangletab.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/movements.h"
#include "twine/scene/grid.h"
#include "twine/shared.h"
#include "twine/twine.h"

namespace TwinE {

#define RENDERTYPE_DRAWLINE 0
#define RENDERTYPE_DRAWPOLYGON 1
#define RENDERTYPE_DRAWSPHERE 2

Renderer::Renderer(TwinEEngine *engine) : _engine(engine), shadeAngleTab3(&shadeAngleTable[384]) {
}

Renderer::~Renderer() {
	free(_polyTab);
	free(_polyTab2);
}

void Renderer::init(int32 w, int32 h) {
	_polyTabSize = _engine->height() * 2;
	_polyTab = (int16*)malloc(_polyTabSize * sizeof(int16));
	_polyTab2 = (int16*)malloc(_polyTabSize * sizeof(int16));
}

int32 Renderer::projectPositionOnScreen(int32 cX, int32 cY, int32 cZ) {
	if (isUsingOrthoProjection) {
		projPosX = ((cX - cZ) * 24) / BRICK_SIZE + orthoProjX;
		projPosY = (((cX + cZ) * 12) - cY * 30) / BRICK_SIZE + orthoProjY;
		projPosZ = cZ - cY - cX;
		return 1;
	}

	cX -= baseRotPosX;
	cY -= baseRotPosY;
	cZ -= baseRotPosZ;

	if (cZ < 0) {
		projPosX = 0;
		projPosY = 0;
		projPosZ = 0;
		return 0;
	}

	int32 posZ = cZ + cameraDepthOffset;
	if (posZ < 0) {
		posZ = 0x7FFF;
	}

	projPosX = (cX * cameraScaleY) / posZ + orthoProjX;
	projPosY = (-cY * cameraScaleZ) / posZ + orthoProjY;
	projPosZ = posZ;
	return -1;
}

void Renderer::setCameraPosition(int32 x, int32 y, int32 depthOffset, int32 scaleY, int32 scaleZ) {
	orthoProjX = x;
	orthoProjY = y;

	cameraDepthOffset = depthOffset;
	cameraScaleY = scaleY;
	cameraScaleZ = scaleZ;

	isUsingOrthoProjection = false;
}

void Renderer::setBaseTranslation(int32 x, int32 y, int32 z) {
	baseTransPosX = x;
	baseTransPosY = y;
	baseTransPosZ = z;
}

void Renderer::setOrthoProjection(int32 x, int32 y, int32 z) {
	orthoProjX = x;
	orthoProjY = y;
	orthoProjZ = z;

	isUsingOrthoProjection = true;
}

void Renderer::baseMatrixTranspose() {
	const Matrix tmpMatrix = baseMatrix;
	baseMatrix.row1[0] = tmpMatrix.row1[0];
	baseMatrix.row1[1] = tmpMatrix.row2[0];
	baseMatrix.row1[2] = tmpMatrix.row3[0];
	baseMatrix.row2[0] = tmpMatrix.row1[1];
	baseMatrix.row2[1] = tmpMatrix.row2[1];
	baseMatrix.row2[2] = tmpMatrix.row3[1];
	baseMatrix.row3[0] = tmpMatrix.row1[2];
	baseMatrix.row3[1] = tmpMatrix.row2[2];
	baseMatrix.row3[2] = tmpMatrix.row3[2];
}

void Renderer::setBaseRotation(int32 x, int32 y, int32 z, bool transpose) {
	const double Xradians = (double)((ANGLE_90 - x) % ANGLE_360) * 2 * M_PI / ANGLE_360;
	const double Yradians = (double)((ANGLE_90 - y) % ANGLE_360) * 2 * M_PI / ANGLE_360;
	const double Zradians = (double)((ANGLE_90 - z) % ANGLE_360) * 2 * M_PI / ANGLE_360;

	baseMatrix.row1[0] = (int32)(sin(Zradians) * sin(Yradians) * SCENE_SIZE_HALFF);
	baseMatrix.row1[1] = (int32)(-cos(Zradians) * SCENE_SIZE_HALFF);
	baseMatrix.row1[2] = (int32)(sin(Zradians) * cos(Yradians) * SCENE_SIZE_HALFF);
	baseMatrix.row2[0] = (int32)(cos(Zradians) * sin(Xradians) * SCENE_SIZE_HALFF);
	baseMatrix.row2[1] = (int32)(sin(Zradians) * sin(Xradians) * SCENE_SIZE_HALFF);
	baseMatrix.row3[0] = (int32)(cos(Zradians) * cos(Xradians) * SCENE_SIZE_HALFF);
	baseMatrix.row3[1] = (int32)(sin(Zradians) * cos(Xradians) * SCENE_SIZE_HALFF);

	int32 matrixElem = baseMatrix.row2[0];

	baseMatrix.row2[0] = (int32)(sin(Yradians) * matrixElem + SCENE_SIZE_HALFF * cos(Yradians) * cos(Xradians));
	baseMatrix.row2[2] = (int32)(cos(Yradians) * matrixElem - SCENE_SIZE_HALFF * sin(Yradians) * cos(Xradians));

	matrixElem = baseMatrix.row3[0];

	baseMatrix.row3[0] = (int32)(sin(Yradians) * matrixElem - SCENE_SIZE_HALFF * sin(Xradians) * cos(Yradians));
	baseMatrix.row3[2] = (int32)(cos(Yradians) * matrixElem + SCENE_SIZE_HALFF * sin(Xradians) * sin(Yradians));

	if (transpose) {
		baseMatrixTranspose();
	}
	getBaseRotationPosition(baseTransPosX, baseTransPosY, baseTransPosZ);

	baseRotPosX = destX;
	baseRotPosY = destY;
	baseRotPosZ = destZ;
}

void Renderer::getBaseRotationPosition(int32 x, int32 y, int32 z) {
	destX = (baseMatrix.row1[0] * x + baseMatrix.row1[1] * y + baseMatrix.row1[2] * z) / SCENE_SIZE_HALF;
	destY = (baseMatrix.row2[0] * x + baseMatrix.row2[1] * y + baseMatrix.row2[2] * z) / SCENE_SIZE_HALF;
	destZ = (baseMatrix.row3[0] * x + baseMatrix.row3[1] * y + baseMatrix.row3[2] * z) / SCENE_SIZE_HALF;
}

void Renderer::getCameraAnglePositions(int32 x, int32 y, int32 z) {
	destX = (baseMatrix.row1[0] * x + baseMatrix.row2[0] * y + baseMatrix.row3[0] * z) / SCENE_SIZE_HALF;
	destY = (baseMatrix.row1[1] * x + baseMatrix.row2[1] * y + baseMatrix.row3[1] * z) / SCENE_SIZE_HALF;
	destZ = (baseMatrix.row1[2] * x + baseMatrix.row2[2] * y + baseMatrix.row3[2] * z) / SCENE_SIZE_HALF;
}

void Renderer::translateGroup(int32 x, int32 y, int32 z) {
	destX = (shadeMatrix.row1[0] * x + shadeMatrix.row1[1] * y + shadeMatrix.row1[2] * z) / SCENE_SIZE_HALF;
	destY = (shadeMatrix.row2[0] * x + shadeMatrix.row2[1] * y + shadeMatrix.row2[2] * z) / SCENE_SIZE_HALF;
	destZ = destY;
}

void Renderer::setCameraAngle(int32 transPosX, int32 transPosY, int32 transPosZ, int32 rotPosX, int32 rotPosY, int32 rotPosZ, int32 param6) {
	baseTransPosX = transPosX;
	baseTransPosY = transPosY;
	baseTransPosZ = transPosZ;

	setBaseRotation(rotPosX, rotPosY, rotPosZ);

	baseRotPosZ += param6;

	getCameraAnglePositions(baseRotPosX, baseRotPosY, baseRotPosZ);

	baseTransPosX = destX;
	baseTransPosY = destY;
	baseTransPosZ = destZ;
}

void Renderer::applyRotation(Matrix *targetMatrix, const Matrix *currentMatrix) {
	Matrix matrix1;
	Matrix matrix2;

	if (renderAngleX) {
		int32 angle = renderAngleX;
		int32 angleVar2 = shadeAngleTable[ClampAngle(angle)];
		angle += ANGLE_90;
		int32 angleVar1 = shadeAngleTable[ClampAngle(angle)];

		matrix1.row1[0] = currentMatrix->row1[0];
		matrix1.row2[0] = currentMatrix->row2[0];
		matrix1.row3[0] = currentMatrix->row3[0];

		matrix1.row1[1] = (currentMatrix->row1[2] * angleVar2 + currentMatrix->row1[1] * angleVar1) / SCENE_SIZE_HALF;
		matrix1.row1[2] = (currentMatrix->row1[2] * angleVar1 - currentMatrix->row1[1] * angleVar2) / SCENE_SIZE_HALF;
		matrix1.row2[1] = (currentMatrix->row2[2] * angleVar2 + currentMatrix->row2[1] * angleVar1) / SCENE_SIZE_HALF;
		matrix1.row2[2] = (currentMatrix->row2[2] * angleVar1 - currentMatrix->row2[1] * angleVar2) / SCENE_SIZE_HALF;
		matrix1.row3[1] = (currentMatrix->row3[2] * angleVar2 + currentMatrix->row3[1] * angleVar1) / SCENE_SIZE_HALF;
		matrix1.row3[2] = (currentMatrix->row3[2] * angleVar1 - currentMatrix->row3[1] * angleVar2) / SCENE_SIZE_HALF;
	} else {
		matrix1 = *currentMatrix;
	}

	if (renderAngleZ) {
		int32 angle = renderAngleZ;
		int32 angleVar2 = shadeAngleTable[ClampAngle(angle)];
		angle += ANGLE_90;
		int32 angleVar1 = shadeAngleTable[ClampAngle(angle)];

		matrix2.row1[2] = matrix1.row1[2];
		matrix2.row2[2] = matrix1.row2[2];
		matrix2.row3[2] = matrix1.row3[2];

		matrix2.row1[0] = (matrix1.row1[1] * angleVar2 + matrix1.row1[0] * angleVar1) / SCENE_SIZE_HALF;
		matrix2.row1[1] = (matrix1.row1[1] * angleVar1 - matrix1.row1[0] * angleVar2) / SCENE_SIZE_HALF;
		matrix2.row2[0] = (matrix1.row2[1] * angleVar2 + matrix1.row2[0] * angleVar1) / SCENE_SIZE_HALF;
		matrix2.row2[1] = (matrix1.row2[1] * angleVar1 - matrix1.row2[0] * angleVar2) / SCENE_SIZE_HALF;
		matrix2.row3[0] = (matrix1.row3[1] * angleVar2 + matrix1.row3[0] * angleVar1) / SCENE_SIZE_HALF;
		matrix2.row3[1] = (matrix1.row3[1] * angleVar1 - matrix1.row3[0] * angleVar2) / SCENE_SIZE_HALF;
	} else {
		matrix2 = matrix1;
	}

	if (renderAngleY) {
		int32 angle = renderAngleY;
		int32 angleVar2 = shadeAngleTable[ClampAngle(angle)];
		angle += ANGLE_90;
		int32 angleVar1 = shadeAngleTable[ClampAngle(angle)];

		targetMatrix->row1[1] = matrix2.row1[1];
		targetMatrix->row2[1] = matrix2.row2[1];
		targetMatrix->row3[1] = matrix2.row3[1];

		targetMatrix->row1[0] = (matrix2.row1[0] * angleVar1 - matrix2.row1[2] * angleVar2) / SCENE_SIZE_HALF;
		targetMatrix->row1[2] = (matrix2.row1[0] * angleVar2 + matrix2.row1[2] * angleVar1) / SCENE_SIZE_HALF;
		targetMatrix->row2[0] = (matrix2.row2[0] * angleVar1 - matrix2.row2[2] * angleVar2) / SCENE_SIZE_HALF;
		targetMatrix->row2[2] = (matrix2.row2[0] * angleVar2 + matrix2.row2[2] * angleVar1) / SCENE_SIZE_HALF;

		targetMatrix->row3[0] = (matrix2.row3[0] * angleVar1 - matrix2.row3[2] * angleVar2) / SCENE_SIZE_HALF;
		targetMatrix->row3[2] = (matrix2.row3[0] * angleVar2 + matrix2.row3[2] * angleVar1) / SCENE_SIZE_HALF;
	} else {
		*targetMatrix = matrix2;
	}
}

void Renderer::applyPointsRotation(const pointTab *pointsPtr, int32 numPoints, pointTab *destPoints, const Matrix *rotationMatrix) {
	int32 numOfPoints2 = numPoints;

	do {
		const int32 tmpX = pointsPtr->x;
		const int32 tmpY = pointsPtr->y;
		const int32 tmpZ = pointsPtr->z;

		destPoints->x = ((rotationMatrix->row1[0] * tmpX + rotationMatrix->row1[1] * tmpY + rotationMatrix->row1[2] * tmpZ) / SCENE_SIZE_HALF) + destX;
		destPoints->y = ((rotationMatrix->row2[0] * tmpX + rotationMatrix->row2[1] * tmpY + rotationMatrix->row2[2] * tmpZ) / SCENE_SIZE_HALF) + destY;
		destPoints->z = ((rotationMatrix->row3[0] * tmpX + rotationMatrix->row3[1] * tmpY + rotationMatrix->row3[2] * tmpZ) / SCENE_SIZE_HALF) + destZ;

		destPoints++;
		pointsPtr++;
	} while (--numOfPoints2);
}

void Renderer::processRotatedElement(Matrix *targetMatrix, const pointTab *pointsPtr, int32 rotZ, int32 rotY, int32 rotX, const elementEntry *elemPtr, ModelData *modelData) {
	int32 firstPoint = elemPtr->firstPoint / sizeof(pointTab);
	int32 numOfPoints2 = elemPtr->numOfPoints;

	renderAngleX = rotX;
	renderAngleY = rotY;
	renderAngleZ = rotZ;

	const Matrix *currentMatrix;
	// if its the first point
	if (elemPtr->baseElement == -1) {
		currentMatrix = &baseMatrix;

		destX = 0;
		destY = 0;
		destZ = 0;
	} else {
		const int32 pointIdx = elemPtr->basePoint / sizeof(pointTab);
		const int32 matrixIndex = elemPtr->baseElement;
		assert(matrixIndex >= 0 && matrixIndex < ARRAYSIZE(matricesTable));
		currentMatrix = &matricesTable[matrixIndex];

		destX = modelData->computedPoints[pointIdx].x;
		destY = modelData->computedPoints[pointIdx].y;
		destZ = modelData->computedPoints[pointIdx].z;
	}

	applyRotation(targetMatrix, currentMatrix);

	if (!numOfPoints2) {
		warning("RENDER WARNING: No points in this model!");
	}

	applyPointsRotation(&pointsPtr[firstPoint], numOfPoints2, &modelData->computedPoints[firstPoint], targetMatrix);
}

void Renderer::applyPointsTranslation(const pointTab *pointsPtr, int32 numPoints, pointTab *destPoints, const Matrix *translationMatrix) {
	int32 numOfPoints2 = numPoints;

	do {
		const int32 tmpX = pointsPtr->x + renderAngleZ;
		const int32 tmpY = pointsPtr->y + renderAngleY;
		const int32 tmpZ = pointsPtr->z + renderAngleX;

		destPoints->x = ((translationMatrix->row1[0] * tmpX + translationMatrix->row1[1] * tmpY + translationMatrix->row1[2] * tmpZ) / SCENE_SIZE_HALF) + destX;
		destPoints->y = ((translationMatrix->row2[0] * tmpX + translationMatrix->row2[1] * tmpY + translationMatrix->row2[2] * tmpZ) / SCENE_SIZE_HALF) + destY;
		destPoints->z = ((translationMatrix->row3[0] * tmpX + translationMatrix->row3[1] * tmpY + translationMatrix->row3[2] * tmpZ) / SCENE_SIZE_HALF) + destZ;

		destPoints++;
		pointsPtr++;
	} while (--numOfPoints2);
}

void Renderer::processTranslatedElement(Matrix *targetMatrix, const pointTab *pointsPtr, int32 rotX, int32 rotY, int32 rotZ, const elementEntry *elemPtr, ModelData *modelData) {
	renderAngleX = rotX;
	renderAngleY = rotY;
	renderAngleZ = rotZ;

	if (elemPtr->baseElement == -1) { // base point
		destX = 0;
		destY = 0;
		destZ = 0;

		*targetMatrix = baseMatrix;
	} else { // dependent
		const int pointsIdx = elemPtr->basePoint / 6;
		destX = modelData->computedPoints[pointsIdx].x;
		destY = modelData->computedPoints[pointsIdx].y;
		destZ = modelData->computedPoints[pointsIdx].z;

		const int32 matrixIndex = elemPtr->baseElement;
		assert(matrixIndex >= 0 && matrixIndex < ARRAYSIZE(matricesTable));
		*targetMatrix = matricesTable[matrixIndex];
	}

	applyPointsTranslation(&pointsPtr[elemPtr->firstPoint / sizeof(pointTab)], elemPtr->numOfPoints, &modelData->computedPoints[elemPtr->firstPoint / sizeof(pointTab)], targetMatrix);
}

void Renderer::setLightVector(int32 angleX, int32 angleY, int32 angleZ) {
	// TODO: RECHECK THIS
	/*_cameraAngleX = angleX;
	_cameraAngleY = angleY;
	_cameraAngleZ = angleZ;*/

	renderAngleX = angleX;
	renderAngleY = angleY;
	renderAngleZ = angleZ;

	applyRotation(&shadeMatrix, &baseMatrix);
	translateGroup(0, 0, 59);

	lightPos.x = destX;
	lightPos.y = destY;
	lightPos.z = destZ;
}

FORCEINLINE int16 clamp(int16 x, int16 a, int16 b) {
	return x < a ? a : (x > b ? b : x);
}

void Renderer::computeBoundingBox(Vertex *vertices, int32 numVertices, int &vleft, int &vright, int &vtop, int &vbottom) const {
	vleft = vtop = SCENE_SIZE_MAX;
	vright = vbottom = SCENE_SIZE_MIN;

	for (int32 i = 0; i < numVertices; i++) {
		vertices[i].x = clamp(vertices[i].x, 0, _engine->width() - 1);
		vertices[i].y = clamp(vertices[i].y, 0, _engine->height() - 1);
		const int vertexX = vertices[i].x;
		vleft = MIN(vleft, vertexX);
		vright = MAX(vright, vertexX);
		const int vertexY = vertices[i].y;
		vtop = MIN(vtop, vertexY);
		vbottom = MAX(vbottom, vertexY);
	}
}

void Renderer::computePolygons(int16 polyRenderType, const Vertex *vertices, int32 numVertices) {
	uint8 vertexParam1 = vertices[numVertices - 1].colorIndex;
	int16 currentVertexX = vertices[numVertices - 1].x;
	int16 currentVertexY = vertices[numVertices - 1].y;
	const int16 *polyTabBegin = _polyTab;
	const int16 *polyTabEnd = &_polyTab[_polyTabSize - 1];
	const int16 *polyTab2Begin = _polyTab2;
	const int16 *polyTab2End = &_polyTab2[_polyTabSize - 1];
	const int screenHeight = _engine->height();

	for (int32 nVertex = 0; nVertex < numVertices; nVertex++) {
		const int16 oldVertexY = currentVertexY;
		const int16 oldVertexX = currentVertexX;
		const uint8 oldVertexParam = vertexParam1;

		vertexParam1 = vertices[nVertex].colorIndex;
		const uint8 vertexParam2 = vertexParam1;
		currentVertexX = vertices[nVertex].x;
		currentVertexY = vertices[nVertex].y;

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

		for (int16 i = 0; i < vsize + 2; i++) {
			if (outPtr >= polyTabBegin && outPtr <= polyTabEnd) {
				*outPtr = xpos;
			}
			outPtr += direction;
			xpos += slope;
		}

		if (polyRenderType >= POLYGONTYPE_GOURAUD) { // we must compute the color progression
			int16 *outPtr2 = &_polyTab2[polyTabIndex];

			for (int16 i = 0; i < vsize + 2; i++) {
				if (outPtr2 >= polyTab2Begin && outPtr2 <= polyTab2End) {
					*outPtr2 = cvalue;
				}
				outPtr2 += direction;
				cvalue += cdelta;
			}
		}
	}
}

void Renderer::renderPolygonsCopper(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &_polyTab[vtop];
	int32 currentLine = vtop;
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	do {
		if (currentLine >= 0 && currentLine < screenHeight) {
			int16 start = ptr1[0];
			int16 stop = ptr1[screenHeight];

			ptr1++;
			int32 hsize = stop - start;

			if (hsize >= 0) {
				uint16 mask = 0x43DB;

				uint16 dx = (uint8)color;
				dx |= 0x300;

				hsize++;
				const int32 startCopy = start;

				for (int32 j = startCopy; j < hsize + startCopy; j++) {
					start += mask;
					start = (start & 0xFF00) | ((start & 0xFF) & (uint8)(dx / 256));
					start = (start & 0xFF00) | ((start & 0xFF) + (dx & 0xFF));
					if (j >= 0 && j < screenWidth) {
						out[j] = start & 0xFF;
					}
					mask = (mask * 4) | (mask / SCENE_SIZE_HALF);
					mask++;
				}
			}
		}
		out += screenWidth;
		currentLine++;
	} while (--vsize);
}

void Renderer::renderPolygonsBopper(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &_polyTab[vtop];
	int32 currentLine = vtop;
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	do {
		if (currentLine >= 0 && currentLine < screenHeight) {
			int16 start = ptr1[0];
			int16 stop = ptr1[screenHeight];
			ptr1++;
			int32 hsize = stop - start;

			if (hsize >= 0) {
				hsize++;
				for (int32 j = start; j < hsize + start; j++) {
					if (start & 1) {
						if (j >= 0 && j < screenWidth) {
							out[j] = color;
						}
					}
				}
			}
		}
		out += screenWidth;
		currentLine++;
	} while (--vsize);
}

void Renderer::renderPolygonsFlat(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &_polyTab[vtop];
	int32 currentLine = vtop;
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	do {
		if (currentLine >= 0 && currentLine < screenHeight) {
			int16 start = ptr1[0];
			int16 stop = ptr1[screenHeight];
			ptr1++;
			int32 hsize = stop - start;

			if (hsize >= 0) {
				hsize++;
				for (int32 j = start; j < hsize + start; j++) {
					if (j >= 0 && j < screenWidth) {
						out[j] = color;
					}
				}
			}
		}
		out += screenWidth;
		currentLine++;
	} while (--vsize);
}

void Renderer::renderPolygonsTele(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &_polyTab[vtop];
	int bx = (uint16)color << 16;
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	int32 renderLoop = vsize;
	do {
		int16 start;
		int16 stop;
		int32 hsize;
		while (1) {
			start = ptr1[0];
			stop = ptr1[screenHeight];
			ptr1++;
			hsize = stop - start;

			if (hsize) {
				break;
			}

			uint8 *out2 = start + out;
			*out2 = ((unsigned short)(bx / 24)) & 0x0F;

			color = *(out2 + 1);

			out += screenWidth;

			--renderLoop;
			if (!renderLoop) {
				return;
			}
		}

		if (stop >= start) {
			hsize++;
			bx = (unsigned short)(color / 16);
			uint8 *out2 = start + out;

			int ax = (bx & 0xF0) * 256;
			bx = bx * 256;
			ax += (bx & 0x0F);
			ax -= bx;
			ax++;
			ax = ax >> 16;

			ax = ax / hsize;
			uint16 temp = (ax & 0xF0);
			temp = temp / 256;
			temp += (ax & 0x0F);
			ax = temp;

			uint16 dx = ax;

			ax = (ax & 0x0F) + (bx & 0xF0);
			hsize++;

			if (hsize & 1) {
				ax = 0; // not sure about this
			}

			int32 j = hsize >> 1;

			while (1) {
				*(out2++) = ax & 0x0F;
				ax += dx;

				--j;
				if (!j) {
					break;
				}

				*(out2++) = ax & 0x0F;
				ax += dx;
			}
		}

		out += screenWidth;
		--renderLoop;

	} while (renderLoop);
}

// FIXME: buggy
void Renderer::renderPolygonsTras(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &_polyTab[vtop];
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	do {
		unsigned short int bx;

		int16 start = ptr1[0];
		int16 stop = ptr1[screenHeight];

		ptr1++;
		int32 hsize = stop - start;

		if (hsize >= 0) {
			hsize++;
			uint8 *out2 = start + out;

			if (hsize / 2 < 0) {
				bx = color & 0xFF;
				bx = bx * 256;
				bx += color & 0xFF;
				for (int32 j = 0; j < hsize; j++) {
					*(out2) = (*(out2)&0x0F0F) | bx;
				}
			} else {
				*(out2) = (*(out2)&0x0F) | color;
				out2++;
			}
		}
		out += screenWidth;
	} while (--vsize);
}

// FIXME: buggy
void Renderer::renderPolygonTrame(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &_polyTab[vtop];
	unsigned char bh = 0;
	const int screenWidth = _engine->width();
	const int screenHeight = _engine->height();

	int32 currentLine = vtop;
	do {
		if (currentLine >= 0 && currentLine < screenHeight) {
			int16 start = ptr1[0];
			int16 stop = ptr1[screenHeight];
			ptr1++;
			int32 hsize = stop - start;

			if (hsize >= 0) {
				hsize++;
				uint8 *out2 = start + out;

				hsize /= 2;
				if (hsize > 1) {
					uint16 ax;
					bh ^= 1;
					ax = (uint16)(*out2);
					ax &= 1;
					if (ax ^ bh) {
						out2++;
					}

					for (int32 j = 0; j < hsize; j++) {
						*(out2) = (uint8)color;
						out2 += 2;
					}
				}
			}
		}
		out += screenWidth;
		currentLine++;
	} while (--vsize);
}

void Renderer::renderPolygonsGouraud(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &_polyTab[vtop];
	const int16 *ptr2 = &_polyTab2[vtop];
	int32 renderLoop = vsize;
	int32 currentLine = vtop;
	do {
		if (currentLine >= 0 && currentLine < _engine->height()) {
			uint16 startColor = ptr2[0];
			uint16 stopColor = ptr2[_engine->height()];

			int16 colorSize = stopColor - startColor;

			int16 stop = ptr1[_engine->height()]; // stop
			int16 start = ptr1[0];            // start

			ptr1++;
			uint8 *out2 = start + out;
			int32 hsize = stop - start;

			//varf2 = ptr2[_engine->height()];
			//varf3 = ptr2[0];

			ptr2++;

			//varf4 = (float)((int32)varf2 - (int32)varf3);

			if (hsize == 0) {
				if (start >= 0 && start < _engine->width()) {
					*out2 = ((startColor + stopColor) / 2) / 256; // moyenne des 2 couleurs
				}
			} else if (hsize > 0) {
				if (hsize == 1) {
					if (start >= -1 && start < _engine->width() - 1) {
						*(out2 + 1) = stopColor / 256;
					}

					if (start >= 0 && start < _engine->width()) {
						*(out2) = startColor / 256;
					}
				} else if (hsize == 2) {
					if (start >= -2 && start < _engine->width() - 2) {
						*(out2 + 2) = stopColor / 256;
					}

					if (start >= -1 && start < _engine->width() - 1) {
						*(out2 + 1) = ((startColor + stopColor) / 2) / 256;
					}

					if (start >= 0 && start < _engine->width()) {
						*(out2) = startColor / 256;
					}
				} else {
					int32 currentXPos = start;
					colorSize /= hsize;
					hsize++;

					if (hsize % 2) {
						hsize /= 2;
						if (currentXPos >= 0 && currentXPos < _engine->width()) {
							*(out2) = startColor / 256;
						}
						out2++;
						currentXPos++;
						startColor += colorSize;
					} else {
						hsize /= 2;
					}

					do {
						if (currentXPos >= 0 && currentXPos < _engine->width()) {
							*(out2) = startColor / 256;
						}

						currentXPos++;
						startColor += colorSize;

						if (currentXPos >= 0 && currentXPos < _engine->width()) {
							*(out2 + 1) = startColor / 256;
						}

						currentXPos++;
						out2 += 2;
						startColor += colorSize;
					} while (--hsize);
				}
			}
		}
		out += _engine->width();
		currentLine++;
	} while (--renderLoop);
}

void Renderer::renderPolygonsDither(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &_polyTab[vtop];
	const int16 *ptr2 = &_polyTab2[vtop];
	int32 renderLoop = vsize;

	int32 currentLine = vtop;
	do {
		if (currentLine >= 0 && currentLine < _engine->height()) {
			int16 stop = ptr1[_engine->height()]; // stop
			int16 start = ptr1[0];            // start
			ptr1++;
			int32 hsize = stop - start;

			if (hsize >= 0) {
				uint16 startColor = ptr2[0];
				uint16 stopColor = ptr2[_engine->height()];
				int32 currentXPos = start;

				uint8 *out2 = start + out;
				ptr2++;

				if (hsize == 0) {
					if (currentXPos >= 0 && currentXPos < _engine->width()) {
						*(out2) = (uint8)(((startColor + stopColor) / 2) / 256);
					}
				} else {
					int16 colorSize = stopColor - startColor;
					if (hsize == 1) {
						uint16 currentColor = startColor;
						hsize++;
						hsize /= 2;

						currentColor &= 0xFF;
						currentColor += startColor;
						if (currentXPos >= 0 && currentXPos < _engine->width()) {
							*(out2) = currentColor / 256;
						}

						currentColor &= 0xFF;
						startColor += colorSize;
						currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
						currentColor += startColor;

						currentXPos++;
						if (currentXPos >= 0 && currentXPos < _engine->width()) {
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
						if (currentXPos >= 0 && currentXPos < _engine->width()) {
							*(out2) = currentColor / 256;
						}

						out2++;
						currentXPos++;
						startColor += colorSize;

						currentColor &= 0xFF;
						currentColor += startColor;

						if (currentXPos >= 0 && currentXPos < _engine->width()) {
							*(out2) = currentColor / 256;
						}

						currentColor &= 0xFF;
						startColor += colorSize;
						currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
						currentColor += startColor;

						currentXPos++;
						if (currentXPos >= 0 && currentXPos < _engine->width()) {
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
							if (currentXPos >= 0 && currentXPos < _engine->width()) {
								*(out2) = currentColor / 256;
							}
							out2++;
							currentXPos++;
						} else {
							hsize /= 2;
						}

						do {
							currentColor &= 0xFF;
							currentColor += startColor;
							if (currentXPos >= 0 && currentXPos < _engine->width()) {
								*(out2) = currentColor / 256;
							}
							currentXPos++;
							currentColor &= 0xFF;
							startColor += colorSize;
							currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
							currentColor += startColor;
							if (currentXPos >= 0 && currentXPos < _engine->width()) {
								*(out2 + 1) = currentColor / 256;
							}
							currentXPos++;
							out2 += 2;
							startColor += colorSize;
						} while (--hsize);
					}
				}
			}
		}
		out += _engine->width();
		currentLine++;
	} while (--renderLoop);
}

void Renderer::renderPolygonsMarble(uint8 *out, int vtop, int32 vsize, int32 color) const {
}

void Renderer::renderPolygons(const CmdRenderPolygon &polygon, Vertex *vertices) {
	int vleft = 0;
	int vright = 0;
	int vtop = 0;
	int vbottom = 0;

	if (polygon.numVertices > 0) {
		computeBoundingBox(vertices, polygon.numVertices, vleft, vright, vtop, vbottom);
	}
	computePolygons(polygon.renderType, vertices, polygon.numVertices);

	uint8 *out = (uint8 *)_engine->frontVideoBuffer.getBasePtr(0, vtop);
	const int32 vsize = vbottom - vtop + 1;

	switch (polygon.renderType) {
	case POLYGONTYPE_FLAT:
		renderPolygonsFlat(out, vtop, vsize, polygon.colorIndex);
		break;
	case POLYGONTYPE_COPPER:
		renderPolygonsCopper(out, vtop, vsize, polygon.colorIndex);
		break;
	case POLYGONTYPE_BOPPER:
		renderPolygonsBopper(out, vtop, vsize, polygon.colorIndex);
		break;
	case POLYGONTYPE_TELE:
		renderPolygonsTele(out, vtop, vsize, polygon.colorIndex);
		break;
	case POLYGONTYPE_TRAS:
		renderPolygonsTras(out, vtop, vsize, polygon.colorIndex);
		break;
	case POLYGONTYPE_TRAME:
		renderPolygonTrame(out, vtop, vsize, polygon.colorIndex);
		break;
	case POLYGONTYPE_GOURAUD:
		renderPolygonsGouraud(out, vtop, vsize, polygon.colorIndex);
		break;
	case POLYGONTYPE_DITHER:
		renderPolygonsDither(out, vtop, vsize, polygon.colorIndex);
		break;
	case POLYGONTYPE_MARBLE:
		renderPolygonsMarble(out, vtop, vsize, polygon.colorIndex);
		break;
	default:
		warning("RENDER WARNING: Unsupported render type %d", polygon.renderType);
		break;
	}
}

void Renderer::circleFill(int32 x, int32 y, int32 radius, uint8 color) {
	radius += 1;

	for (int32 currentLine = -radius; currentLine <= radius; currentLine++) {
		double width;

		if (ABS(currentLine) != radius) {
			width = ABS(sin(acos((float)currentLine / (float)radius)) * radius);
		} else {
			width = 0.0;
		}

		_engine->_interface->drawLine((int32)(x - width), currentLine + y, (int32)(x + width), currentLine + y, color);
	}
}

uint8 *Renderer::prepareSpheres(Common::MemoryReadStream &stream, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData) {
	int16 numSpheres = stream.readSint16LE();
	if (numSpheres <= 0) {
		return renderBufferPtr;
	}
	numOfPrimitives += numSpheres;
	do {
		CmdRenderSphere *sphere = (CmdRenderSphere *)renderBufferPtr;
		stream.skip(1);
		sphere->colorIndex = stream.readByte();
		stream.skip(2);
		sphere->radius = stream.readUint16LE();
		const int16 centerOffset = stream.readUint16LE();
		const int16 centerIndex = centerOffset / 6;
		sphere->x = modelData->flattenPoints[centerIndex].x;
		sphere->y = modelData->flattenPoints[centerIndex].y;

		(*renderCmds)->depth = modelData->flattenPoints[centerIndex].z;
		(*renderCmds)->renderType = RENDERTYPE_DRAWSPHERE;
		(*renderCmds)->dataPtr = renderBufferPtr;
		(*renderCmds)++;

		renderBufferPtr += sizeof(CmdRenderSphere);
	} while (--numSpheres);

	return renderBufferPtr;
}

uint8 *Renderer::prepareLines(Common::MemoryReadStream &stream, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData) {
	int16 numLines = stream.readSint16LE();
	if (numLines <= 0) {
		return renderBufferPtr;
	}
	numOfPrimitives += numLines;

	do {
		CmdRenderLine *lineCoordinatesPtr = (CmdRenderLine *)renderBufferPtr;
		lineCoordinatesPtr->colorIndex = stream.readByte();
		stream.skip(3);
		const int32 point1Index = stream.readSint16LE() / 6;
		const int32 point2Index = stream.readSint16LE() / 6;
		lineCoordinatesPtr->x1 = modelData->flattenPoints[point1Index].x;
		lineCoordinatesPtr->y1 = modelData->flattenPoints[point1Index].y;
		lineCoordinatesPtr->x2 = modelData->flattenPoints[point2Index].x;
		lineCoordinatesPtr->y2 = modelData->flattenPoints[point2Index].y;
		(*renderCmds)->depth = MAX(modelData->flattenPoints[point1Index].z, modelData->flattenPoints[point2Index].z);
		(*renderCmds)->renderType = RENDERTYPE_DRAWLINE;
		(*renderCmds)->dataPtr = renderBufferPtr;
		(*renderCmds)++;

		renderBufferPtr += sizeof(CmdRenderLine);
	} while (--numLines);

	return renderBufferPtr;
}

uint8 *Renderer::preparePolygons(Common::MemoryReadStream &stream, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData) {
	int16 numPolygons = stream.readSint16LE();
	if (numPolygons <= 0) {
		return renderBufferPtr;
	}
	int16 primitiveCounter = numPolygons; // the number of primitives = the number of polygons

	do { // loop that load all the polygons
		const uint8 renderType = stream.readByte();
		const uint8 numVertices = stream.readByte();
		assert(numVertices <= 16);
		const int16 colorIndex = stream.readSint16LE();

		int16 bestDepth = -32000;

		CmdRenderPolygon *destinationPolygon = (CmdRenderPolygon *)renderBufferPtr;
		destinationPolygon->numVertices = numVertices;

		renderBufferPtr += sizeof(CmdRenderPolygon);

		Vertex *const vertices = (Vertex *)renderBufferPtr;
		renderBufferPtr += destinationPolygon->numVertices * sizeof(Vertex);

		Vertex *vertex = vertices;
		int16 counter = destinationPolygon->numVertices;

		// TODO: RECHECK coordinates axis
		if (renderType >= 9) {
			destinationPolygon->renderType = renderType - 2;
			destinationPolygon->colorIndex = colorIndex;

			do {
				const int16 shadeEntry = stream.readSint16LE();
				const int16 shadeValue = colorIndex + modelData->shadeTable[shadeEntry];

				const int16 vertexOffset = stream.readSint16LE();
				const int16 vertexIndex = vertexOffset / 6;
				const pointTab *point = &modelData->flattenPoints[vertexIndex];

				vertex->colorIndex = shadeValue;
				vertex->x = point->x;
				vertex->y = point->y;
				bestDepth = MAX(bestDepth, point->z);
				++vertex;
			} while (--counter > 0);
		} else {
			if (renderType >= POLYGONTYPE_GOURAUD) {
				// only 1 shade value is used
				destinationPolygon->renderType = renderType - POLYGONTYPE_GOURAUD;
				const int16 shadeEntry = stream.readSint16LE();
				const int16 shadeValue = colorIndex + modelData->shadeTable[shadeEntry];
				destinationPolygon->colorIndex = shadeValue;
			} else {
				// no shade is used
				destinationPolygon->renderType = renderType;
				destinationPolygon->colorIndex = colorIndex;
			}

			do {
				const int16 vertexOffset = stream.readSint16LE();
				const int16 vertexIndex = vertexOffset / 6;
				const pointTab *point = &modelData->flattenPoints[vertexIndex];

				vertex->colorIndex = destinationPolygon->colorIndex;
				vertex->x = point->x;
				vertex->y = point->y;
				bestDepth = MAX(bestDepth, point->z);
				++vertex;
			} while (--counter > 0);
		}

		numOfPrimitives++;

		(*renderCmds)->depth = bestDepth;
		(*renderCmds)->renderType = RENDERTYPE_DRAWPOLYGON;
		(*renderCmds)->dataPtr = (uint8 *)destinationPolygon;
		(*renderCmds)++;
	} while (--primitiveCounter);

	return renderBufferPtr;
}

const Renderer::RenderCommand *Renderer::depthSortRenderCommands(int32 numOfPrimitives) {
	Common::sort(&_renderCmds[0], &_renderCmds[numOfPrimitives], [] (const RenderCommand &lhs, const RenderCommand &rhs) {return lhs.depth > rhs.depth;});
	return _renderCmds;
}

bool Renderer::renderModelElements(int32 numOfPrimitives, const uint8 *polygonPtr, RenderCommand **renderCmds, ModelData *modelData) {
	// TODO: proper size
	Common::MemoryReadStream stream(polygonPtr, 100000);

	uint8 *renderBufferPtr = renderCoordinatesBuffer;
	renderBufferPtr = preparePolygons(stream, numOfPrimitives, renderCmds, renderBufferPtr, modelData);
	renderBufferPtr = prepareLines(stream, numOfPrimitives, renderCmds, renderBufferPtr, modelData);
	renderBufferPtr = prepareSpheres(stream, numOfPrimitives, renderCmds, renderBufferPtr, modelData);

	if (numOfPrimitives == 0) {
		_engine->_redraw->renderRect.right = -1;
		_engine->_redraw->renderRect.bottom = -1;
		_engine->_redraw->renderRect.left = -1;
		_engine->_redraw->renderRect.top = -1;
		return false;
	}
	const RenderCommand *cmds = depthSortRenderCommands(numOfPrimitives);

	int16 primitiveCounter = numOfPrimitives;

	do {
		int16 type = cmds->renderType;
		uint8 *pointer = cmds->dataPtr;

		switch (type) {
		case RENDERTYPE_DRAWLINE: {
			const CmdRenderLine *lineCoords = (const CmdRenderLine *)pointer;
			const int32 x1 = lineCoords->x1;
			const int32 y1 = lineCoords->y1;
			const int32 x2 = lineCoords->x2;
			const int32 y2 = lineCoords->y2;
			_engine->_interface->drawLine(x1, y1, x2, y2, lineCoords->colorIndex);
			break;
		}
		case RENDERTYPE_DRAWPOLYGON: {
			const CmdRenderPolygon *header = (const CmdRenderPolygon *)pointer;
			Vertex *vertices = (Vertex *)(pointer + sizeof(CmdRenderPolygon));
			renderPolygons(*header, vertices);
			break;
		}
		case RENDERTYPE_DRAWSPHERE: {
			CmdRenderSphere *sphere = (CmdRenderSphere *)pointer;
			int32 radius = sphere->radius;

			//if (isUsingOrthoProjection) {
				radius = (radius * 34) / 512;
			//} else {
			//	radius = (radius * cameraScaleY) / (cameraDepthOffset + *(const int16 *)pointer); // TODO: this does not make sense.
			//}

			radius += 3;

			if (sphere->x + radius > _engine->_redraw->renderRect.right) {
				_engine->_redraw->renderRect.right = sphere->x + radius;
			}

			if (sphere->x - radius < _engine->_redraw->renderRect.left) {
				_engine->_redraw->renderRect.left = sphere->x - radius;
			}

			if (sphere->y + radius > _engine->_redraw->renderRect.bottom) {
				_engine->_redraw->renderRect.bottom = sphere->y + radius;
			}

			if (sphere->y - radius < _engine->_redraw->renderRect.top) {
				_engine->_redraw->renderRect.top = sphere->y - radius;
			}

			radius -= 3;

			circleFill(sphere->x, sphere->y, radius, sphere->colorIndex);
			break;
		}
		default:
			break;
		}

		cmds++;
	} while (--primitiveCounter);
	return true;
}

bool Renderer::renderAnimatedModel(ModelData *modelData, const uint8 *bodyPtr, RenderCommand *renderCmds) {
	const int32 numVertices = Model::getNumVertices(bodyPtr);
	const int32 numBones = Model::getNumBones(bodyPtr);

	const pointTab *pointsPtr = (const pointTab *)Model::getVerticesBaseData(bodyPtr);

	Matrix *modelMatrix = &matricesTable[0];

	const elementEntry *bonesPtr0 = (const elementEntry *)Model::getBonesBaseData(bodyPtr, 0);
	processRotatedElement(modelMatrix, pointsPtr, renderAngleX, renderAngleY, renderAngleZ, bonesPtr0, modelData);

	int32 numOfPrimitives = 0;

	if (numBones - 1 != 0) {
		numOfPrimitives = numBones - 1;
		modelMatrix = &matricesTable[1];

		int boneIdx = 1;
		do {
			const elementEntry *bonesPtr = (const elementEntry *)Model::getBonesBaseData(bodyPtr, boneIdx);
			int16 boneType = bonesPtr->flag;

			if (boneType == 0) {
				processRotatedElement(modelMatrix, pointsPtr, bonesPtr->rotateX, bonesPtr->rotateY, bonesPtr->rotateZ, bonesPtr, modelData);
			} else if (boneType == 1) {
				processTranslatedElement(modelMatrix, pointsPtr, bonesPtr->rotateX, bonesPtr->rotateY, bonesPtr->rotateZ, bonesPtr, modelData);
			}

			++modelMatrix;
			++boneIdx;
		} while (--numOfPrimitives);
	}

	numOfPrimitives = numVertices;

	const pointTab *pointPtr = &modelData->computedPoints[0];
	pointTab *pointPtrDest = &modelData->flattenPoints[0];

	if (isUsingOrthoProjection) { // use standard projection
		do {
			const int32 coX = pointPtr->x + renderX;
			const int32 coY = pointPtr->y + renderY;
			const int32 coZ = -(pointPtr->z + renderZ);

			pointPtrDest->x = (coX + coZ) * 24 / BRICK_SIZE + orthoProjX;
			pointPtrDest->y = (((coX - coZ) * 12) - coY * 30) / BRICK_SIZE + orthoProjY;
			pointPtrDest->z = coZ - coX - coY;

			if (pointPtrDest->x < _engine->_redraw->renderRect.left) {
				_engine->_redraw->renderRect.left = pointPtrDest->x;
			}
			if (pointPtrDest->x > _engine->_redraw->renderRect.right) {
				_engine->_redraw->renderRect.right = pointPtrDest->x;
			}

			if (pointPtrDest->y < _engine->_redraw->renderRect.top) {
				_engine->_redraw->renderRect.top = pointPtrDest->y;
			}
			if (pointPtrDest->y > _engine->_redraw->renderRect.bottom) {
				_engine->_redraw->renderRect.bottom = pointPtrDest->y;
			}

			pointPtr++;
			pointPtrDest++;
		} while (--numOfPrimitives);
	} else {
		do {
			int32 coX = pointPtr->x + renderX;
			int32 coY = pointPtr->y + renderY;
			int32 coZ = -(pointPtr->z + renderZ);

			coZ += cameraDepthOffset;

			if (coZ <= 0) {
				coZ = 0x7FFFFFFF;
			}

			// X projection
			{
				coX = orthoProjX + ((coX * cameraScaleY) / coZ);

				if (coX > 0xFFFF) {
					coX = 0x7FFF;
				}

				pointPtrDest->x = coX;

				if (pointPtrDest->x < _engine->_redraw->renderRect.left) {
					_engine->_redraw->renderRect.left = pointPtrDest->x;
				}

				if (pointPtrDest->x > _engine->_redraw->renderRect.right) {
					_engine->_redraw->renderRect.right = pointPtrDest->x;
				}
			}

			// Y projection
			{
				coY = orthoProjY + ((-coY * cameraScaleZ) / coZ);

				if (coY > 0xFFFF) {
					coY = 0x7FFF;
				}

				pointPtrDest->y = coY;

				if (pointPtrDest->y < _engine->_redraw->renderRect.top)
					_engine->_redraw->renderRect.top = pointPtrDest->y;
				if (pointPtrDest->y > _engine->_redraw->renderRect.bottom)
					_engine->_redraw->renderRect.bottom = pointPtrDest->y;
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

	int32 numOfShades = Model::getNumShades(bodyPtr);

	if (numOfShades) { // process normal data
		uint16 *currentShadeDestination = (uint16 *)modelData->shadeTable;
		Matrix *lightMatrix = &matricesTable[0];

		numOfPrimitives = numBones;

		int shadeIndex = 0;
		int boneIdx = 0;
		do { // for each element
			numOfShades = Model::getNumShadesBone(bodyPtr, boneIdx);

			if (numOfShades) {
				int32 numShades = numOfShades;

				shadeMatrix = *lightMatrix * lightPos;

				do { // for each normal
					const uint8 *shadePtr = Model::getShadesBaseData(bodyPtr, shadeIndex);
					const int16 *colPtr = (const int16 *)shadePtr;

					const int16 col1 = *((const int16 *)colPtr++);
					const int16 col2 = *((const int16 *)colPtr++);
					const int16 col3 = *((const int16 *)colPtr++);

					int32 color = 0;
					color += shadeMatrix.row1[0] * col1 + shadeMatrix.row1[1] * col2 + shadeMatrix.row1[2] * col3;
					color += shadeMatrix.row2[0] * col1 + shadeMatrix.row2[1] * col2 + shadeMatrix.row2[2] * col3;
					color += shadeMatrix.row3[0] * col1 + shadeMatrix.row3[1] * col2 + shadeMatrix.row3[2] * col3;

					int32 shade = 0;

					if (color > 0) {
						color >>= 14;
						const uint8 *tmpShadePtr = (const uint8 *)shadePtr;
						color /= *((const uint16 *)(tmpShadePtr + 6));
						shade = (uint16)color;
					}

					*currentShadeDestination = shade;
					currentShadeDestination++;
					++shadeIndex;
				} while (--numShades);
			}

			++boneIdx;
			++lightMatrix;
		} while (--numOfPrimitives);
	}

	return renderModelElements(numOfPrimitives, Model::getPolygonData(bodyPtr), &renderCmds, modelData);
}

void Renderer::prepareIsoModel(uint8 *bodyPtr) { // loadGfxSub
	Model *bodyHeader = (Model *)bodyPtr;

	// This function should only be called ONCE, otherwise it corrupts the model data.
	// The following code implements an unused flag to indicate that a model was already processed.
	if (bodyHeader->bodyFlag.alreadyPrepared) {
		return;
	}
	bodyHeader->bodyFlag.alreadyPrepared = 1;

	// no animation applicable
	if (!Model::isAnimated(bodyPtr)) {
		return;
	}

	uint8 *bonesBase = Model::getBonesBaseData(bodyPtr);
	const int16 numBones = Model::getNumBones(bodyPtr);

	// set up bone indices
	for (int32 i = 0; i < numBones; i++) {
		bonesBase += sizeof(elementEntry);
		elementEntry *ee = (elementEntry *)bonesBase;
		ee->baseElement = ee->baseElement / sizeof(elementEntry);
	}
}

bool Renderer::renderIsoModel(int32 x, int32 y, int32 z, int32 angleX, int32 angleY, int32 angleZ, const uint8 *bodyPtr) {
	renderAngleX = angleX;
	renderAngleY = angleY;
	renderAngleZ = angleZ;

	// model render size reset
	_engine->_redraw->renderRect.left = SCENE_SIZE_MAX;
	_engine->_redraw->renderRect.top = SCENE_SIZE_MAX;
	_engine->_redraw->renderRect.right = SCENE_SIZE_MIN;
	_engine->_redraw->renderRect.bottom = SCENE_SIZE_MIN;

	if (isUsingOrthoProjection) {
		renderX = x;
		renderY = y;
		renderZ = z;
	} else {
		getBaseRotationPosition(x, y, z);

		renderX = destX - baseRotPosX;
		renderY = destY - baseRotPosY; // RECHECK
		renderZ = destZ - baseRotPosZ;
	}

	if (!Model::isAnimated(bodyPtr)) {
		error("Unsupported unanimated model render!");
	}
	// restart at the beginning of the renderTable
	return renderAnimatedModel(&_modelData, bodyPtr, _renderCmds);
}

void Renderer::renderBehaviourModel(const Common::Rect &rect, int32 y, int32 angle, const uint8 *bodyPtr) {
	renderBehaviourModel(rect.left, rect.top, rect.right, rect.bottom, y, angle, bodyPtr);
}

void Renderer::renderBehaviourModel(int32 boxLeft, int32 boxTop, int32 boxRight, int32 boxBottom, int32 y, int32 angle, const uint8 *bodyPtr) {
	const int32 ypos = (boxBottom + boxTop) / 2;
	const int32 xpos = (boxRight + boxLeft) / 2;

	setOrthoProjection(xpos, ypos, 0);
	_engine->_interface->setClip(Common::Rect(boxLeft, boxTop, boxRight, boxBottom));

	if (angle == -1) {
		ActorMoveStruct &move = _engine->_menu->moveMenu;
		const int16 newAngle = move.getRealAngle(_engine->lbaTime);
		if (move.numOfStep == 0) {
			_engine->_movements->setActorAngleSafe(newAngle, newAngle - ANGLE_90, ANGLE_17, &move);
		}
		renderIsoModel(0, y, 0, ANGLE_0, newAngle, ANGLE_0, bodyPtr);
	} else {
		renderIsoModel(0, y, 0, ANGLE_0, angle, ANGLE_0, bodyPtr);
	}
}

void Renderer::renderInventoryItem(int32 x, int32 y, const uint8 *bodyPtr, int32 angle, int32 param) {
	setCameraPosition(x, y, 128, 200, 200);
	setCameraAngle(0, 0, 0, 60, 0, 0, param);

	renderIsoModel(0, 0, 0, ANGLE_0, angle, ANGLE_0, bodyPtr);
}

void Renderer::computeHolomapPolygon(int32 y1, int32 x1, int32 y2, int32 x2, int16 *polygonTabPtr) {
	int32 minY = y2;
	int32 minX = x1;
	if (y1 < y2) {
		minY = y1;
		y1 = y2;
		minX = x2;
		x2 = x1;
	}
	uint32 deltaY = y1 - minY;
	int16 *currentPolygonTabEntry = &polygonTabPtr[minY];
	if (x2 <= minX) {
		uint32 deltaX = (uint32)(uint16)((int16)minX - (int16)x2) << 0x10;
		uint32 deltaRatio = deltaX / deltaY;
		minY = deltaY + 1;
		deltaRatio = deltaRatio << 0x10 | deltaRatio >> 0x10;
		bool bVar5 = false;
		deltaY = (x2 & 0xffffU) |
				 (uint32)(uint16)(((uint16)(deltaX % deltaY >> 1) & 0x7fff) + 0x7fff) << 0x10;
		for (int32 y = 0; y < minY; ++y) {
			if (currentPolygonTabEntry >= _polyTab + _polyTabSize) {
				break;
			}
			*currentPolygonTabEntry++ = (int16)deltaY;
			deltaX = (uint32)bVar5;
			uint32 uVar1 = deltaY + deltaRatio;
			// CARRY4: Return true if there is an arithmetic overflow when adding 'x' and 'y' as unsigned integers.
			// bVar5 = CARRY4(deltaY, deltaRatio) || CARRY4(uVar1, deltaX);
			bVar5 = deltaY > deltaRatio || uVar1 > deltaX;
			deltaY = uVar1 + deltaX;
		}
	} else {
		uint32 deltaX = (uint32)(uint16)((int16)x2 - (int16)minX) << 0x10;
		uint32 deltaRatio = deltaX / deltaY;
		minY = deltaY + 1;
		deltaRatio = deltaRatio << 0x10 | deltaRatio >> 0x10;
		bool bVar5 = false;
		deltaY = (x2 & 0xffffU) | (uint32)(uint16)(((uint16)(deltaX % deltaY >> 1) & 0x7fff) + 0x7fff) << 0x10;
		for (int32 y = 0; y < minY; ++y) {
			if (currentPolygonTabEntry >= _polyTab + _polyTabSize) {
				break;
			}
			*currentPolygonTabEntry++ = (int16)deltaY;
			deltaX = (uint32)bVar5;
			uint32 uVar1 = deltaY - deltaRatio;
			bVar5 = deltaY < deltaRatio || uVar1 < deltaX;
			deltaY = uVar1 - deltaX;
		}
	}
}

static const int hmPolyOffset1 = 0;   /* 0x0000 */
static const int hmPolyOffset2 = 60;  /* 0x003c */
static const int hmPolyOffset3 = 120; /* 0x0078 */
static const int hmPolyOffset4 = 180; /* 0x00b4 */
static const int hmPolyOffset5 = 240; /* 0x00f0 */
static const int hmPolyOffset6 = 300; /* 0x012c */

void Renderer::fillHolomapPolygons(const Vertex &vertex1, const Vertex &vertex2, const Vertex &vertex3, const Vertex &vertex4, int32 &top, int32 &bottom) {
	const int32 yBottom = (int32)(uint16)vertex1.y;
	const int32 yTop = (int32)(uint16)vertex2.y;
	if (yBottom < yTop) {
		top = MIN<int32>(yBottom, top);
		bottom = MAX<int32>(yTop, bottom);
		computeHolomapPolygon(yTop, (uint16)vertex2.x, yBottom, (uint16)vertex1.x, &_polyTab[hmPolyOffset1]);
		computeHolomapPolygon(yTop, (uint16)vertex4.x, yBottom, (uint16)vertex3.x, &_polyTab[hmPolyOffset3]);
		computeHolomapPolygon(yTop, (uint16)vertex4.y, yBottom, (uint16)vertex3.y, &_polyTab[hmPolyOffset4]);
	} else if (yTop < yBottom) {
		top = MIN<int32>(yTop, top);
		bottom = MAX<int32>(yBottom, bottom);
		computeHolomapPolygon(yTop, (uint16)vertex2.x, yBottom, (uint16)vertex1.x, &_polyTab[hmPolyOffset2]);
		computeHolomapPolygon(yTop, (uint16)vertex4.x, yBottom, (uint16)vertex3.x, &_polyTab[hmPolyOffset5]);
		computeHolomapPolygon(yTop, (uint16)vertex4.y, yBottom, (uint16)vertex3.y, &_polyTab[hmPolyOffset6]);
	}
}

void Renderer::renderHolomapVertices(const Vertex vertexCoordinates[3], const Vertex vertexCoordinates2[3]) {
	int32 top = 32000;
	int32 bottom = -32000;
	fillHolomapPolygons(vertexCoordinates[0], vertexCoordinates[1], vertexCoordinates2[0], vertexCoordinates2[1], top, bottom);
	fillHolomapPolygons(vertexCoordinates[1], vertexCoordinates[2], vertexCoordinates2[1], vertexCoordinates2[2], top, bottom);
	fillHolomapPolygons(vertexCoordinates[2], vertexCoordinates[0], vertexCoordinates2[2], vertexCoordinates2[0], top, bottom);
	renderHolomapPolygons(top, bottom);
}

void Renderer::renderHolomapPolygons(int32 top, int32 bottom) {
	uint8 *out = (uint8 *)_engine->frontVideoBuffer.getBasePtr(0, top);
	int32 vsize = bottom - top + 1;
	const int16 *polyTabPtr = &_polyTab[top];
	int32 currentLine = top;
	const void* pixelBegin = _engine->frontVideoBuffer.getBasePtr(0, 0);
	const void* pixelEnd = _engine->frontVideoBuffer.getBasePtr(_engine->frontVideoBuffer.w - 1, _engine->frontVideoBuffer.h - 1);
	do {
		if (currentLine >= 0 && currentLine < _engine->height()) {
			const int32 polyTabVal = (int32)*polyTabPtr;
			uint8 *pixel = (uint8 *)(out + polyTabVal);
			const int32 yHeight = polyTabPtr[hmPolyOffset2] - polyTabVal;
			if (yHeight != 0 && polyTabVal <= polyTabPtr[hmPolyOffset2]) {
				const int32 polyTabVal2 = (int32)(1 - ((uint32)(uint16)polyTabPtr[hmPolyOffset4] -
													(uint32)(uint16)polyTabPtr[hmPolyOffset6])) /
										yHeight;
				uint32 uVar3 = (uint32)(uint16)polyTabPtr[hmPolyOffset3];
				const int32 iVar1 = (int32)(((uint16)polyTabPtr[hmPolyOffset5] - uVar3) + 1) / yHeight;
				uint16 uVar2 = *(const uint16 *)&polyTabPtr[hmPolyOffset4];
				// int16 holomap_maybe_DAT_00433430 = iVar2;
				// int16 holomap_maybe_DAT_00433434 = iVar1;
				for (int32 i = 0; i < yHeight; ++i) {
					const uint32 idx = ((uVar2 & 0xffffff00) | uVar3 >> 8);
					if (pixel < pixelBegin || pixel >= pixelEnd) {
						break;
					}
					if (idx >= _engine->_resources->holomapImageSize) {
						continue;
					}
					*pixel++ = _engine->_resources->holomapImagePtr[idx];
					uVar3 = (uint32)(uint16)((int16)uVar3 + (int16)iVar1);
					uVar2 = ((uint16)(uVar2 & 0xffffff00) | (uVar2 & 0xff)) + (int16)polyTabVal2;
				}
			}
		}
		out += _engine->frontVideoBuffer.w;
		++polyTabPtr;
		++currentLine;
	} while (--vsize);
}

} // namespace TwinE
