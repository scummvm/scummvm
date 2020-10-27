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

#include "twine/renderer.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "twine/interface.h"
#include "twine/menu.h"
#include "twine/movements.h"
#include "twine/redraw.h"
#include "twine/shadeangletab.h"
#include "twine/twine.h"

namespace TwinE {

#define RENDERTYPE_DRAWLINE 0
#define RENDERTYPE_DRAWPOLYGON 1
#define RENDERTYPE_DRAWSPHERE 2

#define POLYGONTYPE_FLAT 0
#define POLYGONTYPE_COPPER 1
#define POLYGONTYPE_BOPPER 2
#define POLYGONTYPE_MARBLE 3
#define POLYGONTYPE_TELE 4
#define POLYGONTYPE_TRAS 5
#define POLYGONTYPE_TRAME 6
#define POLYGONTYPE_GOURAUD 7
#define POLYGONTYPE_DITHER 8

#define ERROR_OUT_OF_SCREEN 2

int32 Renderer::projectPositionOnScreen(int32 cX, int32 cY, int32 cZ) {
	if (!isUsingOrhoProjection) {
		cX -= baseRotPosX;
		cY -= baseRotPosY;
		cZ -= baseRotPosZ;

		if (cZ >= 0) {
			int32 posZ = cZ + cameraPosX;

			if (posZ < 0)
				posZ = 0x7FFF;

			projPosX = (cX * cameraPosY) / posZ + orthoProjX;
			projPosY = (-cY * cameraPosZ) / posZ + orthoProjY;
			projPosZ = posZ;
			return -1;
		}

		projPosX = 0;
		projPosY = 0;
		projPosZ = 0;
		return 0;
	}
	projPosX = ((cX - cZ) * 24) / 512 + orthoProjX;
	projPosY = (((cX + cZ) * 12) - cY * 30) / 512 + orthoProjY;
	projPosZ = cZ - cY - cX;

	return 1;
}

void Renderer::setCameraPosition(int32 x, int32 y, int32 cX, int32 cY, int32 cZ) {
	orthoProjX = x;
	orthoProjY = y;

	cameraPosX = cX;
	cameraPosY = cY;
	cameraPosZ = cZ;

	isUsingOrhoProjection = 0;
}

void Renderer::setBaseTranslation(int32 x, int32 y, int32 z) {
	baseTransPosX = x;
	baseTransPosY = y;
	baseTransPosZ = z;
}

void Renderer::setOrthoProjection(int32 X, int32 Y, int32 Z) {
	orthoProjX = X;
	orthoProjY = Y;
	orthoProjZ = Z;

	isUsingOrhoProjection = 1;
}

void Renderer::getBaseRotationPosition(int32 x, int32 y, int32 z) {
	destX = (baseMatrix[0] * x + baseMatrix[1] * y + baseMatrix[2] * z) >> 14;
	destY = (baseMatrix[3] * x + baseMatrix[4] * y + baseMatrix[5] * z) >> 14;
	destZ = (baseMatrix[6] * x + baseMatrix[7] * y + baseMatrix[8] * z) >> 14;
}

void Renderer::setBaseRotation(int32 x, int32 y, int32 z) {
	shadeAngleTab3 = &shadeAngleTable[384];

	baseMatrixRotationX = x & 0x3FF;
	baseMatrixRotationY = y & 0x3FF;
	baseMatrixRotationZ = z & 0x3FF;

	double Xradians = (double)((256 - x) % 1024) * 2 * M_PI / 1024;
	double Yradians = (double)((256 - y) % 1024) * 2 * M_PI / 1024;
	double Zradians = (double)((256 - z) % 1024) * 2 * M_PI / 1024;

	baseMatrix[0] = (int32)(sin(Zradians) * sin(Yradians) * 16384);
	baseMatrix[1] = (int32)(-cos(Zradians) * 16384);
	baseMatrix[2] = (int32)(sin(Zradians) * cos(Yradians) * 16384);
	baseMatrix[3] = (int32)(cos(Zradians) * sin(Xradians) * 16384);
	baseMatrix[4] = (int32)(sin(Zradians) * sin(Xradians) * 16384);
	baseMatrix[6] = (int32)(cos(Zradians) * cos(Xradians) * 16384);
	baseMatrix[7] = (int32)(sin(Zradians) * cos(Xradians) * 16384);

	int32 matrixElem = baseMatrix[3];

	baseMatrix[3] = (int32)(sin(Yradians) * matrixElem + 16384 * cos(Yradians) * cos(Xradians));
	baseMatrix[5] = (int32)(cos(Yradians) * matrixElem - 16384 * sin(Yradians) * cos(Xradians));

	matrixElem = baseMatrix[6];

	baseMatrix[6] = (int32)(sin(Yradians) * matrixElem - 16384 * sin(Xradians) * cos(Yradians));
	baseMatrix[8] = (int32)(cos(Yradians) * matrixElem + 16384 * sin(Xradians) * sin(Yradians));

	getBaseRotationPosition(baseTransPosX, baseTransPosY, baseTransPosZ);

	baseRotPosX = destX;
	baseRotPosY = destY;
	baseRotPosZ = destZ;
}

void Renderer::getCameraAnglePositions(int32 x, int32 y, int32 z) {
	destX = (baseMatrix[0] * x + baseMatrix[3] * y + baseMatrix[6] * z) >> 14;
	destY = (baseMatrix[1] * x + baseMatrix[4] * y + baseMatrix[7] * z) >> 14;
	destZ = (baseMatrix[2] * x + baseMatrix[5] * y + baseMatrix[8] * z) >> 14;
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

void Renderer::applyRotation(int32 *tempMatrix, int32 *currentMatrix) {
	int32 matrix1[9];
	int32 matrix2[9];

	if (renderAngleX) {
		int32 angle = renderAngleX;
		int32 angleVar2 = shadeAngleTable[angle & 0x3FF];
		angle += 0x100;
		int32 angleVar1 = shadeAngleTable[angle & 0x3FF];

		matrix1[0] = currentMatrix[0];
		matrix1[3] = currentMatrix[3];
		matrix1[6] = currentMatrix[6];

		matrix1[1] = (currentMatrix[2] * angleVar2 + currentMatrix[1] * angleVar1) >> 14;
		matrix1[2] = (currentMatrix[2] * angleVar1 - currentMatrix[1] * angleVar2) >> 14;
		matrix1[4] = (currentMatrix[5] * angleVar2 + currentMatrix[4] * angleVar1) >> 14;
		matrix1[5] = (currentMatrix[5] * angleVar1 - currentMatrix[4] * angleVar2) >> 14;
		matrix1[7] = (currentMatrix[8] * angleVar2 + currentMatrix[7] * angleVar1) >> 14;
		matrix1[8] = (currentMatrix[8] * angleVar1 - currentMatrix[7] * angleVar2) >> 14;
	} else {
		for (int32 i = 0; i < 9; i++)
			matrix1[i] = currentMatrix[i];
	}

	if (renderAngleZ) {
		int32 angle = renderAngleZ;
		int32 angleVar2 = shadeAngleTable[angle & 0x3FF];
		angle += 0x100;
		int32 angleVar1 = shadeAngleTable[angle & 0x3FF];

		matrix2[2] = matrix1[2];
		matrix2[5] = matrix1[5];
		matrix2[8] = matrix1[8];

		matrix2[0] = (matrix1[1] * angleVar2 + matrix1[0] * angleVar1) >> 14;
		matrix2[1] = (matrix1[1] * angleVar1 - matrix1[0] * angleVar2) >> 14;
		matrix2[3] = (matrix1[4] * angleVar2 + matrix1[3] * angleVar1) >> 14;
		matrix2[4] = (matrix1[4] * angleVar1 - matrix1[3] * angleVar2) >> 14;
		matrix2[6] = (matrix1[7] * angleVar2 + matrix1[6] * angleVar1) >> 14;
		matrix2[7] = (matrix1[7] * angleVar1 - matrix1[6] * angleVar2) >> 14;
	} else {
		for (int32 i = 0; i < 9; i++)
			matrix2[i] = matrix1[i];
	}

	if (renderAngleY) {
		int32 angle = renderAngleY;
		int32 angleVar2 = shadeAngleTable[angle & 0x3FF]; // esi
		angle += 0x100;
		int32 angleVar1 = shadeAngleTable[angle & 0x3FF]; // ecx

		tempMatrix[1] = matrix2[1];
		tempMatrix[4] = matrix2[4];
		tempMatrix[7] = matrix2[7];

		tempMatrix[0] = (matrix2[0] * angleVar1 - matrix2[2] * angleVar2) >> 14;
		tempMatrix[2] = (matrix2[0] * angleVar2 + matrix2[2] * angleVar1) >> 14;
		tempMatrix[3] = (matrix2[3] * angleVar1 - matrix2[5] * angleVar2) >> 14;
		tempMatrix[5] = (matrix2[3] * angleVar2 + matrix2[5] * angleVar1) >> 14;

		tempMatrix[6] = (matrix2[6] * angleVar1 - matrix2[8] * angleVar2) >> 14;
		tempMatrix[8] = (matrix2[6] * angleVar2 + matrix2[8] * angleVar1) >> 14;
	} else {
		for (int32 i = 0; i < 9; i++)
			tempMatrix[i] = matrix2[i];
	}
}

void Renderer::applyPointsRotation(uint8 *firstPointsPtr, int32 numPoints, pointTab *destPoints, int32 *rotationMatrix) {
	int32 numOfPoints2 = numPoints;

	do {
		uint8 *pointsPtr2 = firstPointsPtr;
		const int16 *tempPtr = (int16 *)(firstPointsPtr);

		const int16 tmpX = tempPtr[0];
		const int16 tmpY = tempPtr[1];
		const int16 tmpZ = tempPtr[2];

		destPoints->X = ((rotationMatrix[0] * tmpX + rotationMatrix[1] * tmpY + rotationMatrix[2] * tmpZ) >> 14) + destX;
		destPoints->Y = ((rotationMatrix[3] * tmpX + rotationMatrix[4] * tmpY + rotationMatrix[5] * tmpZ) >> 14) + destY;
		destPoints->Z = ((rotationMatrix[6] * tmpX + rotationMatrix[7] * tmpY + rotationMatrix[8] * tmpZ) >> 14) + destZ;

		destPoints++;
		firstPointsPtr = pointsPtr2 + 6;
	} while (--numOfPoints2);
}

void Renderer::processRotatedElement(int32 rotZ, int32 rotY, int32 rotX, elementEntry *elemPtr) { // unsigned char * elemPtr) // loadPart
	int32 firstPoint = elemPtr->firstPoint;
	int32 numOfPoints2 = elemPtr->numOfPoints;

	renderAngleX = rotX;
	renderAngleY = rotY;
	renderAngleZ = rotZ;

	if (firstPoint % 6) {
		error("RENDER ERROR: invalid firstPoint in process_rotated_element func");
	}

	//baseElement = *((unsigned short int*)elemPtr+6);
	const int16 baseElement = elemPtr->baseElement;

	int32 *currentMatrix;
	// if its the first point
	if (baseElement == -1) {
		currentMatrix = baseMatrix;

		destX = 0;
		destY = 0;
		destZ = 0;
	} else {
		int32 pointIdx = (elemPtr->basePoint) / 6;
		currentMatrix = (int32 *)((uint8 *)matricesTable + baseElement);

		destX = computedPoints[pointIdx].X;
		destY = computedPoints[pointIdx].Y;
		destZ = computedPoints[pointIdx].Z;
	}

	applyRotation((int32 *)currentMatrixTableEntry, currentMatrix);

	if (!numOfPoints2) {
		warning("RENDER WARNING: No points in this model!");
	}

	applyPointsRotation(pointsPtr + firstPoint, numOfPoints2, &computedPoints[firstPoint / 6], (int32 *)currentMatrixTableEntry);
}

void Renderer::applyPointsTranslation(uint8 *firstPointsPtr, int32 numPoints, pointTab *destPoints, int32 *translationMatrix) {
	int32 numOfPoints2 = numPoints;

	do {
		uint8 *pointsPtr2 = firstPointsPtr;
		int16 *tempPtr = (int16 *)(firstPointsPtr);

		const int16 tmpX = tempPtr[0] + renderAngleZ;
		const int16 tmpY = tempPtr[1] + renderAngleY;
		const int16 tmpZ = tempPtr[2] + renderAngleX;

		destPoints->X = ((translationMatrix[0] * tmpX + translationMatrix[1] * tmpY + translationMatrix[2] * tmpZ) >> 14) + destX;
		destPoints->Y = ((translationMatrix[3] * tmpX + translationMatrix[4] * tmpY + translationMatrix[5] * tmpZ) >> 14) + destY;
		destPoints->Z = ((translationMatrix[6] * tmpX + translationMatrix[7] * tmpY + translationMatrix[8] * tmpZ) >> 14) + destZ;

		destPoints++;
		firstPointsPtr = pointsPtr2 + 6;
	} while (--numOfPoints2);
}

void Renderer::processTranslatedElement(int32 rotX, int32 rotY, int32 rotZ, elementEntry *elemPtr) {
	renderAngleX = rotX;
	renderAngleY = rotY;
	renderAngleZ = rotZ;

	if (elemPtr->baseElement == -1) { // base point
		destX = 0;
		destY = 0;
		destZ = 0;

		int32 *dest = (int32 *)currentMatrixTableEntry;

		for (int32 i = 0; i < 9; i++)
			dest[i] = baseMatrix[i];
	} else { // dependent
		destX = computedPoints[(elemPtr->basePoint) / 6].X;
		destY = computedPoints[(elemPtr->basePoint) / 6].Y;
		destZ = computedPoints[(elemPtr->basePoint) / 6].Z;

		const int32 *source = (const int32 *)((const uint8 *)matricesTable + elemPtr->baseElement);
		int32 *dest = (int32 *)currentMatrixTableEntry;

		for (int32 i = 0; i < 9; i++)
			dest[i] = source[i];
	}

	applyPointsTranslation(pointsPtr + elemPtr->firstPoint, elemPtr->numOfPoints, &computedPoints[elemPtr->firstPoint / 6], (int32 *)currentMatrixTableEntry);
}

void Renderer::translateGroup(int16 ax, int16 bx, int16 cx) {
	int32 ebp = ax;
	int32 ebx = bx;
	int32 ecx = cx;

	int32 edi = shadeMatrix[0];
	int32 eax = shadeMatrix[1];
	edi *= ebp;
	eax *= ebx;
	edi += eax;
	eax = shadeMatrix[2];
	eax *= ecx;
	eax += edi;
	eax >>= 14;

	destX = eax;

	edi = shadeMatrix[3];
	eax = shadeMatrix[4];
	edi *= ebp;
	eax *= ebx;
	edi += eax;
	eax = shadeMatrix[5];
	eax *= ecx;
	eax += edi;
	eax >>= 14;
	destY = eax;

	ebp *= shadeMatrix[6];
	ebx *= shadeMatrix[7];
	ecx *= shadeMatrix[8];
	ebx += ebp;
	ebx += ecx;
	ebx >>= 14;
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

	applyRotation(shadeMatrix, baseMatrix);
	translateGroup(0, 0, 59);

	lightX = destX;
	lightY = destY;
	lightZ = destZ;
}

FORCEINLINE int16 clamp(int16 x, int16 a, int16 b) {
	return x < a ? a : (x > b ? b : x);
}

int32 Renderer::computePolygons() {
	int16 *outPtr;
	int32 i, nVertex;
	int8 direction, up;
	int64 slope;
	vertexData *vertices;

	pRenderV1 = vertexCoordinates;
	pRenderV2 = pRenderV3;

	vertices = (vertexData *)vertexCoordinates;

	vleft = vtop = 32767;
	vright = vbottom = -32768;

	for (i = 0; i < numOfVertex; i++) {
		vertices[i].x = clamp(vertices[i].x, 0, SCREEN_WIDTH - 1);
		int16 vertexX = vertices[i].x;

		if (vertexX < vleft)
			vleft = vertexX;
		if (vertexX > vright)
			vright = vertexX;

		vertices[i].y = clamp(vertices[i].y, 0, SCREEN_HEIGHT - 1);
		int16 vertexY = vertices[i].y;
		if (vertexY < vtop)
			vtop = vertexY;
		if (vertexY > vbottom)
			vbottom = vertexY;
	}

	vertexParam1 = vertexParam2 = vertices[numOfVertex - 1].param;
	int16 currentVertexX = vertices[numOfVertex - 1].x;
	int16 currentVertexY = vertices[numOfVertex - 1].y;

	for (nVertex = 0; nVertex < numOfVertex; nVertex++) {
		int16 oldVertexY = currentVertexY;
		int16 oldVertexX = currentVertexX;
		oldVertexParam = vertexParam1;

		vertexParam1 = vertexParam2 = vertices[nVertex].param;
		currentVertexX = vertices[nVertex].x;
		currentVertexY = vertices[nVertex].y;

		// drawLine(oldVertexX,oldVertexY,currentVertexX,currentVertexY,255);

		if (currentVertexY == oldVertexY)
			continue;

		up = currentVertexY < oldVertexY;
		direction = up ? -1 : 1;

		int16 vsize = ABS(currentVertexY - oldVertexY);
		int16 hsize = ABS(currentVertexX - oldVertexX);

		int16 cvalue;
		int16 cdelta;
		int16 ypos;
		int16 xpos;
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
		outPtr = &polyTab[ypos + (up ? 480 : 0)]; // outPtr is the output ptr in the renderTab

		slope = (int64)hsize / (int64)vsize;
		slope = up ? -slope : slope;

		for (i = 0; i < vsize + 2; i++) {
			if ((outPtr - polyTab) < 960)
				if ((outPtr - polyTab) > 0)
					*(outPtr) = xpos;
			outPtr += direction;
			xpos += slope;
		}

		if (polyRenderType >= 7) { // we must compute the color progression
			int16 *outPtr2 = &polyTab2[ypos + (up ? 480 : 0)];

			for (i = 0; i < vsize + 2; i++) {
				if ((outPtr2 - polyTab2) < 960)
					if ((outPtr2 - polyTab2) > 0)
						*(outPtr2) = cvalue;
				outPtr2 += direction;
				cvalue += cdelta;
			}
		}
	}

	return (1);
}

void Renderer::renderPolygons(int32 renderType, int32 color) {
	uint8 *out, *out2;
	int16 *ptr1;
	int16 *ptr2;
	int32 vsize, hsize;
	int32 j;
	int32 currentLine;

	int16 start, stop;

	out = (uint8*)_engine->frontVideoBuffer.getPixels() + 640 * vtop;

	ptr1 = &polyTab[vtop];
	ptr2 = &polyTab2[vtop];

	vsize = vbottom - vtop;
	vsize++;

	switch (renderType) {
	case POLYGONTYPE_FLAT: {
		currentLine = vtop;
		do {
			if (currentLine >= 0 && currentLine < 480) {
				stop = ptr1[480];
				start = ptr1[0];

				ptr1++;
				hsize = stop - start;

				if (hsize >= 0) {
					hsize++;
					out2 = start + out;

					for (j = start; j < hsize + start; j++) {
						if (j >= 0 && j < 640)
							out[j] = color;
					}
				}
			}
			out += 640;
			currentLine++;
		} while (--vsize);
		break;
	}
	case POLYGONTYPE_COPPER: {
		currentLine = vtop;
		do {
			if (currentLine >= 0 && currentLine < 480) {
				start = ptr1[0];
				stop = ptr1[480];

				ptr1++;
				hsize = stop - start;

				if (hsize >= 0) {
					uint16 mask = 0x43DB;
					uint16 dx;
					int32 startCopy;

					dx = (uint8)color;
					dx |= 0x300;

					hsize++;
					out2 = start + out;
					startCopy = start;

					for (j = startCopy; j < hsize + startCopy; j++) {
						start += mask;
						start = (start & 0xFF00) | ((start & 0xFF) & (uint8)(dx >> 8));
						start = (start & 0xFF00) | ((start & 0xFF) + (dx & 0xFF));
						if (j >= 0 && j < 640) {
							out[j] = start & 0xFF;
						}
						mask = (mask << 2) | (mask >> 14);
						mask++;
					}
				}
			}
			out += 640;
			currentLine++;
		} while (--vsize);
		break;
	}
	case POLYGONTYPE_BOPPER: { // FIXME: buggy
		currentLine = vtop;
		do {
			if (currentLine >= 0 && currentLine < 480) {
				start = ptr1[0];
				stop = ptr1[480];
				ptr1++;
				hsize = stop - start;

				if (hsize >= 0) {
					hsize++;
					out2 = start + out;
					for (j = start; j < hsize + start; j++) {
						if ((start + (vtop % 1)) & 1) {
							if (j >= 0 && j < 640) {
								out[j] = color;
							}
						}
						out2++;
					}
				}
			}
			out += 640;
			currentLine++;
		} while (--vsize);
		break;
	}
	case POLYGONTYPE_MARBLE: { // TODO: implement this
		break;
	}
	case POLYGONTYPE_TELE: { // FIXME: buggy
		int ax;
		int bx;
		unsigned short int dx;
		unsigned short int temp;
		bx = (unsigned short)color << 0x10;
		renderLoop = vsize;
		do {
			while (1) {
				start = ptr1[0];
				stop = ptr1[480];
				ptr1++;
				hsize = stop - start;

				if (hsize)
					break;

				out2 = start + out;
				*(out2) = ((unsigned short)(bx >> 0x18)) & 0x0F;

				color = *(out2 + 1);

				out += 640;

				--renderLoop;
				if (!renderLoop)
					return;
			}

			if (stop >= start) {
				hsize++;
				bx = (unsigned short)(color >> 0x10);
				out2 = start + out;

				ax = (bx & 0xF0) << 8;
				bx = bx << 8;
				ax += (bx & 0x0F);
				ax -= bx;
				ax++;
				ax = ax >> 16;

				ax = ax / hsize;
				temp = (ax & 0xF0);
				temp = temp >> 8;
				temp += (ax & 0x0F);
				ax = temp;

				dx = ax;

				ax = (ax & 0x0F) + (bx & 0xF0);
				hsize++;

				if (hsize & 1) {
					ax = 0; // not sure about this
				}

				j = hsize >> 1;

				while (1) {
					*(out2++) = ax & 0x0F;
					ax += dx;

					--j;
					if (!j)
						break;

					*(out2++) = ax & 0x0F;
					ax += dx;
				}
			}

			out += 640;
			--renderLoop;

		} while (renderLoop);
		break;
	}
	case POLYGONTYPE_TRAS: { // FIXME: buggy
		do {
			unsigned short int bx;

			start = ptr1[0];
			stop = ptr1[480];

			ptr1++;
			hsize = stop - start;

			if (hsize >= 0) {
				hsize++;
				out2 = start + out;

				if ((hsize >> 1) < 0) {
					bx = color & 0x0FF;
					bx = bx << 8;
					bx += color & 0x0FF;
					for (j = 0; j < hsize; j++) {
						*(out2) = (*(out2)&0x0F0F) | bx;
					}
				} else {
					*(out2) = (*(out2)&0x0F) | color;
					out2++;
				}
			}
			out += 640;
		} while (--vsize);
		break;
	}
	case POLYGONTYPE_TRAME: { // FIXME: buggy
		unsigned char bh = 0;

		currentLine = vtop;
		do {
			if (currentLine >= 0 && currentLine < 480) {
				start = ptr1[0];
				stop = ptr1[480];
				ptr1++;
				hsize = stop - start;

				if (hsize >= 0) {
					hsize++;
					out2 = start + out;

					hsize /= 2;
					if (hsize > 1) {
						uint16 ax;
						bh ^= 1;
						ax = (uint16)(*out2);
						ax &= 1;
						if (ax ^ bh) {
							out2++;
						}

						for (j = 0; j < hsize; j++) {
							*(out2) = (uint8)color;
							out2 += 2;
						}
					}
				}
			}
			out += 640;
			currentLine++;
		} while (--vsize);
		break;
	}
	case POLYGONTYPE_GOURAUD: {
		renderLoop = vsize;
		currentLine = vtop;
		do {
			if (currentLine >= 0 && currentLine < 480) {
				uint16 startColor = ptr2[0];
				uint16 stopColor = ptr2[480];

				int16 colorSize = stopColor - startColor;

				stop = ptr1[480]; // stop
				start = ptr1[0];  // start

				ptr1++;
				out2 = start + out;
				hsize = stop - start;

				//varf2 = ptr2[480];
				//varf3 = ptr2[0];

				ptr2++;

				//varf4 = (int64)((int32)varf2 - (int32)varf3);

				if (hsize == 0) {
					if (start >= 0 && start < 640)
						*out2 = ((startColor + stopColor) / 2) >> 8; // moyenne des 2 couleurs
				} else if (hsize > 0) {
					if (hsize == 1) {
						if (start >= -1 && start < 640 - 1)
							*(out2 + 1) = stopColor >> 8;

						if (start >= 0 && start < 640)
							*(out2) = startColor >> 8;
					} else if (hsize == 2) {
						if (start >= -2 && start < 640 - 2)
							*(out2 + 2) = stopColor >> 8;

						if (start >= -1 && start < 640 - 1)
							*(out2 + 1) = ((startColor + stopColor) / 2) >> 8;

						if (start >= 0 && start < 640)
							*(out2) = startColor >> 8;
					} else {
						int32 currentXPos = start;
						colorSize /= hsize;
						hsize++;

						if (hsize % 2) {
							hsize /= 2;
							if (currentXPos >= 0 && currentXPos < 640)
								*(out2) = startColor >> 8;
							out2++;
							currentXPos++;
							startColor += colorSize;
						} else {
							hsize /= 2;
						}

						do {
							if (currentXPos >= 0 && currentXPos < 640)
								*(out2) = startColor >> 8;

							currentXPos++;
							startColor += colorSize;

							if (currentXPos >= 0 && currentXPos < 640)
								*(out2 + 1) = startColor >> 8;

							currentXPos++;
							out2 += 2;
							startColor += colorSize;
						} while (--hsize);
					}
				}
			}
			out += 640;
			currentLine++;
		} while (--renderLoop);
		break;
	}
	case POLYGONTYPE_DITHER: { // dithering
		renderLoop = vsize;

		currentLine = vtop;
		do {
			if (currentLine >= 0 && currentLine < 480) {
				stop = ptr1[480]; // stop
				start = ptr1[0];  // start
				ptr1++;
				hsize = stop - start;

				if (hsize >= 0) {
					uint16 startColor = ptr2[0];
					uint16 stopColor = ptr2[480];
					int32 currentXPos = start;

					out2 = start + out;
					ptr2++;

					if (hsize == 0) {
						if (currentXPos >= 0 && currentXPos < 640)
							*(out2) = (uint8)(((startColor + stopColor) / 2) >> 8);
					} else {
						int16 colorSize = stopColor - startColor;
						if (hsize == 1) {
							uint16 currentColor = startColor;
							hsize++;
							hsize /= 2;

							currentColor &= 0xFF;
							currentColor += startColor;
							if (currentXPos >= 0 && currentXPos < 640)
								*(out2) = currentColor >> 8;

							currentColor &= 0xFF;
							startColor += colorSize;
							currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
							currentColor += startColor;

							currentXPos++;
							if (currentXPos >= 0 && currentXPos < 640)
								*(out2 + 1) = currentColor >> 8;
						} else if (hsize == 2) {
							uint16 currentColor = startColor;
							hsize++;
							hsize /= 2;

							currentColor &= 0xFF;
							colorSize /= 2;
							currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
							currentColor += startColor;
							if (currentXPos >= 0 && currentXPos < 640)
								*(out2) = currentColor >> 8;

							out2++;
							currentXPos++;
							startColor += colorSize;

							currentColor &= 0xFF;
							currentColor += startColor;

							if (currentXPos >= 0 && currentXPos < 640)
								*(out2) = currentColor >> 8;

							currentColor &= 0xFF;
							startColor += colorSize;
							currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
							currentColor += startColor;

							currentXPos++;
							if (currentXPos >= 0 && currentXPos < 640)
								*(out2 + 1) = currentColor >> 8;
						} else {
							uint16 currentColor = startColor;
							colorSize /= hsize;
							hsize++;

							if (hsize % 2) {
								hsize /= 2;
								currentColor &= 0xFF;
								currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
								currentColor += startColor;
								if (currentXPos >= 0 && currentXPos < 640)
									*(out2) = currentColor >> 8;
								out2++;
								currentXPos++;
							} else {
								hsize /= 2;
							}

							do {
								currentColor &= 0xFF;
								currentColor += startColor;
								if (currentXPos >= 0 && currentXPos < 640)
									*(out2) = currentColor >> 8;
								currentXPos++;
								currentColor &= 0xFF;
								startColor += colorSize;
								currentColor = ((currentColor & (0xFF00)) | ((((currentColor & 0xFF) << (hsize & 0xFF))) & 0xFF));
								currentColor += startColor;
								if (currentXPos >= 0 && currentXPos < 640)
									*(out2 + 1) = currentColor >> 8;
								currentXPos++;
								out2 += 2;
								startColor += colorSize;
							} while (--hsize);
						}
					}
				}
			}
			out += 640;
			currentLine++;
		} while (--renderLoop);
		break;
	}
	default: {
		warning("RENDER WARNING: Unsuported render type %d", renderType);
		break;
	}
	};
}

void Renderer::circleFill(int32 x, int32 y, int32 radius, int8 color) {
	int32 currentLine;

	radius += 1;

	for (currentLine = -radius; currentLine <= radius; currentLine++) {
		double width;

		if (ABS(currentLine) != radius) {
			width = sin(acos((int64)currentLine / (int64)radius));
		} else {
			width = 0;
		}

		width *= radius;

		if (width < 0)
			width = -width;

		_engine->_interface->drawLine((int32)(x - width), currentLine + y, (int32)(x + width), currentLine + y, color);
	}
}

int32 Renderer::renderModelElements(uint8 *pointer) {
	uint8 *edi;
	int16 temp;
	int32 eax;
	//	int32 ecx;

	int16 counter;
	int16 type;
	int16 color;

	lineData *lineDataPtr;
	lineCoordinates *lineCoordinatesPtr;

	int32 point1;

	int32 point2;

	int32 depth;
	int32 bestDepth;
	int32 currentDepth;
	int16 bestZ;
	int32 j;
	int32 bestPoly = 0;
	int16 shadeEntry;
	int16 shadeValue;

	int16 ax, bx, cx;

	uint8 *destPtr;
	int32 i;

	uint8 *render23;
	uint8 *render24;
	int32 render25;

	polyVertexHeader *currentPolyVertex;
	polyHeader *currentPolyHeader;
	polyHeader *destinationHeader;
	computedVertex *currentComputedVertex;
	pointTab *currentVertex;
	pointTab *destinationVertex;

	// prepare polygons

	edi = renderTab7;           // renderTab7 coordinates buffer
	temp = *((int16 *)pointer); // we read the number of polygons
	pointer += 2;

	if (temp) {
		primitiveCounter = temp; // the number of primitives = the number of polygons

		do { // loop that load all the polygons
			render23 = edi;
			currentPolyHeader = (polyHeader *)pointer;
			//ecx = *((int32*) pointer);
			pointer += 2;
			polyRenderType = currentPolyHeader->renderType;

			// TODO: RECHECK coordinates axis
			if (polyRenderType >= 9) {
				destinationHeader = (polyHeader *)edi;

				destinationHeader->renderType = currentPolyHeader->renderType - 2;
				destinationHeader->numOfVertex = currentPolyHeader->numOfVertex;
				destinationHeader->colorIndex = currentPolyHeader->colorIndex;

				pointer += 2;
				edi += 4;

				counter = destinationHeader->numOfVertex;

				bestDepth = -32000;
				renderV19 = edi;

				do {
					currentPolyVertex = (polyVertexHeader *)pointer;

					shadeValue = currentPolyHeader->colorIndex + shadeTable[currentPolyVertex->shadeEntry];

					currentComputedVertex = (computedVertex *)edi;

					currentComputedVertex->shadeValue = shadeValue;

					currentVertex = &flattenPoints[currentPolyVertex->dataOffset / 6];
					destinationVertex = (pointTab *)(edi + 2);

					destinationVertex->X = currentVertex->X;
					destinationVertex->Y = currentVertex->Y;

					edi += 6;
					pointer += 4;

					currentDepth = currentVertex->Z;

					if (currentDepth > bestDepth)
						bestDepth = currentDepth;
				} while (--counter);
			} else if (polyRenderType >= 7) { // only 1 shade value is used
				destinationHeader = (polyHeader *)edi;

				destinationHeader->renderType = currentPolyHeader->renderType - 7;
				destinationHeader->numOfVertex = currentPolyHeader->numOfVertex;

				color = currentPolyHeader->colorIndex;

				shadeEntry = *((int16 *)(pointer + 2));

				pointer += 4;

				*((int16 *)(edi + 2)) = color + shadeTable[shadeEntry];

				edi += 4;
				renderV19 = edi;
				bestDepth = -32000;
				counter = destinationHeader->numOfVertex;

				do {
					eax = *((int16 *)pointer);
					pointer += 2;

					currentVertex = &flattenPoints[eax / 6];

					destinationVertex = (pointTab *)(edi + 2);

					destinationVertex->X = currentVertex->X;
					destinationVertex->Y = currentVertex->Y;

					edi += 6;

					currentDepth = currentVertex->Z;

					if (currentDepth > bestDepth)
						bestDepth = currentDepth;
				} while (--counter);
			} else { // no shade is used
				destinationHeader = (polyHeader *)edi;

				destinationHeader->renderType = currentPolyHeader->renderType;
				destinationHeader->numOfVertex = currentPolyHeader->numOfVertex;
				destinationHeader->colorIndex = currentPolyHeader->colorIndex;

				pointer += 2;
				edi += 4;

				bestDepth = -32000;
				renderV19 = edi;
				eax = 0;
				counter = currentPolyHeader->numOfVertex;

				do {
					eax = *((int16 *)pointer);
					pointer += 2;

					currentVertex = &flattenPoints[eax / 6];

					destinationVertex = (pointTab *)(edi + 2);

					destinationVertex->X = currentVertex->X;
					destinationVertex->Y = currentVertex->Y;

					edi += 6;

					currentDepth = currentVertex->Z;

					if (currentDepth > bestDepth)
						bestDepth = currentDepth;
				} while (--(counter));
			}

			render24 = edi;
			edi = renderV19;

			render25 = bestDepth;

			ax = *((int16 *)(edi + 4));
			bx = *((int16 *)(edi + 8));

			ax -= *((int16 *)(edi + 16));
			bx -= *((int16 *)(edi + 2));

			ax *= bx;

			bestDepth = ax;
			bx = currentDepth;

			ax = *((int16 *)(edi + 2));
			cx = *((int16 *)(edi + 10));

			ax -= *((int16 *)(edi + 14));
			cx -= *((int16 *)(edi + 4));

			ax *= cx;

			ax -= bestDepth;
			currentDepth -= (bx)-1; // peut-etre une erreur la

			if (currentDepth < 0) {
				edi = render23;
			} else {
				numOfPrimitives++;

				renderTabEntryPtr->depth = render25;
				renderTabEntryPtr->renderType = 1;
				renderTabEntryPtr->dataPtr = render23;
				renderTabEntryPtr++;

				edi = render24;
			}
		} while (--primitiveCounter);
	}

	// prepare lines

	temp = *((int16 *)pointer);
	pointer += 2;
	if (temp) {
		numOfPrimitives += temp;
		do {
			int32 param;
			lineDataPtr = (lineData *)pointer;
			lineCoordinatesPtr = (lineCoordinates *)edi;

			if (*((int16 *)&lineDataPtr->p1) % 6 != 0 || *((int16 *)&lineDataPtr->p2) % 6 != 0) {
				error("RENDER ERROR: lineDataPtr reference is malformed!");
			}

			point1 = *((int16 *)&lineDataPtr->p1) / 6;
			point2 = *((int16 *)&lineDataPtr->p2) / 6;
			param = *((int32 *)&lineDataPtr->data);
			*((int32 *)&lineCoordinatesPtr->data) = param;
			*((int16 *)&lineCoordinatesPtr->x1) = flattenPoints[point1].X;
			*((int16 *)&lineCoordinatesPtr->y1) = flattenPoints[point1].Y;
			*((int16 *)&lineCoordinatesPtr->x2) = flattenPoints[point2].X;
			*((int16 *)&lineCoordinatesPtr->y2) = flattenPoints[point2].Y;
			bestDepth = flattenPoints[point1].Z;
			depth = flattenPoints[point2].Z;

			if (depth >= bestDepth)
				bestDepth = depth;

			renderTabEntryPtr->depth = bestDepth;
			renderTabEntryPtr->renderType = 0;
			renderTabEntryPtr->dataPtr = edi;
			renderTabEntryPtr++;

			pointer += 8;
			edi += 12;
		} while (--temp);
	}

	// prepare spheres

	temp = *((int16 *)pointer);
	pointer += 2;
	if (temp) {
		numOfPrimitives += temp;
		do {
			uint8 color2 = *(pointer + 1);
			int16 center = *((uint16 *)(pointer + 6));
			int16 size = *((uint16 *)(pointer + 4));

			*(uint8 *)edi = color2;
			*((int16 *)(edi + 1)) = flattenPoints[center / 6].X;
			*((int16 *)(edi + 3)) = flattenPoints[center / 6].Y;
			*((int16 *)(edi + 5)) = size;

			renderTabEntryPtr->depth = flattenPoints[center / 6].Z;
			renderTabEntryPtr->renderType = 2;
			renderTabEntryPtr->dataPtr = edi;
			renderTabEntryPtr++;

			pointer += 8;
			edi += 7;
		} while (--temp);
	}

	renderTabEntryPtr2 = renderTab;

	renderTabSortedPtr = renderTabSorted;
	for (i = 0; i < numOfPrimitives; i++) { // then we sort the polygones | WARNING: very slow | TODO: improve this
		renderTabEntryPtr2 = renderTab;
		bestZ = -0x7FFF;
		for (j = 0; j < numOfPrimitives; j++) {
			if (renderTabEntryPtr2->depth > bestZ) {
				bestZ = renderTabEntryPtr2->depth;
				bestPoly = j;
			}
			renderTabEntryPtr2++;
		}
		renderTabSortedPtr->depth = renderTab[bestPoly].depth;
		renderTabSortedPtr->renderType = renderTab[bestPoly].renderType;
		renderTabSortedPtr->dataPtr = renderTab[bestPoly].dataPtr;
		renderTabSortedPtr++;
		renderTab[bestPoly].depth = -0x7FFF;
	}
	renderTabEntryPtr2 = renderTabSorted;

	// prepare to render elements

	if (numOfPrimitives) {
		primitiveCounter = numOfPrimitives;
		renderV19 = pointer;

		do {
			type = renderTabEntryPtr2->renderType;
			pointer = renderTabEntryPtr2->dataPtr;
			renderV19 += 8;

			switch (type) {
			case RENDERTYPE_DRAWLINE: { // draw a line
				int32 x1;
				int32 y1;
				int32 x2;
				int32 y2;

				lineCoordinatesPtr = (lineCoordinates *)pointer;
				color = (*((int32 *)&lineCoordinatesPtr->data) & 0xFF00) >> 8;

				x1 = *((int16 *)&lineCoordinatesPtr->x1);
				y1 = *((int16 *)&lineCoordinatesPtr->y1);
				x2 = *((int16 *)&lineCoordinatesPtr->x2);
				y2 = *((int16 *)&lineCoordinatesPtr->y2);

				_engine->_interface->drawLine(x1, y1, x2, y2, color);
				break;
			}
			case RENDERTYPE_DRAWPOLYGON: { // draw a polygon
				eax = *((int *)pointer);
				pointer += 4;

				polyRenderType = eax & 0xFF;
				numOfVertex = (eax & 0xFF00) >> 8;
				color = (eax & 0xFF0000) >> 16;

				destPtr = (uint8 *)vertexCoordinates;

				for (i = 0; i < (numOfVertex * 3); i++) {
					*((int16 *)destPtr) = *((int16 *)pointer);
					destPtr += 2;
					pointer += 2;
				}

				if (computePolygons() != ERROR_OUT_OF_SCREEN) {
					renderPolygons(polyRenderType, color);
				}

				break;
			}
			case RENDERTYPE_DRAWSPHERE: { // draw a sphere
				int32 circleParam1;
				//int32 circleParam2;
				int32 circleParam3;
				int32 circleParam4;
				int32 circleParam5;

				eax = *(int *)pointer;

				circleParam1 = *(uint8 *)pointer;
				circleParam4 = *((int16 *)(pointer + 1));
				circleParam5 = *((int16 *)(pointer + 3));
				circleParam3 = *((int16 *)(pointer + 5));

				if (!isUsingOrhoProjection) {
					circleParam3 = (circleParam3 * cameraPosY) / (cameraPosX + *(int16 *)pointer);
				} else {
					circleParam3 = (circleParam3 * 34) >> 9;
				}

				circleParam3 += 3;

				if (circleParam4 + circleParam3 > _engine->_redraw->renderRight)
					_engine->_redraw->renderRight = circleParam4 + circleParam3;

				if (circleParam4 - circleParam3 < _engine->_redraw->renderLeft)
					_engine->_redraw->renderLeft = circleParam4 - circleParam3;

				if (circleParam5 + circleParam3 > _engine->_redraw->renderBottom)
					_engine->_redraw->renderBottom = circleParam5 + circleParam3;

				if (circleParam5 - circleParam3 < _engine->_redraw->renderTop)
					_engine->_redraw->renderTop = circleParam5 - circleParam3;

				circleParam3 -= 3;

				circleFill(circleParam4, circleParam5, circleParam3, circleParam1);
			}
			default: {
				break;
			}
			}

			pointer = renderV19;
			renderTabEntryPtr2++;
		} while (--primitiveCounter);
	} else {
		_engine->_redraw->renderRight = -1;
		_engine->_redraw->renderBottom = -1;
		_engine->_redraw->renderLeft = -1;
		_engine->_redraw->renderTop = -1;
		return (-1);
	}

	return (0);
}

int32 Renderer::renderAnimatedModel(uint8 *bodyPtr) {
	elementEntry *elemEntryPtr;
	pointTab *pointPtr;
	pointTab *pointPtrDest;
	int32 coX;
	int32 coY;
	int32 coZ;
	uint8 *tmpElemPtr;
	//	int32 *tmpLightMatrix;
	uint8 *tmpShadePtr;
	int32 numOfShades;

	numOfPoints = *((uint16 *)bodyPtr);
	bodyPtr += 2;
	pointsPtr = bodyPtr;

	bodyPtr += numOfPoints * 6;

	numOfElements = *((uint16 *)bodyPtr);
	bodyPtr += 2;
	elementsPtr = elementsPtr2 = bodyPtr;

	currentMatrixTableEntry = (uint8 *)matricesTable;

	processRotatedElement(renderAngleX, renderAngleY, renderAngleZ, (elementEntry *)elementsPtr);

	elementsPtr += 38;

	elemEntryPtr = (elementEntry *)elementsPtr;

	if (numOfElements - 1 != 0) {
		numOfPrimitives = numOfElements - 1;
		currentMatrixTableEntry = (uint8 *)&matricesTable[9];

		do {
			int16 boneType = elemEntryPtr->flag;

			if (boneType == 0) {
				processRotatedElement(elemEntryPtr->rotateX, elemEntryPtr->rotateY, elemEntryPtr->rotateZ, elemEntryPtr); // rotation
			} else if (boneType == 1) {
				processTranslatedElement(elemEntryPtr->rotateX, elemEntryPtr->rotateY, elemEntryPtr->rotateZ, elemEntryPtr); // translation
			}

			currentMatrixTableEntry += 36;
			elementsPtr += 38;
			elemEntryPtr = (elementEntry *)elementsPtr;
		} while (--numOfPrimitives);
	}

	numOfPrimitives = numOfPoints;

	pointPtr = (pointTab *)computedPoints;
	pointPtrDest = (pointTab *)flattenPoints;

	if (isUsingOrhoProjection != 0) { // use standard projection
		do {
			coX = pointPtr->X + renderX;
			coY = pointPtr->Y + renderY;
			coZ = -(pointPtr->Z + renderZ);

			pointPtrDest->X = (coX + coZ) * 24 / 512 + orthoProjX;
			pointPtrDest->Y = (((coX - coZ) * 12) - coY * 30) / 512 + orthoProjY;
			pointPtrDest->Z = coZ - coX - coY;

			if (pointPtrDest->X < _engine->_redraw->renderLeft)
				_engine->_redraw->renderLeft = pointPtrDest->X;
			if (pointPtrDest->X > _engine->_redraw->renderRight)
				_engine->_redraw->renderRight = pointPtrDest->X;

			if (pointPtrDest->Y < _engine->_redraw->renderTop)
				_engine->_redraw->renderTop = pointPtrDest->Y;
			if (pointPtrDest->Y > _engine->_redraw->renderBottom)
				_engine->_redraw->renderBottom = pointPtrDest->Y;

			pointPtr++;
			pointPtrDest++;
		} while (--numOfPrimitives);
	} else {
		do {
			coX = pointPtr->X + renderX;
			coY = pointPtr->Y + renderY;
			coZ = -(pointPtr->Z + renderZ);

			coZ += cameraPosX;

			if (coZ <= 0)
				coZ = 0x7FFFFFFF;

			// X projection
			{
				coX = orthoProjX + ((coX * cameraPosY) / coZ);

				if (coX > 0xFFFF)
					coX = 0x7FFF;

				pointPtrDest->X = coX;

				if (pointPtrDest->X < _engine->_redraw->renderLeft)
					_engine->_redraw->renderLeft = pointPtrDest->X;

				if (pointPtrDest->X > _engine->_redraw->renderRight)
					_engine->_redraw->renderRight = pointPtrDest->X;
			}

			// Y projection
			{
				coY = orthoProjY + ((-coY * cameraPosZ) / coZ);

				if (coY > 0xFFFF)
					coY = 0x7FFF;

				pointPtrDest->Y = coY;

				if (pointPtrDest->Y < _engine->_redraw->renderTop)
					_engine->_redraw->renderTop = pointPtrDest->Y;
				if (pointPtrDest->Y > _engine->_redraw->renderBottom)
					_engine->_redraw->renderBottom = pointPtrDest->Y;
			}

			// Z projection
			{
				if (coZ > 0xFFFF)
					coZ = 0x7FFF;

				pointPtrDest->Z = coZ;
			}

			pointPtr++;
			pointPtrDest++;

		} while (--numOfPrimitives);
	}

	shadePtr = (int32 *)elementsPtr;

	numOfShades = *((uint16 *)shadePtr);

	shadePtr = (int32 *)(((uint8 *)shadePtr) + 2);

	if (numOfShades) { // process normal data
		int32 color;
		int32 shade;

		uint8 *currentShadeDestination = (uint8 *)shadeTable;
		int32 *lightMatrix = matricesTable;
		uint8 *pri2Ptr3;

		numOfPrimitives = numOfElements;

		tmpElemPtr = pri2Ptr3 = elementsPtr2 + 18;

		do { // for each element
			numOfShades = *((uint16 *)tmpElemPtr);

			if (numOfShades) {
				int32 numShades = numOfShades;

				shadeMatrix[0] = (*lightMatrix) * lightX;
				shadeMatrix[1] = (*(lightMatrix + 1)) * lightX;
				shadeMatrix[2] = (*(lightMatrix + 2)) * lightX;

				shadeMatrix[3] = (*(lightMatrix + 3)) * lightY;
				shadeMatrix[4] = (*(lightMatrix + 4)) * lightY;
				shadeMatrix[5] = (*(lightMatrix + 5)) * lightY;

				shadeMatrix[6] = (*(lightMatrix + 6)) * lightZ;
				shadeMatrix[7] = (*(lightMatrix + 7)) * lightZ;
				shadeMatrix[8] = (*(lightMatrix + 8)) * lightZ;

				do { // for each normal
					int16 col1;
					int16 col2;
					int16 col3;

					int16 *colPtr;

					colPtr = (int16 *)shadePtr;

					col1 = *((int16 *)colPtr++);
					col2 = *((int16 *)colPtr++);
					col3 = *((int16 *)colPtr++);

					color = shadeMatrix[0] * col1 + shadeMatrix[1] * col2 + shadeMatrix[2] * col3;
					color += shadeMatrix[3] * col1 + shadeMatrix[4] * col2 + shadeMatrix[5] * col3;
					color += shadeMatrix[6] * col1 + shadeMatrix[7] * col2 + shadeMatrix[8] * col3;

					shade = 0;

					if (color > 0) {
						color >>= 14;
						tmpShadePtr = (uint8 *)shadePtr;
						color /= *((uint16 *)(tmpShadePtr + 6));
						shade = (uint16)color;
					}

					*((uint16 *)currentShadeDestination) = shade;
					currentShadeDestination += 2;
					shadePtr += 2;
				} while (--numShades);
			}

			tmpElemPtr = pri2Ptr3 = pri2Ptr3 + 38; // next element

			/*tmpLightMatrix =*/lightMatrix = lightMatrix + 9;
		} while (--numOfPrimitives);
	}

	return renderModelElements((uint8 *)shadePtr);
}

void Renderer::prepareIsoModel(uint8 *bodyPtr) { // loadGfxSub
	bodyHeaderStruct *bodyHeader;
	int16 offsetToData;
	uint8 *bodyDataPtr;
	int16 numOfElement1;
	int16 numOfPoint;
	uint8 *ptrToKeyData;
	int32 i;
	int32 bp = 36;
	int32 bx = 38;
	uint8 *ptr2;

	bodyHeader = (bodyHeaderStruct *)bodyPtr;

	// This function should only be called ONCE, otherwise it corrupts the model data.
	// The following code implements an unused flag to indicate that a model was already processed.
	if ((bodyHeader->bodyFlag & 0x80)) {
		return;
	}
	bodyHeader->bodyFlag |= 0x80;

	if (!(bodyHeader->bodyFlag & 2)) { // no animation applicable
		return;
	}

	offsetToData = bodyHeader->offsetToData;

	bodyDataPtr = bodyPtr + offsetToData + 16;

	numOfElement1 = *((int16 *)bodyDataPtr);
	ptr2 = bodyDataPtr + 2 + numOfElement1 * 6;

	numOfPoint = *((int16 *)ptr2);

	ptrToKeyData = ptr2 + 2;

	for (i = 0; i < numOfPoint; i++) {
		ptrToKeyData += 38;
		*((int16 *)(ptrToKeyData + 6)) = (*((int16 *)(ptrToKeyData + 6)) * bp) / bx;
	}
}

int32 Renderer::renderIsoModel(int32 X, int32 Y, int32 Z, int32 angleX, int32 angleY, int32 angleZ, uint8 *bodyPtr) { // AffObjetIso
	uint8 *ptr;
	int16 bodyHeader;

	renderAngleX = angleX;
	renderAngleY = angleY;
	renderAngleZ = angleZ;

	// model render size reset
	_engine->_redraw->renderLeft = 32767;
	_engine->_redraw->renderTop = 32767;
	_engine->_redraw->renderRight = -32767;
	_engine->_redraw->renderBottom = -32767;

	if (isUsingOrhoProjection == 0) {
		getBaseRotationPosition(X, Y, Z);

		renderX = destX - baseRotPosX;
		renderY = destY - baseRotPosY; // RECHECK
		renderZ = destZ - baseRotPosZ;
	} else {
		renderX = X;
		renderY = Y;
		renderZ = Z;
	}

	// reset the number of primitives in the model
	numOfPrimitives = 0;

	// restart at the beginning of the renderTable
	renderTabEntryPtr = renderTab;

	bodyHeader = *((uint16 *)bodyPtr);

	// jump after the header
	ptr = bodyPtr + 16 + *((uint16 *)(bodyPtr + 14));

	if (bodyHeader & 2) { // if animated
		// the mostly used renderer code
		return renderAnimatedModel(ptr);
	}
	error("Unsupported unanimated model render!");
	return 0;
}

void Renderer::copyActorInternAnim(uint8 *bodyPtrSrc, uint8 *bodyPtrDest) {
	// check if both characters allow animation
	if (!(*((int16 *)bodyPtrSrc) & 2))
		return;

	if (!(*((int16 *)bodyPtrDest) & 2))
		return;

	// skip header
	bodyPtrSrc += 16;
	bodyPtrDest += 16;

	*((uint32 *)bodyPtrDest) = *((uint32 *)bodyPtrSrc);
	*((uint32 *)(bodyPtrDest + 4)) = *((uint32 *)(bodyPtrSrc + 4));

	bodyPtrSrc = bodyPtrSrc + *((int16 *)(bodyPtrSrc - 2));
	bodyPtrSrc = bodyPtrSrc + (*((int16 *)bodyPtrSrc)) * 6 + 2;
	int16 cx = *((int16 *)bodyPtrSrc);

	bodyPtrDest = bodyPtrDest + *((int16 *)(bodyPtrDest - 2));
	bodyPtrDest = bodyPtrDest + (*((int16 *)bodyPtrDest)) * 6 + 2;
	int16 ax = *((int16 *)bodyPtrDest);

	if (cx > ax)
		cx = ax;

	bodyPtrSrc += 10;
	bodyPtrDest += 10;

	for (int32 i = 0; i < cx; i++) {
		*((uint32 *)bodyPtrDest) = *((uint32 *)bodyPtrSrc);
		*((uint32 *)(bodyPtrDest + 4)) = *((uint32 *)(bodyPtrSrc + 4));

		bodyPtrDest += 30;
		bodyPtrSrc += 30;
	}
}

void Renderer::renderBehaviourModel(int32 boxLeft, int32 boxTop, int32 boxRight, int32 boxBottom, int32 Y, int32 angle, uint8 *entityPtr) {
	int tmpBoxRight;
	int x;
	int y;
	short int newAngle;

	tmpBoxRight = boxRight;

	y = boxBottom + boxTop;
	y >>= 1;

	x = boxRight + boxLeft;
	x >>= 1;

	setOrthoProjection(x, y, 0);
	_engine->_interface->setClip(boxLeft, boxTop, tmpBoxRight, boxBottom);

	if (angle == -1) {
		newAngle = _engine->_movements->getRealAngle(&_engine->_menu->moveMenu);
		if (_engine->_menu->moveMenu.numOfStep == 0) {
			_engine->_movements->setActorAngleSafe(newAngle, newAngle - 256, 50, &_engine->_menu->moveMenu);
		}
		renderIsoModel(0, Y, 0, 0, newAngle, 0, entityPtr);
	} else {
		renderIsoModel(0, Y, 0, 0, angle, 0, entityPtr);
	}
}

void Renderer::renderInventoryItem(int32 X, int32 Y, uint8 *itemBodyPtr, int32 angle, int32 param) { // Draw3DObject
	setCameraPosition(X, Y, 128, 200, 200);
	setCameraAngle(0, 0, 0, 60, 0, 0, param);

	renderIsoModel(0, 0, 0, 0, angle, 0, itemBodyPtr);
}

} // namespace TwinE
