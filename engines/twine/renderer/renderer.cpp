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

int32 Renderer::projectPositionOnScreen(int32 cX, int32 cY, int32 cZ) {
	if (isUsingOrhoProjection) {
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

	int32 posZ = cZ + cameraPosX;
	if (posZ < 0) {
		posZ = 0x7FFF;
	}

	projPosX = (cX * cameraPosY) / posZ + orthoProjX;
	projPosY = (-cY * cameraPosZ) / posZ + orthoProjY;
	projPosZ = posZ;
	return -1;
}

void Renderer::setCameraPosition(int32 x, int32 y, int32 cX, int32 cY, int32 cZ) {
	orthoProjX = x;
	orthoProjY = y;

	cameraPosX = cX;
	cameraPosY = cY;
	cameraPosZ = cZ;

	isUsingOrhoProjection = false;
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

	isUsingOrhoProjection = true;
}

void Renderer::getBaseRotationPosition(int32 x, int32 y, int32 z) {
	destX = (baseMatrix.row1[0] * x + baseMatrix.row1[1] * y + baseMatrix.row1[2] * z) / 16384;
	destY = (baseMatrix.row2[0] * x + baseMatrix.row2[1] * y + baseMatrix.row2[2] * z) / 16384;
	destZ = (baseMatrix.row3[0] * x + baseMatrix.row3[1] * y + baseMatrix.row3[2] * z) / 16384;
}

void Renderer::setBaseRotation(int32 x, int32 y, int32 z) {
	const double Xradians = (double)((ANGLE_90 - x) % ANGLE_360) * 2 * M_PI / ANGLE_360;
	const double Yradians = (double)((ANGLE_90 - y) % ANGLE_360) * 2 * M_PI / ANGLE_360;
	const double Zradians = (double)((ANGLE_90 - z) % ANGLE_360) * 2 * M_PI / ANGLE_360;

	baseMatrix.row1[0] = (int32)(sin(Zradians) * sin(Yradians) * 16384);
	baseMatrix.row1[1] = (int32)(-cos(Zradians) * 16384);
	baseMatrix.row1[2] = (int32)(sin(Zradians) * cos(Yradians) * 16384);
	baseMatrix.row2[0] = (int32)(cos(Zradians) * sin(Xradians) * 16384);
	baseMatrix.row2[1] = (int32)(sin(Zradians) * sin(Xradians) * 16384);
	baseMatrix.row3[0] = (int32)(cos(Zradians) * cos(Xradians) * 16384);
	baseMatrix.row3[1] = (int32)(sin(Zradians) * cos(Xradians) * 16384);

	int32 matrixElem = baseMatrix.row2[0];

	baseMatrix.row2[0] = (int32)(sin(Yradians) * matrixElem + 16384 * cos(Yradians) * cos(Xradians));
	baseMatrix.row2[2] = (int32)(cos(Yradians) * matrixElem - 16384 * sin(Yradians) * cos(Xradians));

	matrixElem = baseMatrix.row3[0];

	baseMatrix.row3[0] = (int32)(sin(Yradians) * matrixElem - 16384 * sin(Xradians) * cos(Yradians));
	baseMatrix.row3[2] = (int32)(cos(Yradians) * matrixElem + 16384 * sin(Xradians) * sin(Yradians));

	getBaseRotationPosition(baseTransPosX, baseTransPosY, baseTransPosZ);

	baseRotPosX = destX;
	baseRotPosY = destY;
	baseRotPosZ = destZ;
}

void Renderer::getCameraAnglePositions(int32 x, int32 y, int32 z) {
	destX = (baseMatrix.row1[0] * x + baseMatrix.row2[0] * y + baseMatrix.row3[0] * z) / 16384;
	destY = (baseMatrix.row1[1] * x + baseMatrix.row2[1] * y + baseMatrix.row3[1] * z) / 16384;
	destZ = (baseMatrix.row1[2] * x + baseMatrix.row2[2] * y + baseMatrix.row3[2] * z) / 16384;
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

		matrix1.row1[1] = (currentMatrix->row1[2] * angleVar2 + currentMatrix->row1[1] * angleVar1) / 16384;
		matrix1.row1[2] = (currentMatrix->row1[2] * angleVar1 - currentMatrix->row1[1] * angleVar2) / 16384;
		matrix1.row2[1] = (currentMatrix->row2[2] * angleVar2 + currentMatrix->row2[1] * angleVar1) / 16384;
		matrix1.row2[2] = (currentMatrix->row2[2] * angleVar1 - currentMatrix->row2[1] * angleVar2) / 16384;
		matrix1.row3[1] = (currentMatrix->row3[2] * angleVar2 + currentMatrix->row3[1] * angleVar1) / 16384;
		matrix1.row3[2] = (currentMatrix->row3[2] * angleVar1 - currentMatrix->row3[1] * angleVar2) / 16384;
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

		matrix2.row1[0] = (matrix1.row1[1] * angleVar2 + matrix1.row1[0] * angleVar1) / 16384;
		matrix2.row1[1] = (matrix1.row1[1] * angleVar1 - matrix1.row1[0] * angleVar2) / 16384;
		matrix2.row2[0] = (matrix1.row2[1] * angleVar2 + matrix1.row2[0] * angleVar1) / 16384;
		matrix2.row2[1] = (matrix1.row2[1] * angleVar1 - matrix1.row2[0] * angleVar2) / 16384;
		matrix2.row3[0] = (matrix1.row3[1] * angleVar2 + matrix1.row3[0] * angleVar1) / 16384;
		matrix2.row3[1] = (matrix1.row3[1] * angleVar1 - matrix1.row3[0] * angleVar2) / 16384;
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

		targetMatrix->row1[0] = (matrix2.row1[0] * angleVar1 - matrix2.row1[2] * angleVar2) / 16384;
		targetMatrix->row1[2] = (matrix2.row1[0] * angleVar2 + matrix2.row1[2] * angleVar1) / 16384;
		targetMatrix->row2[0] = (matrix2.row2[0] * angleVar1 - matrix2.row2[2] * angleVar2) / 16384;
		targetMatrix->row2[2] = (matrix2.row2[0] * angleVar2 + matrix2.row2[2] * angleVar1) / 16384;

		targetMatrix->row3[0] = (matrix2.row3[0] * angleVar1 - matrix2.row3[2] * angleVar2) / 16384;
		targetMatrix->row3[2] = (matrix2.row3[0] * angleVar2 + matrix2.row3[2] * angleVar1) / 16384;
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

		destPoints->x = ((rotationMatrix->row1[0] * tmpX + rotationMatrix->row1[1] * tmpY + rotationMatrix->row1[2] * tmpZ) / 16384) + destX;
		destPoints->y = ((rotationMatrix->row2[0] * tmpX + rotationMatrix->row2[1] * tmpY + rotationMatrix->row2[2] * tmpZ) / 16384) + destY;
		destPoints->z = ((rotationMatrix->row3[0] * tmpX + rotationMatrix->row3[1] * tmpY + rotationMatrix->row3[2] * tmpZ) / 16384) + destZ;

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

		destPoints->x = ((translationMatrix->row1[0] * tmpX + translationMatrix->row1[1] * tmpY + translationMatrix->row1[2] * tmpZ) / 16384) + destX;
		destPoints->y = ((translationMatrix->row2[0] * tmpX + translationMatrix->row2[1] * tmpY + translationMatrix->row2[2] * tmpZ) / 16384) + destY;
		destPoints->z = ((translationMatrix->row3[0] * tmpX + translationMatrix->row3[1] * tmpY + translationMatrix->row3[2] * tmpZ) / 16384) + destZ;

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

void Renderer::translateGroup(int16 ax, int16 bx, int16 cx) {
	const int32 ebp = ax;
	const int32 ebx = bx;
	const int32 ecx = cx;

	int32 edi = shadeMatrix.row1[0];
	int32 eax = shadeMatrix.row1[1];
	edi *= ebp;
	eax *= ebx;
	edi += eax;
	eax = shadeMatrix.row1[2];
	eax *= ecx;
	eax += edi;
	eax /= 16384;

	destX = eax;

	edi = shadeMatrix.row2[0];
	eax = shadeMatrix.row2[1];
	edi *= ebp;
	eax *= ebx;
	edi += eax;
	eax = shadeMatrix.row2[2];
	eax *= ecx;
	eax += edi;
	eax /= 16384;
	destY = eax;
	destZ = eax;
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
		vertices[i].x = clamp(vertices[i].x, 0, SCREEN_WIDTH - 1);
		vertices[i].y = clamp(vertices[i].y, 0, SCREEN_HEIGHT - 1);
		const int vertexX = vertices[i].x;
		vleft = MIN(vleft, vertexX);
		vright = MAX(vright, vertexX);
		const int vertexY = vertices[i].y;
		vtop = MIN(vtop, vertexY);
		vbottom = MAX(vbottom, vertexY);
	}
}

void Renderer::computePolygons(int16 polyRenderType, Vertex *vertices, int32 numVertices, int &vleft, int &vright, int &vtop, int &vbottom) {
	if (numVertices <= 0) {
		return;
	}
	computeBoundingBox(vertices, numVertices, vleft, vright, vtop, vbottom);

	uint8 vertexParam1 = vertices[numVertices - 1].colorIndex;
	int16 currentVertexX = vertices[numVertices - 1].x;
	int16 currentVertexY = vertices[numVertices - 1].y;

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
			cvalue = (vertexParam2 << 8) + ((oldVertexParam - vertexParam2) << 8) % vsize;
			cdelta = ((oldVertexParam - vertexParam2) << 8) / vsize;
			direction = -direction; // we will draw by going down the tab
		} else {
			xpos = oldVertexX;
			ypos = oldVertexY;
			cvalue = (oldVertexParam << 8) + ((vertexParam2 - oldVertexParam) << 8) % vsize;
			cdelta = ((vertexParam2 - oldVertexParam) << 8) / vsize;
		}
		int16 *outPtr = &polyTab[ypos + (up ? SCREEN_HEIGHT : 0)]; // outPtr is the output ptr in the renderTab

		float slope = (float)hsize / (float)vsize;
		slope = up ? -slope : slope;

		for (int32 i = 0; i < vsize + 2; i++) {
			if (outPtr - polyTab < ARRAYSIZE(polyTab)) {
				if (outPtr - polyTab > 0) {
					*outPtr = xpos;
				}
			}
			outPtr += direction;
			xpos += slope;
		}

		if (polyRenderType >= POLYGONTYPE_GOURAUD) { // we must compute the color progression
			int16 *outPtr2 = &polyTab2[ypos + (up ? SCREEN_HEIGHT : 0)];

			for (int32 i = 0; i < vsize + 2; i++) {
				if (outPtr2 - polyTab2 < ARRAYSIZE(polyTab2)) {
					if (outPtr2 - polyTab2 > 0) {
						*outPtr2 = cvalue;
					}
				}
				outPtr2 += direction;
				cvalue += cdelta;
			}
		}
	}
}

void Renderer::renderPolygonsCopper(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &polyTab[vtop];
	int32 currentLine = vtop;
	do {
		if (currentLine >= 0 && currentLine < SCREEN_HEIGHT) {
			int16 start = ptr1[0];
			int16 stop = ptr1[SCREEN_HEIGHT];

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
					start = (start & 0xFF00) | ((start & 0xFF) & (uint8)(dx >> 8));
					start = (start & 0xFF00) | ((start & 0xFF) + (dx & 0xFF));
					if (j >= 0 && j < SCREEN_WIDTH) {
						out[j] = start & 0xFF;
					}
					mask = (mask << 2) | (mask >> 14);
					mask++;
				}
			}
		}
		out += SCREEN_WIDTH;
		currentLine++;
	} while (--vsize);
}

void Renderer::renderPolygonsBopper(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &polyTab[vtop];
	int32 currentLine = vtop;
	do {
		if (currentLine >= 0 && currentLine < SCREEN_HEIGHT) {
			int16 start = ptr1[0];
			int16 stop = ptr1[SCREEN_HEIGHT];
			ptr1++;
			int32 hsize = stop - start;

			if (hsize >= 0) {
				hsize++;
				for (int32 j = start; j < hsize + start; j++) {
					if ((start + (vtop % 1)) & 1) {
						if (j >= 0 && j < SCREEN_WIDTH) {
							out[j] = color;
						}
					}
				}
			}
		}
		out += SCREEN_WIDTH;
		currentLine++;
	} while (--vsize);
}

void Renderer::renderPolygonsFlat(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &polyTab[vtop];
	int32 currentLine = vtop;
	do {
		if (currentLine >= 0 && currentLine < SCREEN_HEIGHT) {
			int16 start = ptr1[0];
			int16 stop = ptr1[SCREEN_HEIGHT];
			ptr1++;
			int32 hsize = stop - start;

			if (hsize >= 0) {
				hsize++;
				for (int32 j = start; j < hsize + start; j++) {
					if (j >= 0 && j < SCREEN_WIDTH) {
						out[j] = color;
					}
				}
			}
		}
		out += SCREEN_WIDTH;
		currentLine++;
	} while (--vsize);
}

void Renderer::renderPolygonsTele(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &polyTab[vtop];
	int bx = (uint16)color << 16;
	int32 renderLoop = vsize;
	do {
		int16 start;
		int16 stop;
		int32 hsize;
		while (1) {
			start = ptr1[0];
			stop = ptr1[SCREEN_HEIGHT];
			ptr1++;
			hsize = stop - start;

			if (hsize) {
				break;
			}

			uint8 *out2 = start + out;
			*out2 = ((unsigned short)(bx >> 0x18)) & 0x0F;

			color = *(out2 + 1);

			out += SCREEN_WIDTH;

			--renderLoop;
			if (!renderLoop) {
				return;
			}
		}

		if (stop >= start) {
			hsize++;
			bx = (unsigned short)(color >> 0x10);
			uint8 *out2 = start + out;

			int ax = (bx & 0xF0) << 8;
			bx = bx << 8;
			ax += (bx & 0x0F);
			ax -= bx;
			ax++;
			ax = ax >> 16;

			ax = ax / hsize;
			uint16 temp = (ax & 0xF0);
			temp = temp >> 8;
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

		out += SCREEN_WIDTH;
		--renderLoop;

	} while (renderLoop);
}

// FIXME: buggy
void Renderer::renderPolygonsTras(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &polyTab[vtop];
	do {
		unsigned short int bx;

		int16 start = ptr1[0];
		int16 stop = ptr1[SCREEN_HEIGHT];

		ptr1++;
		int32 hsize = stop - start;

		if (hsize >= 0) {
			hsize++;
			uint8 *out2 = start + out;

			if ((hsize >> 1) < 0) {
				bx = color & 0xFF;
				bx = bx << 8;
				bx += color & 0xFF;
				for (int32 j = 0; j < hsize; j++) {
					*(out2) = (*(out2)&0x0F0F) | bx;
				}
			} else {
				*(out2) = (*(out2)&0x0F) | color;
				out2++;
			}
		}
		out += SCREEN_WIDTH;
	} while (--vsize);
}

// FIXME: buggy
void Renderer::renderPolygonTrame(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &polyTab[vtop];
	unsigned char bh = 0;

	int32 currentLine = vtop;
	do {
		if (currentLine >= 0 && currentLine < SCREEN_HEIGHT) {
			int16 start = ptr1[0];
			int16 stop = ptr1[SCREEN_HEIGHT];
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
		out += SCREEN_WIDTH;
		currentLine++;
	} while (--vsize);
}

void Renderer::renderPolygonsGouraud(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &polyTab[vtop];
	const int16 *ptr2 = &polyTab2[vtop];
	int32 renderLoop = vsize;
	int32 currentLine = vtop;
	do {
		if (currentLine >= 0 && currentLine < SCREEN_HEIGHT) {
			uint16 startColor = ptr2[0];
			uint16 stopColor = ptr2[SCREEN_HEIGHT];

			int16 colorSize = stopColor - startColor;

			int16 stop = ptr1[SCREEN_HEIGHT]; // stop
			int16 start = ptr1[0];            // start

			ptr1++;
			uint8 *out2 = start + out;
			int32 hsize = stop - start;

			//varf2 = ptr2[SCREEN_HEIGHT];
			//varf3 = ptr2[0];

			ptr2++;

			//varf4 = (float)((int32)varf2 - (int32)varf3);

			if (hsize == 0) {
				if (start >= 0 && start < SCREEN_WIDTH) {
					*out2 = ((startColor + stopColor) / 2) >> 8; // moyenne des 2 couleurs
				}
			} else if (hsize > 0) {
				if (hsize == 1) {
					if (start >= -1 && start < SCREEN_WIDTH - 1) {
						*(out2 + 1) = stopColor >> 8;
					}

					if (start >= 0 && start < SCREEN_WIDTH) {
						*(out2) = startColor >> 8;
					}
				} else if (hsize == 2) {
					if (start >= -2 && start < SCREEN_WIDTH - 2) {
						*(out2 + 2) = stopColor >> 8;
					}

					if (start >= -1 && start < SCREEN_WIDTH - 1) {
						*(out2 + 1) = ((startColor + stopColor) / 2) >> 8;
					}

					if (start >= 0 && start < SCREEN_WIDTH) {
						*(out2) = startColor >> 8;
					}
				} else {
					int32 currentXPos = start;
					colorSize /= hsize;
					hsize++;

					if (hsize % 2) {
						hsize /= 2;
						if (currentXPos >= 0 && currentXPos < SCREEN_WIDTH) {
							*(out2) = startColor >> 8;
						}
						out2++;
						currentXPos++;
						startColor += colorSize;
					} else {
						hsize /= 2;
					}

					do {
						if (currentXPos >= 0 && currentXPos < SCREEN_WIDTH) {
							*(out2) = startColor >> 8;
						}

						currentXPos++;
						startColor += colorSize;

						if (currentXPos >= 0 && currentXPos < SCREEN_WIDTH) {
							*(out2 + 1) = startColor >> 8;
						}

						currentXPos++;
						out2 += 2;
						startColor += colorSize;
					} while (--hsize);
				}
			}
		}
		out += SCREEN_WIDTH;
		currentLine++;
	} while (--renderLoop);
}

void Renderer::renderPolygonsDither(uint8 *out, int vtop, int32 vsize, int32 color) const {
	const int16 *ptr1 = &polyTab[vtop];
	const int16 *ptr2 = &polyTab2[vtop];
	int32 renderLoop = vsize;

	int32 currentLine = vtop;
	do {
		if (currentLine >= 0 && currentLine < SCREEN_HEIGHT) {
			int16 stop = ptr1[SCREEN_HEIGHT]; // stop
			int16 start = ptr1[0];            // start
			ptr1++;
			int32 hsize = stop - start;

			if (hsize >= 0) {
				uint16 startColor = ptr2[0];
				uint16 stopColor = ptr2[SCREEN_HEIGHT];
				int32 currentXPos = start;

				uint8 *out2 = start + out;
				ptr2++;

				if (hsize == 0) {
					if (currentXPos >= 0 && currentXPos < SCREEN_WIDTH) {
						*(out2) = (uint8)(((startColor + stopColor) / 2) >> 8);
					}
				} else {
					int16 colorSize = stopColor - startColor;
					if (hsize == 1) {
						uint16 currentColor = startColor;
						hsize++;
						hsize /= 2;

						currentColor &= 0xFF;
						currentColor += startColor;
						if (currentXPos >= 0 && currentXPos < SCREEN_WIDTH) {
							*(out2) = currentColor >> 8;
						}

						currentColor &= 0xFF;
						startColor += colorSize;
						currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
						currentColor += startColor;

						currentXPos++;
						if (currentXPos >= 0 && currentXPos < SCREEN_WIDTH) {
							*(out2 + 1) = currentColor >> 8;
						}
					} else if (hsize == 2) {
						uint16 currentColor = startColor;
						hsize++;
						hsize /= 2;

						currentColor &= 0xFF;
						colorSize /= 2;
						currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
						currentColor += startColor;
						if (currentXPos >= 0 && currentXPos < SCREEN_WIDTH) {
							*(out2) = currentColor >> 8;
						}

						out2++;
						currentXPos++;
						startColor += colorSize;

						currentColor &= 0xFF;
						currentColor += startColor;

						if (currentXPos >= 0 && currentXPos < SCREEN_WIDTH) {
							*(out2) = currentColor >> 8;
						}

						currentColor &= 0xFF;
						startColor += colorSize;
						currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
						currentColor += startColor;

						currentXPos++;
						if (currentXPos >= 0 && currentXPos < SCREEN_WIDTH) {
							*(out2 + 1) = currentColor >> 8;
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
							if (currentXPos >= 0 && currentXPos < SCREEN_WIDTH) {
								*(out2) = currentColor >> 8;
							}
							out2++;
							currentXPos++;
						} else {
							hsize /= 2;
						}

						do {
							currentColor &= 0xFF;
							currentColor += startColor;
							if (currentXPos >= 0 && currentXPos < SCREEN_WIDTH) {
								*(out2) = currentColor >> 8;
							}
							currentXPos++;
							currentColor &= 0xFF;
							startColor += colorSize;
							currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
							currentColor += startColor;
							if (currentXPos >= 0 && currentXPos < SCREEN_WIDTH) {
								*(out2 + 1) = currentColor >> 8;
							}
							currentXPos++;
							out2 += 2;
							startColor += colorSize;
						} while (--hsize);
					}
				}
			}
		}
		out += SCREEN_WIDTH;
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
	computePolygons(polygon.renderType, vertices, polygon.numVertices, vleft, vright, vtop, vbottom);

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
		warning("RENDER WARNING: Unsuported render type %d", polygon.renderType);
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

			if (isUsingOrhoProjection) {
				radius = (radius * 34) >> 9;
			} else {
				radius = (radius * cameraPosY) / (cameraPosX + *(const int16 *)pointer); // TODO: this does not make sense.
			}

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

	if (isUsingOrhoProjection) { // use standard projection
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

			coZ += cameraPosX;

			if (coZ <= 0) {
				coZ = 0x7FFFFFFF;
			}

			// X projection
			{
				coX = orthoProjX + ((coX * cameraPosY) / coZ);

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
				coY = orthoProjY + ((-coY * cameraPosZ) / coZ);

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

	if (isUsingOrhoProjection) {
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
	int32 tmpBoxRight = boxRight;

	int32 ypos = boxBottom + boxTop;
	ypos >>= 1;

	int32 xpos = boxRight + boxLeft;
	xpos >>= 1;

	setOrthoProjection(xpos, ypos, 0);
	_engine->_interface->setClip(Common::Rect(boxLeft, boxTop, tmpBoxRight, boxBottom));

	if (angle == -1) {
		ActorMoveStruct &move = _engine->_menu->moveMenu;
		const int16 newAngle = move.getRealAngle(_engine->lbaTime);
		if (move.numOfStep == 0) {
			_engine->_movements->setActorAngleSafe(newAngle, newAngle - ANGLE_90, ANGLE_17, &move);
		}
		renderIsoModel(0, y, 0, 0, newAngle, 0, bodyPtr);
	} else {
		renderIsoModel(0, y, 0, 0, angle, 0, bodyPtr);
	}
}

void Renderer::renderInventoryItem(int32 x, int32 y, const uint8 *bodyPtr, int32 angle, int32 param) {
	setCameraPosition(x, y, 128, 200, 200);
	setCameraAngle(0, 0, 0, 60, 0, 0, param);

	renderIsoModel(0, 0, 0, 0, angle, 0, bodyPtr);
}

} // namespace TwinE
